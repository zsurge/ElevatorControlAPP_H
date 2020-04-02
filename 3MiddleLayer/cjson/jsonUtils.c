/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : jsonUtils.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��12��19��
  ����޸�   :
  ��������   : JSON���ݴ���C�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��12��19��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "jsonUtils.h"
#include "version.h"
#include "bsp_rtc.h"
#include "eth_cfg.h"
#include "LocalData.h"
#include "templateprocess.h"
#include "malloc.h"



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/



/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
//LOCAL_USER_STRU gLoalUserData;
TEMPLATE_PARAM_STRU gTemplateParam;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : modifyJsonItem
 ��������  : �޸Ļ�������JSON���ݰ���ָ��item��ֵ
 �������  : const char *srcJson   json���ݰ�
             const char *item    ��Ҫ����ֵ��key 
             const char *value   ��Ҫ���µ�value 
             uint8_t isSubitem   =1 �� data������ 
             char *descJson      ���º��json���ݰ�
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��12��20��
    ��    ��   : �Ŷ�
SYSERRORCODE_E modifyJsonItem(const char *srcJson,const char *item,const char *value,uint8_t isSubitem,char *descJson);
    �޸�����   : �����ɺ���

*****************************************************************************/
SYSERRORCODE_E modifyJsonItem(const uint8_t *srcJson,const uint8_t *item,const uint8_t *value,uint8_t isSubitem,uint8_t *descJson)
{
    cJSON *root ,*dataObj;
    char *tmpBuf;

    if(!srcJson)
    {
        cJSON_Delete(root);
        log_d("error json data\r\n");
        return STR_EMPTY_ERR;
    }    
    
    root = cJSON_Parse((char *)srcJson);    //�������ݰ�
    if (!root)  
    {  
        cJSON_Delete(root);
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 

    if(isSubitem == 1)
    {
        //����Э�飬Ĭ�����е�������data
        dataObj = cJSON_GetObjectItem ( root, "data" );         
        cJSON_AddStringToObject(dataObj,item,value);
    }
    else
    {
        cJSON_AddStringToObject(root,item,value);
    }  

    
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        cJSON_Delete(root);
        log_d("cJSON_PrintUnformatted error \r\n");
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);


    log_d("send json data = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);
    
    return NO_ERR;

}



/*****************************************************************************
 �� �� ��  : GetJsonItem
 ��������  : ��ȡJSON�ַ�����ָ����Ŀ��ֵ
 �������  : const char *jsonBuff  json�ַ���
           const char *item      Ҫ������KEY
           uint8_t isSubitem     �Ƿ��DATA�ڵ���Ŀ��=1 ��data����Ŀ��=0 ��root����Ŀ
 �������  : ��
 �� �� ֵ  : char *

 �޸���ʷ      :
  1.��    ��   : 2019��12��20��
    ��    ��   : �Ŷ�
 char *GetJsonItem(const char *jsonBuff,const char *item,uint8_t isSubitem)
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t* GetJsonItem ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t isSubitem)
{
	static uint8_t value[JSON_ITEM_MAX_LEN] = {0};
	cJSON* root,*json_item,*dataObj;
	root = cJSON_Parse ( ( char* ) jsonBuff );    //�������ݰ�

	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
		return NULL;
	}
	else
	{
        if(isSubitem == 1)
        {
            //����Э�飬Ĭ�����е�������data
            dataObj = cJSON_GetObjectItem ( root, "data" );  
            json_item = cJSON_GetObjectItem ( dataObj, item );
        }
        else
        {
            json_item = cJSON_GetObjectItem ( root, item );
        }  
		
		if ( json_item->type == cJSON_String )
		{
			//�������
			if ( strlen ( json_item->valuestring ) > JSON_ITEM_MAX_LEN )
			{
				memcpy ( value, json_item->valuestring,JSON_ITEM_MAX_LEN );
			}

			strcpy ( value, json_item->valuestring );
			log_d ( "json_item =  %s\r\n",json_item->valuestring );
		}
		else if ( json_item->type == cJSON_Number )
		{
			sprintf ( value,"%d",json_item->valueint );
			log_d ( "json_item =  %s\r\n",value);
		}
		else
		{
			log_d ( "can't parse json buff\r\n" );
            cJSON_Delete(root);
			return NULL;
		}

	}

    cJSON_Delete(root);
	return value;
}

SYSERRORCODE_E PacketDeviceInfo ( const uint8_t* jsonBuff,const uint8_t* descJson)
{
	SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*newroot,*json_item,*dataObj,*json_cmdid,*json_devcode,*identification;
    char *tmpBuf;
    char buf[8] = {0};
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //�������ݰ�
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        cJSON_Delete(newroot);
        my_free(tmpBuf);            
		return CJSON_PARSE_ERR;
	}
	else
	{
        json_cmdid = cJSON_GetObjectItem ( root, "commandCode" );
        json_devcode = cJSON_GetObjectItem ( root, "deviceCode" );
        identification = cJSON_GetObjectItem ( root, "data" );

        newroot = cJSON_CreateObject();
        dataObj = cJSON_CreateObject();
        if(!newroot && !dataObj)
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
            my_free(tmpBuf);            
    		return CJSON_CREATE_ERR;
        }

        cJSON_AddStringToObject(newroot, "commandCode", json_cmdid->valuestring);
        cJSON_AddStringToObject(newroot, "deviceCode", json_devcode->valuestring);
        cJSON_AddStringToObject(newroot, "identification", identification->valuestring);

        cJSON_AddItemToObject(newroot, "data", dataObj);

        cJSON_AddStringToObject(dataObj, "version", gDevinfo.SoftwareVersion);
        cJSON_AddStringToObject(dataObj, "appName", gDevinfo.Model);

        memset(buf,0x00,sizeof(buf));
        sprintf(buf,"%d",gCurCardHeaderIndex);
        cJSON_AddStringToObject(dataObj, "regRersion", buf);
        cJSON_AddStringToObject(dataObj, "regface", " ");
        cJSON_AddStringToObject(dataObj, "ip", gDevinfo.GetIP());
                
        tmpBuf = cJSON_PrintUnformatted(newroot); 

        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");
            cJSON_Delete(root);
            cJSON_Delete(newroot);         
            my_free(tmpBuf);            
            return CJSON_FORMAT_ERR;
        }    

        strcpy((char *)descJson,tmpBuf);

	}

    cJSON_Delete(root);
    cJSON_Delete(newroot);

    my_free(tmpBuf);

    return result;
}

SYSERRORCODE_E upgradeDataPacket(uint8_t *descBuf)
{
    SYSERRORCODE_E result = NO_ERR;
    cJSON *root ,*dataObj;
    char *up_status;    
    char *tmpBuf;
    
    root = cJSON_Parse((const char*)ef_get_env("upData"));    //�������ݰ�
    if (!root)  
    {          
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        cJSON_Delete(root);
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    } 
    
     strcpy(up_status,(const char*)ef_get_env("up_status"));

     cJSON_AddStringToObject(root,"commandCode","1017");

    //��ȡdataobject
    dataObj = cJSON_GetObjectItem( root , "data" );   
    
    //����ʧ��
    if(memcmp(up_status,"101700",6) == 0)
    {
        cJSON_AddStringToObject(dataObj,"status","2");
    }
    else if(memcmp(up_status,"101711",6) == 0) //�����ɹ�
    {
        cJSON_AddStringToObject(dataObj,"status","1");
    }
    else if(memcmp(up_status,"101722",6) == 0) //�����ɹ�
    {
        cJSON_AddStringToObject(dataObj,"status","1"); 
    }
    else if(memcmp(up_status,"101733",6) == 0) //��ֹ����
    {
        cJSON_AddStringToObject(dataObj,"status","3");
    }
    else
    {
        //����������
    }

    tmpBuf = cJSON_PrintUnformatted(root); 

    if(tmpBuf == NULL)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        my_free(tmpBuf);
        cJSON_Delete(root);        
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descBuf,tmpBuf);


    log_d("send json data = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);

    return result;
}

SYSERRORCODE_E getTimePacket(uint8_t *descBuf)
{
    SYSERRORCODE_E result = NO_ERR;
    cJSON *root;  
    char *tmpBuf;
    
    root = cJSON_CreateObject();
    if (!root)  
    {          
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        cJSON_Delete(root);
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    } 
    
    cJSON_AddStringToObject(root,"commandCode","3013");
    cJSON_AddStringToObject(root,"deviceCode",gMqttDevSn.sn);
     
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(tmpBuf == NULL)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        my_free(tmpBuf);
        cJSON_Delete(root);        
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descBuf,tmpBuf);


    log_d("getTimePacket = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);

    return result;
}



uint8_t* packetBaseJson(uint8_t *jsonBuff)
{
    static uint8_t value[256] = {0};
    
	cJSON* root,*newroot,*tmpdataObj,*json_item,*dataObj,*json_cmdCode,*json_devCode,*identification,*id;
    char *tmpBuf;
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //�������ݰ�
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        
        cJSON_Delete(root);        
        cJSON_Delete(newroot);
        my_free(tmpBuf);
		return NULL;
	}
	else
	{
        json_cmdCode = cJSON_GetObjectItem ( root, "commandCode" );
        json_devCode = cJSON_GetObjectItem ( root, "deviceCode" );
        
        tmpdataObj = cJSON_GetObjectItem ( root, "data" );        
        identification = cJSON_GetObjectItem ( tmpdataObj, "identification" );
        id = cJSON_GetObjectItem ( tmpdataObj, "id" );

        newroot = cJSON_CreateObject();
        dataObj = cJSON_CreateObject();
        if(!newroot && !dataObj)
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
            my_free(tmpBuf);            
    		return NULL;
        }

        if(json_cmdCode)
            cJSON_AddStringToObject(newroot, "commandCode", json_cmdCode->valuestring);

        if(json_devCode)
            cJSON_AddStringToObject(newroot, "deviceCode", json_devCode->valuestring);


        cJSON_AddItemToObject(newroot, "data", dataObj);
        
        if(identification)
            cJSON_AddStringToObject(dataObj, "identification", identification->valuestring);

        if(id)
            cJSON_AddNumberToObject(dataObj, "id", id->valueint);

        cJSON_AddStringToObject(dataObj, "status", "1");

                
        tmpBuf = cJSON_PrintUnformatted(newroot); 

        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");

            cJSON_Delete(root);
            cJSON_Delete(newroot);      
            my_free(tmpBuf);
            return NULL;
        }    

        strcpy((char *)value,tmpBuf);

	}

    cJSON_Delete(root);

    cJSON_Delete(newroot);

    my_free(tmpBuf);


    return value;    
}


uint8_t packetPayload(LOCAL_USER_STRU *localUserData,uint8_t *descJson)
{ 
    
    SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*dataObj;
    char *tmpBuf;


    root = cJSON_CreateObject();
    dataObj = cJSON_CreateObject();

    if(!root && !dataObj)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        my_free(tmpBuf);            
		return CJSON_CREATE_ERR;
    }

    cJSON_AddStringToObject(root, "deviceCode", gMqttDevSn.sn);
    cJSON_AddItemToObject(root, "data", dataObj);

    if(localUserData->qrType == 4)
    {    
        cJSON_AddStringToObject(root, "commandCode","3007");
        cJSON_AddStringToObject(dataObj, "userId", localUserData->qrID);  
        cJSON_AddStringToObject(dataObj, "cardNo", localUserData->cardNo);
        cJSON_AddNumberToObject(dataObj, "callType", localUserData->authMode); 
        cJSON_AddNumberToObject(dataObj, "status", ON_LINE);   
        cJSON_AddNumberToObject(dataObj, "type",CALL_OK);
        cJSON_AddStringToObject(dataObj, "callElevatorTime",(const char*)bsp_ds1302_readtime());        
        cJSON_AddStringToObject(dataObj, "timeStamp",localUserData->timeStamp);
    }
    else
    {
        cJSON_AddStringToObject(root, "commandCode","4002");
        cJSON_AddNumberToObject(dataObj, "enterType",  4);//��������Ϊ4��QRCODE�ڿ�����ƽ̨�ϵ����ݣ��ݿ��ϵ�����Ϊ7        
        cJSON_AddStringToObject(dataObj, "qrId",  localUserData->qrID);
        cJSON_AddNumberToObject(dataObj, "status", ON_LINE);   
        cJSON_AddStringToObject(dataObj, "enterTime",(const char*)bsp_ds1302_readtime());                
        cJSON_AddNumberToObject(dataObj, "faceCompare",CALL_OK);//��1���ɹ� 2ʧ�ܣ�
        cJSON_AddNumberToObject(dataObj, "direction", DIRECTION_IN);//1����2��        
        cJSON_AddStringToObject(dataObj, "cardNo", localUserData->cardNo);
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


//����ģ����Ϣ
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff)
{
    SYSERRORCODE_E result = NO_ERR;
    cJSON* root,*data,*templateData,*templateMap,*holidayTimeMap,*peakTimeMap;   
    cJSON* tmpArray,*arrayElement;

     memset(&gTemplateParam,0x00,sizeof(gTemplateParam));
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
        goto ERROR;
        
    }    

    templateData = cJSON_GetObjectItem(data, "template");
    if(templateMap == NULL)
    {
        log_d("templateData NULL\r\n");
        result = CJSON_GETITEM_ERR;
        goto ERROR;

    }    

    templateMap = cJSON_GetObjectItem(templateData, "templateMap");
    if(templateMap == NULL)
    {
        log_d("templateMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        goto ERROR;
    }

    holidayTimeMap = cJSON_GetObjectItem(templateData, "hoildayTimeMap");
    if(holidayTimeMap == NULL)
    {
        log_d("hoildayTimeMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        goto ERROR;
    }

    peakTimeMap = cJSON_GetObjectItem(templateData, "peakTimeMap");
    if(peakTimeMap ==NULL)
    {
        log_d("peakTimeMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        goto ERROR;
    }


//--------------------------------------------------    
    //��ȡtemplateMap����
    cJSON *json_item = cJSON_GetObjectItem(templateMap, "id");
    templateParam->id = json_item->valueint;
    sprintf(tmpbuf,"%8d",templateParam->id);
    ef_set_env_blob("templateID",tmpbuf,8); 
    log_d("templateParam->id = %d\r\n",templateParam->id);

    json_item = cJSON_GetObjectItem(templateMap, "templateCode");
    strcpy(templateParam->templateCode,json_item->valuestring);
    ef_set_env_blob("templateCode",templateParam->templateCode,strlen(templateParam->templateCode)); 
    log_d("templateParam->templateCode = %s\r\n",templateParam->templateCode);

    json_item = cJSON_GetObjectItem(templateMap, "templateName");
    strcpy(templateParam->templateName,json_item->valuestring);
//    ef_set_env_blob("templateName",templateParam->templateName,strlen(templateParam->templateName)); 
    log_d("templateParam->templateName = %s\r\n",templateParam->templateName);    

    json_item = cJSON_GetObjectItem(templateMap, "templateStatus");
    templateParam->templateStatus = json_item->valueint;
    memset(tmpbuf,0x00,sizeof(tmpbuf));
    sprintf(tmpbuf,"%8d",templateParam->templateStatus);
    ef_set_env_blob("templateStatus",tmpbuf,8); 
    log_d("templateParam->templateStatus = %d\r\n",templateParam->templateStatus);      
    
    json_item = cJSON_GetObjectItem(templateMap, "callingWay");
    strcpy(templateParam->callingWay,json_item->valuestring);
    ef_set_env_blob("T_callingWay",templateParam->callingWay,strlen(templateParam->callingWay));     
    log_d("templateParam->callingWay = %s\r\n",templateParam->callingWay);    

    json_item = cJSON_GetObjectItem(templateMap, "offlineProcessing");
    templateParam->offlineProcessing = json_item->valueint;
    memset(tmpbuf,0x00,sizeof(tmpbuf));
    sprintf(tmpbuf,"%8d",templateParam->offlineProcessing);
    ef_set_env_blob("templateStatus",tmpbuf,8);     
    log_d("templateParam->offlineProcessing = %d\r\n",templateParam->offlineProcessing);     


    json_item = cJSON_GetObjectItem(templateMap, "modeType");
    strcpy(templateParam->modeType,json_item->valuestring);
    ef_set_env_blob("modeType",templateParam->modeType,strlen(templateParam->modeType));     
    log_d("templateParam->modeType = %s\r\n",templateParam->modeType);

    json_item = cJSON_GetObjectItem(templateMap, "peakCallingWay");
    strcpy(templateParam->peakInfo[0].callingWay,json_item->valuestring);
    ef_set_env_blob("peakCallingWay",templateParam->peakInfo[0].callingWay,strlen(templateParam->peakInfo[0].callingWay));
    log_d("templateParam->peakInfo[0].callingWay = %s\r\n",templateParam->peakInfo[0].callingWay);

    json_item = cJSON_GetObjectItem(templateMap, "peakStartDate");
    strcpy(templateParam->peakInfo[0].beginTime,json_item->valuestring);
    ef_set_env_blob("peakStartDate",templateParam->peakInfo[0].beginTime,strlen(templateParam->peakInfo[0].beginTime));
    log_d("templateParam->peakInfo[0].beginTime = %s\r\n",templateParam->peakInfo[0].beginTime);

    json_item = cJSON_GetObjectItem(templateMap, "peakEndDate");
    strcpy(templateParam->peakInfo[0].endTime,json_item->valuestring);
    ef_set_env_blob("peakEndDate",templateParam->peakInfo[0].endTime,strlen(templateParam->peakInfo[0].endTime));
    log_d("templateParam->peakInfo[0].endTime = %s\r\n",templateParam->peakInfo[0].endTime);

    json_item = cJSON_GetObjectItem(templateMap, "peakHolidaysType");
    strcpy(templateParam->peakInfo[0].outsideTimeMode,json_item->valuestring);
    ef_set_env_blob("peakHolidaysType",templateParam->peakInfo[0].outsideTimeMode,strlen(templateParam->peakInfo[0].outsideTimeMode));
    log_d("templateParam->peakInfo[0].outsideTimeMode = %s\r\n",templateParam->peakInfo[0].outsideTimeMode);

    json_item = cJSON_GetObjectItem(templateMap, "peakHolidays");
    strcpy(templateParam->peakInfo[0].outsideTimeData,json_item->valuestring);
    ef_set_env_blob("peakHolidays",templateParam->peakInfo[0].outsideTimeData,strlen(templateParam->peakInfo[0].outsideTimeData));
    log_d("templateParam->peakInfo[0].outsideTimeData = %s\r\n",templateParam->peakInfo[0].outsideTimeData);    
//------------------------------------------------------------------------------
    json_item = cJSON_GetObjectItem(templateMap, "holidayCallingWay");
    strcpy(templateParam->hoildayInfo[0].callingWay,json_item->valuestring);
    ef_set_env_blob("holidayCallingWay",templateParam->hoildayInfo[0].callingWay,strlen(templateParam->hoildayInfo[0].callingWay));
    log_d("templateParam->hoildayInfo[0].callingWay = %s\r\n",templateParam->hoildayInfo[0].callingWay);

    json_item = cJSON_GetObjectItem(templateMap, "holidayStartDate");
    strcpy(templateParam->hoildayInfo[0].beginTime,json_item->valuestring);
    ef_set_env_blob("holidayStartDate",templateParam->hoildayInfo[0].beginTime,strlen(templateParam->hoildayInfo[0].beginTime));
    log_d("templateParam->hoildayInfo[0].beginTime = %s\r\n",templateParam->hoildayInfo[0].beginTime);

    json_item = cJSON_GetObjectItem(templateMap, "holidayEndDate");
    strcpy(templateParam->hoildayInfo[0].endTime,json_item->valuestring);
    ef_set_env_blob("holidayEndDate",templateParam->hoildayInfo[0].endTime,strlen(templateParam->hoildayInfo[0].endTime));
    log_d("templateParam->hoildayInfo[0].endTime = %s\r\n",templateParam->hoildayInfo[0].endTime);

    json_item = cJSON_GetObjectItem(templateMap, "holidayHolidaysType");
    strcpy(templateParam->hoildayInfo[0].outsideTimeMode,json_item->valuestring);
    ef_set_env_blob("holidayHolidaysType",templateParam->hoildayInfo[0].outsideTimeMode,strlen(templateParam->hoildayInfo[0].outsideTimeMode));
    log_d("templateParam->hoildayInfo[0].outsideTimeMode = %s\r\n",templateParam->hoildayInfo[0].outsideTimeMode);

    json_item = cJSON_GetObjectItem(templateMap, "holidayHolidays");
    strcpy(templateParam->hoildayInfo[0].outsideTimeData,json_item->valuestring);
    ef_set_env_blob("holidayHolidays",templateParam->hoildayInfo[0].outsideTimeData,strlen(templateParam->hoildayInfo[0].outsideTimeData));
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
        strcpy(templateParam->holidayMode[index].startTime,arrayElement->valuestring);
        sprintf(tmpIndex,"%d",index);
        strcpy(tmpKey,"hoildayModeStartTime");
        strcat(tmpKey,tmpIndex); 
        ef_set_env_blob(tmpKey,templateParam->holidayMode[index].startTime,strlen(templateParam->holidayMode[index].startTime));        
        log_d("%s = %s\r\n",tmpKey,templateParam->holidayMode[index].startTime);
        
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "endTime");
        strcpy(templateParam->holidayMode[index].endTime,arrayElement->valuestring);  
        memset(tmpKey,0x00,sizeof(tmpKey));
        strcpy(tmpKey,"hoildayModeEndTime");
        strcat(tmpKey,tmpIndex);      
        ef_set_env_blob(tmpKey,templateParam->holidayMode[index].endTime,strlen(templateParam->holidayMode[index].endTime));                
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
    
ERROR:  
    cJSON_Delete(root);


    log_d("saveTemplateParam took %d ms to save\r\n",xTaskGetTickCount()-curtick);

    return result;
}



uint8_t parseQrCode(uint8_t *jsonBuff,QRCODE_INFO_STRU *qrCodeInfo)
{
    cJSON *root ,*devArray,*tagFloorArray,*tmpArray;
    int devNum = 0;
    int tagFloorNum = 0;
    int index = 0;
    uint8_t isFind = 0;
    int localSn = 11111111;

    
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

    devArray = cJSON_GetObjectItem(root, "d");
    if(devArray == NULL)
    {
        log_d("devArray NULL\r\n");
        cJSON_Delete(root);
        return STR_EMPTY_ERR;
    }   

    devNum = cJSON_GetArraySize(devArray);

    log_d("devNum = %d\r\n",devNum);

    //�����Ƿ��ڷ�Χ֮��
    for(index=0;index<devNum;index++)
    {
        tmpArray = cJSON_GetArrayItem(devArray, index);
        log_d("tmpArray->valueint = %d\r\n",tmpArray->valueint);
        if(localSn == tmpArray->valueint)
        {
            isFind = 1;
        }
    }
    
    tagFloorArray = cJSON_GetObjectItem(root, "l");
    if(tagFloorArray == NULL)
    {
        log_d("tagFloorArray NULL\r\n");
        cJSON_Delete(root);
        return STR_EMPTY_ERR;
    }

    tagFloorNum = cJSON_GetArraySize(tagFloorArray);
    log_d("tagFloorNum = %d\r\n",tagFloorNum);

    //ֻ�ǰ�Ȩ��¥���ӡ������û�д洢
    for(index=0;index<tagFloorNum;index++)
    {
        tmpArray = cJSON_GetArrayItem(tagFloorArray, index);
        log_d("tmpArray->valueint = %d\r\n",tmpArray->valueint);
    }
        

    tmpArray = cJSON_GetObjectItem(root, "tF");
    qrCodeInfo->tagFloor=tmpArray->valueint;
    log_d("qrCodeInfo->tagFloor = %d\r\n",qrCodeInfo->tagFloor);  

    
    tmpArray = cJSON_GetObjectItem(root, "oN");
    qrCodeInfo->openNum = tmpArray->valueint;
    log_d("qrCodeInfo->openNum= %d\r\n",qrCodeInfo->openNum); 
    
    tmpArray = cJSON_GetObjectItem(root, "t");
    qrCodeInfo->type=tmpArray->valueint;
    log_d("qrCodeInfo->type= %d\r\n",qrCodeInfo->type); 
    
    tmpArray = cJSON_GetObjectItem(root, "sT");
    strcpy(qrCodeInfo->startTime,tmpArray->valuestring);
    log_d("qrCodeInfo->startTime= %s\r\n",qrCodeInfo->startTime); 

    tmpArray = cJSON_GetObjectItem(root, "eT");
    strcpy(qrCodeInfo->endTime,tmpArray->valuestring);
    log_d("qrCodeInfo->endTime= %s\r\n",qrCodeInfo->endTime); 

    tmpArray = cJSON_GetObjectItem(root, "qS");
    strcpy(qrCodeInfo->qrStarttimeStamp,tmpArray->valuestring);
    log_d("qrCodeInfo->qrStarttimeStamp= %s\r\n",qrCodeInfo->qrStarttimeStamp); 
    
    tmpArray = cJSON_GetObjectItem(root, "qE");
    strcpy(qrCodeInfo->qrEndtimeStamp,tmpArray->valuestring);
    log_d("qrCodeInfo->qrEndtimeStamp= %s\r\n",qrCodeInfo->qrEndtimeStamp); 
    
    tmpArray = cJSON_GetObjectItem(root, "qI");
    strcpy(qrCodeInfo->qrID,tmpArray->valuestring);
    log_d("qrCodeInfo->qrID= %s\r\n",qrCodeInfo->qrID); 
    
    cJSON_Delete(root);
    
    return isFind;

}



