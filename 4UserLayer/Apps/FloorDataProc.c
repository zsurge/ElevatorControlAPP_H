/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : FloorDataProc.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��12��23��
  ����޸�   :
  ��������   : ���ݿ�������ָ����ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��12��23��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "FloorDataProc.h"

#define LOG_TAG    "FloorData"
#include "elog.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define AUTO_REG            1
#define MANUAL_REG          2

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
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
                return ;  //��Ȩ��
            }

            //1.�������ݵ�����
            ret = packetToElevator(localUserData);
            if(ret != NO_ERR)
            {
                log_d("invalid floor\r\n");
                return ;  //��Ȩ��   
            }
            
            //2.����������
            packetPayload(localUserData,jsonBuf); 

            len = strlen((const char*)jsonBuf);

            len = mqttSendData(jsonBuf,len);
            log_d("send = %d\r\n",len);            
            break;
        case AUTH_MODE_REMOTE:
            //ֱ�ӷ���Ŀ��¥��
            log_d("send desc floor = %s,%d\r\n",pQueue->data,pQueue->dataLen);  

            ret = packetRemoteRequestToElevator(pQueue->data,pQueue->dataLen);
            if(ret != NO_ERR)
            {
                log_d("invalid floor\r\n");
                return ;  //��Ȩ��   
            }
                    
            break;
        case AUTH_MODE_UNBIND:
            //ֱ�ӷ���ͣ���豸ָ��
            xQueueReset(xDataProcessQueue); 
            log_d("send AUTH_MODE_UNBIND floor\r\n");
            break;
        case AUTH_MODE_BIND:
            //ֱ�ӷ�����������ָ��
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
        //��ά��
        log_d("pQueue->data = %s\r\n",pQueue->data);

        localUserData->authMode = pQueue->authMode; 
        isFind = parseQrCode(pQueue->data,localUserData);

        log_d("qrCodeInfo->startTime= %s\r\n",localUserData->startTime); 
        log_d("qrCodeInfo->endTime= %s\r\n",localUserData->endTime);  
        log_d("isfind = %d\r\n",isFind);      

        if(isFind != NO_ERR)
        {
            //δ�ҵ���¼����Ȩ��
            log_d("not find record\r\n");
            return NO_AUTHARITY_ERR;
        }         
       
    }
    else
    {
        //���� CARD 230000000089E1E35D,23         
        memcpy(key,pQueue->data,CARD_NO_LEN);
        log_d("key = %s\r\n",key);     
        
        isFind = readUserData(key,CARD_MODE,localUserData);   

        log_d("isFind = %d,rUserData.cardState = %d\r\n",isFind,localUserData->cardState);

        if(localUserData->cardState != CARD_VALID || isFind != 0)
        {
            //δ�ҵ���¼����Ȩ��
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
    char *buf[6] = {0}; //��ŷָ������ַ��� 
    int num = 0;
    uint8_t key[8+1] = {0};    

    memset(key,0x00,sizeof(key));   
    
    memset(value,0x00,sizeof(value));

    val_len = ef_get_env_blob((const char*)key, value, sizeof(value) , NULL);
   

    log_d("get env = %s,val_len = %d\r\n",value,val_len);

    if(val_len <= 0)
    {
        //δ�ҵ���¼����Ȩ��
        log_e("not find record\r\n");
        return NO_AUTHARITY_ERR;
    }

    split(value,";",buf,&num); //���ú������зָ� 
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
    char authLayer[64] = {0}; //Ȩ��¥�㣬���64��
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
			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
			if(xQueueSend(xTransDataQueue,				/* ��Ϣ���о�� */
						 (void *) &devSendData,   /* ����ָ�����recv_buf�ĵ�ַ */
						 (TickType_t)10) != pdPASS )
			{
				log_d("the queue is full!\r\n");				
				xQueueReset(xTransDataQueue);
			} 
		   
		}	
		
		return result;
	}
    
    if(num > 1)//���Ȩ�ޣ��ֶ�
    {
        for(i=0;i<num;i++)
        {
            log_d("current floor = %d\r\n",authLayer[i]);
            calcFloor(authLayer[i],MANUAL_REG,sendBuf,tmpBuf);  
            memcpy(sendBuf,tmpBuf,MAX_SEND_LEN);
        }        
    }
    else    //����Ȩ�ޣ�ֱ�Ӻ�Ĭ��Ȩ��¥�㣬�Զ�
    {
        if(localUserData->defaultFloor != authLayer[0])
        {
        
            log_d("defaultFloor != authLayer,%d,%d\r\n",localUserData->defaultFloor,authLayer[0]);
            localUserData->defaultFloor = authLayer[0];
        }
        
        floor = localUserData->defaultFloor;//authLayer[0];   
        
	    if(floor == 0)
	    {
	        return INVALID_FLOOR;//��Ч��¥��
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
        /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
        if(xQueueSend(xTransDataQueue,              /* ��Ϣ���о�� */
        			 (void *) &devSendData,   /* ����ָ�����recv_buf�ĵ�ַ */
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
            /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
            if(xQueueSend(xTransDataQueue,              /* ��Ϣ���о�� */
                         (void *) &devSendData,   /* ����ָ�����recv_buf�ĵ�ַ */
                         (TickType_t)10) != pdPASS )
            {
                log_d("the queue is full!\r\n");                
                xQueueReset(xTransDataQueue);
            } 
           
        }   
        
        return result;
    }
    
    if(len > 1)//���Ȩ�ޣ��ֶ�
    {
        for(i=0;i<len;i++)
        {
            log_d("current floor = %d\r\n",tagFloor[i]);
            calcFloor(tagFloor[i],MANUAL_REG,sendBuf,tmpBuf);  
            memcpy(sendBuf,tmpBuf,MAX_SEND_LEN);
        }        
    }
    else    //����Ȩ�ޣ�ֱ�Ӻ�Ĭ��Ȩ��¥�㣬�Զ�
    {        
        floor = tagFloor[0];   
        
        if(floor == 0)
        {
            return INVALID_FLOOR;//��Ч��¥��
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
        /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
        if(xQueueSend(xTransDataQueue,              /* ��Ϣ���о�� */
                     (void *) &devSendData,   /* ����ָ�����recv_buf�ĵ�ַ */
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
    uint8_t floor = layer + 3; //��������Ϊ�е�������
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
    else //1~7��ͷ�8��ı���
    {
        sendBuf[index] = setbit(sendBuf[index],remainder-1);
    }

    memcpy(outFloor,sendBuf,MAX_SEND_LEN);

//    dbh("after", sendBuf, MAX_SEND_LEN);
}





