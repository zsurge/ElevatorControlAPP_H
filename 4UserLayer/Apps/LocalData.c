/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : LocalData.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年3月21日
  最近修改   :
  功能描述   : 管理本地数据，对卡号，用户ID为索引的数-
                   据进行增删改查
  函数列表   :
  修改历史   :
  1.日    期   : 2020年3月21日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "LocalData.h"
#include "bsp_spi_flash.h"
#include "easyflash.h"
#include "stdio.h"
#include "tool.h"
#include "malloc.h"

#define LOG_TAG    "localData"
#include "elog.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
// *----------------------------------------------*/
uint16_t gCurCardHeaderIndex = 0;
uint16_t gCurUserHeaderIndex = 0;
uint16_t gCurRecordIndex = 0;


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static uint8_t checkFlashSpace(uint8_t mode);
static void eraseUserDataIndex(void);
 
/*****************************************************************************
 函 数 名  : writeHeader
 功能描述  : 写表头文件
 输入参数  : uint8_t  * header  卡号或者是用户ID
             uint8_t mode   0：写卡号       1:写用户ID
 输出参数  : 无
 返 回 值  : 0:成功，其它：失败
 
 修改历史      :
  1.日    期   : 2020年3月24日
    作    者   :  
uint8_t writeHeader(uint8_t  * header,uint8_t mode)
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t writeHeader(uint8_t  * header,uint8_t mode)
{
    uint8_t times = 3;
    uint8_t readBuff[HEAD_lEN] = {0};
	uint8_t ret = 0;
    uint8_t headCnt[6] = {0};
    uint8_t temp[HEAD_lEN] = {0};
    uint32_t addr = 0;
         

    if(mode == CARD_MODE)
    {
        addr = CARD_NO_HEAD_ADDR + gCurCardHeaderIndex * HEAD_lEN;
    }
    else 
    {
        addr = USER_ID_HEAD_ADDR + gCurUserHeaderIndex * HEAD_lEN;
    }


    

    if(header == NULL)
    {
        log_d("cardNoHeader 为空\r\n");
        return 1;
    }

    memset(temp,0x00,sizeof(temp));
    asc2bcd(temp, header,HEAD_lEN*2, 0);
    
   	while(times)
	{		
		ret = bsp_sf_WriteBuffer (temp, addr, HEAD_lEN);
        
		//再读出来，对比是否一致
		memset(readBuff,0x00,sizeof(readBuff));
		bsp_sf_ReadBuffer (readBuff, addr, HEAD_lEN);
		
		ret = memcmp(temp,readBuff,HEAD_lEN);
		
		if(ret == 0)
		{
			break;
		}

		if(ret != 0 && times == 1)
		{
            log_d("写sn失败\r\n");
			return 3;
		}

		times--;
	} 

    //这里需要保存  
    if(mode == CARD_MODE)
    {
        gCurCardHeaderIndex++;
        memset(headCnt,0x00,sizeof(headCnt));
        sprintf(headCnt,"%06d",gCurCardHeaderIndex);
        ef_set_env_blob("CardHeaderIndex",headCnt,6);          
    }
    else
    {
        gCurUserHeaderIndex++;
        memset(headCnt,0x00,sizeof(headCnt));
        sprintf(headCnt,"%06d",gCurUserHeaderIndex);
        ef_set_env_blob("UserHeaderIndex",headCnt,6);           
    }

	log_d("gCurCardHeaderIndex = %d,gCurUserHeaderIndex = %d\r\n",gCurCardHeaderIndex,gCurUserHeaderIndex);

    
    return 0;
    
}



//isFind = 0 未找到；isFind = 1 已找到
/*****************************************************************************
 函 数 名  : searchHeaderIndex
 功能描述  : 查找指定的卡号/用户ID的索引值
 输入参数  : uint8_t* header 
           uint8_t mode     
           
 输出参数  : uint16_t *index 
 返 回 值  : 1:找到索引值 0:未找到索引值
 
 修改历史      :
  1.日    期   : 2020年3月24日
    作    者   :  
uint8_t searchHeaderIndex(uint8_t* header,uint8_t mode,uint16_t *index)
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t searchHeaderIndex(uint8_t* header,uint8_t mode,uint16_t *index)
{
    char *buff = my_malloc(CARD_NO_HEAD_SIZE);
	uint8_t temp[HEAD_lEN] = {0};
	uint32_t addr = 0;
	uint16_t i = 0;
    uint32_t num = 0;
    uint8_t isFind = 0;   

     if (buff == NULL || header == NULL)
     {
        my_free(buff);
        log_d("my_malloc error\r\n");
        return isFind;
     }

	memset(temp,0x00,sizeof(temp));
    asc2bcd(temp, header,HEAD_lEN*2, 0);     
//    dbh("header", temp, HEAD_lEN);
    if(mode == CARD_MODE)
    {
        addr = CARD_NO_HEAD_ADDR;
        bsp_sf_ReadBuffer (buff, addr, gCurCardHeaderIndex*4);        
        num = gCurCardHeaderIndex;

//        dbh("all header", buff, gCurCardHeaderIndex*4);
    }
    else
    {
        addr = USER_ID_HEAD_ADDR;
        bsp_sf_ReadBuffer (buff, addr, gCurUserHeaderIndex*4);
        num = gCurUserHeaderIndex;

//        dbh("all header", buff, gCurCardHeaderIndex*4);
        
    } 

//    dbh("table", buff, CARD_NO_HEAD_SIZE);

	for(i=0; i<num; i++)
	{
        if(memcmp(temp,buff+i*HEAD_lEN,HEAD_lEN) == 0)
        {
            isFind = 1;
            *index = i;

            my_free(buff);
            return isFind;
        }        
    }

    *index = 0;
    my_free(buff);
    return isFind;	     
}

void eraseUserDataAll(void)
{
    int32_t iTime1, iTime2;
    iTime1 = xTaskGetTickCount();	/* 记下开始时间 */
    eraseHeadSector();
    eraseDataSector();    
    iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d("eraseUserDataAll成功，耗时: %dms\r\n",iTime2 - iTime1);
}

static void eraseUserDataIndex(void)
{
    uint8_t headCnt[6] = {0};
    gCurCardHeaderIndex = 0;
    sprintf(headCnt,"%06d",gCurCardHeaderIndex);
    ef_set_env_blob("CardHeaderIndex",headCnt,6);    

    memset(headCnt,0x00,sizeof(headCnt));
    gCurUserHeaderIndex = 0;
    sprintf(headCnt,"%06d",gCurUserHeaderIndex);
    ef_set_env_blob("UserHeaderIndex",headCnt,6);      

}


void eraseHeadSector(void)
{  
    uint16_t i = 0;

    for(i=0;i<USER_SECTOR_NUM+CARD_SECTOR_NUM;i++)
    {
        bsp_sf_EraseSector(CARD_NO_HEAD_ADDR+i*SECTOR_SIZE);
    }

    eraseUserDataIndex();
}
void eraseDataSector(void)
{  
    uint16_t i = 0;

    for(i=0;i<DATA_SECTOR_NUM;i++)
    {
        bsp_sf_EraseSector(CARD_NO_DATA_ADDR+i*SECTOR_SIZE);
        bsp_sf_EraseSector(USER_ID_DATA_ADDR+i*SECTOR_SIZE);
    }
}


uint8_t writeUserData(USERDATA_STRU userData,uint8_t mode)
{
    uint8_t wBuff[255] = {0};
    uint8_t rBuff[255] = {0}; 
    uint8_t isFull = 0;
    uint8_t header[CARD_USER_LEN] = {0};
    uint8_t crc=0;
    uint8_t ret = 0;
    uint8_t times = 3;
    uint32_t addr = 0;
    
    int32_t iTime1, iTime2;
	log_d("sizeof(USERDATA_STRU) = %d\r\n",sizeof(USERDATA_STRU));
    
    iTime1 = xTaskGetTickCount();	/* 记下开始时间 */
 
	
    //检查存储空间是否已满；
    isFull = checkFlashSpace(mode);

    if(isFull == 1)
    {
        return 1; //提示已经满了
    }

    //写表头
    if(mode == CARD_MODE)
    {
        addr = CARD_NO_DATA_ADDR + gCurCardHeaderIndex * (sizeof(USERDATA_STRU)+2);
        memcpy(header,userData.cardNo,CARD_USER_LEN);
    }
    else
    {
        addr = USER_ID_DATA_ADDR + gCurUserHeaderIndex * (sizeof(USERDATA_STRU)+2);
        memcpy(header,userData.userId,CARD_USER_LEN);
    }

    if(header == NULL)
    {
        return 1;//提示表头无效
    }
    

   ret = writeHeader(header,mode);

   if(ret != 0)
   {
       return 1;//提示写表头失败
   }

    //packet write buff
    memset(wBuff,0x00,sizeof(wBuff));

    //set head
	wBuff[0] = TABLE_HEAD;    
	
    //copy to buff
    memcpy(wBuff+1, &userData, sizeof(USERDATA_STRU));
	
    //calc crc
	crc = xorCRC(wBuff, sizeof(USERDATA_STRU)+1);

	//copy crc
	wBuff[sizeof(USERDATA_STRU) + 1] = crc;	


    //write flash
	while(times)
	{
	
		bsp_sf_WriteBuffer (wBuff, addr, sizeof(USERDATA_STRU)+2);

		//再读出来，对比是否一致
		memset(rBuff,0x00,sizeof(rBuff));
		bsp_sf_ReadBuffer (rBuff, addr, sizeof(USERDATA_STRU)+2);

		ret = memcmp(wBuff,rBuff,sizeof(USERDATA_STRU)+2);
		
		if(ret == 0)
		{
			break;
		}
				

		if(ret != 0 && times == 1)
		{
			log_d("写交易记录表失败!\r\n");
			return 3;
		}

		times--;
	}

    iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d("writeUserData成功，耗时: %dms\r\n",iTime2 - iTime1);	

    return 0;
}

uint8_t readUserData(uint8_t* header,uint8_t mode,USERDATA_STRU *userData)
{
	uint8_t rBuff[255] = {0};
	uint8_t crc = 0;	
	uint8_t ret = 0;
	uint16_t index = 0;
    uint32_t addr = 0;
    
    int32_t iTime1, iTime2;
    iTime1 = xTaskGetTickCount();	/* 记下开始时间 */
    

	if(header == NULL)
	{
        return 1; //提示错误的参数
	}

		
	ret = searchHeaderIndex(header,mode,&index);

	log_d("searchHeaderIndex ret = %d",ret);
	
	if(ret != 1)
	{
		return 3;//提示未找到索引
	}
	
	log_d("sizeof(USERDATA_STRU) = %d\r\n",sizeof(USERDATA_STRU));

    if(mode == CARD_MODE)
    {
        addr = CARD_NO_DATA_ADDR + index * (sizeof(USERDATA_STRU)+2);
    }
    else
    {
        addr = USER_ID_DATA_ADDR + index * (sizeof(USERDATA_STRU)+2);
    }	

    bsp_sf_ReadBuffer (rBuff, addr, sizeof(USERDATA_STRU)+2);

    //judge head
	if(rBuff[0] != TABLE_HEAD)
	{
		return 4;
	}

    //calc crc
	crc = xorCRC(rBuff, sizeof(USERDATA_STRU)+1);

	//judge crc
	if(crc != rBuff[sizeof(USERDATA_STRU) + 1])
	{
		return 5;
	}

	memcpy(userData, rBuff+1, sizeof(USERDATA_STRU));

    iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d("readUserData成功，耗时: %dms\r\n",iTime2 - iTime1);		

	return 0;

}


static uint8_t checkFlashSpace(uint8_t mode)
{
    if(mode == CARD_MODE)
    {
        if(gCurCardHeaderIndex > MAX_HEAD_RECORD-5)
        {
            log_d("card flash is full\r\n");

            return 1;
        }
    }
    else
    {
        if(gCurUserHeaderIndex > MAX_HEAD_RECORD-5)
        {
            log_d("card flash is full\r\n");
            return 1;
        }        
    }


    return 0;
}



uint8_t modifyUserData(USERDATA_STRU userData,uint8_t mode)
{
    uint8_t wBuff[255] = {0};
    uint8_t rBuff[255] = {0}; 
    uint8_t isFull = 0;
    uint8_t header[CARD_USER_LEN] = {0};
    uint8_t crc=0;
    uint8_t ret = 0;
    uint8_t times = 3;
    uint32_t addr = 0;
    uint16_t index = 0;
    
    int32_t iTime1, iTime2;
	log_d("sizeof(USERDATA_STRU) = %d\r\n",sizeof(USERDATA_STRU));
    
    iTime1 = xTaskGetTickCount();	/* 记下开始时间 */
 
	
    //检查存储空间是否已满；
    isFull = checkFlashSpace(mode);

    if(isFull == 1)
    {
        return 1; //提示已经满了
    } 

    if(mode == CARD_MODE)
    {
        memcpy(header,userData.cardNo,CARD_USER_LEN);
    }
    else
    {
        memcpy(header,userData.userId,CARD_USER_LEN);
    }   

	ret = searchHeaderIndex(header,mode,&index);

	log_d("searchHeaderIndex ret = %d",ret);
	
	if(ret != 1)
	{
		return 3;//提示未找到索引
	}

	if(mode == CARD_MODE)
	{
        addr = CARD_NO_DATA_ADDR + index * (sizeof(USERDATA_STRU)+2);
   }
    else
    {
        addr = USER_ID_DATA_ADDR + index * (sizeof(USERDATA_STRU)+2);
    }

    //packet write buff
    memset(wBuff,0x00,sizeof(wBuff));

    //set head
	wBuff[0] = TABLE_HEAD;    
	
    //copy to buff
    memcpy(wBuff+1, &userData, sizeof(USERDATA_STRU));
	
    //calc crc
	crc = xorCRC(wBuff, sizeof(USERDATA_STRU)+1);

	//copy crc
	wBuff[sizeof(USERDATA_STRU) + 1] = crc;	


    //write flash
	while(times)
	{
	
		bsp_sf_WriteBuffer (wBuff, addr, sizeof(USERDATA_STRU)+2);

		//再读出来，对比是否一致
		memset(rBuff,0x00,sizeof(rBuff));
		bsp_sf_ReadBuffer (rBuff, addr, sizeof(USERDATA_STRU)+2);

		ret = memcmp(wBuff,rBuff,sizeof(USERDATA_STRU)+2);
		
		if(ret == 0)
		{
			break;
		}
				

		if(ret != 0 && times == 1)
		{
			log_d("修改记录失败!\r\n");
			return 3;
		}

		times--;
	}

    iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d("修改记录成功，耗时: %dms\r\n",iTime2 - iTime1);	

    return 0;
}


