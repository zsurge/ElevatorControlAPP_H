/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : ini.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月28日
  最近修改   :
  功能描述   : 参数设置
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月28日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "ini.h"
#include "easyflash.h"
#include "version.h"
#include "string.h"
#include "templateprocess.h"
#include "tool.h"
#include "LocalData.h"
#include "eth_cfg.h"



#define LOG_TAG    "ini"
#include "elog.h"

MQTT_DEVICE_SN_STRU gMqttDevSn;


/*****************************************************************************
 函 数 名  : RestoreDefaultSetting
 功能描述  : 恢复默认设置,实际是写入系统启动所需要的参数
           若使用ef_env_set_default 则需要在ef_port.c中对默认值进行修改
 输入参数  : 
             
 输出参数  : 无
 返 回 值  : void
 
 修改历史      :
  1.日    期   : 2019年5月28日
    作    者   : 张舵

    修改内容   : 新生成函数

*****************************************************************************/
void RestoreDefaultSetting(void)
{
    if(ef_env_set_default()== EF_NO_ERR)
    {
        DBG("RestoreDefaultSetting success\r\n");
    }
    
}

void SystemUpdate(void)
{
    //写升级标志位
    if(ef_set_env("WSPIFLASH","5050") == EF_NO_ERR)
    {
        ef_set_env("WMCUFLASH","6060");
        //jump iap
        DBG("jump iap\r\n");
        NVIC_SystemReset();
    }
}

void readTemplateData(void)
{
    char tempBuff[8] = {0};
    int valueLen = 0;
    int index = 0;
    char tmpIndex[2] = {0};
    char tmpKey[32] = {0};
    char *callingWay[4] = {0}; //存放分割后的子字符串 
    int callingWayNum = 0;

    memset(&gTemplateParam,0x00,sizeof(TEMPLATE_PARAM_STRU));
    TEMPLATE_PARAM_STRU *templateParam = &gTemplateParam; 


    //读取模板状态
    memset(tempBuff,0x00,sizeof(tempBuff));
    valueLen = ef_get_env_blob("templateStatus", tempBuff, sizeof(tempBuff) , NULL);
    templateParam->templateStatus = atoi(tempBuff);

    //读取模板是否支持离线状态
    memset(tempBuff,0x00,sizeof(tempBuff));
    valueLen = ef_get_env_blob("offlineStatus", tempBuff, sizeof(tempBuff) , NULL);
    templateParam->offlineProcessing = atoi(tempBuff);    
    
    //读取工作模式，是否开节假日模式
    valueLen = ef_get_env_blob("modeType", templateParam->modeType, sizeof(templateParam->modeType) , NULL);
    if(valueLen == 0)
    {
        log_d("get templateParam error!\r\n");
    }   

    //读取模板的识别模式
    valueLen = 0;
    valueLen = ef_get_env_blob("T_callingWay", templateParam->callingWay, sizeof(templateParam->callingWay) , NULL);

    //解析模板识别方式    
    split((char *)templateParam->callingWay,",",callingWay,&callingWayNum); //调用函数进行分割 
    while(callingWayNum)
    {
        switch(atoi(callingWay[callingWayNum-1]))
        {
            case 1:
                templateParam->templateCallingWay.isFace = 1;
                break;
            case 2:
                templateParam->templateCallingWay.isQrCode = 1;
                break;
            case 3:
                templateParam->templateCallingWay.isIcCard = 1;
                break; 
        }
        callingWayNum-- ;  
    }    
    

    //读取节假日的识别模式
    valueLen = 0;
    valueLen = ef_get_env_blob("peakCallingWay", templateParam->peakInfo[0].callingWay, sizeof(templateParam->peakInfo[0].callingWay) , NULL);

    //解析识别方式    
    memset(callingWay,0x00,sizeof(callingWay));
    callingWayNum = 0;
    split((char *)templateParam->peakInfo[0].callingWay,",",callingWay,&callingWayNum); //调用函数进行分割 
    while(callingWayNum)
    {
        switch(atoi(callingWay[callingWayNum-1]))
        {
            case 1:
                templateParam->peakCallingWay.isFace = 1;
                break;
            case 2:
                templateParam->peakCallingWay.isQrCode = 1;
                break;
            case 3:
                templateParam->peakCallingWay.isIcCard = 1;
                break; 
        }
        callingWayNum-- ;  
    }  


    //解析是否开启节假日模式
    memset(callingWay,0x00,sizeof(callingWay));
    callingWayNum = 0;
    split((char *)templateParam->modeType,",",callingWay,&callingWayNum); //调用函数进行分割 
    while(callingWayNum)
    {
        switch(atoi(callingWay[callingWayNum-1]))
        {
            case 1:
                templateParam->workMode.isPeakMode = 1;
                break;
            case 2:
                templateParam->workMode.isHolidayMode = 1;
                break;
            case 5:
                templateParam->workMode.isNormalMode = 1;
                break; 
        }
        callingWayNum-- ;  
    }  
    

    //读取节假日排除日期的类型,1周六，2周日，3节假日（目前节假日无法屏蔽）
    valueLen = 0;
    valueLen = ef_get_env_blob("peakHolidaysType", templateParam->peakInfo[0].outsideTimeMode, sizeof(templateParam->peakInfo[0].outsideTimeMode) , NULL);

    //读取模板有效开始日期
    valueLen = 0;
    valueLen = ef_get_env_blob("peakStartDate", templateParam->peakInfo[0].beginTime, sizeof(templateParam->peakInfo[0].beginTime) , NULL);

    //读取模板有效结束日期
    valueLen = 0;
    valueLen = ef_get_env_blob("peakEndDate", templateParam->peakInfo[0].endTime, sizeof(templateParam->peakInfo[0].endTime) , NULL);

    //先判定有几个时间段
    memset(tempBuff,0x00,sizeof(tempBuff));
    valueLen = ef_get_env_blob("holidayTimeMapCnt", tempBuff, sizeof(tempBuff) , NULL);
    templateParam->peakModeCnt = atoi(tempBuff);
    
    
    memset(tmpKey,0x00,sizeof(tmpKey));
    memset(tmpIndex,0x00,sizeof(tmpIndex));  
    for(index= 0;index<templateParam->peakModeCnt;index++)
    {
        //读取节假日模板的不受控时间段--开始时间
        sprintf(tmpIndex,"%d",index);
        strcpy(tmpKey,"hoildayModeStartTime");
        strcat(tmpKey,tmpIndex); 
 

        valueLen = ef_get_env_blob(tmpKey, templateParam->holidayMode[index].startTime, sizeof(templateParam->holidayMode[index].startTime) , NULL);
        log_d("gTemplateParam->%s = %s\r\n",tmpKey,gTemplateParam.holidayMode[index].startTime);
        

         //读取节假日模板的不受控时间段--结束时间
        memset(tmpKey,0x00,sizeof(tmpKey));
        strcpy(tmpKey,"hoildayModeEndTime");
        strcat(tmpKey,tmpIndex);  
        valueLen = 0;
        valueLen = ef_get_env_blob(tmpKey, templateParam->holidayMode[index].endTime, sizeof(templateParam->holidayMode[index].endTime) , NULL);
        log_d("gTemplateParam->%s= %s\r\n",tmpKey,gTemplateParam.holidayMode[index].endTime);        
        
     }


    log_d("gTemplateParam->peakInfo[0].beginTime = %s\r\n",gTemplateParam.peakInfo[0].beginTime);      
    log_d("gTemplateParam->peakInfo[0].endTime = %s\r\n",gTemplateParam.peakInfo[0].endTime);      
    

    log_d("gTemplateParam->templateCallingWay = %d\r\n",gTemplateParam.templateCallingWay.isFace);      
    log_d("gTemplateParam->templateCallingWay = %d\r\n",gTemplateParam.templateCallingWay.isQrCode);      
    log_d("gTemplateParam->templateCallingWay = %d\r\n",gTemplateParam.templateCallingWay.isIcCard);
    
    log_d("gTemplateParam->peakCallingWay = %d\r\n",gTemplateParam.peakCallingWay.isFace);   
    log_d("gTemplateParam->peakCallingWay = %d\r\n",gTemplateParam.peakCallingWay.isQrCode);
    log_d("gTemplateParam->peakCallingWay = %d\r\n",gTemplateParam.peakCallingWay.isIcCard);      
    

    log_d("gTemplateParam->templateStatus = %d\r\n",gTemplateParam.templateStatus);     
    log_d("gTemplateParam->offlineProcessing = %d\r\n",gTemplateParam.offlineProcessing);
    log_d("gTemplateParam->modeType = %s\r\n",gTemplateParam.modeType);
    log_d("gTemplateParam->callingWay = %s\r\n",gTemplateParam.callingWay);    
    log_d("gTemplateParam->peakInfo[0].callingWay = %s\r\n",gTemplateParam.peakInfo[0].callingWay);
    log_d("gTemplateParam->peakInfo[0].beginTime = %s\r\n",gTemplateParam.peakInfo[0].beginTime);
    log_d("gTemplateParam->peakInfo[0].endTime = %s\r\n",gTemplateParam.peakInfo[0].endTime);
    log_d("gTemplateParam->peakInfo[0].outsideTimeMode = %s\r\n",gTemplateParam.peakInfo[0].outsideTimeMode);
    
}



void readCardAndUserIdIndex(void)
{
    uint8_t tempBuff[8] = {0};
    
    ef_get_env_blob("CardHeaderIndex", tempBuff, sizeof(tempBuff) , NULL);
    gCurCardHeaderIndex = atoi((const char*)tempBuff);
    log_d("gCurCardHeaderIndex = %d\r\n",gCurCardHeaderIndex);

    memset(tempBuff,0x00,sizeof(tempBuff));
    ef_get_env_blob("UserHeaderIndex", tempBuff, sizeof(tempBuff) , NULL);
    gCurUserHeaderIndex = atoi((const char*)tempBuff);  
    log_d("gCurUserHeaderIndex = %d\r\n",gCurUserHeaderIndex);

    memset(tempBuff,0x00,sizeof(tempBuff));
    ef_get_env_blob("DelCardHeaderIndex", tempBuff, sizeof(tempBuff) , NULL);                     
    gDelCardHeaderIndex = atoi((const char*)tempBuff);  
    log_d("gDelCardHeaderIndex = %d\r\n",gDelCardHeaderIndex);

    memset(tempBuff,0x00,sizeof(tempBuff));
    ef_get_env_blob("DelUserHeaderIndex", tempBuff, sizeof(tempBuff) , NULL);
    gDelUserHeaderIndex = atoi((const char*)tempBuff);  
    log_d("gDelUserHeaderIndex = %d\r\n",gDelUserHeaderIndex);    
}

void SaveDevState(char state)
{
    uint8_t devState[5] = {0};
    
    gDeviceStateFlag = state;        
    memset(devState,0x00,sizeof(devState));
    sprintf((char *)devState,"%04d",state);
    ef_set_env_blob("DeviceState",devState,4);  

}

void readDevState(void)
{
    uint8_t devState[5] = {0};   
         
    memset(devState,0x00,sizeof(devState));
    ef_get_env_blob("DeviceState",devState,sizeof(devState) , NULL); 
    log_d("DeviceState = %s\r\n",devState);
    gDeviceStateFlag = atoi((const char*)devState); 
 
    log_d("gDeviceStateFlag = %d\r\n",gDeviceStateFlag);        

}

void ReadLocalDevSn ( void )
{
    char sn_flag[5] = {0};
    char mac[6+1] = {0};
    char id[8] = {0};
    char temp[32] = {0};
    char asc[12+1] = {0};
    char remote_sn[20+1] = {0};
    uint8_t read_len = 0;

    memset ( &gMqttDevSn,0x00,sizeof ( gMqttDevSn ) );

    read_len = ef_get_env_blob ( "sn_flag", sn_flag, sizeof ( sn_flag ), NULL );

    log_d ( "sn_flag = %s, sn_flag_len = %d\r\n",sn_flag,read_len );

    if ( ( memcmp ( sn_flag,"1111",4 ) == 0 ) && ( read_len == 4 ) )
    {
        read_len = ef_get_env_blob ( "remote_sn", remote_sn, sizeof ( remote_sn ), NULL );
        ef_get_env_blob("device_sn",id,sizeof ( id ), NULL ); 
        if ( read_len == 20 )
        {
            strcpy ( gMqttDevSn.deviceSn,id);
            
            log_d ( "sn = %s,len = %d\r\n",remote_sn,read_len );
            strcpy ( gMqttDevSn.sn,remote_sn );
            log_d("1 deviceCode = %s\r\n",gMqttDevSn.sn);
            strcpy ( gMqttDevSn.publish,DEVICE_PUBLISH );
            strcpy ( gMqttDevSn.subscribe,DEVICE_SUBSCRIBE );
            strcat ( gMqttDevSn.subscribe,remote_sn );          
        }

         log_d("2 deviceCode = %s\r\n",gMqttDevSn.sn);
         log_d("gMqttDevSn.deviceSn = %s\r\n",gMqttDevSn.deviceSn);
    }
    else
    {
        //使用MAC做为SN
        calcMac ( (unsigned char*)mac );
        bcd2asc ( (unsigned char*)asc, (unsigned char*)mac, 12, 0 );
        Insertchar ( asc,temp,':' );
        memcpy ( gMqttDevSn.sn,temp,strlen ( temp )-1 );

        log_d ( "strToUpper asc = %s\r\n",gMqttDevSn.sn );
        ef_set_env_blob ( "remote_sn",gMqttDevSn.sn,strlen ( gMqttDevSn.sn ) );

        strcpy ( gMqttDevSn.publish,DEV_FACTORY_PUBLISH );
        strcpy ( gMqttDevSn.subscribe,DEV_FACTORY_SUBSCRIBE );
        strcat ( gMqttDevSn.subscribe,gMqttDevSn.sn );
        memcpy ( gMqttDevSn.deviceSn,gMqttDevSn.sn,8);
    }


    log_d ( "publish = %s,subscribe = %s\r\n",gMqttDevSn.publish,gMqttDevSn.subscribe );
}


