/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : comm.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��18��
  ����޸�   :
  ��������   : ��������ָ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "CmdHandle"
#include "elog.h"	

#include "cmdhandle.h"
#include "tool.h"
#include "bsp_led.h"
#include "malloc.h"
#include "ini.h"
#include "bsp_uart_fifo.h"
#include "version.h"
#include "easyflash.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "jsonUtils.h"
#include "version.h"
#include "eth_cfg.h"
#include "bsp_ds1302.h"
#include "LocalData.h"
#include "deviceInfo.h"


					



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define DIM(x)  (sizeof(x)/sizeof(x[0])) //�������鳤��


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
    

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
int gConnectStatus = 0;
int	gMySock = 0;
uint8_t gUpdateDevSn = 0; 


ELEVATOR_BUFF_STRU gElevtorData;

READER_BUFF_STRU gReaderMsg;

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode);
static SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf ); //����
static SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf ); //Զ�̱���
static SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf ); //��ӿ���
static SYSERRORCODE_E DelCardNo ( uint8_t* msgBuf ); //ɾ������
static SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf ); //���豸��������
static SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf ); //����Ӧ��
static SYSERRORCODE_E EnableDev ( uint8_t* msgBuf ); //�����豸
static SYSERRORCODE_E DisableDev ( uint8_t* msgBuf ); //�ر��豸
static SYSERRORCODE_E SetJudgeMode ( uint8_t* msgBuf ); //����ʶ��ģʽ
static SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf ); //��ȡ�豸��Ϣ
static SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf ); //��ȡģ�����
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf ); //��ȡģ�����
static SYSERRORCODE_E GetUserInfo ( uint8_t* msgBuf ); //��ȡ�û���Ϣ
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf ); //Զ�̺���
static SYSERRORCODE_E PCOptDev ( uint8_t* msgBuf ); //PC�˺���
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf ); //ɾ���û���Ϣ
static SYSERRORCODE_E AddSingleUser( uint8_t* msgBuf ); //��ӵ����û�
static SYSERRORCODE_E UnbindDev( uint8_t* msgBuf ); //�����
static SYSERRORCODE_E SetLocalTime( uint8_t* msgBuf ); //���ñ���ʱ��
static SYSERRORCODE_E SetLocalSn( uint8_t* msgBuf ); //���ñ���SN��MQTT��
static SYSERRORCODE_E DelCard( uint8_t* msgBuf ); //ɾ������
static SYSERRORCODE_E DelUserId( uint8_t* msgBuf ); //ɾ���û�
static SYSERRORCODE_E getRemoteTime ( uint8_t* msgBuf );//��ȡԶ�̷�����ʱ��

//static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ); //����Ĭ����Ϣ


typedef SYSERRORCODE_E ( *cmd_fun ) ( uint8_t *msgBuf ); 

typedef struct
{
	const char* cmd_id;             /* ����id */
	cmd_fun  fun_ptr;               /* ����ָ�� */
} CMD_HANDLE_T;

const CMD_HANDLE_T CmdList[] =
{
	{"201",  OpenDoor},
	{"1006", AbnormalAlarm},
    {"1010", DelUserId},
	{"1012", AddCardNo},
	{"1013", DelCardNo},
	{"1015", AddSingleUser},
	{"1016", UpgradeDev},
	{"1017", UpgradeAck},
	{"1024", SetJudgeMode},
	{"1026", GetDevInfo},  
	{"1027", DelCard},         
	{"3001", SetLocalSn},
    {"3002", GetServerIp},
    {"3003", GetTemplateParam},
    {"3004", GetUserInfo},   
    {"3005", RemoteOptDev},        
    {"3006", ClearUserInof},   
    {"3009", UnbindDev},  
    {"3010", PCOptDev},
    {"3011", EnableDev}, //ͬ��
    {"3012", DisableDev},//ͬ���
    {"3013", SetLocalTime}, 
    {"30131", getRemoteTime},
};


SYSERRORCODE_E exec_proc ( char* cmd_id, uint8_t *msg_buf )
{
	SYSERRORCODE_E result = NO_ERR;
	int i = 0;

    if(cmd_id == NULL)
    {
        log_d("empty cmd \r\n");
        return CMD_EMPTY_ERR; 
    }

	for ( i = 0; i < DIM ( CmdList ); i++ )
	{
		if ( 0 == strcmp ( CmdList[i].cmd_id, cmd_id ) )
		{
			CmdList[i].fun_ptr ( msg_buf );
			return result;
		}
	}
	log_d ( "invalid id %s\n", cmd_id );

    
//    ReturnDefault(msg_buf);
	return result;
}


void Proscess(void* data)
{
    char cmd[8+1] = {0};    
    strcpy(cmd,(const char *)GetJsonItem ( data, ( const uint8_t* ) "commandCode",0 ));  
    log_d("-----commandCode = %s-----\r\n",cmd);    
    exec_proc (cmd ,data);
}

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode)
{
    SYSERRORCODE_E result = NO_ERR;

//    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));
    READER_BUFF_STRU *ptQR = &gReaderMsg;
    
	/* ���� */
    ptQR->authMode = authMode; 
    ptQR->dataLen = 0;
    memset(ptQR->data,0x00,sizeof(ptQR->data)); 

    ptQR->dataLen = len;                
    memcpy(ptQR->data,buf,len);
    
    /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    if(xQueueSend(xDataProcessQueue,              /* ��Ϣ���о�� */
                 (void *) &ptQR,   /* ����ָ�����recv_buf�ĵ�ַ */
                 (TickType_t)300) != pdPASS )
    {
        DBG("the queue is full!\r\n");                
        xQueueReset(xDataProcessQueue);
    } 
    else
    {
        //dbh("SendToQueue",(char *)buf,len);
        log_d("SendToQueue buf = %s,len = %d\r\n",buf,len);
    } 


    return result;
}


int mqttSendData(uint8_t *payload_out,uint16_t payload_out_len)
{   
	MQTTString topicString = MQTTString_initializer;
    
	uint32_t len = 0;
	int32_t rc = 0;
	unsigned char buf[1280];
	int buflen = sizeof(buf);

	unsigned short msgid = 1;
	int req_qos = 0;
	unsigned char retained = 0;  

    if(!payload_out)
    {
        return STR_EMPTY_ERR;
    }


   if(gConnectStatus == 1)
   { 
       topicString.cstring = gDevBaseParam.mqttTopic.publish;       //�����ϱ� ����

       log_d("payloadlen = %d,payload = %s",payload_out_len,payload_out);

       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//������Ϣ
       rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
       if(rc == len) 
        {
           log_d("send PUBLISH Successfully,rc = %d,len = %d\r\n",rc,len);
       }
       else
       {
           log_d("send PUBLISH failed,rc = %d,len = %d\r\n",rc,len);     
       }
      
   }
   else
   {
        log_d("MQTT Lost the connect!!!\r\n");
   }
  

   return len;
}



//�����Ϊ�˷������˵��ԣ���д��Ĭ�Ϸ��صĺ���
//static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ) //����Ĭ����Ϣ
//{
//        SYSERRORCODE_E result = NO_ERR;
//        uint8_t buf[MQTT_TEMP_LEN] = {0};
//        uint16_t len = 0;
//    
//        if(!msgBuf)
//        {
//            return STR_EMPTY_ERR;
//        }
//    
//        result = modifyJsonItem(packetBaseJson(msgBuf),"status","1",1,buf);      
//        result = modifyJsonItem(packetBaseJson(buf),"UnknownCommand","random return",1,buf);   
//    
//        if(result != NO_ERR)
//        {
//            return result;
//        }
//    
//        len = strlen((const char*)buf);
//    
//        log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);
//    
//        mqttSendData(buf,len);
//        
//        return result;

//}


SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }
    
    result = modifyJsonItem(packetBaseJson(msgBuf),"openStatus","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);
    
	return result;
}

SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	//1.������ͨѶ�쳣��
	//2.�豸��ͣ�ã�����豸�����ʲô�ģ�������һ��״̬,�㻹���ҷ�Զ�̵ĺ���,�Ҿ͸�����һ���������쳣״̬���㡣
	//3.�洢���𻵣�
	//4.����������
	return result;
}

//ɾ���û�
static SYSERRORCODE_E DelUserId( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t userId[CARD_USER_LEN] = {0};
    uint8_t tmp[CARD_USER_LEN] = {0};
    uint16_t len = 0;
    uint8_t rRet=1;
    uint8_t wRet=1;
    
    USERDATA_STRU userData = {0};
    memset(&userData,0x00,sizeof(USERDATA_STRU));    

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.���濨�ź��û�ID

//  2.��    ��   : 2020��4��13��
//    ��    ��   :  
//    �޸�����   : ���ﷵ����ԱID ���飬�ǿ�������ɾ����Ա��
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf((char *)userId,"%08s",tmp);
    log_d("userId = %s\r\n",userId);

    log_d("=================================\r\n");
    rRet = readUserData(userId,USER_MODE,&userData);

    log_d("ret = %d\r\n",rRet);    
    log_d("userData.cardState = %d\r\n",userData.cardState);    
    log_d("userData.userState = %d\r\n",userData.userState);
    log_d("userData.cardNo = %s\r\n",userData.cardNo);
    log_d("userData.userId = %s\r\n",userData.userId);
    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);


    if(rRet == 0)
    {
        userData.head = TABLE_HEAD;
        userData.userState = USER_DEL; //���ÿ�״̬Ϊ0��ɾ����
        wRet = modifyUserData(&userData,USER_MODE);
    }

    if(wRet ==0)
    {
        //��Ӧ������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);
    }
    else
    {
        //����û�и�����¼����������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",1,buf);
    }  

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    mqttSendData(buf,len);    

    log_d("=================================\r\n");

    memset(&userData,0x00,sizeof(userData));
    len = readUserData(userId,USER_MODE,&userData);
    log_d("ret = %d\r\n",len);    
    log_d("userData.cardState = %d\r\n",userData.cardState);    
    log_d("userData.userState = %d\r\n",userData.userState);
    log_d("userData.cardNo = %s\r\n",userData.cardNo);
    log_d("userData.userId = %s\r\n",userData.userId);
    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);
    
    return result;


}

SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;

    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t cardNo[CARD_USER_LEN] = {0};
    uint8_t userId[CARD_USER_LEN] = {0};
    uint16_t len = 0;  
    USERDATA_STRU userData = {0};  
    uint8_t ret = 0;
 

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    memset(&userData,0x00,sizeof(USERDATA_STRU));   

    //1.�����û�ID
    memset(userId,0x00,sizeof(userId));
    strcpy((char *)userId,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf((char*)userData.userId,"%08s",userId);
    log_d("userData.userId = %s,len = %d\r\n",userData.userId,strlen((const char*)userData.userId));

    //2.���濨��
    memset(cardNo,0x00,sizeof(cardNo));
    strcpy((char *)cardNo,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
//    sprintf(userData.cardNo,"%08s",cardNo);
    log_d("cardNo = %s,len = %d\r\n",cardNo,strlen((const char*)cardNo));

    log_d("=================================\r\n");
    ret  = readUserData(userData.userId,USER_MODE,&userData);
    
    log_d("ret = %d\r\n",ret);    
    log_d("userData.cardState = %d\r\n",userData.cardState);    
    log_d("userData.userState = %d\r\n",userData.userState);
    log_d("userData.cardNo = %s\r\n",userData.cardNo);
    log_d("userData.userId = %s\r\n",userData.userId);
//    dbh("userData.accessFloor", userData.accessFloor, sizeof(userData.accessFloor));
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);

//      userData.accessFloor[0] = 8;
//      userData.accessFloor[0] = 25;
//      userData.accessFloor[1] = 24;
//      userData.accessFloor[2] = 23;
//      userData.accessFloor[3] = 22;
//      userData.accessFloor[4] = 21;
//      userData.accessFloor[5] = 20;
//      userData.accessFloor[6] = 19;
//      userData.accessFloor[7] = 18;
//      userData.accessFloor[8] = 17;
//      userData.accessFloor[9] = 16;
//      userData.accessFloor[10] = 15;
//      userData.accessFloor[11] = 14;
//      userData.accessFloor[12] = 13;
//      userData.accessFloor[13] = 12;
//      userData.accessFloor[14] = 11;
//      userData.accessFloor[15] = 10;
//      userData.accessFloor[16] = 9;
//      userData.accessFloor[17] = 7;
//      userData.accessFloor[18] = 6;
//      userData.accessFloor[19] = 5;
//      userData.accessFloor[20] = 4;
//      userData.accessFloor[21] = 3;
//      userData.accessFloor[22] = 2;
//      userData.accessFloor[23] = 1;
//      userData.accessFloor[24] = 8;
  
 
    
    if(ret == 0)
    {
        sprintf((char*)userData.cardNo,"%08s",cardNo);
        userData.head = TABLE_HEAD;
        userData.cardState = CARD_VALID;
        ret = writeUserData(&userData,CARD_MODE);  
    }

    if(ret == 0)
    {
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","success",1,buf);
    }
    else
    {
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","add error",1,buf);
    }

    
    if(result != NO_ERR)
    {
        return result;
    }
    
    len = strlen((const char*)buf);
    
    mqttSendData(buf,len); 

    ret  = readUserData(userData.cardNo,CARD_MODE,&userData);
    log_d("ret = %d\r\n",ret);    
    log_d("userData.cardState = %d\r\n",userData.cardState);    
    log_d("userData.userState = %d\r\n",userData.userState);
    log_d("userData.cardNo = %s\r\n",userData.cardNo);
    log_d("userData.userId = %s\r\n",userData.userId);
    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
	dbh("userData.accessFloor", userData.accessFloor, sizeof(userData.accessFloor));
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);    
  
	return result;
}

//1013 ɾ����Ա�����п�
SYSERRORCODE_E DelCardNo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t userId[CARD_USER_LEN] = {0};
    uint8_t tmp[CARD_USER_LEN] = {0};
    uint16_t len = 0;
    uint8_t rRet=1;
    uint8_t wRet=1;
    uint8_t num=0;
    int i = 0;  
    uint8_t **cardArray;
    
    USERDATA_STRU userData = {0};
    memset(&userData,0x00,sizeof(USERDATA_STRU));    

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    cardArray = (uint8_t **)my_malloc(20 * sizeof(uint8_t *));
    
    for (i = 0; i < 20; i++)
    {
        cardArray[i] = (uint8_t *)my_malloc(8 * sizeof(uint8_t));
    }  

    if(cardArray == NULL)
    {
        for (i = 0; i < 20; i++)
        {
            my_free(cardArray[i]);
        }      
        
        return STR_EMPTY_ERR;
    }

    //1.���濨�ź��û�ID
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf((char *)userId,"%08s",tmp);
    log_d("userId = %s\r\n",userId);

    cardArray = GetCardArray ((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",&num);
    
    log_d("array num =%d\r\n",num); 
    
    //ɾ��USERID
    //wRet = delUserData(userId,USER_MODE);
    
    //ɾ��CARDNO
    for(i=0; i<num;i++)
    {
        wRet = delUserData(cardArray[i],CARD_MODE);
        log_d("cardArray %d = %s\r\n",i,cardArray[i]);        
    }
    
    
    //2.��ѯ�Կ���ΪID�ļ�¼����ɾ��
    if(wRet ==0)
    {
        //��Ӧ������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);
    }
    else
    {
        //����û�и�����¼����������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",1,buf);
    }  

    if(result != NO_ERR)
    {
        for (i = 0; i < 20; i++)
        {
            my_free(cardArray[i]);
        }  

        return result;
    }

    len = strlen((const char*)buf);

    mqttSendData(buf,len);    

    log_d("=================================\r\n");  
    
    memset(&userData,0x00,sizeof(userData));
    rRet = readUserData(userId,USER_MODE,&userData);
    log_d("ret = %d\r\n",rRet);    
    log_d("userData.cardState = %d\r\n",userData.cardState);    
    log_d("userData.userState = %d\r\n",userData.userState);
    log_d("userData.cardNo = %s\r\n",userData.cardNo);
    log_d("userData.userId = %s\r\n",userData.userId);
    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);

    for (i = 0; i < 20; i++)
    {
        my_free(cardArray[i]);
    }   
    
    return result;
}

SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t tmpUrl[MQTT_TEMP_LEN] = {0};
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //3.��������JSON����
    saveUpgradeData(msgBuf);

    //1.����URL
    strcpy((char *)tmpUrl,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"softwareUrl",1));
    log_d("tmpUrl = %s\r\n",tmpUrl);
    
    ef_set_env("url", (const char*)GetJsonItem((const uint8_t *)tmpUrl,(const uint8_t *)"picUrl",0)); 

    //2.��������״̬Ϊ������״̬
    ef_set_env("up_status", "101700"); 
    
    //4.���ñ�־λ������
    SystemUpdate();
    
	return result;

}



SYSERRORCODE_E getRemoteTime ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;

    result = getTimePacket(buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("getRemoteTime len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);
    
	return result;

}



SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;

    //��ȡ�������ݲ�����JSON��   

    result = upgradeDataPacket(buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("UpgradeAck len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);
    
	return result;

}

SYSERRORCODE_E EnableDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t type[4] = {"1"};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }    

    SaveDevState(DEVICE_ENABLE);


    //add 2020.04.27
    xQueueReset(xDataProcessQueue); 
        
    //������Ҫ����Ϣ����Ϣ���У�����
    SendToQueue(type,strlen((const char*)type),AUTH_MODE_BIND);

    len = strlen((const char*)buf);

    log_d("EnableDev len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);

    return result;


}

SYSERRORCODE_E DisableDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t type[4] = {"0"};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }


    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    SaveDevState(DEVICE_DISABLE);
    
    //������Ҫ����Ϣ����Ϣ���У�����
    SendToQueue(type,strlen((const char*)type),AUTH_MODE_UNBIND);
    
    len = strlen((const char*)buf);

    log_d("DisableDev len = %d,buf = %s,status = %x\r\n",len,buf,gDevBaseParam.deviceState.iFlag);

    mqttSendData(buf,len);

    return result;


}

SYSERRORCODE_E SetJudgeMode ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t *identification;
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = PacketDeviceInfo(msgBuf,buf);


    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("GetDevInfo len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);
    
    my_free(identification);
    
	return result;

}
 
//ɾ������  ����ɾ��
static SYSERRORCODE_E DelCard( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	uint8_t rRet = 1;
	uint8_t wRet = 1;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t cardNo[CARD_USER_LEN] = {0};
    uint8_t userId[CARD_USER_LEN] = {0};
    uint8_t tmp[CARD_USER_LEN] = {0};
    uint16_t len = 0;
    
    USERDATA_STRU userData = {0};
    memset(&userData,0x00,sizeof(USERDATA_STRU));    

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.��ȡ���ź��û�ID
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));   
    sprintf((char *)userId,"%08s",tmp); 
    log_d("userId = %s\r\n",userId);

    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    sprintf((char *)cardNo,"%08s",tmp);   
    
    log_d("cardNo = %s��userId = %s\r\n",cardNo,userId);

    wRet = delUserData(cardNo,CARD_MODE);
    
    //2.��ѯ�Կ���ΪID�ļ�¼����ɾ��
    if(wRet ==0)
    {
        //��Ӧ������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);
    }
    else
    {
        //����û�и�����¼����������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",1,buf);
    }  

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    mqttSendData(buf,len);    

    log_d("=================================\r\n");

    memset(&userData,0x00,sizeof(userData));
    rRet = readUserData(cardNo,CARD_MODE,&userData);
    log_d("ret = %d\r\n",rRet); 
    log_d("userData.cardState = %d\r\n",userData.cardState);    
    log_d("userData.userState = %d\r\n",userData.userState);
    log_d("userData.cardNo = %s\r\n",userData.cardNo);
    log_d("userData.userId = %s\r\n",userData.userId);
    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);
    
	return result;

}

SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }


    //����ģ������ ����Ӧ����һ���߳�ר�����ڶ�дFLASH�������ڼ䣬��ʱ������Ӧ���
    //saveTemplateParam(msgBuf);    
    
    result = modifyJsonItem(packetBaseJson(msgBuf),"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("GetParam len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);

    //����ģ������
    saveTemplateParam(msgBuf);

    //��ȡģ������
//    readTemplateData();
    
	return result;
}

//�������IP
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t ip[32] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.����IP     
    strcpy((char *)ip,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"ip",1));
    log_d("server ip = %s\r\n",ip);

    //Ӱ�������
    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    mqttSendData(buf,len);
    
	return result;

}

//��ȡ�û���Ϣ
static SYSERRORCODE_E GetUserInfo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	uint16_t len =0;
    uint8_t buf[512] = {0};
    USERDATA_STRU tempUserData = {0};
    uint8_t ret = 1;
    uint8_t tmp[128] = {0};
    char *multipleFloor[64] = {0};
    int multipleFloorNum = 0;
    char *cardArray[20] = {0};
    int multipleCardNum=0;
    uint16_t i = 0;
    memset(&tempUserData,0x00,sizeof(USERDATA_STRU));

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    log_d("msgBuf = %s\r\n",msgBuf);

//    cardArray = (uint8_t **)my_malloc(20 * sizeof(uint8_t *));
//    
//    for (i = 0; i < 20; i++)
//    {
//        cardArray[i] = (uint8_t *)my_malloc(8 * sizeof(uint8_t));
//    }  

//    if(cardArray == NULL)
//    {
//        for (i = 0; i < 20; i++)
//        {
//            my_free(cardArray[i]);
//        }  
//        
//        return STR_EMPTY_ERR;
//    }    

    tempUserData.head = TABLE_HEAD;

    //1.������userIDΪkey�ı�
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf((char *)tempUserData.userId,"%08s",tmp);
    log_d("tempUserData.userId = %s,len = %d\r\n",tempUserData.userId,strlen((const char *)tempUserData.userId));  
    
    //3.����¥��Ȩ��
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
    split((char *)tmp,",",multipleFloor,&multipleFloorNum); //���ú������зָ� 

    if(multipleFloorNum >= 1)
    {
        for(len=0;len<multipleFloorNum;len++)
        {
            tempUserData.accessFloor[len] = atoi(multipleFloor[len]);
        }
    }
    else
    {          
        log_d("tempUserData.accessFloor error!!!!!!!!!!!!!!!\r\n");
    }


    //4.����Ĭ��¥��
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"defaultLayer",1));
    tempUserData.defaultFloor = atoi((const char*)tmp);
    log_d("tempUserData.defaultFloor = %d\r\n",tempUserData.defaultFloor);

    //5.���濪ʼʱ��
    strcpy((char *)tempUserData.startTime,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"startTime",1));
    log_d("tempUserData.startTime = %s\r\n",tempUserData.startTime);
    
    //6.�������ʱ��
    strcpy((char *)tempUserData.endTime,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"endTime",1));
    log_d("tempUserData.endTime = %s\r\n",tempUserData.endTime);


    //2.���濨��
    memset(buf,0x00,sizeof(buf));
    strcpy((char *)buf,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    split((char *)buf,",",cardArray,&multipleCardNum); //���ú������зָ� 

    //ȫ0��USER ID����¼
    if(memcmp(tempUserData.userId,"00000000",CARD_USER_LEN) != 0)
    {
        tempUserData.userState = USER_VALID;
        ret = writeUserData(&tempUserData,USER_MODE);
        log_d("write user id = %d\r\n",ret); 
        if(ret != 0)
        {   
            log_e("write user id error\r\n");
            result = FLASH_W_ERR;
        }        
    }   
    

    if(multipleCardNum >= 1)
    {
        for(i=0;i<multipleCardNum;i++)
        {
            tempUserData.cardState = CARD_VALID;     
            memset(tempUserData.cardNo,0x00,sizeof(tempUserData.cardNo));
            memcpy(tempUserData.cardNo,cardArray[i],CARD_USER_LEN);   

            log_d("->%d th,cardid = %s\r\n",i,tempUserData.cardNo);
            
            ret = writeUserData(&tempUserData,CARD_MODE);

            if(ret != 0)
            {    
                log_e("write card id error\r\n");            
                result = FLASH_W_ERR;
            }

        }
    }
    else
    {
        log_d("the empty card Number\r\n");
    }    
    

//    //2.���濨��
//    cardArray = GetCardArray ((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",&multipleCardNum);
//    
//    for(i=0;i<multipleCardNum;i++)
//    {
//        tempUserData.cardState = CARD_VALID;     
//        memset(tempUserData.cardNo,0x00,sizeof(tempUserData.cardNo));
//        memcpy(tempUserData.cardNo,cardArray[i],CARD_USER_LEN);    
//        log_d("%d / %d :tempUserData.cardNo = %s\r\n",multipleCardNum,i,tempUserData.cardNo);            
//        ret = writeUserData(&tempUserData,CARD_MODE);
//    
//        if(ret != 0)
//        {
//            for (i = 0; i < 20; i++)
//            {
//                my_free(cardArray[i]);
//            }   
//            
//            result = FLASH_W_ERR;
//        }    
//    }



    memset(buf,0x00,sizeof(buf));    
    if(result == NO_ERR)
    {
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","success",1,buf);
    }
    else
    {
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","error",1,buf);
    }

    
    if(result != NO_ERR)
    {
//        for (i = 0; i < 20; i++)
//        {
//            my_free(cardArray[i]);
//        }  
        
        return result;
    }

    len = strlen((const char*)buf);
    
    mqttSendData(buf,len);    

//    for (i = 0; i < 20; i++)
//    {
//        my_free(cardArray[i]);
//    }      
    
	return result;

}

//Զ�̺���
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t tagFloor[1] = {0};
    uint8_t accessFloor[64] = {0};
    uint16_t len = 0;
    char *multipleFloor[64] = {0};
    int multipleFloorNum = 0;
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    log_d("3005 = >> %s\r\n",msgBuf);

    log_d("%d,%d\r\n",gtemplateParam.templateCallingWay.isFace,gDevBaseParam.deviceState.iFlag);
    if(gDevBaseParam.deviceState.iFlag == DEVICE_ENABLE)
//    if(gtemplateParam.templateCallingWay.isFace && gDevBaseParam.deviceState.iFlag == DEVICE_ENABLE)
    {
        //1.����Ŀ��¥��
        memset(accessFloor,0x00,sizeof(accessFloor));
        strcpy((char *)accessFloor,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"currentLayer",1));
        tagFloor[0] = atoi((const char*)accessFloor);

        log_d("tagFloor = %d\r\n",tagFloor[0]);

        //3.����¥��Ȩ��
        memset(accessFloor,0x00,sizeof(accessFloor));
        strcpy((char *)accessFloor,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
        split((char *)accessFloor,",",multipleFloor,&multipleFloorNum); //���ú������зָ� 
        
        if(multipleFloorNum > 1)
        {
            for(len=0;len<multipleFloorNum;len++)
            {
                accessFloor[len] = atoi(multipleFloor[len]); 
                
                log_d("accessFloor[%d] = %d\r\n",len,accessFloor[len]);
            }
        }   

         //����Ŀ��¥��
         if(strlen((const char*)tagFloor) == 1) 
         {
             //������Ҫ����Ϣ����Ϣ���У����к���
             SendToQueue(tagFloor,strlen((const char*)tagFloor),AUTH_MODE_REMOTE);
         }

         //���Ͷ�¥��Ȩ��
         if(strlen((const char*)accessFloor) > 1)
         {
            //������Ҫ����Ϣ����Ϣ���У����к���
            SendToQueue(accessFloor,strlen((const char*)accessFloor),AUTH_MODE_REMOTE);
         }         


        if(strlen((const char*)tagFloor) == 0 && strlen((const char*)accessFloor)==0)
        {
            result = modifyJsonItem(msgBuf,"status","0",1,buf);
        }
        else
        {
            result = modifyJsonItem(msgBuf,"status","1",1,buf);
        }        

        if(result != NO_ERR)
        {
            return result;
        }      
        
        len = strlen((const char*)buf);

        log_d("RemoteOptDev len = %d,buf = %s\r\n",len,buf);

        mqttSendData(buf,len); 
    }    
    
    return result;

}

//PC�˺���
static SYSERRORCODE_E PCOptDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t tmp[8] = {0};
    uint8_t purposeLayer[1] = {0};    
    uint16_t len = 0;

    USERDATA_STRU userData = {0};
    memset(&userData,0x00,sizeof(USERDATA_STRU));
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"purposeLayer",1));
    
    purposeLayer[0] = atoi((const char*)tmp);
    
    log_d("tmp = %s,purposeLayer[0] = %d\r\n",tmp, purposeLayer[0]);
    
    memset(buf,0x00,sizeof(buf));
    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }
    

    //������Ҫ����Ϣ����Ϣ���У����к���
    SendToQueue(purposeLayer,1,AUTH_MODE_REMOTE);

    len = strlen((const char*)buf);

    log_d("RemoteOptDev len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len); 
    
//    userData.head = TABLE_HEAD;
//    userData.authMode =2;
//    userData.defaultFloor = 9;
//    userData.cardState = CARD_VALID;
//    memcpy(userData.cardNo,"89E1E35D",CARD_NO_LEN);
//    memcpy(userData.userId,"00000788",USER_ID_LEN);
//    strcpy(userData.accessFloor,"7,8,9");    
//    memcpy(userData.startTime,"2020-01-01",TIME_LENGTH);
//    memcpy(userData.endTime,"2030-01-01",TIME_LENGTH);    
//    
//    writeUserData(&userData,CARD_MODE);
//    
//    log_d("===============TEST==================\r\n");
//    len = readUserData("12345688",CARD_MODE,&userData);

//    log_d("ret = %d\r\n",len);    
//    log_d("userData.userState = %d\r\n",userData.cardState);
//    log_d("userData.cardNo = %s\r\n",userData.cardNo);
//    log_d("userData.userId = %s\r\n",userData.userId);
//    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
//    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
//    log_d("userData.startTime = %s\r\n",userData.startTime);

//    log_d("===============TEST==================\r\n");
//    memset(&userData,0x00,sizeof(USERDATA_STRU));
//    len = readUserData("00020419",USER_MODE,&userData);
//    
//    log_d("ret = %d\r\n",len);    
//    log_d("userData.userState = %d\r\n",userData.cardState);
//    log_d("userData.cardNo = %s\r\n",userData.cardNo);
//    log_d("userData.userId = %s\r\n",userData.userId);
//    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
//    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
//    log_d("userData.startTime = %s\r\n",userData.startTime);



log_d("===============CARD_MODE==================\r\n");
//TestFlash(CARD_MODE);


log_d("===============USER_MODE==================\r\n");
//TestFlash(USER_MODE);


log_d("===============CARD_DEL_MODE==================\r\n");
//TestFlash(CARD_DEL_MODE);


log_d("===============USER_DEL_MODE==================\r\n");
//TestFlash(USER_DEL_MODE);








//    userData.userState = 1;
//    len = modifyUserData(userData,USER_MODE);

//    log_d("==============TEST===================\r\n");
//    userData.userState = 0;
//    memset(&userData,0x00,sizeof(userData));
//    len = readUserData("00010359",USER_MODE,&userData);
//    log_d("ret = %d\r\n",len);    
//    log_d("userData.userState = %d\r\n",userData.userState);
//    log_d("userData.cardNo = %s\r\n",userData.cardNo);
//    log_d("userData.userId = %s\r\n",userData.userId);
//    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
//    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
//    log_d("userData.startTime = %s\r\n",userData.startTime);    

    
    return result;

}



//ɾ���û���Ϣ
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("ClearUserInof len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);


    //����û���Ϣ
    eraseUserDataAll();
    
    return result;

}

//��ӵ����û�
static SYSERRORCODE_E AddSingleUser( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	uint16_t len =0;
    uint8_t buf[1024] = {0};
    USERDATA_STRU tempUserData = {0};
    uint8_t ret = 1;
    uint8_t tmp[128] = {0};
    char *multipleFloor[64] = {0};
    int multipleFloorNum = 0;
    memset(&tempUserData,0x00,sizeof(USERDATA_STRU));

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    printf("msgBuf = %s\r\n",msgBuf);

    //1.�����ʼ��־
    tempUserData.head = TABLE_HEAD; 

    //1.������userIDΪkey�ı�
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf((char *)tempUserData.userId,"%08s",tmp);
    log_d("tempUserData.userId = %s,len = %d\r\n",tempUserData.userId,strlen((const char*)tempUserData.userId));

     //4.����Ĭ��¥��
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"defaultLayer",1));
    tempUserData.defaultFloor = atoi((const char*)tmp);
    log_d("tempUserData.defaultFloor = %d\r\n",tempUserData.defaultFloor);   
    
    //3.����¥��Ȩ��
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
    split((char *)tmp,",",multipleFloor,&multipleFloorNum); //���ú������зָ� 

    if(multipleFloorNum >= 1)
    {
        for(len=0;len<multipleFloorNum;len++)
        {
            tempUserData.accessFloor[len] = atoi(multipleFloor[len]);
        }
    }
    else
    {          
        //modify 2020.06.08 ����¥��Ȩ��ΪĬ��¥��
        tempUserData.accessFloor[0] = tempUserData.defaultFloor;
    }
    
//    dbh("tempUserData.accessFloor", tempUserData.accessFloor,multipleFloorNum );
    



    //5.���濪ʼʱ��
    strcpy((char *)tempUserData.startTime,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"startTime",1));
    log_d("tempUserData.startTime = %s\r\n",tempUserData.startTime);
    
    //6.�������ʱ��
    strcpy((char *)tempUserData.endTime,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"endTime",1));
    log_d("tempUserData.endTime = %s\r\n",tempUserData.endTime);


    //ȫ0��USER ID����¼
    if(memcmp(tempUserData.userId,"00000000",CARD_USER_LEN) != 0)
    {
        tempUserData.userState = USER_VALID;
        ret = writeUserData(&tempUserData,USER_MODE);
        log_d("write user id = %d\r\n",ret);       
    }      


    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"commandCode","3004",0,buf);

    if(ret == 0)
    {
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"status","1",1,buf);
        
    }
    else
    {
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"status","1",1,buf);        
    }
    
    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    mqttSendData(buf,len);    
    
	return result;


}

//�����
static SYSERRORCODE_E UnbindDev( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t type[2] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)type,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"type",1));

    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    //������Ҫ����Ϣ����Ϣ���У������

    if(memcmp(type,"0",1) == 0)
    {
        SaveDevState(DEVICE_DISABLE);
        SendToQueue(type,strlen((const char*)type),AUTH_MODE_UNBIND);
    }
    else if(memcmp(type,"1",1) == 0)
    {  
        //add 2020.04.27
        xQueueReset(xDataProcessQueue); 

        SaveDevState(DEVICE_ENABLE);          
        SendToQueue(type,strlen((const char*)type),AUTH_MODE_BIND);
    } 
    
    len = strlen((const char*)buf);

    log_d("UnbindDev len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);

    return result;

}




//���ñ���ʱ��
static SYSERRORCODE_E SetLocalTime( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t localTime[32] = {0};
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)localTime,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"time",1));

    //���汾��ʱ��
    log_d("server time is %s\r\n",localTime);

    bsp_ds1302_mdifytime(localTime);


    return result;

}

//���ñ���SN��MQTT��
static SYSERRORCODE_E SetLocalSn( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t deviceCode[32] = {0};//�豸ID
    uint8_t deviceID[5] = {0};//QRID
    uint16_t len = 0;

    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)deviceCode,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"deviceCode",0));
    strcpy((char *)deviceID,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"id",0));


    result = modifyJsonItem(msgBuf,"status","1",0,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("SetLocalSn len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);
    

    //��¼SN
    ClearDevBaseParam();
    optDevBaseParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    
    log_d("gDevBaseParam.deviceCode.deviceSn = %s,len = %d\r\n",gDevBaseParam.deviceCode.deviceSn,gDevBaseParam.deviceCode.deviceSnLen);

    gDevBaseParam.deviceCode.qrSnLen = strlen((const char*)deviceID);
    gDevBaseParam.deviceCode.deviceSnLen = strlen((const char*)deviceCode);
    memcpy(gDevBaseParam.deviceCode.deviceSn,deviceCode,gDevBaseParam.deviceCode.deviceSnLen);
    memcpy(gDevBaseParam.deviceCode.qrSn,deviceID,gDevBaseParam.deviceCode.qrSnLen);

    gDevBaseParam.deviceCode.downLoadFlag.iFlag = DEFAULT_BASE_INIVAL;    
    
    strcpy ( gDevBaseParam.mqttTopic.publish,DEVICE_PUBLISH );
    strcpy ( gDevBaseParam.mqttTopic.subscribe,DEVICE_SUBSCRIBE );    
    strcat ( gDevBaseParam.mqttTopic.subscribe,(const char*)deviceCode );     
    optDevBaseParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    gUpdateDevSn = 1;

    return result;


}



