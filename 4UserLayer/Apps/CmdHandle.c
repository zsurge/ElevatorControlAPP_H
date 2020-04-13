/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : comm.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年6月18日
  最近修改   :
  功能描述   : 解析串口指令
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月18日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
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


#define LOG_TAG    "CmdHandle"
#include "elog.h"						



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define DIM(x)  (sizeof(x)/sizeof(x[0])) //计算数组长度


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
    

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
int gConnectStatus = 0;
int	gMySock = 0;
uint8_t gUpdateDevSn = 0; 

uint32_t gCurTick = 0;



READER_BUFF_STRU gReaderMsg;

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode);
static SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf ); //开门
static SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf ); //远程报警
static SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf ); //添加卡号
static SYSERRORCODE_E DelCardNo ( uint8_t* msgBuf ); //删除卡号
static SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf ); //对设备进行升级
static SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf ); //升级应答
static SYSERRORCODE_E EnableDev ( uint8_t* msgBuf ); //开启设备
static SYSERRORCODE_E DisableDev ( uint8_t* msgBuf ); //关闭设备
static SYSERRORCODE_E SetJudgeMode ( uint8_t* msgBuf ); //设置识别模式
static SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf ); //获取设备信息
static SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf ); //获取模板参数
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf ); //获取模板参数
static SYSERRORCODE_E GetUserInfo ( uint8_t* msgBuf ); //获取用户信息
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf ); //远程呼梯
static SYSERRORCODE_E PCOptDev ( uint8_t* msgBuf ); //PC端呼梯
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf ); //删除用户信息
static SYSERRORCODE_E AddSingleUser( uint8_t* msgBuf ); //添加单个用户
static SYSERRORCODE_E UnbindDev( uint8_t* msgBuf ); //解除绑定
static SYSERRORCODE_E SetLocalTime( uint8_t* msgBuf ); //设置本地时间
static SYSERRORCODE_E SetLocalSn( uint8_t* msgBuf ); //设置本地SN，MQTT用
static SYSERRORCODE_E DelCard( uint8_t* msgBuf ); //删除卡号
static SYSERRORCODE_E DelUserId( uint8_t* msgBuf ); //删除用户
static SYSERRORCODE_E getRemoteTime ( uint8_t* msgBuf );//获取远程服务器时间

static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ); //返回默认消息


typedef SYSERRORCODE_E ( *cmd_fun ) ( uint8_t *msgBuf ); 

typedef struct
{
	const char* cmd_id;             /* 命令id */
	cmd_fun  fun_ptr;               /* 函数指针 */
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
    {"3011", EnableDev}, //同绑定
    {"3012", DisableDev},//同解绑
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
    
	/* 清零 */
    ptQR->authMode = authMode; 
    ptQR->dataLen = 0;
    memset(ptQR->data,0x00,sizeof(ptQR->data)); 

    ptQR->dataLen = len;                
    memcpy(ptQR->data,buf,len);
    
    /* 使用消息队列实现指针变量的传递 */
    if(xQueueSend(xTransQueue,              /* 消息队列句柄 */
                 (void *) &ptQR,   /* 发送指针变量recv_buf的地址 */
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


int PublishData(uint8_t *payload_out,uint16_t payload_out_len)
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
//       topicString.cstring = DEVICE_PUBLISH;       //属性上报 发布       
       topicString.cstring = gMqttDevSn.publish;       //属性上报 发布    

       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//发布消息
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



//这个是为了方便服务端调试，给写的默认返回的函数
static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ) //返回默认消息
{
        SYSERRORCODE_E result = NO_ERR;
        uint8_t buf[MQTT_TEMP_LEN] = {0};
        uint16_t len = 0;
    
        if(!msgBuf)
        {
            return STR_EMPTY_ERR;
        }
    
        result = modifyJsonItem(packetBaseJson(msgBuf),"status","1",1,buf);      
        result = modifyJsonItem(packetBaseJson(buf),"UnknownCommand","random return",1,buf);   
    
        if(result != NO_ERR)
        {
            return result;
        }
    
        len = strlen((const char*)buf);
    
        log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);
    
        PublishData(buf,len);
        
        return result;

}


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

    PublishData(buf,len);
    
	return result;
}

SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	//1.跟电梯通讯异常；
	//2.设备已停用；你把设备解绑了什么的，我这有一个状态,你还给我发远程的呼梯,我就给你抛一个这样的异常状态给你。
	//3.存储器损坏；
	//4.读卡器已损坏
	return result;
}

//删除用户
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

    //1.保存卡号和用户ID

//  2.日    期   : 2020年4月13日
//    作    者   :  
//    修改内容   : 这里返回人员ID 数组，是可以批量删除人员的
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
        userData.userState = USER_DEL; //设置卡状态为0，删除卡
        wRet = modifyUserData(userData,USER_MODE);
    }

    if(wRet ==0)
    {
        //响应服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);
    }
    else
    {
        //包括没有该条记录和其它错误
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",1,buf);
    }  

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);    

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

    //1.保存用户ID
    memset(userId,0x00,sizeof(userId));
    strcpy((char *)userId,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf(userData.userId,"%08s",userId);
    log_d("userData.userId = %s,len = %d\r\n",userData.userId,strlen(userData.userId));

    //2.保存卡号
    memset(cardNo,0x00,sizeof(cardNo));
    strcpy((char *)cardNo,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
//    sprintf(userData.cardNo,"%08s",cardNo);
    log_d("cardNo = %s,len = %d\r\n",cardNo,strlen(cardNo));

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
        sprintf(userData.cardNo,"%08s",cardNo);
        userData.head = TABLE_HEAD;
        userData.cardState = CARD_VALID;
        ret = writeUserData(userData,CARD_MODE);  
    }

    if(ret == 0)
    {
        //影响服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","success",1,buf);
    }
    else
    {
        //影响服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","add error",1,buf);
    }

    
    if(result != NO_ERR)
    {
        return result;
    }
    
    len = strlen((const char*)buf);
    
    PublishData(buf,len); 

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

//1013 删除人员的所有卡
SYSERRORCODE_E DelCardNo ( uint8_t* msgBuf )
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

    //1.保存卡号和用户ID

//  2.日    期   : 2020年4月13日
//    作    者   :  
//    修改内容   : 这里需要取JSON数组，数组里包含所有要删除的人员
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
        userData.cardState = CARD_DEL; //设置卡状态为0，删除卡
        wRet = modifyUserData(userData,USER_MODE);
    }
    
    //2.查询以卡号为ID的记录，并删除
    if(wRet ==0)
    {
        //响应服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);
    }
    else
    {
        //包括没有该条记录和其它错误
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",1,buf);
    }  

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);    

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

    //1.保存URL
    strcpy((char *)tmpUrl,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"softwareUrl",1));
    log_d("tmpUrl = %s\r\n",tmpUrl);
    
    ef_set_env("url", (const char*)GetJsonItem((const uint8_t *)tmpUrl,(const uint8_t *)"picUrl",0)); 

    //2.设置升级状态为待升级状态
    ef_set_env("up_status", "101700");
    
    //3.保存整个JSON数据
    ef_set_env("upData", (const char*)msgBuf);
    
    //4.设置标志位并重启
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

    log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
	return result;

}



SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;

    //读取升级数据并解析JSON包   

    result = upgradeDataPacket(buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
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

    //这里需要发消息到消息队列，启用
    SendToQueue(type,strlen((const char*)type),AUTH_MODE_BIND);

    len = strlen((const char*)buf);

    log_d("EnableDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);

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

    //这里需要发消息到消息队列，禁用
    SendToQueue(type,strlen((const char*)type),AUTH_MODE_UNBIND);
    
    len = strlen((const char*)buf);

    log_d("DisableDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);

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

    PublishData(buf,len);
    
    my_free(identification);
    
	return result;

}
 
//删除卡号  单卡删除
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

    //1.获取卡号和用户ID
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));   
    sprintf(userId,"%08s",tmp); 

    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    sprintf(cardNo,"%08s",tmp);     
    log_d("cardNo = %s，userId = %s\r\n",cardNo,userId);



    log_d("=================================\r\n");
    rRet = readUserData(cardNo,CARD_MODE,&userData);

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
        userData.head   = TABLE_HEAD;
        userData.cardState = CARD_DEL; //设置卡状态为0，删除卡
        wRet = modifyUserData(userData,CARD_MODE);
    }
    
    //2.查询以卡号为ID的记录，并删除
    if(wRet ==0)
    {
        //响应服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);
    }
    else
    {
        //包括没有该条记录和其它错误
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",1,buf);
    }  

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);    

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

SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }


    //保存模板数据 这里应该有一个线程专门用于读写FLASH，调试期间，暂时放在响应后边
    //saveTemplateParam(msgBuf);    
    
    result = modifyJsonItem(packetBaseJson(msgBuf),"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("GetParam len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);

    //保存模板数据
    saveTemplateParam(msgBuf);

    //读取模板数据
    readTemplateData();
    
	return result;
}

//获服务器IP
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

    //1.保存IP     
    strcpy((char *)ip,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"ip",1));
    log_d("server ip = %s\r\n",ip);

    //影响服务器
    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);
    
	return result;

}

//获取用户信息
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

    //1.保存以userID为key的表
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf(tempUserData.userId,"%08s",tmp);
    log_d("tempUserData.userId = %s,len = %d\r\n",tempUserData.userId,strlen(tempUserData.userId));
    
    //3.保存楼层权限
    strcpy((char *)tempUserData.accessFloor,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
    log_d("tempUserData.accessFloor = %s\r\n",tempUserData.accessFloor);

    //4.保存默认楼层
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"defaultLayer",1));
    tempUserData.defaultFloor = atoi(tmp);
    log_d("tempUserData.defaultFloor = %d\r\n",tempUserData.defaultFloor);

    //5.保存开始时间
    strcpy((char *)tempUserData.startTime,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"startTime",1));
    log_d("tempUserData.startTime = %s\r\n",tempUserData.startTime);
    
    //6.保存结束时间
    strcpy((char *)tempUserData.endTime,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"endTime",1));
    log_d("tempUserData.endTime = %s\r\n",tempUserData.endTime);

    //2.保存卡号
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));

    if(strlen((const char *)tmp) > CARD_NO_LEN)
    {
        split(tmp,",",multipleCard,&multipleCardNum); //调用函数进行分割 
        log_d("multipleCardNum = %d\r\n",multipleCardNum);
        log_d("m0 = %s,m1= %s\r\n",multipleCard[0],multipleCard[1]);            
        
    }
    else
    {
        sprintf(tempUserData.cardNo,"%08s",tmp);
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
            log_d("write card id = %d\r\n",ret);   
            if(ret != 0)
            {
                result = FLASH_W_ERR;
            }            
        }
    }

    //全0的USER ID不记录
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
        //影响服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","success",1,buf);
    }
    else
    {
        //影响服务器
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","error",1,buf);
    }

    
    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);    
    
	return result;

}

//远程呼梯
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

    
    strcpy((char *)accessFloor,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));

    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }
    

    //这里需要发消息到消息队列，进行呼梯
    SendToQueue(accessFloor,strlen((const char*)accessFloor),AUTH_MODE_REMOTE);

    len = strlen((const char*)buf);

    log_d("RemoteOptDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len); 

    
    
    return result;

}

//PC端呼梯
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
    

    //这里需要发消息到消息队列，进行呼梯
    SendToQueue(purposeLayer,strlen((const char*)purposeLayer),AUTH_MODE_REMOTE);

    len = strlen((const char*)buf);

    log_d("RemoteOptDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len); 
    
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
//    log_d("===============TEST==================\r\n");
//    len = readUserData("89E1E35D",CARD_MODE,&userData);

//    log_d("ret = %d\r\n",len);    
//    log_d("userData.userState = %d\r\n",userData.cardState);
//    log_d("userData.cardNo = %s\r\n",userData.cardNo);
//    log_d("userData.userId = %s\r\n",userData.userId);
//    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
//    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
//    log_d("userData.startTime = %s\r\n",userData.startTime);

//    log_d("===============TEST==================\r\n");
//    memset(&userData,0x00,sizeof(USERDATA_STRU));
//    len = readUserData("00002815",USER_MODE,&userData);
//    
//    log_d("ret = %d\r\n",len);    
//    log_d("userData.userState = %d\r\n",userData.cardState);
//    log_d("userData.cardNo = %s\r\n",userData.cardNo);
//    log_d("userData.userId = %s\r\n",userData.userId);
//    log_d("userData.accessFloor = %s\r\n",userData.accessFloor);
//    log_d("userData.defaultFloor = %d\r\n",userData.defaultFloor);
//    log_d("userData.startTime = %s\r\n",userData.startTime);


TestFlash(CARD_MODE);

// 
TestFlash(USER_MODE);







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



//删除用户信息
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("ClearUserInof len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);


    //清空用户信息
    eraseUserDataAll();
    
    return result;

}

//添加单个用户
static SYSERRORCODE_E AddSingleUser( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	uint16_t len =0;
    uint8_t buf[1024] = {0};
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

    //1.添加起始标志
    tempUserData.head = TABLE_HEAD; 

    //1.保存以userID为key的表
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    sprintf(tempUserData.userId,"%08s",tmp);
    log_d("tempUserData.userId = %s,len = %d\r\n",tempUserData.userId,strlen(tempUserData.userId));

    
    
    //3.保存楼层权限
    strcpy((char *)tempUserData.accessFloor,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
    log_d("tempUserData.accessFloor = %s\r\n",tempUserData.accessFloor);

    //4.保存默认楼层
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"defaultLayer",1));
    tempUserData.defaultFloor = atoi(tmp);
    log_d("tempUserData.defaultFloor = %d\r\n",tempUserData.defaultFloor);

    //5.保存开始时间
    strcpy((char *)tempUserData.startTime,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"startTime",1));
    log_d("tempUserData.startTime = %s\r\n",tempUserData.startTime);
    
    //6.保存结束时间
    strcpy((char *)tempUserData.endTime,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"endTime",1));
    log_d("tempUserData.endTime = %s\r\n",tempUserData.endTime);


    //全0的USER ID不记录
    if(memcmp(tempUserData.userId,"00000000",CARD_USER_LEN) != 0)
    {
        tempUserData.userState = USER_VALID;
        ret = writeUserData(tempUserData,USER_MODE);
        log_d("write user id = %d\r\n",ret);       
    }   

    
    log_d("tempUserData.cardNo = %s,len = %d\r\n",tempUserData.cardNo,strlen(tempUserData.cardNo));    



    if(ret == 0)
    {
        //影响服务器
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

    PublishData(buf,len);    
    
	return result;


}

//解除绑定
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

    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    //这里需要发消息到消息队列，解除绑定

    if(memcmp(type,"0",1) == 0)
    {
        SendToQueue(type,strlen((const char*)type),AUTH_MODE_UNBIND);
    }
    else if(memcmp(type,"1",1) == 0)
    {
        SendToQueue(type,strlen((const char*)type),AUTH_MODE_BIND);
    } 
    
    len = strlen((const char*)buf);

    log_d("UnbindDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);

    return result;

}




//设置本地时间
static SYSERRORCODE_E SetLocalTime( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t localTime[32] = {0};
    uint16_t len = 0;
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)localTime,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"time",1));

    //保存本地时间
    log_d("server time is %s\r\n",localTime);

    bsp_ds1302_mdifytime(localTime);


    return result;

}

//设置本地SN，MQTT用
static SYSERRORCODE_E SetLocalSn( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t deviceCode[32] = {0};
    uint8_t deviceID[4] = {0};
    uint16_t len = 0;
//    char *tmpBuf[6] = {0}; //存放分割后的子字符串 
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

    //记录SN
    ef_set_env_blob("sn_flag","1111",4);    
    ef_set_env_blob("remote_sn",deviceCode,strlen(deviceCode));   
    ef_set_env_blob("device_sn",deviceID,strlen(deviceID)); 

    log_d("remote_sn = %s\r\n",deviceCode);
    
    len = strlen((const char*)buf);

    log_d("SetLocalSn len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);

    ReadLocalDevSn();

    gUpdateDevSn = 1;

    return result;


}



