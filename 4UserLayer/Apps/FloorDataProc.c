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
#define LOG_TAG    "FloorData"
#include "elog.h"
#include "FloorDataProc.h"




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


void packetDefaultSendBuf(uint8_t *buf)
{
    uint8_t sendBuf[64] = {0};

    sendBuf[0] = CMD_STX;
    sendBuf[1] = 0x01;//bsp_dipswitch_read();
    sendBuf[2] = 0x01;//bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);

    memcpy(buf,sendBuf,MAX_SEND_LEN);
}


SYSERRORCODE_E packetToElevator(USERDATA_STRU *localUserData)
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t tmpBuf[MAX_SEND_LEN+1] = {0};
    char authLayer[64] = {0}; //权限楼层，最多64层
    int num = 0;    
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};
    
    ELEVATOR_BUFF_STRU *devSendData = &gElevtorData;
	
    uint8_t floor = 0;
    uint8_t i = 0;



    memcpy(authLayer,localUserData->accessFloor,FLOOR_ARRAY_LEN);
    num = strlen((const char*)authLayer);
    
    log_d("localUserData->accessFloor num = %d\r\n",num);
    
    memset(sendBuf,0x00,sizeof(sendBuf));

/* BEGIN: Deleted by  , 2021/2/3   问题单号:012 */
//	if(num >= 25)
//	{
//		dbh("send multiple", (char *)allBuff, MAX_SEND_LEN);
//		memcpy(devSendData->data,allBuff,MAX_SEND_LEN);
//		
//		for(i = 0 ;i<10;i++)
//		{
//			/* 使用消息队列实现指针变量的传递 */
//			if(xQueueSend(xTransDataQueue,				/* 消息队列句柄 */
//						 (void *) &devSendData,   /* 发送指针变量recv_buf的地址 */
//						 (TickType_t)10) != pdPASS )
//			{
//				log_d("the queue is full!\r\n");				
//				xQueueReset(xTransDataQueue);
//			} 
//		   
//		}	
//		
//		return result;
//	}
/* END: Deleted by  , 2021/2/3 */
    
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
    sendBuf[1] = 0x01;//bsp_dipswitch_read();
    memcpy(sendBuf+2,tmpBuf,MAX_SEND_LEN-5);
    
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);  
    
    memcpy(devSendData->data,sendBuf,MAX_SEND_LEN);
    
    dbh("send single", (char *)devSendData->data, MAX_SEND_LEN);

    for(i = 0 ;i<7;i++)
    {
        /* 使用消息队列实现指针变量的传递 */
        if(xQueueSend(xTransDataQueue,              /* 消息队列句柄 */
        			 (void *) &devSendData,   /* 发送指针变量recv_buf的地址 */
        			 (TickType_t)30) != pdPASS )
        {
            log_d("the queue is full!\r\n");                
            xQueueReset(xTransDataQueue);
        } 
       
    }
    return result;
}


SYSERRORCODE_E packetRemoteRequestToElevator(uint8_t *tagFloor,uint8_t len)
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t tmpBuf[MAX_SEND_LEN+1] = {0};
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};
    
    ELEVATOR_BUFF_STRU *devSendData = &gElevtorData;    

    uint8_t floor = 0;

    uint8_t i = 0; 

/* BEGIN: Deleted by  , 2021/2/3   问题单号:012 */
//    if(len >= 25)
//    {
//        dbh("send multiple", (char *)allBuff, MAX_SEND_LEN);
//        memcpy(devSendData->data,allBuff,MAX_SEND_LEN);
//        
//        for(i = 0 ;i<10;i++)
//        {
//            /* 使用消息队列实现指针变量的传递 */
//            if(xQueueSend(xTransDataQueue,              /* 消息队列句柄 */
//                         (void *) &devSendData,   /* 发送指针变量recv_buf的地址 */
//                         (TickType_t)10) != pdPASS )
//            {
//                log_d("the queue is full!\r\n");                
//                xQueueReset(xTransDataQueue);
//            } 
//           
//        }   
//        
//        return result;
//    }
/* END: Deleted by  , 2021/2/3 */

    
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

//        log_e("1 floor = %d\r\n",floor);
        
        if(floor == 0)
        {
            return INVALID_FLOOR;//无效的楼层
        }
        
        calcFloor(floor,AUTO_REG,sendBuf,tmpBuf);            
    }   

    memset(sendBuf,0x00,sizeof(sendBuf));
    
    sendBuf[0] = CMD_STX;
    sendBuf[1] = 0x01;//bsp_dipswitch_read();
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




void calcFloor(uint8_t layer,uint8_t regMode,uint8_t *src,uint8_t *outFloor)
{
    uint8_t div = 0;
    uint8_t remainder = 0;
    uint8_t floor = 0; //这里是因为有地下三层
    uint8_t sendBuf[MAX_SEND_LEN+1] = {0};
    uint8_t index = 0;
    
    /* BEGIN: Added by  , 2021/2/3   问题单号:012 */
    //负楼层补偿
    uint8_t offset = ((bsp_dipswitch_read()>>2) & 0x03);
    /* END:   Added by  , 2021/2/3 */
    
    memcpy(sendBuf,src,MAX_SEND_LEN);
    
/* BEGIN: Added by  , 2021/2/3   问题单号:012 */
    if(layer > 200)
    {
        if(256-layer == offset)
        {
            floor = offset - (256-layer)+1;
        }
        else if(256-layer < offset)
        {
            floor = offset-(256-layer)+1;
        }
        else
        {
            floor = offset+1;
        }        
    }
    else
    {
        floor = layer + ((bsp_dipswitch_read()>>2) & 0x03); //根据拨码补偿楼层数
    }
/* END:   Added by  , 2021/2/3 */

/* BEGIN: Deleted by  , 2021/2/3   问题单号:012 */
//    if(layer == 253)
//    {
//        floor = 1;
//    }
//    else if(layer == 254)
//    {
//        floor = 2;
//    }
//    else if(layer == 255)
//    {
//        floor = 3;
//    }
//    else
//    {
//        floor = layer + 3;
//    }

//    log_e("2 floor = %d\r\n",floor);
/* END: Deleted by  , 2021/2/3 */
        
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





