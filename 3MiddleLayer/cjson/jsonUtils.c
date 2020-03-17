/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : jsonUtils.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月19日
  最近修改   :
  功能描述   : JSON数据处理C文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月19日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "jsonUtils.h"
#include "version.h"
#include "bsp_rtc.h"
#include "eth_cfg.h"
#include "templateprocess.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/



/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
LOCAL_USER_STRU gLoalUserData;
TEMPLATE_PARAM_STRU gTemplateParam;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : modifyJsonItem
 功能描述  : 修改或者新增JSON数据包中指令item的值
 输入参数  : const char *srcJson   json数据包
             const char *item    需要更新值的key 
             const char *value   需要更新的value 
             uint8_t isSubitem   =1 是 data内数据 
             char *descJson      更新后的json数据包
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年12月20日
    作    者   : 张舵
SYSERRORCODE_E modifyJsonItem(const char *srcJson,const char *item,const char *value,uint8_t isSubitem,char *descJson);
    修改内容   : 新生成函数

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
    
    root = cJSON_Parse((char *)srcJson);    //解析数据包
    if (!root)  
    {  
        cJSON_Delete(root);
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 

    if(isSubitem == 1)
    {
        //根据协议，默认所有的子项是data
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
 函 数 名  : GetJsonItem
 功能描述  : 获取JSON字符串中指定项目的值
 输入参数  : const char *jsonBuff  json字符串
           const char *item      要读到的KEY
           uint8_t isSubitem     是否读DATA内的项目，=1 读data内项目；=0 读root下项目
 输出参数  : 无
 返 回 值  : char *

 修改历史      :
  1.日    期   : 2019年12月20日
    作    者   : 张舵
 char *GetJsonItem(const char *jsonBuff,const char *item,uint8_t isSubitem)
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t* GetJsonItem ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t isSubitem)
{
	static uint8_t value[JSON_ITEM_MAX_LEN] = {0};
	cJSON* root,*json_item,*dataObj;
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包

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
            //根据协议，默认所有的子项是data
            dataObj = cJSON_GetObjectItem ( root, "data" );  
            json_item = cJSON_GetObjectItem ( dataObj, item );
        }
        else
        {
            json_item = cJSON_GetObjectItem ( root, item );
        }  
		
		if ( json_item->type == cJSON_String )
		{
			//避免溢出
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
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
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
        cJSON_AddStringToObject(dataObj, "regRersion", "8");
        cJSON_AddStringToObject(dataObj, "regface", "7");
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
    
    root = cJSON_Parse((const char*)ef_get_env("upData"));    //解析数据包
    if (!root)  
    {          
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        cJSON_Delete(root);
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    } 
    
     strcpy(up_status,(const char*)ef_get_env("up_status"));

     cJSON_AddStringToObject(root,"commandCode","1017");

    //获取dataobject
    dataObj = cJSON_GetObjectItem( root , "data" );   
    
    //升级失败
    if(memcmp(up_status,"101700",6) == 0)
    {
        cJSON_AddStringToObject(dataObj,"status","2");
    }
    else if(memcmp(up_status,"101711",6) == 0) //升级成功
    {
        cJSON_AddStringToObject(dataObj,"status","1");
    }
    else if(memcmp(up_status,"101722",6) == 0) //升级成功
    {
        cJSON_AddStringToObject(dataObj,"status","1"); 
    }
    else if(memcmp(up_status,"101733",6) == 0) //禁止升级
    {
        cJSON_AddStringToObject(dataObj,"status","3");
    }
    else
    {
        //无升级动作
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


uint8_t* packetBaseJson(uint8_t *jsonBuff)
{
    static uint8_t value[JSON_ITEM_MAX_LEN] = {0};
    
	cJSON* root,*newroot,*tmpdataObj,*json_item,*dataObj,*json_cmdCode,*json_devCode,*identification,*id;
    char *tmpBuf;
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
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
//    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
//    log_d("localUserData->userId = %s\r\n",localUserData->userId);
//    log_d("localUserData->accessLayer = %s\r\n",localUserData->accessLayer);
//    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultLayer);    
//    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
//    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
//    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);

    root = cJSON_CreateObject();
    dataObj = cJSON_CreateObject();

    if(!root && !dataObj)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        my_free(tmpBuf);            
		return CJSON_CREATE_ERR;
    }

    cJSON_AddStringToObject(root, "commandCode","3007");
//    cJSON_AddStringToObject(root, "deviceCode", DEVICE_SN);
    cJSON_AddStringToObject(root, "deviceCode", gMqttDevSn.sn);    

    cJSON_AddItemToObject(root, "data", dataObj);

    cJSON_AddStringToObject(dataObj, "cardNo", localUserData->cardNo);
    cJSON_AddStringToObject(dataObj, "userId", localUserData->userId);
    cJSON_AddNumberToObject(dataObj, "callType", localUserData->authMode);
    cJSON_AddStringToObject(dataObj, "callElevatorTime",  GetLocalTime());
    cJSON_AddStringToObject(dataObj, "type","1");
    cJSON_AddStringToObject(dataObj, "timeStamp",localUserData->timeStamp);
    cJSON_AddStringToObject(dataObj, "status", "1");
            
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


//保存模板信息
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff)
{
    SYSERRORCODE_E result = NO_ERR;
    cJSON* root,*data,*templateData,*templateMap,*holidayTimeMap,*peakTimeMap;   
    cJSON* tmpArray,*arrayElement;

    TEMPLATE_PARAM_STRU *templateParam = &gTemplateParam; 
    int holidayTimeMapCnt=0,peakTimeMapCnt=0,index = 0;
    char tmpbuf[8] = {0};

    char tmpIndex[2] = {0};
    char tmpKey[32] = {0};

    root = cJSON_Parse((char *)jsonBuff);    //解析数据包
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
    //获取templateMap数据
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
    //存储hoildayTimeMap中数据
    holidayTimeMapCnt = cJSON_GetArraySize(holidayTimeMap); /*获取数组长度*/
    log_d("array len = %d\r\n",holidayTimeMapCnt);

    //存储不受控时间段的个数
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
        templateParam->hoildayMode[index].templateType = arrayElement->valueint;        
        log_d("templateType = %d\r\n",templateParam->hoildayMode[index].templateType);

        arrayElement = cJSON_GetObjectItem(tmpArray, "voiceSize");
        templateParam->hoildayMode[index].voiceSize = arrayElement->valueint;        
        log_d("voiceSize = %d\r\n",templateParam->hoildayMode[index].voiceSize);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "modeType");
        templateParam->hoildayMode[index].channelType = arrayElement->valueint;
        log_d("modeType = %d\r\n",templateParam->hoildayMode[index].channelType);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "startTime");
        //因为节假日跟高峰共用，所以只记录到FLASH一种就可以了
        strcpy(templateParam->hoildayMode[index].startTime,arrayElement->valuestring);
        sprintf(tmpIndex,"%d",index);
        strcpy(tmpKey,"hoildayModeStartTime");
        strcat(tmpKey,tmpIndex); 
        ef_set_env_blob(tmpKey,templateParam->hoildayMode[index].startTime,strlen(templateParam->hoildayMode[index].startTime));        
        log_d("%s = %s\r\n",tmpKey,templateParam->hoildayMode[index].startTime);
        
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "endTime");
        strcpy(templateParam->hoildayMode[index].endTime,arrayElement->valuestring);  
        memset(tmpKey,0x00,sizeof(tmpKey));
        strcpy(tmpKey,"hoildayModeEndTime");
        strcat(tmpKey,tmpIndex);      
        ef_set_env_blob(tmpKey,templateParam->hoildayMode[index].endTime,strlen(templateParam->hoildayMode[index].endTime));                
        log_d("%s= %s\r\n",tmpKey,templateParam->hoildayMode[index].endTime);        
    }
    
    log_d("=====================================================\r\n");
//--------------------------------------------------
    peakTimeMapCnt = cJSON_GetArraySize(peakTimeMap); /*获取数组长度*/
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

    return result;
}



