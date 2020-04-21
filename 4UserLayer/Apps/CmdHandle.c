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
#include "templateprocess.h"


#define LOG_TAG    "CmdHandle"
#include "elog.h"						



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

uint32_t gCurTick = 0;
uint8_t gDeviceStateFlag = DEVICE_ENABLE;



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

CMD_HANDLE_T CmdList[] =
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
    log_d("Start parsing JSON data\r\n");
    
    strcpy(cmd,(const char *)GetJsonItem ( data, ( const uint8_t* ) "commandCode",0 ));   
    
    exec_proc (cmd ,data);
}

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode)
{
    SYSERRORCODE_E result = NO_ERR;

    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));
    READER_BUFF_STRU *ptQR = &gReaderMsg;
    
	/* ���� */
    ptQR->authMode = authMode; 
    ptQR->dataLen = 0;
    memset(ptQR->data,0x00,sizeof(ptQR->data)); 

    ptQR->dataLen = len;                
    memcpy(ptQR->data,buf,len);
    
    /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    if(xQueueSend(xTransQueue,              /* ��Ϣ���о�� */
                 (void *) &ptQR,   /* ����ָ�����recv_buf�ĵ�ַ */
                 (TickType_t)300) != pdPASS )
    {
        DBG("the queue is full!\r\n");                
        xQueueReset(xTransQueue);
    } 
    else
    {
        dbh("SendToQueue",(char *)buf,len);
        log_d("SendToQueue buf = %s,len = %d\r\n",buf,len);
    } 


    return result;
}


int mqttSendData(uint8_t *payload_out,uint16_t payload_out_len)
{   
	MQTTString topicString = MQTTString_initializer;
    
	uint32_t len = 0;
	int32_t rc = 0;
	unsigned char buf[MQTT_MAX_LEN];
	int buflen = sizeof(buf);

	unsigned short msgid = 1;
	int req_qos = 0;
	unsigned char retained = 0;  

    if(!payload_out)
    {
        return STR_EMPTY_ERR;
    }

    log_d("payload_out = %s,payload_out_len = %d\r\n",payload_out,payload_out_len);

   if(gConnectStatus == 1)
   { 
//       topicString.cstring = DEVICE_PUBLISH;       //�����ϱ� ����       
       topicString.cstring = gMqttDevSn.publish;       //�����ϱ� ����    

       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//������Ϣ
       rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
       if(rc == len) 
        {//
           gCurTick =  xTaskGetTickCount();
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

//    result = modifyJsonItem(msgBuf,"openStatus","1",1,buf);
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
        wRet = modifyUserData(userData,USER_MODE);
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
    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);

    if(ret == 0)
    {
        sprintf((char*)userData.cardNo,"%08s",cardNo);
        userData.head = TABLE_HEAD;
        userData.cardState = CARD_VALID;
        ret = writeUserData(userData,CARD_MODE);  
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
        return STR_EMPTY_ERR;
    }

    //1.���濨�ź��û�ID
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf((char *)userId,"%08s",tmp);
    log_d("userId = %s\r\n",userId);

    cardArray = GetJsonArray ((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",&num);
    
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
    uint8_t type[4] = {0};
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

    SaveDevState(DEVICE_DISABLE);
    
    //������Ҫ����Ϣ����Ϣ���У�����
    SendToQueue(type,strlen((const char*)type),AUTH_MODE_UNBIND);
    
    len = strlen((const char*)buf);

    log_d("DisableDev len = %d,buf = %s\r\n",len,buf);

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
    readTemplateData();
    
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
    char *multipleCard[20] = {0};
    int multipleCardNum = 0;

    
    memset(&tempUserData,0x00,sizeof(USERDATA_STRU));

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    tempUserData.head = TABLE_HEAD;

    //1.������userIDΪkey�ı�
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf((char *)tempUserData.userId,"%08s",tmp);
    log_d("tempUserData.userId = %s,len = %d\r\n",tempUserData.userId,strlen((const char *)tempUserData.userId));
    
    //3.����¥��Ȩ��
    strcpy((char *)tempUserData.accessFloor,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
    log_d("tempUserData.accessFloor = %s\r\n",tempUserData.accessFloor);

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
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));

    if(strlen((const char *)tmp) > CARD_NO_LEN)
    {
        split((char *)tmp,",",multipleCard,&multipleCardNum); //���ú������зָ� 
        log_d("multipleCardNum = %d\r\n",multipleCardNum);
        log_d("m0 = %s,m1= %s\r\n",multipleCard[0],multipleCard[1]);            
        
    }
    else
    {
        sprintf((char *)tempUserData.cardNo,"%08s",tmp);
    }    

    if(multipleCardNum > 1)
    {
        for(len=0;len<multipleCardNum;len++)
        {
            tempUserData.cardState = CARD_VALID;     
            memset(tempUserData.cardNo,0x00,sizeof(tempUserData.cardNo));
            memcpy(tempUserData.cardNo,multipleCard[len],CARD_USER_LEN);    
            log_d("multipleCard[len] = %s,tempUserData.cardNo= %s\r\n",multipleCard[len],tempUserData.cardNo);            
            ret = writeUserData(tempUserData,CARD_MODE);

            if(ret != 0)
            {
                result = FLASH_W_ERR;
            }

        }
    }
    else
    {
        if(memcmp(tempUserData.cardNo,"00000000",CARD_USER_LEN) != 0)
        {
            tempUserData.cardState = CARD_VALID;
            ret = writeUserData(tempUserData,CARD_MODE);
               
            if(ret != 0)
            {
                result = FLASH_W_ERR;
            }   

            log_d("write card id is success! ret = %d\r\n",ret);
        }
    }

    //ȫ0��USER ID����¼
    if(memcmp(tempUserData.userId,"00000000",CARD_USER_LEN) != 0)
    {
        tempUserData.userState = USER_VALID;
        ret = writeUserData(tempUserData,USER_MODE);
        log_d("write user id = %d\r\n",ret); 
        if(ret != 0)
        {
            result = FLASH_W_ERR;
        }        
    } 

    
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
        return result;
    }

    len = strlen((const char*)buf);

    mqttSendData(buf,len);    
    
	return result;

}

//Զ�̺���
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t accessFloor[4] = {0};
    uint16_t len = 0;
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    if(gTemplateParam.templateCallingWay.isFace && gDeviceStateFlag == DEVICE_ENABLE)
    {
        strcpy((char *)accessFloor,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));

        result = modifyJsonItem(msgBuf,"status","1",1,buf);

        if(result != NO_ERR)
        {
            return result;
        }
        

        //������Ҫ����Ϣ����Ϣ���У����к���
        SendToQueue(accessFloor,strlen((const char*)accessFloor),AUTH_MODE_REMOTE);

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
    uint8_t purposeLayer[4] = {0};
    uint16_t len = 0;

    USERDATA_STRU userData = {0};
    memset(&userData,0x00,sizeof(USERDATA_STRU));
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    
    strcpy((char *)purposeLayer,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"purposeLayer",1));

    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }
    

    //������Ҫ����Ϣ����Ϣ���У����к���
    SendToQueue(purposeLayer,strlen((const char*)purposeLayer),AUTH_MODE_REMOTE);

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
//    writeUserData(userData,CARD_MODE);
//    
    log_d("===============TEST==================\r\n");
    len = readUserData("12345688",CARD_MODE,&userData);

    log_d("ret = %d\r\n",len);    
    log_d("userData.userState = %d\r\n",userData.cardState);
    log_d("userData.cardNo = %s\r\n",userData.cardNo);
    log_d("userData.userId = %s\r\n",userData.userId);
    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);

    log_d("===============TEST==================\r\n");
    memset(&userData,0x00,sizeof(USERDATA_STRU));
    len = readUserData("00020419",USER_MODE,&userData);
    
    log_d("ret = %d\r\n",len);    
    log_d("userData.userState = %d\r\n",userData.cardState);
    log_d("userData.cardNo = %s\r\n",userData.cardNo);
    log_d("userData.userId = %s\r\n",userData.userId);
    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
    log_d("userData.startTime = %s\r\n",userData.startTime);



log_d("===============CARD_MODE==================\r\n");
TestFlash(CARD_MODE);


log_d("===============USER_MODE==================\r\n");
TestFlash(USER_MODE);


log_d("===============CARD_DEL_MODE==================\r\n");
TestFlash(CARD_DEL_MODE);


log_d("===============USER_DEL_MODE==================\r\n");
TestFlash(USER_DEL_MODE);








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

    memset(&tempUserData,0x00,sizeof(USERDATA_STRU));

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.�����ʼ��־
    tempUserData.head = TABLE_HEAD; 

    //1.������userIDΪkey�ı�
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf((char *)tempUserData.userId,"%08s",tmp);
    log_d("tempUserData.userId = %s,len = %d\r\n",tempUserData.userId,strlen((const char*)tempUserData.userId));

    
    
    //3.����¥��Ȩ��
    strcpy((char *)tempUserData.accessFloor,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
    log_d("tempUserData.accessFloor = %s\r\n",tempUserData.accessFloor);

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


    //ȫ0��USER ID����¼
    if(memcmp(tempUserData.userId,"00000000",CARD_USER_LEN) != 0)
    {
        tempUserData.userState = USER_VALID;
        ret = writeUserData(tempUserData,USER_MODE);
        log_d("write user id = %d\r\n",ret);       
    }   

    
    log_d("tempUserData.cardNo = %s,len = %d\r\n",tempUserData.cardNo,strlen((const char*)tempUserData.cardNo));    



    if(ret == 0)
    {
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
    }
    else
    {
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
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
    uint8_t type[4] = {0};
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
    uint8_t deviceCode[32] = {0};
    uint8_t deviceID[4] = {0};
    uint16_t len = 0;
//    char *tmpBuf[6] = {0}; //��ŷָ������ַ��� 
//    int num = 0;
    
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

    //��¼SN
    ef_set_env_blob("sn_flag","1111",4);    
    ef_set_env_blob("remote_sn",deviceCode,strlen((const char*)deviceCode));   
    ef_set_env_blob("device_sn",deviceID,strlen((const char*)deviceID)); 

    log_d("remote_sn = %s\r\n",deviceCode);
    
    len = strlen((const char*)buf);

    log_d("SetLocalSn len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);

    ReadLocalDevSn();

    gUpdateDevSn = 1;

    return result;


}



