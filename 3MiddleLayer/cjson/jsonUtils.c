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
#define LOG_TAG    "jsonutils"
#include "elog.h"

#include "jsonUtils.h"
#include "version.h"
#include "calcDevNO.h"
#include "bsp_rtc.h"
//#include "eth_cfg.h"
#include "LocalData.h"
#include "malloc.h"
#include "bsp_ds1302.h"
#include "deviceinfo.h"


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
	cJSON* arrayElement;
    int tmpArrayNum = 0;

    if(strlen((const char*)jsonBuff) == 0 || strlen((const char*)jsonBuff) > JSON_ITEM_MAX_LEN )
    {
        log_d ( "invalid data\r\n");       
		return NULL;
    }
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
            json_item = cJSON_GetObjectItem ( dataObj, (const char*)item );
        }
        else
        {
            json_item = cJSON_GetObjectItem ( root, (const char*)item );
        }  
		
		if ( json_item->type == cJSON_String )
		{
			//�������
			if ( strlen ( json_item->valuestring ) > JSON_ITEM_MAX_LEN )
			{
				memcpy ( value, json_item->valuestring,JSON_ITEM_MAX_LEN );
			}
			else
			{
			    strcpy ( (char*)value, json_item->valuestring );
			}
//			log_d ( "json_item =  %s\r\n",json_item->valuestring );
		}
		else if ( json_item->type == cJSON_Number )
		{
			sprintf ( (char*)value,"%d",json_item->valueint );
//			log_d ( "json_item =  %s\r\n",value);
		}
		else if( json_item->type == cJSON_Array )
		{

            //  2.��    ��   : 2020��4��11��
            //    ��    ��   :  
            //    �޸�����   : ��Ӷ������֧�֣�����ֵ��������    
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

        cJSON_AddStringToObject(dataObj, "version", (const char*)gDevinfo.SoftwareVersion);
        cJSON_AddStringToObject(dataObj, "appName", (const char*)gDevinfo.Model);

        memset(buf,0x00,sizeof(buf));
        sprintf(buf,"%d",gRecordIndex.userIdIndex);
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
    
    //����ʧ��
    if(memcmp(up_status,"101700",6) == 0)
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"2",1,send);

    }
    else if(memcmp(up_status,"101711",6) == 0) //�����ɹ�
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"1",1,send);

    }
    else if(memcmp(up_status,"101722",6) == 0) //�����ɹ�
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"1",1,send);

    }
    else if(memcmp(up_status,"101733",6) == 0) //��ֹ����
    {
        result = modifyJsonItem((const uint8_t *)tmp,(const uint8_t *)"status",(const uint8_t *)"3",1,send);

    }
    else
    {
        //����������
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
    
    root = cJSON_Parse ( ( char* ) jsonBuff );    //�������ݰ�
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
    cJSON_AddStringToObject(root,"deviceCode",gDevBaseParam.deviceCode.deviceSn);
     
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
#endif






uint8_t** GetCardArray ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t *num)
{
    uint8_t** result; 
    cJSON* root,*json_item,*dataObj;
    cJSON* arrayElement;
    int tmpArrayNum = 0;
    int i = 0;
    
    root = cJSON_Parse ( ( char* ) jsonBuff );    //�������ݰ�
    
    if ( !root )
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        return NULL;
    }
    else
    {
        //����Э�飬Ĭ�����е�������data
        dataObj = cJSON_GetObjectItem ( root, "data" );  
        json_item = cJSON_GetObjectItem ( dataObj, (const char*)item );        

        if( json_item->type == cJSON_Array )
        {
            tmpArrayNum = cJSON_GetArraySize(json_item);
            log_d("cardArrayNum = %d\r\n",tmpArrayNum);
            
            //ÿ�������20�ſ�
            if(tmpArrayNum > 20)
            {
                tmpArrayNum = 20;
            }

            result = (uint8_t **)my_malloc(tmpArrayNum*sizeof(uint8_t *));

            if(result == NULL)
            {
                 *num = 0;
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
        else if( json_item->type == cJSON_String )
        {
            //һ���ߵ�������ž��ǿյ�
            if(strlen((const char*)json_item->valuestring) == 0)
            {
                 *num = 0;
                log_d("card no is empty \r\n");
                cJSON_Delete(root);
                return NULL;    
            }
        
            tmpArrayNum = 1;
            *num = tmpArrayNum;

            result[0] = (uint8_t *)my_malloc(8 * sizeof(uint8_t)); 
            
			if ( strlen ( json_item->valuestring ) > 8 )
			{
				memcpy ( result[0], json_item->valuestring,8 );
			}
			else
			{
			    strcpy ( (char*)result[0], json_item->valuestring ); 
			}

			log_d ( "json_item =  %s\r\n",json_item->valuestring );

            
            
        }
        else
        {
            *num = 0;
            log_d ( "can't parse json buff\r\n" );
            cJSON_Delete(root);
            return NULL;
        }
        
    }        
    
    cJSON_Delete(root);
    return result;
}




