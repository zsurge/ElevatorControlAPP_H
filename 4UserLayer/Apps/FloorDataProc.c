/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : FloorDataProc.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月23日
  最近修改   :
  功能描述   : 电梯控制器的指令处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月23日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "FloorDataProc.h"

#define LOG_TAG    "FloorData"
#include "elog.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define AUTO_REG            1
#define MANUAL_REG          2

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static SYSERRORCODE_E packetToElevator(USERDATA_STRU *localUserData);
static void calcFloor(uint8_t layer,uint8_t regMode,uint8_t *src,uint8_t *outFloor);
static SYSERRORCODE_E authReader(READER_BUFF_STRU *pQueue,USERDATA_STRU *localUserData);
static SYSERRORCODE_E packetRemoteRequestToElevator(uint8_t *tagFloor,uint8_t len);

static ELEVATOR_BUFF_STRU  gtmpElevtorData;



void packetDefaultSendBuf(uint8_t *buf)
{
    uint8_t sendBuf[64] = {0};

    sendBuf[0] = CMD_STX;
    sendBuf[1] = 0x01;//bsp_dipswitch_read();
    sendBuf[2] = 0x01;//bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);

    memcpy(buf,sendBuf,MAX_SEND_LEN);
}


void packetSendBuf(READER_BUFF_STRU *pQueue)
{
    uint8_t jsonBuf[512] = {0};
    uint8_t sendBuf[64] = {0};
    uint16_t len = 0;
    uint16_t ret = 0;
    int tagFloor = 0;
    USERDATA_STRU *localUserData = &gUserDataStru;
    memset(localUserData,0x00,sizeof(USERDATA_STRU));
    
    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);
    log_d("card or QR data = %s\r\n",pQueue->data);

    switch(pQueue->authMode)
    {
        case AUTH_MODE_CARD:
        case AUTH_MODE_QR:
            log_d("card or QR auth,pQueue->authMode = %d\r\n",pQueue->authMode);
            ret = authReader(pQueue,localUserData);  
            
            if(ret != NO_ERR)
            {
                log_d("reject access\r\n");
                return ;  //无权限
            }

            //1.发给电梯的数据
            ret = packetToElevator(localUserData);
            if(ret != NO_ERR)
            {
                log_d("invalid floor\r\n");
                return ;  //无权限   
            }
            
            //2.发给服务器
            packetPayload(localUserData,jsonBuf); 

            len = strlen((const char*)jsonBuf);

            len = mqttSendData(jsonBuf,len);
            log_d("send = %d\r\n",len);            
            break;
        case AUTH_MODE_REMOTE:
            //直接发送目标楼层
            log_d("send desc floor = %s,%d\r\n",pQueue->data,pQueue->dataLen);  

            ret = packetRemoteRequestToElevator(pQueue->data,pQueue->dataLen);
            if(ret != NO_ERR)
            {
                log_d("invalid floor\r\n");
                return ;  //无权限   
            }
                    
            break;
        case AUTH_MODE_UNBIND:
            //直接发送停用设备指令
            xQueueReset(xDataProcessQueue); 
            log_d("send AUTH_MODE_UNBIND floor\r\n");
            break;
        case AUTH_MODE_BIND:
            //直接发送启动设置指令
            xQueueReset(xDataProcessQueue); 
            log_d("send AUTH_MODE_BIND floor\r\n");
            break;
        default:
            log_d("invalid authMode\r\n");
            break;    
   }

}

SYSERRORCODE_E authReader(READER_BUFF_STRU *pQueue,USERDATA_STRU *localUserData)
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t key[CARD_NO_LEN+1] = {0};  
    uint8_t isFind = 0;  
    
    memset(key,0x00,sizeof(key)); 
    log_d("card or QR data = %s,mode = %d\r\n",pQueue->data,pQueue->authMode);
    
    if(pQueue->authMode == AUTH_MODE_QR) 
    {
        //二维码
        log_d("pQueue->data = %s\r\n",pQueue->data);

        localUserData->authMode = pQueue->authMode; 
        isFind = parseQrCode(pQueue->data,localUserData);

        log_d("qrCodeInfo->startTime= %s\r\n",localUserData->startTime); 
        log_d("qrCodeInfo->endTime= %s\r\n",localUserData->endTime);  
        log_d("isfind = %d\r\n",isFind);      

        if(isFind != NO_ERR)
        {
            //未找到记录，无权限
            log_d("not find record\r\n");
            return NO_AUTHARITY_ERR;
        }         
       
    }
    else
    {
        //读卡 CARD 230000000089E1E35D,23         
        memcpy(key,pQueue->data,CARD_NO_LEN);
        log_d("key = %s\r\n",key);     
        
        isFind = readUserData(key,CARD_MODE,localUserData);   

        log_d("isFind = %d,rUserData.cardState = %d\r\n",isFind,localUserData->cardState);

        if(localUserData->cardState != CARD_VALID || isFind != 0)
        {
            //未找到记录，无权限
            log_e("not find record\r\n");
            return NO_AUTHARITY_ERR;
        } 
        
        localUserData->platformType = 4;
        localUserData->authMode = pQueue->authMode; 
        memcpy(localUserData->timeStamp,time_to_timestamp(),TIMESTAMP_LEN);
        log_d("localUserData->timeStamp = %s\r\n",localUserData->timeStamp);         
    }

    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
    log_d("localUserData->userId = %s\r\n",localUserData->userId);
    dbh("localUserData->accessLayer",localUserData->accessFloor,sizeof(localUserData->accessFloor));
    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultFloor);    
    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);
    log_d("localUserData->timeStamp = %s\r\n",localUserData->timeStamp);
    log_d("localUserData->platformType = %s\r\n",localUserData->platformType);

    return result;
}




SYSERRORCODE_E authRemote(READER_BUFF_STRU *pQueue,USERDATA_STRU *localUserData)
{
    SYSERRORCODE_E result = NO_ERR;
    char value[128] = {0};
    int val_len = 0;
    char *buf[6] = {0}; //存放分割后的子字符串 
    int num = 0;
    uint8_t key[8+1] = {0};    

    memset(key,0x00,sizeof(key));   
    
    memset(value,0x00,sizeof(value));

    val_len = ef_get_env_blob((const char*)key, value, sizeof(value) , NULL);
   

    log_d("get env = %s,val_len = %d\r\n",value,val_len);

    if(val_len <= 0)
    {
        //未找到记录，无权限
        log_e("not find record\r\n");
        return NO_AUTHARITY_ERR;
    }

    split(value,";",buf,&num); //调用函数进行分割 
    log_d("num = %d\r\n",num);

    if(num != 5)
    {
        log_e("read record error\r\n");
        return READ_RECORD_ERR;       
    }

    localUserData->authMode = pQueue->authMode;    
    
    if(AUTH_MODE_QR == pQueue->authMode)
    {
        strcpy((char*)localUserData->userId,(const char*)key);
        
        strcpy((char*)localUserData->cardNo,buf[0]);        
    }
    else
    {
        memcpy(localUserData->cardNo,key,CARD_NO_LEN);

        log_d("buf[0] = %s\r\n",buf[0]);
        strcpy((char*)localUserData->userId,buf[0]);        
    }   

    //3867;0;0;2019-12-29;2029-12-31
    
    
    strcpy((char*)localUserData->accessFloor,buf[1]);
    localUserData->defaultFloor = atoi(buf[2]);
    strcpy((char*)localUserData->startTime,buf[3]);
    strcpy((char*)localUserData->endTime,buf[4]);    



    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
    log_d("localUserData->userId = %s\r\n",localUserData->userId);
//    dbh("localUserData->accessLayer",localUserData->accessFloor,sizeof(localUserData->accessFloor));
    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultFloor);    
    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);

    return result;

}

static SYSERRORCODE_E packetToElevator(USERDATA_STRU *localUserData)
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t tmpBuf[MAX_SEND_LEN+1] = {0};
    char authLayer[64] = {0}; //权限楼层，最多64层
    int num = 0;    
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};
    
    ELEVATOR_BUFF_STRU *devSendData = &gElevtorData;
	
    uint8_t allBuff[MAX_SEND_LEN] = { 0x5A,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5B };

    uint8_t floor = 0;

    uint8_t i = 0;

    
//    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
//    log_d("localUserData->userId = %s\r\n",localUserData->userId);
//    dbh("localUserData->accessLayer",localUserData->accessFloor,sizeof(localUserData->accessFloor));
//    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultFloor);    
//    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
//    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
//    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);   

    memcpy(authLayer,localUserData->accessFloor,FLOOR_ARRAY_LEN);
    num = strlen((const char*)authLayer);

    log_d("localUserData->accessFloor num = %d\r\n",num);
    
    memset(sendBuf,0x00,sizeof(sendBuf));

	if(num >= 25)
	{
		dbh("send multiple", (char *)allBuff, MAX_SEND_LEN);
		memcpy(devSendData->data,allBuff,MAX_SEND_LEN);
		
		for(i = 0 ;i<10;i++)
		{
			/* 使用消息队列实现指针变量的传递 */
			if(xQueueSend(xTransDataQueue,				/* 消息队列句柄 */
						 (void *) &devSendData,   /* 发送指针变量recv_buf的地址 */
						 (TickType_t)10) != pdPASS )
			{
				log_d("the queue is full!\r\n");				
				xQueueReset(xTransDataQueue);
			} 
		   
		}	
		
		return result;
	}
    
    if(num > 1)//多层权限，手动
    {
        for(i=0;i<num;i++)
        {
            log_d("current floor = %d\r\n",authLayer[i]);
            calcFloor(authLayer[i],MANUAL_REG,sendBuf,tmpBuf);  
            memcpy(sendBuf,tmpBuf,MAX_SEND_LEN);
        }        
    }
    else    //单层权限，直接呼默认权限楼层，自动
    {
        if(localUserData->defaultFloor != authLayer[0])
        {
        
            log_d("defaultFloor != authLayer,%d,%d\r\n",localUserData->defaultFloor,authLayer[0]);
            localUserData->defaultFloor = authLayer[0];
        }
        
        floor = localUserData->defaultFloor;//authLayer[0];   
        
	    if(floor == 0)
	    {
	        return INVALID_FLOOR;//无效的楼层
	    }
		
        calcFloor(floor,AUTO_REG,sendBuf,tmpBuf);            
    }   

    memset(sendBuf,0x00,sizeof(sendBuf));
    
    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    memcpy(sendBuf+2,tmpBuf,MAX_SEND_LEN-5);
    
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);  
    
    memcpy(devSendData->data,sendBuf,MAX_SEND_LEN);
    
//    dbh("send single1", (char *)devSendData->data, MAX_SEND_LEN);

    for(i = 0 ;i<7;i++)
    {
        /* 使用消息队列实现指针变量的传递 */
        if(xQueueSend(xTransDataQueue,              /* 消息队列句柄 */
        			 (void *) &devSendData,   /* 发送指针变量recv_buf的地址 */
        			 (TickType_t)10) != pdPASS )
        {
            log_d("the queue is full!\r\n");                
            xQueueReset(xTransDataQueue);
        } 
       
    }
    return result;
}


static SYSERRORCODE_E packetRemoteRequestToElevator(uint8_t *tagFloor,uint8_t len)
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t tmpBuf[MAX_SEND_LEN+1] = {0};
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};
    
    ELEVATOR_BUFF_STRU *devSendData = &gElevtorData;
    
    uint8_t allBuff[MAX_SEND_LEN] = { 0x5A,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5B };

    uint8_t floor = 0;

    uint8_t i = 0; 

    if(len >= 25)
    {
        dbh("send multiple", (char *)allBuff, MAX_SEND_LEN);
        memcpy(devSendData->data,allBuff,MAX_SEND_LEN);
        
        for(i = 0 ;i<10;i++)
        {
            /* 使用消息队列实现指针变量的传递 */
            if(xQueueSend(xTransDataQueue,              /* 消息队列句柄 */
                         (void *) &devSendData,   /* 发送指针变量recv_buf的地址 */
                         (TickType_t)10) != pdPASS )
            {
                log_d("the queue is full!\r\n");                
                xQueueReset(xTransDataQueue);
            } 
           
        }   
        
        return result;
    }
    
    if(len > 1)//多层权限，手动
    {
        for(i=0;i<len;i++)
        {
            log_d("current floor = %d\r\n",tagFloor[i]);
            calcFloor(tagFloor[i],MANUAL_REG,sendBuf,tmpBuf);  
            memcpy(sendBuf,tmpBuf,MAX_SEND_LEN);
        }        
    }
    else    //单层权限，直接呼默认权限楼层，自动
    {        
        floor = tagFloor[0];   
        
        if(floor == 0)
        {
            return INVALID_FLOOR;//无效的楼层
        }
        
        calcFloor(floor,AUTO_REG,sendBuf,tmpBuf);            
    }   

    memset(sendBuf,0x00,sizeof(sendBuf));
    
    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    memcpy(sendBuf+2,tmpBuf,MAX_SEND_LEN-5);
    
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);  
    
    memcpy(devSendData->data,sendBuf,MAX_SEND_LEN);
    

    for(i = 0 ;i<8;i++)
    {
        /* 使用消息队列实现指针变量的传递 */
        if(xQueueSend(xTransDataQueue,              /* 消息队列句柄 */
                     (void *) &devSendData,   /* 发送指针变量recv_buf的地址 */
                     (TickType_t)10) != pdPASS )
        {
            log_d("the queue is full!\r\n");                
            xQueueReset(xTransDataQueue);
        } 
       
    }
    return result;
}




static void calcFloor(uint8_t layer,uint8_t regMode,uint8_t *src,uint8_t *outFloor)
{
    uint8_t div = 0;
    uint8_t remainder = 0;
    uint8_t floor = layer + 3; //这里是因为有地下三层
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};
    uint8_t tmpFloor = 0;
    uint8_t index = 0;
    
    memcpy(sendBuf,src,MAX_SEND_LEN);

//    dbh("before", sendBuf, MAX_SEND_LEN);
        
    div = floor / 8;
    remainder = floor % 8;

    if(regMode == AUTO_REG)
    {
        index = div + 8;
    }
    else
    {
        index = div;
    }

    log_d("div = %d,remain = %d\r\n",div,remainder);
    

    if(div != 0 && remainder == 0)// 8,16,24
    {       
        sendBuf[index-1] = setbit(sendBuf[index-1],8-1);
    } 
    else //1~7层和非8层的倍数
    {
        sendBuf[index] = setbit(sendBuf[index],remainder-1);
    }

    memcpy(outFloor,sendBuf,MAX_SEND_LEN);

//    dbh("after", sendBuf, MAX_SEND_LEN);
}





