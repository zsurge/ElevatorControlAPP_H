/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : ini.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��28��
  ����޸�   :
  ��������   : ��������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��28��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "ini.h"



#define LOG_TAG    "ini"
#include "elog.h"

DEVICE_ID_STRU gDeviceId;
MQTT_TOPIC_STRU gMqttTopic;
TEMPLATE_PARAM_STRU gTemplateParam;



/*****************************************************************************
 �� �� ��  : RestoreDefaultSetting
 ��������  : �ָ�Ĭ������,ʵ����д��ϵͳ��������Ҫ�Ĳ���
           ��ʹ��ef_env_set_default ����Ҫ��ef_port.c�ж�Ĭ��ֵ�����޸�
 �������  : 
             
 �������  : ��
 �� �� ֵ  : void
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��28��
    ��    ��   : �Ŷ�

    �޸�����   : �����ɺ���

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
    //д������־λ
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
    char *callingWay[4] = {0}; //��ŷָ������ַ��� 
    int callingWayNum = 0;

    memset(&gTemplateParam,0x00,sizeof(TEMPLATE_PARAM_STRU));
    TEMPLATE_PARAM_STRU *templateParam = &gTemplateParam; 


    //��ȡģ��״̬
    memset(tempBuff,0x00,sizeof(tempBuff));
    valueLen = ef_get_env_blob("templateStatus", tempBuff, sizeof(tempBuff) , NULL);
    templateParam->templateStatus = atoi(tempBuff);

    //��ȡģ���Ƿ�֧������״̬
    memset(tempBuff,0x00,sizeof(tempBuff));
    valueLen = ef_get_env_blob("offlineStatus", tempBuff, sizeof(tempBuff) , NULL);
    templateParam->offlineProcessing = atoi(tempBuff);    
    
    //��ȡ����ģʽ���Ƿ񿪽ڼ���ģʽ
    valueLen = ef_get_env_blob("modeType", templateParam->modeType, sizeof(templateParam->modeType) , NULL);
    if(valueLen == 0)
    {
        log_d("get templateParam error!\r\n");
    }   

    //��ȡģ���ʶ��ģʽ
    valueLen = 0;
    valueLen = ef_get_env_blob("T_callingWay", templateParam->callingWay, sizeof(templateParam->callingWay) , NULL);

    //����ģ��ʶ��ʽ    
    split((char *)templateParam->callingWay,",",callingWay,&callingWayNum); //���ú������зָ� 
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
    

    //��ȡ�ڼ��յ�ʶ��ģʽ
    valueLen = 0;
    valueLen = ef_get_env_blob("peakCallingWay", templateParam->peakInfo[0].callingWay, sizeof(templateParam->peakInfo[0].callingWay) , NULL);

    //����ʶ��ʽ    
    memset(callingWay,0x00,sizeof(callingWay));
    callingWayNum = 0;
    split((char *)templateParam->peakInfo[0].callingWay,",",callingWay,&callingWayNum); //���ú������зָ� 
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


    //�����Ƿ����ڼ���ģʽ
    memset(callingWay,0x00,sizeof(callingWay));
    callingWayNum = 0;
    split((char *)templateParam->modeType,",",callingWay,&callingWayNum); //���ú������зָ� 
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
    

    //��ȡ�ڼ����ų����ڵ�����,1������2���գ�3�ڼ��գ�Ŀǰ�ڼ����޷����Σ�
    valueLen = 0;
    valueLen = ef_get_env_blob("peakHolidaysType", templateParam->peakInfo[0].outsideTimeMode, sizeof(templateParam->peakInfo[0].outsideTimeMode) , NULL);

    //��ȡģ����Ч��ʼ����
    valueLen = 0;
    valueLen = ef_get_env_blob("peakStartDate", templateParam->peakInfo[0].beginTime, sizeof(templateParam->peakInfo[0].beginTime) , NULL);

    //��ȡģ����Ч��������
    valueLen = 0;
    valueLen = ef_get_env_blob("peakEndDate", templateParam->peakInfo[0].endTime, sizeof(templateParam->peakInfo[0].endTime) , NULL);

    //���ж��м���ʱ���
    memset(tempBuff,0x00,sizeof(tempBuff));
    valueLen = ef_get_env_blob("holidayTimeMapCnt", tempBuff, sizeof(tempBuff) , NULL);
    templateParam->peakModeCnt = atoi(tempBuff);
    
    
    memset(tmpKey,0x00,sizeof(tmpKey));
    memset(tmpIndex,0x00,sizeof(tmpIndex));  
    for(index= 0;index<templateParam->peakModeCnt;index++)
    {
        //��ȡ�ڼ���ģ��Ĳ��ܿ�ʱ���--��ʼʱ��
        sprintf(tmpIndex,"%d",index);
        strcpy(tmpKey,"hoildayModeStartTime");
        strcat(tmpKey,tmpIndex); 
 

        valueLen = ef_get_env_blob(tmpKey, templateParam->holidayMode[index].startTime, sizeof(templateParam->holidayMode[index].startTime) , NULL);
        log_d("gTemplateParam->%s = %s\r\n",tmpKey,gTemplateParam.holidayMode[index].startTime);
        

         //��ȡ�ڼ���ģ��Ĳ��ܿ�ʱ���--����ʱ��
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
    uint8_t tempBuff[8+1] = {0};


	//ef_print_env();
	
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

    memset ( &gMqttTopic,0x00,sizeof ( gMqttTopic ) );

    read_len = ef_get_env_blob ( "sn_flag", sn_flag, sizeof ( sn_flag ), NULL );

    log_d ( "sn_flag = %s, sn_flag_len = %d\r\n",sn_flag,read_len );

    if ( ( memcmp ( sn_flag,"1111",4 ) == 0 ) && ( read_len == 4 ) )
    {
        read_len = ef_get_env_blob ( "remote_sn", remote_sn, sizeof ( remote_sn ), NULL );
        ef_get_env_blob("device_sn",id,sizeof ( id ), NULL ); 
        if ( read_len == 20 )
        {
            strcpy ( gDeviceId.qrSn,id);
            
            log_d ( "sn = %s,len = %d\r\n",remote_sn,read_len );
            strcpy ( gDeviceId.deviceSn,remote_sn );
            log_d("1 deviceCode = %s\r\n",gDeviceId.deviceSn);
            
            strcpy ( gMqttTopic.publish,DEVICE_PUBLISH );
            strcpy ( gMqttTopic.subscribe,DEVICE_SUBSCRIBE );
            
            strcat ( gMqttTopic.subscribe,remote_sn );          
        }

         log_d("2 deviceCode = %s\r\n",gDeviceId.deviceSn);
    }
    else
    {
        //ʹ��MAC��ΪSN
        calcMac ( (unsigned char*)mac );
        bcd2asc ( (unsigned char*)asc, (unsigned char*)mac, 12, 0 );
        Insertchar ( asc,temp,':' );
        memcpy ( gDeviceId.deviceSn,temp,strlen ( temp )-1 );

        log_d ( "strToUpper asc = %s\r\n",gDeviceId.deviceSn );
        ef_set_env_blob ( "remote_sn",gDeviceId.deviceSn,strlen ( gDeviceId.deviceSn ) );

        strcpy ( gMqttTopic.publish,DEV_FACTORY_PUBLISH );
        strcpy ( gMqttTopic.subscribe,DEV_FACTORY_SUBSCRIBE );
        strcat ( gMqttTopic.subscribe,gDeviceId.deviceSn );
        memcpy ( gDeviceId.qrSn,gDeviceId.deviceSn,8); //ʹ��ǰ8λ��Ϊ������QRID
    }


    log_d ( "publish = %s,subscribe = %s\r\n",gMqttTopic.publish,gMqttTopic.subscribe );
}

uint8_t packetPayload(USERDATA_STRU *localUserData,uint8_t *descJson)
{ 
    
    SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*dataObj;
    char *tmpBuf;
    char tmpTime[32] = {0};

    root = cJSON_CreateObject();
    dataObj = cJSON_CreateObject();

    if(!root && !dataObj)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        my_free(tmpBuf);            
		return CJSON_CREATE_ERR;
    }

    cJSON_AddStringToObject(root, "deviceCode", gDeviceId.deviceSn);
    log_d("deviceCode = %s",gDeviceId.deviceSn);
    
    cJSON_AddItemToObject(root, "data", dataObj);

    if(localUserData->platformType == 4 )
    {    
        cJSON_AddStringToObject(root, "commandCode","3007");
        cJSON_AddStringToObject(dataObj, "cardNo", (const char*)localUserData->cardNo);
        cJSON_AddNumberToObject(dataObj, "callType",localUserData->authMode); 
        cJSON_AddNumberToObject(dataObj, "status", ON_LINE); 
        strcpy(tmpTime,(const char*)bsp_ds1302_readtime());  
        cJSON_AddStringToObject(dataObj, "callElevatorTime",tmpTime);        
        cJSON_AddStringToObject(dataObj, "timeStamp",(const char*)localUserData->startTime);
        cJSON_AddStringToObject(dataObj, "userId", (const char*)localUserData->userId);              
        
        if(localUserData->authMode == 7)
        {
            cJSON_AddNumberToObject(dataObj, "type",localUserData->platformType);
            cJSON_AddNumberToObject(dataObj, "callState",CALL_OK);
//            cJSON_AddStringToObject(dataObj, "qrId", (const char*)localUserData->qrID);
        }
        else
        {
            cJSON_AddNumberToObject(dataObj, "type",CALL_OK);            
        }
    }
    else
    {
        cJSON_AddStringToObject(root, "commandCode","4002");
        cJSON_AddNumberToObject(dataObj, "enterType",  4);//��������Ϊ4��QRCODE�ڿ�����ƽ̨�ϵ����ݣ��ݿ��ϵ�����Ϊ7        
        cJSON_AddStringToObject(dataObj, "qrId",  (const char*)localUserData->userId);
        cJSON_AddNumberToObject(dataObj, "status", ON_LINE);   
        cJSON_AddStringToObject(dataObj, "enterTime",(const char*)bsp_ds1302_readtime());                
        cJSON_AddNumberToObject(dataObj, "faceCompare",CALL_OK);//��1���ɹ� 2ʧ�ܣ�
        cJSON_AddNumberToObject(dataObj, "direction", DIRECTION_IN);//1����2��        
        cJSON_AddStringToObject(dataObj, "cardNo", (const char*)localUserData->cardNo);
    }
    
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        cJSON_Delete(root);
        my_free(tmpBuf);            
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);    

    cJSON_Delete(root);
    my_free(tmpBuf);
    return result;

}


uint8_t parseQrCode(uint8_t *jsonBuff,USERDATA_STRU *qrCodeInfo)
{
    cJSON *root ,*tmpArray;
    uint8_t buf[300] = {0};
    uint8_t isFind = 0;
    
    if(!jsonBuff || !qrCodeInfo)
    {
        cJSON_Delete(root);
        log_d("error json data\r\n");
        return STR_EMPTY_ERR;
    }    
    
    root = cJSON_Parse((char *)jsonBuff);    //�������ݰ�
    if (!root)  
    {  
        cJSON_Delete(root);
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 


    tmpArray = cJSON_GetObjectItem(root, "qS");
    strcpy((char *)qrCodeInfo->startTime,tmpArray->valuestring);
    log_d("qrCodeInfo->startTime= %s\r\n",qrCodeInfo->startTime); 
    
    tmpArray = cJSON_GetObjectItem(root, "qE");
    strcpy((char *)qrCodeInfo->endTime,tmpArray->valuestring);
    log_d("qrCodeInfo->endTime= %s\r\n",qrCodeInfo->endTime); 
    
    tmpArray = cJSON_GetObjectItem(root, "qI");
    strcpy((char *)qrCodeInfo->userId,tmpArray->valuestring);
    log_d("qrCodeInfo->qrID= %s\r\n",qrCodeInfo->userId); 

    tmpArray = cJSON_GetObjectItem(root, "t");
    qrCodeInfo->platformType = tmpArray->valueint;
    log_d("qrCodeInfo->type= %d\r\n",qrCodeInfo->platformType); 
    
    tmpArray = cJSON_GetObjectItem(root, "f1");
    memcpy(qrCodeInfo->accessFloor,parseAccessFloor(tmpArray->valuestring),FLOOR_ARRAY_LENGTH);

//    dbh("qrCodeInfo->accessFloor",(char *)qrCodeInfo->accessFloor,FLOOR_ARRAY_LENGTH);

    qrCodeInfo->defaultFloor = qrCodeInfo->accessFloor[0];
    log_d("qrCodeInfo->defaultFloor = %d\r\n",qrCodeInfo->defaultFloor);
    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d1");
    strcpy((char *)buf,tmpArray->valuestring);  
    if(strlen((const char*)buf) > 0)
    {
        log_d("d1 = %s\r\n",buf); 
        if(findDev(buf,1) == 1)
        {
            isFind =1;
            goto QR_END;
        }
    }

    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d2");
    strcpy((char *)buf,tmpArray->valuestring);    
    if(strlen((const char*)buf) > 0)
    {
        log_d("d2 = %s\r\n",buf); 
        if(findDev(buf,2) == 1)
        {
            isFind =1;
            goto QR_END;
        }
    }
    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d3");
    strcpy((char *)buf,tmpArray->valuestring);    
    if(strlen((const char*)buf) > 0)
    {
        log_d("d3= %s\r\n",buf); 
        if(findDev(buf,3) == 1)
        {
            isFind =1;
            goto QR_END;
        }
    }

    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d4");
    strcpy((char *)buf,tmpArray->valuestring);
    if(strlen((const char*)buf) > 0)
    {
        log_d("d4 = %s\r\n",buf);    
        if(findDev(buf,4) == 1)
        {
            isFind =1;
            goto QR_END;
        }    
    }


    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d5");
    strcpy((char *)buf,tmpArray->valuestring);
    if(strlen((const char*)buf) > 0)
    {
        log_d("d5 = %s\r\n",buf);    
        if(findDev(buf,5) == 1)
        {
           isFind =1;
        }    
    }
  
QR_END:    
    cJSON_Delete(root);
    
    return isFind;

}


//����ģ����Ϣ
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff)
{
    SYSERRORCODE_E result = NO_ERR;
    cJSON* root,*data,*templateData,*templateMap,*holidayTimeMap,*peakTimeMap;   
    cJSON* tmpArray,*arrayElement;

    memset(&gTemplateParam,0x00,sizeof(TEMPLATE_PARAM_STRU));
    TEMPLATE_PARAM_STRU *templateParam = &gTemplateParam; 

    //TEMPLATE_PARAM_STRU *templateParam = my_malloc(sizeof(TEMPLATE_PARAM_STRU)); 
    int holidayTimeMapCnt=0,peakTimeMapCnt=0,index = 0;
    char tmpbuf[8] = {0};

    char tmpIndex[2] = {0};
    char tmpKey[32] = {0};

    

    uint32_t curtick  =  xTaskGetTickCount();
    
    root = cJSON_Parse((char *)jsonBuff);    //�������ݰ�
    if (!root)  
    {  
        cJSON_Delete(root);
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 

    data = cJSON_GetObjectItem(root, "data");
    if(data == NULL)
    {
        log_d("data NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;        
    }    

    templateData = cJSON_GetObjectItem(data, "template");
    if(templateMap == NULL)
    {
        log_d("templateData NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;   
    }    

    templateMap = cJSON_GetObjectItem(templateData, "templateMap");
    if(templateMap == NULL)
    {
        log_d("templateMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;   
    }

    holidayTimeMap = cJSON_GetObjectItem(templateData, "hoildayTimeMap");
    if(holidayTimeMap == NULL)
    {
        log_d("hoildayTimeMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;   
    }

    peakTimeMap = cJSON_GetObjectItem(templateData, "peakTimeMap");
    if(peakTimeMap ==NULL)
    {
        log_d("peakTimeMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;   
    }


//--------------------------------------------------    
    //��ȡtemplateMap����
    cJSON *json_item = cJSON_GetObjectItem(templateMap, "id");
    templateParam->id = json_item->valueint;
    sprintf(tmpbuf,"%8d",templateParam->id);
    ef_set_env_blob("templateID",tmpbuf,8); 
    log_d("templateParam->id = %d\r\n",templateParam->id);

    json_item = cJSON_GetObjectItem(templateMap, "templateCode");
    strcpy((char *)templateParam->templateCode,json_item->valuestring);
    ef_set_env_blob("templateCode",templateParam->templateCode,strlen((const char *)templateParam->templateCode)); 
    log_d("templateParam->templateCode = %s\r\n",templateParam->templateCode);

    json_item = cJSON_GetObjectItem(templateMap, "templateName");
    strcpy((char *)templateParam->templateName,json_item->valuestring);
//    ef_set_env_blob("templateName",templateParam->templateName,strlen(templateParam->templateName)); 
    log_d("templateParam->templateName = %s\r\n",templateParam->templateName);    

    json_item = cJSON_GetObjectItem(templateMap, "templateStatus");
    templateParam->templateStatus = json_item->valueint;
    memset(tmpbuf,0x00,sizeof(tmpbuf));
    sprintf(tmpbuf,"%8d",templateParam->templateStatus);
    ef_set_env_blob("templateStatus",tmpbuf,8); 
    log_d("templateParam->templateStatus = %d\r\n",templateParam->templateStatus);      
    
    json_item = cJSON_GetObjectItem(templateMap, "callingWay");
    strcpy((char *)templateParam->callingWay,json_item->valuestring);
    ef_set_env_blob("T_callingWay",templateParam->callingWay,strlen((const char*)templateParam->callingWay));     
    log_d("templateParam->callingWay = %s\r\n",templateParam->callingWay);    

    json_item = cJSON_GetObjectItem(templateMap, "offlineProcessing");
    templateParam->offlineProcessing = json_item->valueint;
    memset(tmpbuf,0x00,sizeof(tmpbuf));
    sprintf(tmpbuf,"%8d",templateParam->offlineProcessing);
    ef_set_env_blob("offlineStatus",tmpbuf,8);     
    log_d("templateParam->offlineProcessing = %d\r\n",templateParam->offlineProcessing);     


    json_item = cJSON_GetObjectItem(templateMap, "modeType");
    strcpy((char *)templateParam->modeType,json_item->valuestring);
    ef_set_env_blob("modeType",templateParam->modeType,strlen((const char*)templateParam->modeType));     
    log_d("templateParam->modeType = %s\r\n",templateParam->modeType);

    json_item = cJSON_GetObjectItem(templateMap, "peakCallingWay");
    strcpy((char *)templateParam->peakInfo[0].callingWay,json_item->valuestring);
    ef_set_env_blob("peakCallingWay",templateParam->peakInfo[0].callingWay,strlen((const char *)templateParam->peakInfo[0].callingWay));
    log_d("templateParam->peakInfo[0].callingWay = %s\r\n",templateParam->peakInfo[0].callingWay);

    json_item = cJSON_GetObjectItem(templateMap, "peakStartDate");
    strcpy((char *)templateParam->peakInfo[0].beginTime,json_item->valuestring);
    ef_set_env_blob("peakStartDate",templateParam->peakInfo[0].beginTime,strlen((const char*)templateParam->peakInfo[0].beginTime));
    log_d("templateParam->peakInfo[0].beginTime = %s\r\n",templateParam->peakInfo[0].beginTime);

    json_item = cJSON_GetObjectItem(templateMap, "peakEndDate");
    strcpy((char *)templateParam->peakInfo[0].endTime,json_item->valuestring);
    ef_set_env_blob("peakEndDate",templateParam->peakInfo[0].endTime,strlen((const char*)templateParam->peakInfo[0].endTime));
    log_d("templateParam->peakInfo[0].endTime = %s\r\n",templateParam->peakInfo[0].endTime);

    json_item = cJSON_GetObjectItem(templateMap, "peakHolidaysType");
    strcpy((char *)templateParam->peakInfo[0].outsideTimeMode,json_item->valuestring);
    ef_set_env_blob("peakHolidaysType",templateParam->peakInfo[0].outsideTimeMode,strlen((const char*)templateParam->peakInfo[0].outsideTimeMode));
    log_d("templateParam->peakInfo[0].outsideTimeMode = %s\r\n",templateParam->peakInfo[0].outsideTimeMode);

    json_item = cJSON_GetObjectItem(templateMap, "peakHolidays");
    strcpy((char *)templateParam->peakInfo[0].outsideTimeData,json_item->valuestring);
    ef_set_env_blob("peakHolidays",templateParam->peakInfo[0].outsideTimeData,strlen((const char*)templateParam->peakInfo[0].outsideTimeData));
    log_d("templateParam->peakInfo[0].outsideTimeData = %s\r\n",templateParam->peakInfo[0].outsideTimeData);    
//------------------------------------------------------------------------------
    json_item = cJSON_GetObjectItem(templateMap, "holidayCallingWay");
    strcpy((char *)templateParam->hoildayInfo[0].callingWay,json_item->valuestring);
    ef_set_env_blob("holidayCallingWay",templateParam->hoildayInfo[0].callingWay,strlen((const char*)templateParam->hoildayInfo[0].callingWay));
    log_d("templateParam->hoildayInfo[0].callingWay = %s\r\n",templateParam->hoildayInfo[0].callingWay);

    json_item = cJSON_GetObjectItem(templateMap, "holidayStartDate");
    strcpy((char *)templateParam->hoildayInfo[0].beginTime,json_item->valuestring);
    ef_set_env_blob("holidayStartDate",templateParam->hoildayInfo[0].beginTime,strlen((const char*)templateParam->hoildayInfo[0].beginTime));
    log_d("templateParam->hoildayInfo[0].beginTime = %s\r\n",templateParam->hoildayInfo[0].beginTime);

    json_item = cJSON_GetObjectItem(templateMap, "holidayEndDate");
    strcpy((char *)templateParam->hoildayInfo[0].endTime,json_item->valuestring);
    ef_set_env_blob("holidayEndDate",templateParam->hoildayInfo[0].endTime,strlen((const char*)templateParam->hoildayInfo[0].endTime));
    log_d("templateParam->hoildayInfo[0].endTime = %s\r\n",templateParam->hoildayInfo[0].endTime);

    json_item = cJSON_GetObjectItem(templateMap, "holidayHolidaysType");
    strcpy((char *)templateParam->hoildayInfo[0].outsideTimeMode,json_item->valuestring);
    ef_set_env_blob("holidayHolidaysType",templateParam->hoildayInfo[0].outsideTimeMode,strlen((const char*)templateParam->hoildayInfo[0].outsideTimeMode));
    log_d("templateParam->hoildayInfo[0].outsideTimeMode = %s\r\n",templateParam->hoildayInfo[0].outsideTimeMode);

    json_item = cJSON_GetObjectItem(templateMap, "holidayHolidays");
    strcpy((char *)templateParam->hoildayInfo[0].outsideTimeData,json_item->valuestring);
    ef_set_env_blob("holidayHolidays",templateParam->hoildayInfo[0].outsideTimeData,strlen((const char*)templateParam->hoildayInfo[0].outsideTimeData));
    log_d("templateParam->hoildayInfo[0].outsideTimeData = %s\r\n",templateParam->hoildayInfo[0].outsideTimeData);       

//--------------------------------------------------
    //�洢hoildayTimeMap������
    holidayTimeMapCnt = cJSON_GetArraySize(holidayTimeMap); /*��ȡ���鳤��*/
    log_d("array len = %d\r\n",holidayTimeMapCnt);

    //�洢���ܿ�ʱ��εĸ���
    if(holidayTimeMapCnt > 0)
    {
        sprintf(tmpKey,"%04d",holidayTimeMapCnt);
        ef_set_env_blob("holidayTimeMapCnt",tmpKey,4);        
    }

    for(index=0; index<holidayTimeMapCnt; index++)
    {
        tmpArray = cJSON_GetArrayItem(holidayTimeMap, index);

        memset(tmpKey,0x00,sizeof(tmpKey));
        memset(tmpIndex,0x00,sizeof(tmpIndex));       
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "templateType");
        templateParam->holidayMode[index].templateType = arrayElement->valueint;        
        log_d("templateType = %d\r\n",templateParam->holidayMode[index].templateType);

        arrayElement = cJSON_GetObjectItem(tmpArray, "voiceSize");
        templateParam->holidayMode[index].voiceSize = arrayElement->valueint;        
        log_d("voiceSize = %d\r\n",templateParam->holidayMode[index].voiceSize);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "modeType");
        templateParam->holidayMode[index].channelType = arrayElement->valueint;
        log_d("modeType = %d\r\n",templateParam->holidayMode[index].channelType);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "startTime");
        //��Ϊ�ڼ��ո��߷干�ã�����ֻ��¼��FLASHһ�־Ϳ�����
        strcpy((char*)templateParam->holidayMode[index].startTime,arrayElement->valuestring);
        sprintf(tmpIndex,"%d",index);
        strcpy(tmpKey,"hoildayModeStartTime");
        strcat(tmpKey,tmpIndex); 
        ef_set_env_blob(tmpKey,templateParam->holidayMode[index].startTime,strlen((const char*)templateParam->holidayMode[index].startTime));        
        log_d("%s = %s\r\n",tmpKey,templateParam->holidayMode[index].startTime);
        
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "endTime");
        strcpy((char*)templateParam->holidayMode[index].endTime,arrayElement->valuestring);  
        memset(tmpKey,0x00,sizeof(tmpKey));
        strcpy(tmpKey,"hoildayModeEndTime");
        strcat(tmpKey,tmpIndex);      
        ef_set_env_blob(tmpKey,templateParam->holidayMode[index].endTime,strlen((const char*)templateParam->holidayMode[index].endTime));                
        log_d("%s= %s\r\n",tmpKey,templateParam->holidayMode[index].endTime);        
    }
    
    log_d("=====================================================\r\n");
//--------------------------------------------------
    peakTimeMapCnt = cJSON_GetArraySize(peakTimeMap); /*��ȡ���鳤��*/
    log_d("peakTimeMapCnt len = %d\r\n",peakTimeMapCnt);

    for(index=0; index<peakTimeMapCnt; index++)
    {
        tmpArray = cJSON_GetArrayItem(peakTimeMap, index);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "templateType");
        log_d("templateType = %d\r\n",arrayElement->valueint);

        arrayElement = cJSON_GetObjectItem(tmpArray, "voiceSize");
        log_d("voiceSize = %d\r\n",arrayElement->valueint);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "modeType");
        log_d("modeType = %d\r\n",arrayElement->valueint);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "startTime");
        log_d("startTime = %s\r\n",arrayElement->valuestring);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "endTime");
        log_d("endTime = %s\r\n",arrayElement->valuestring);        
    }
    
 
    cJSON_Delete(root);

    log_d("saveTemplateParam took %d ms to save\r\n",xTaskGetTickCount()-curtick);

    return result;
}





