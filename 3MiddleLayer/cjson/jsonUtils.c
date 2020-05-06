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
#include "calcDevNO.h"
#include "bsp_rtc.h"
#include "eth_cfg.h"
#include "LocalData.h"
#include "templateprocess.h"
#include "malloc.h"
#include "bsp_ds1302.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/



/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
//LOCAL_USER_STRU gLoalUserData;
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
        cJSON_AddStringToObject(dataObj,(const char*)item,(const char*)value);
    }
    else
    {
        cJSON_AddStringToObject(root,(const char*)item,(const char*)value);
    }  

    
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        cJSON_Delete(root);
        log_d("cJSON_PrintUnformatted error \r\n");
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);


//    log_d("send json data = %s\r\n",tmpBuf);

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
	cJSON* arrayElement;
    int tmpArrayNum = 0;
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
            json_item = cJSON_GetObjectItem ( dataObj, (const char*)item );
        }
        else
        {
            json_item = cJSON_GetObjectItem ( root, (const char*)item );
        }  
		
		if ( json_item->type == cJSON_String )
		{
			//避免溢出
			if ( strlen ( json_item->valuestring ) > JSON_ITEM_MAX_LEN )
			{
				memcpy ( value, json_item->valuestring,JSON_ITEM_MAX_LEN );
			}

			strcpy ( (char*)value, json_item->valuestring );
//			log_d ( "json_item =  %s\r\n",json_item->valuestring );
		}
		else if ( json_item->type == cJSON_Number )
		{
			sprintf ( (char*)value,"%d",json_item->valueint );
//			log_d ( "json_item =  %s\r\n",value);
		}
		else if( json_item->type == cJSON_Array )
		{

            //  2.日    期   : 2020年4月11日
            //    作    者   :  
            //    修改内容   : 添加对数组的支持，返回值还不完善    
            tmpArrayNum = cJSON_GetArraySize(json_item);

            for(int n=0;n<tmpArrayNum;n++)
            {
                arrayElement = cJSON_GetArrayItem(json_item, n);                 
                strcpy ((char*)value, arrayElement->valuestring );
            
                log_d("cJSON_Array = %s\r\n",arrayElement->valuestring );
            }

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
	cJSON* root,*newroot,*dataObj,*json_cmdid,*json_devcode,*identification;
    char *tmpBuf;
    char buf[8] = {0};
    
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

        cJSON_AddStringToObject(dataObj, "version", (const char*)gDevinfo.SoftwareVersion);
        cJSON_AddStringToObject(dataObj, "appName", (const char*)gDevinfo.Model);

        memset(buf,0x00,sizeof(buf));
        sprintf(buf,"%d",gCurCardHeaderIndex);
        cJSON_AddStringToObject(dataObj, "regRersion", buf);
        cJSON_AddStringToObject(dataObj, "regface", " ");
        cJSON_AddStringToObject(dataObj, "ip", (const char*)gDevinfo.GetIP());
                
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
    char up_status[7] = {0};  
    uint8_t value[200] = {0};
    uint8_t tmp[300] = {0};
    uint8_t send[300] = {0};
    
    ef_get_env_blob ( "upData", value, sizeof ( value ), NULL );

    log_d("tmpBuf = %s\r\n",value);

    result = modifyJsonItem((const uint8_t *)value,(const uint8_t *)"commandCode",(const uint8_t *)"1017",0,tmp);    
    

    strcpy(up_status,(const char*)ef_get_env("up_status"));
    
    //升级失败
    if(memcmp(up_status,"101700",6) == 0)
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"2",1,send);

    }
    else if(memcmp(up_status,"101711",6) == 0) //升级成功
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"1",1,send);

    }
    else if(memcmp(up_status,"101722",6) == 0) //升级成功
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"1",1,send);

    }
    else if(memcmp(up_status,"101733",6) == 0) //禁止升级
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"3",1,send);

    }
    else
    {
        //无升级动作
    }    
 
    strcpy((char *)descBuf,send);


    return result;

}

SYSERRORCODE_E saveUpgradeData(uint8_t *jsonBuff)
{
    SYSERRORCODE_E result = NO_ERR;
    
    cJSON* root,*newroot,*tmpdataObj,*dataObj,*json_devCode,*productionModel,*id;
    cJSON* version,*softwareFirmware,*versionType;
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

    newroot = cJSON_CreateObject();
    dataObj = cJSON_CreateObject();
    if(!newroot && !dataObj)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        cJSON_Delete(newroot);
        my_free(tmpBuf);            
        return CJSON_PARSE_ERR;
    }  
    
    json_devCode = cJSON_GetObjectItem ( root, "deviceCode" );
    
    tmpdataObj = cJSON_GetObjectItem ( root, "data" );        
    productionModel = cJSON_GetObjectItem ( tmpdataObj, "productionModel" );
    id = cJSON_GetObjectItem ( tmpdataObj, "id" );

    version = cJSON_GetObjectItem ( tmpdataObj, "version" );
    softwareFirmware = cJSON_GetObjectItem ( tmpdataObj, "softwareFirmware" );
    versionType = cJSON_GetObjectItem ( tmpdataObj, "versionType" ); 

    if(json_devCode)
        cJSON_AddStringToObject(newroot, "deviceCode", json_devCode->valuestring);


    cJSON_AddItemToObject(newroot, "data", dataObj);
    
    if(productionModel)
        cJSON_AddStringToObject(dataObj, "productionModel", productionModel->valuestring);

    if(id)
        cJSON_AddNumberToObject(dataObj, "id", id->valueint);

    if(version)
        cJSON_AddStringToObject(dataObj, "version", version->valuestring);

    if(softwareFirmware)
        cJSON_AddNumberToObject(dataObj, "softwareFirmware", softwareFirmware->valueint);
        
    if(versionType)
        cJSON_AddNumberToObject(dataObj, "versionType", versionType->valueint);  
            
    tmpBuf = cJSON_PrintUnformatted(newroot); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");

        cJSON_Delete(root);
        cJSON_Delete(newroot);      
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    }

    ef_set_env_blob("upData",(const char*)tmpBuf,strlen ((const char*)tmpBuf));

    cJSON_Delete(root);

    cJSON_Delete(newroot);

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
    
	cJSON* root,*newroot,*tmpdataObj,*dataObj,*json_cmdCode,*json_devCode,*identification,*id;
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

    cJSON_AddStringToObject(root, "deviceCode", gMqttDevSn.sn);
    log_d("deviceCode = %s",gMqttDevSn.sn);
    
    cJSON_AddItemToObject(root, "data", dataObj);

    if(localUserData->qrType == 4 )
    {    
        cJSON_AddStringToObject(root, "commandCode","3007");
        cJSON_AddStringToObject(dataObj, "cardNo", (const char*)localUserData->cardNo);
        cJSON_AddNumberToObject(dataObj, "callType",localUserData->authMode); 
        cJSON_AddNumberToObject(dataObj, "status", ON_LINE); 
        strcpy(tmpTime,(const char*)bsp_ds1302_readtime());  
        cJSON_AddStringToObject(dataObj, "callElevatorTime",tmpTime);        
        cJSON_AddStringToObject(dataObj, "timeStamp",(const char*)localUserData->timeStamp);
        
        if(localUserData->authMode == 7)
        {
            cJSON_AddNumberToObject(dataObj, "type",localUserData->qrType);
            cJSON_AddNumberToObject(dataObj, "callState",CALL_OK);
//            cJSON_AddStringToObject(dataObj, "qrId", (const char*)localUserData->qrID);
            cJSON_AddStringToObject(dataObj, "userId", (const char*)localUserData->qrID);              
        }
        else
        {
            cJSON_AddNumberToObject(dataObj, "type",CALL_OK);
            cJSON_AddStringToObject(dataObj, "userId", (const char*)localUserData->userId);              
            
        }
    }
    else
    {
        cJSON_AddStringToObject(root, "commandCode","4002");
        cJSON_AddNumberToObject(dataObj, "enterType",  4);//这里设置为4是QRCODE在开发者平台上的数据，梯控上的数据为7        
        cJSON_AddStringToObject(dataObj, "qrId",  (const char*)localUserData->qrID);
        cJSON_AddNumberToObject(dataObj, "status", ON_LINE);   
        cJSON_AddStringToObject(dataObj, "enterTime",(const char*)bsp_ds1302_readtime());                
        cJSON_AddNumberToObject(dataObj, "faceCompare",CALL_OK);//：1、成功 2失败，
        cJSON_AddNumberToObject(dataObj, "direction", DIRECTION_IN);//1进，2出        
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


//保存模板信息
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
    //获取templateMap数据
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
        templateParam->holidayMode[index].templateType = arrayElement->valueint;        
        log_d("templateType = %d\r\n",templateParam->holidayMode[index].templateType);

        arrayElement = cJSON_GetObjectItem(tmpArray, "voiceSize");
        templateParam->holidayMode[index].voiceSize = arrayElement->valueint;        
        log_d("voiceSize = %d\r\n",templateParam->holidayMode[index].voiceSize);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "modeType");
        templateParam->holidayMode[index].channelType = arrayElement->valueint;
        log_d("modeType = %d\r\n",templateParam->holidayMode[index].channelType);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "startTime");
        //因为节假日跟高峰共用，所以只记录到FLASH一种就可以了
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
    
 
    cJSON_Delete(root);

    log_d("saveTemplateParam took %d ms to save\r\n",xTaskGetTickCount()-curtick);

    return result;
}


#if 0
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
    
    root = cJSON_Parse((char *)jsonBuff);    //解析数据包
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

    //查找是否在范围之内
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

    //只是把权限楼层打印出来并没有存储
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
#endif

uint8_t parseQrCode(uint8_t *jsonBuff,QRCODE_INFO_STRU *qrCodeInfo)
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
    
    root = cJSON_Parse((char *)jsonBuff);    //解析数据包
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
    strcpy((char *)qrCodeInfo->qrID,tmpArray->valuestring);
    log_d("qrCodeInfo->qrID= %s\r\n",qrCodeInfo->qrID); 

    tmpArray = cJSON_GetObjectItem(root, "t");
    qrCodeInfo->type = tmpArray->valueint;
    log_d("qrCodeInfo->type= %d\r\n",qrCodeInfo->type); 
    
    tmpArray = cJSON_GetObjectItem(root, "f1");
    memcpy(qrCodeInfo->accessFloor,parseAccessFloor((uint8_t *)tmpArray->valuestring),FLOOR_ARRAY_LENGTH);

    dbh("qrCodeInfo->accessFloor",(char *)qrCodeInfo->accessFloor,FLOOR_ARRAY_LENGTH);

    qrCodeInfo->defaultFloor = qrCodeInfo->accessFloor[0];
    log_d("qrCodeInfo->defaultFloor = %d\r\n",qrCodeInfo->defaultFloor);
    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d1");
    strcpy((char *)buf,tmpArray->valuestring);
    log_d("d1 = %s\r\n",buf);  
    if(findDev(buf,1) == 1)
    {
        isFind =1;
        goto QR_END;
    }

    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d2");
    strcpy((char *)buf,tmpArray->valuestring);
    log_d("d2 = %s\r\n",buf); 
    if(findDev(buf,2) == 1)
    {
        isFind =1;
        goto QR_END;
    }
    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d3");
    strcpy((char *)buf,tmpArray->valuestring);
    log_d("d3= %s\r\n",buf); 
    if(findDev(buf,3) == 1)
    {
        isFind =1;
        goto QR_END;
    }

    
    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d4");
    strcpy((char *)buf,tmpArray->valuestring);
    log_d("d4 = %s\r\n",buf);
    if(findDev(buf,4) == 1)
    {
        isFind =1;
        goto QR_END;
    }    


    memset(buf,0x00,sizeof(buf));
    tmpArray = cJSON_GetObjectItem(root, "d5");
    strcpy((char *)buf,tmpArray->valuestring);
    log_d("d5 = %s\r\n",buf);
    if(findDev(buf,5) == 1)
    {
       isFind =1;
    }    
  
QR_END:    
    cJSON_Delete(root);
    
    return isFind;

}




uint8_t** GetJsonArray ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t *num)
{
    uint8_t** result; 
    cJSON* root,*json_item,*dataObj;
    cJSON* arrayElement;
    int tmpArrayNum = 0;
    int i = 0;
    
    root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
    
    if ( !root )
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        return NULL;
    }
    else
    {
        //根据协议，默认所有的子项是data
        dataObj = cJSON_GetObjectItem ( root, "data" );  
        json_item = cJSON_GetObjectItem ( dataObj, (const char*)item );        

        if( json_item->type == cJSON_Array )
        {
            tmpArrayNum = cJSON_GetArraySize(json_item);

            //每个人最多20张卡
            if(tmpArrayNum > 20)
            {
                tmpArrayNum = 20;
            }

            result = (uint8_t **)my_malloc(tmpArrayNum*sizeof(uint8_t *));

            if(result == NULL)
            {
                log_d("create array error\r\n");
                cJSON_Delete(root);
                return NULL;                
            }

            *num = tmpArrayNum;
            
            for (i = 0; i < tmpArrayNum; i++)
            {
                result[i] = (uint8_t *)my_malloc(8 * sizeof(uint8_t));
            }            

            for(i=0;i<tmpArrayNum;i++)
            {
                arrayElement = cJSON_GetArrayItem(json_item, i);                 
                strcpy ((char*)result[i], arrayElement->valuestring ); 
                log_d("result :%d = %s\r\n",i,result[i]); 
            }

        }
        else
        {
            log_d ( "can't parse json buff\r\n" );
            cJSON_Delete(root);
            return NULL;
        }

    }

    for (i = 0; i < tmpArrayNum; i++)
    {
        my_free(result[i]);
    }     
    my_free(result);        
    
    cJSON_Delete(root);
    return result;
}




