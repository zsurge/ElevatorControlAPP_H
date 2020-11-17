/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : LocalData.c
  �� �� ��   : ����
  ��    ��   :
  ��������   : 2020��3��21��
  ����޸�   :
  ��������   : ���������ݣ��Կ��ţ��û�IDΪ��������-
                   �ݽ�����ɾ�Ĳ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��3��21��
    ��    ��   :
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "LocalData.h"
#include "bsp_spi_flash.h"
#include "easyflash.h"
#include "stdio.h"
#include "tool.h"
#include "malloc.h"
#include "bsp_MB85RC128.h"
#include "deviceInfo.h"



#define LOG_TAG    "localData"
#include "elog.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
// *----------------------------------------------*/
//volatile uint16_t gCurCardHeaderIndex = 0;
//volatile uint16_t gCurUserHeaderIndex = 0;
//volatile uint16_t gCurRecordIndex = 0;
//volatile uint16_t gDelCardHeaderIndex = 0;    //��ɾ����������
//volatile uint16_t gDelUserHeaderIndex = 0;    //��ɾ���û�ID����

USERDATA_STRU gUserDataStru = {0};


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static uint8_t checkFlashSpace ( uint8_t mode );
static void eraseUserDataIndex ( void );
static ISFIND_ENUM findIndex ( uint8_t* header,uint32_t address,uint16_t curIndex,uint16_t* index );
static uint32_t readDelIndexValue ( uint8_t mode,uint16_t curIndex );

static uint8_t delSourceHeader ( uint16_t index,uint8_t mode );
static ISFIND_ENUM fIndex( uint8_t* header,uint8_t mode,uint16_t* index );

typedef ISFIND_ENUM(*opFindIndex)(uint8_t* header,uint8_t mode,uint16_t* index); //��ȡindex�ص�

static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target);






uint32_t readDelIndexValue ( uint8_t mode,uint16_t curIndex )
{
	uint8_t readBuff[HEAD_lEN+1] = {0};
	uint8_t temp[HEAD_lEN*2+1] = {0};
	uint32_t addr = 0;
	uint32_t value = 0;

	memset ( readBuff,0x00,sizeof ( readBuff ) );
	//д����ɾ���Ŀռ���,������ɾ����������ȡ����ǰ�����¿������û�������

	if ( mode == CARD_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR + curIndex * HEAD_lEN;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_DEL_HEAD_ADDR + curIndex * HEAD_lEN;
	}

	FRAM_Read ( FM24V10_1, addr, readBuff, HEAD_lEN );


	bcd2asc ( temp, readBuff,CARD_NO_LEN_ASC, 0 );

	value = atoi ( ( const char* ) temp );

	//log_d("readDelIndexValue = %s,value =%d\r\n",temp,value);

	return value;

}


ISFIND_ENUM searchFlashIndex( uint8_t* header,uint8_t mode,uint16_t* index, opFindIndex funFindex)
{
	return funFindex(header,mode,index);
}


ISFIND_ENUM fIndex( uint8_t* header,uint8_t mode,uint16_t* index )
{
	ISFIND_ENUM isFind = ISFIND_NO;
	uint8_t i = 0;
	uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint16_t loop = 0;
	uint32_t address = 0;
	uint32_t curIndex = 0;
    uint8_t temp[HEAD_lEN] = {0};
    
    char *buff = mymalloc(SRAMIN,SECTOR_SIZE);

    if (buff == NULL)
    {
       myfree(SRAMIN,buff);
       log_d("my_malloc error\r\n");
       return isFind;
    }

	if ( header == NULL )
	{
	    myfree(SRAMIN,buff);
		return isFind;
	}	
	
	memset ( temp,0x00,sizeof ( temp ) );
    memset ( buff,0x00,sizeof ( buff ) );	
    
	asc2bcd ( temp, header,CARD_NO_LEN_ASC, 0 );	

	if ( mode == CARD_MODE )
	{
		address = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == USER_MODE )
	{
		address = USER_ID_HEAD_ADDR;
		curIndex = gRecordIndex.userIdIndex;		
	}
	else if ( mode == CARD_DEL_MODE )
	{
		address = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
	else if ( mode == USER_DEL_HEAD_ADDR )
	{
		address = USER_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delUserIdIndex;
	}	
	
	multiple = curIndex / HEAD_NUM_SECTOR;
	remainder = curIndex % HEAD_NUM_SECTOR;

	//log_d("multiple = %d,remainder = %d\r\n",multiple,remainder);

	for ( i= 0; i<multiple; i++ )
	{
		memset ( buff,0x00,sizeof ( buff ) );
		FRAM_Read ( FM24V10_1, address+i*SECTOR_SIZE, buff, SECTOR_SIZE );

        
		for ( loop=0; loop<HEAD_NUM_SECTOR; loop++ )
		{
//            dbh("1read", buff+loop*HEAD_lEN,HEAD_lEN);
			if ( memcmp ( temp,buff+loop*HEAD_lEN,HEAD_lEN ) == 0 )
			{
				isFind = ISFIND_YES;
				*index = loop + i*HEAD_NUM_SECTOR;
				
                myfree(SRAMIN,buff);
				return isFind;
			}
		}
	}

	memset ( buff,0x00,sizeof ( buff ) );
	FRAM_Read ( FM24V10_1,address + multiple*SECTOR_SIZE, buff, remainder*HEAD_lEN );

	for ( loop=0; loop<remainder; loop++ )
	{
//            dbh("2read", buff+loop*HEAD_lEN,HEAD_lEN);
		if ( memcmp ( temp,buff+loop*HEAD_lEN,HEAD_lEN ) == 0 )
		{
			isFind = ISFIND_YES;
			*index = loop + i*HEAD_NUM_SECTOR;
			log_d("index = %d\r\n",*index);
            myfree(SRAMIN,buff);			
			return isFind;
		}
	}
	
    myfree(SRAMIN,buff);
	*index = 0;
	return isFind;
}



void eraseUserDataAll ( void )
{
	int32_t iTime1, iTime2;
	iTime1 = xTaskGetTickCount();	/* ���¿�ʼʱ�� */
	eraseHeadSector();
	eraseDataSector();
	clearTemplateFRAM();
    initTemplateParam();	
	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d ( "eraseUserDataAll�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );
}

static void eraseUserDataIndex ( void )
{
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,WRITE_PRARM);
}


void eraseHeadSector ( void )
{
	FRAM_Erase ( FM24V10_1,0,122880 );	
}
void eraseDataSector ( void )
{
	uint16_t i = 0;

	for ( i=0; i<DATA_SECTOR_NUM; i++ )
	{
		bsp_sf_EraseSector ( CARD_NO_DATA_ADDR+i*SECTOR_SIZE );
		bsp_sf_EraseSector ( USER_ID_DATA_ADDR+i*SECTOR_SIZE );
	}
}


uint8_t writeUserData ( USERDATA_STRU *userData,uint8_t mode )
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t isFull = 0;
	uint8_t crc=0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = 0;
	uint32_t index = 0;
    uint8_t head[8] = {0};

	int32_t iTime1, iTime2;

	iTime1 = xTaskGetTickCount();	/* ���¿�ʼʱ�� */

	//���洢�ռ��Ƿ�������
	isFull = checkFlashSpace ( mode );

	if ( isFull == 1 )
	{
		return 1; //��ʾ�Ѿ�����
	}

	if ( mode == CARD_MODE )
	{
	    asc2bcd ( head, userData->cardNo,CARD_NO_LEN_ASC, 1 );	
	}
	else
	{
	    asc2bcd ( head, userData->userId,CARD_NO_LEN_ASC, 1 );		
	}	

    //д��ͷ
	addHead(head,mode);

    //����ǰFLASH������
	ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	//��ȡ��ַ
	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_DATA_ADDR + (gRecordIndex.cardNoIndex-1) * ( sizeof ( USERDATA_STRU ) );;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_DATA_ADDR + (gRecordIndex.userIdIndex-1) * ( sizeof ( USERDATA_STRU ) );
	}

	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

	//copy to buff
	memcpy ( wBuff, userData, sizeof ( USERDATA_STRU )-1 );


	//����Ӧ�ô�ӡһ��WBUFF���������Ƿ���ȷ

	//calc crc
	crc = xorCRC ( wBuff, sizeof ( USERDATA_STRU )-1 );

	//copy crc
	wBuff[sizeof ( USERDATA_STRU )-1] = crc;


	//write flash
	while ( times )
	{

		bsp_sf_WriteBuffer ( wBuff, addr, sizeof ( USERDATA_STRU ) );

		//�ٶ��������Ա��Ƿ�һ��
		memset ( rBuff,0x00,sizeof ( rBuff ) );
		bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );
		ret = compareArray ( wBuff,rBuff,sizeof ( USERDATA_STRU ) );

		if ( ret == 0 )
		{
			break;
		}


		if ( ret != 0 && times == 1 )
		{
			//log_d("д���׼�¼��ʧ��!\r\n");
			return 3;
		}

		times--;
	}

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_e ( "writeUserData�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );

//    dbh("writeUserData", wBuff, sizeof(USERDATA_STRU));

	return 0;
}

uint8_t readUserData ( uint8_t* header,uint8_t mode,USERDATA_STRU* userData )
{
	uint8_t rBuff[255] = {0};
	uint8_t crc = 0;
	uint8_t ret = 0;
	int index = 0;
	uint32_t addr = 0;
	HEADINFO_STRU head;

    int32_t iTime1, iTime2;

	if ( header == NULL )
	{
		//log_d("invalid head\r\n");
		return 1; //��ʾ����Ĳ���
	}
	
	asc2bcd (head.headData.sn, header,CARD_NO_LEN_ASC, 1 );
	
	
    iTime1 = xTaskGetTickCount();	/* ���¿�ʼʱ�� */

    log_d("1 want find head = %x\r\n",head.headData.id);

	index = readHead(&head,mode);
	
    iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d("readUserData�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1);	

	if(index == NO_FIND_HEAD)
	{
        log_d("no find head\r\n");
        return 2;
	}

	if ( mode == CARD_MODE )
	{
        addr = CARD_NO_DATA_ADDR + index * ( sizeof ( USERDATA_STRU ) );
	}
	else if(mode == USER_MODE)
	{
		addr = USER_ID_DATA_ADDR + index * ( sizeof ( USERDATA_STRU ) );
	}		

	bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );

	//judge head
	if ( rBuff[0] != TABLE_HEAD )
	{
		//log_d("invalid start char\r\n");
		return 3;
	}

	//calc crc
	crc = xorCRC ( rBuff, sizeof ( USERDATA_STRU )-1 );

	//judge crc
	if ( crc != rBuff[sizeof ( USERDATA_STRU ) - 1] )
	{
		//log_d("crc check error\r\n");
		return 4;
	}

	memcpy ( userData, rBuff, sizeof ( USERDATA_STRU ) );



//    iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
//	log_d("readUserData�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1);

	//dbh("readUserData", (char *)rBuff, sizeof(USERDATA_STRU));

	return 0;

}


static uint8_t checkFlashSpace ( uint8_t mode )
{
	if ( mode == CARD_MODE )
	{
		if ( gRecordIndex.cardNoIndex > MAX_HEAD_RECORD-5 )
		{
			//log_d("card flash is full\r\n");

			return 1;
		}
	}
	else
	{
		if ( gRecordIndex.userIdIndex > MAX_HEAD_RECORD-5 )
		{
			//log_d("card flash is full\r\n");
			return 1;
		}
	}


	return 0;
}



uint8_t modifyUserData ( USERDATA_STRU *userData,uint8_t mode )
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
    HEADINFO_STRU head;

	int32_t iTime1, iTime2;
	//log_d("sizeof(USERDATA_STRU) = %d\r\n",sizeof(USERDATA_STRU));

	iTime1 = xTaskGetTickCount();	/* ���¿�ʼʱ�� */


	//���洢�ռ��Ƿ�������
	isFull = checkFlashSpace ( mode );

	if ( isFull == 1 )
	{
		//log_d("not enough speac storage the data\r\n");
		return 1; //��ʾ�Ѿ�����
	}

	if ( mode == CARD_MODE )
	{
        asc2bcd (head.headData.sn, userData->cardNo,CARD_NO_LEN_ASC, 1 );
	
//		memcpy ( header,userData->cardNo,CARD_USER_LEN );
	}
	else
	{
        asc2bcd (head.headData.sn, userData->userId,CARD_NO_LEN_ASC, 1 );
	
//		memcpy ( header,userData->userId,CARD_USER_LEN );
	}

//	ret = searchHeaderIndex ( header,mode,&index );
//	ret = searchFlashIndex(header,mode,&index,fIndex);

    index = readHead(&head,mode);

	//log_d("searchHeaderIndex ret = %d",ret);

	if ( ret != 1 )
	{
		//log_d("can't find the head index\r\n");
		return 3;//��ʾδ�ҵ�����
	}

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_DATA_ADDR + index * ( sizeof ( USERDATA_STRU ) );
	}
	else
	{
		addr = USER_ID_DATA_ADDR + index * ( sizeof ( USERDATA_STRU ) );
	}

	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

	//copy to buff
	memcpy ( wBuff, userData, sizeof ( USERDATA_STRU ) );

	//calc crc
	crc = xorCRC ( wBuff, sizeof ( USERDATA_STRU )-1 );

	//copy crc
	wBuff[sizeof ( USERDATA_STRU ) - 1] = crc;


	//write flash
	while ( times )
	{

		bsp_sf_WriteBuffer ( wBuff, addr, sizeof ( USERDATA_STRU ) );

		//�ٶ��������Ա��Ƿ�һ��
		memset ( rBuff,0x00,sizeof ( rBuff ) );
		bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );

		ret = compareArray ( wBuff,rBuff,sizeof ( USERDATA_STRU ) );

		if ( ret == 0 )
		{
			break;
		}


		if ( ret != 0 && times == 1 )
		{
			//log_d("modify record is error\r\n");
			return 3;
		}

		times--;
	}

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d ( "�޸ļ�¼�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );

	return 0;
}

uint8_t delUserData ( uint8_t* header,uint8_t mode )
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t isFull = 0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = 0;
	uint16_t index = 0;
    HEADINFO_STRU head;

	int32_t iTime1, iTime2;
	//log_d("sizeof(USERDATA_STRU) = %d\r\n",sizeof(USERDATA_STRU));

	iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */


	//���洢�ռ��Ƿ�������
	isFull = checkFlashSpace ( mode );

	if ( isFull == 1 )
	{
		//log_d("not enough speac storage the data\r\n");
		return 1; //��ʾ�Ѿ�����
	}
    asc2bcd (head.headData.sn, header,CARD_NO_LEN_ASC, 1 );

//	ret = searchHeaderIndex ( header,mode,&index );
//	ret = searchFlashIndex(header,mode,&index,fIndex);
    index = readHead(&head,mode);

	log_d ( "searchHeaderIndex index = %d",index );

	if ( ret != 1 )
	{
		//log_d("can't find the head index\r\n");
		return 3;//��ʾδ�ҵ�����
	}

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_DATA_ADDR + index * ( sizeof ( USERDATA_STRU ) );
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_DATA_ADDR + index * ( sizeof ( USERDATA_STRU ) );
	}

	//packet write buff
	memset ( wBuff,0xFF,sizeof ( wBuff ) );

	//write flash
	while ( times )
	{
		bsp_sf_WriteBuffer ( wBuff, addr, sizeof ( USERDATA_STRU ) );

		//�ٶ��������Ա��Ƿ�һ��
		memset ( rBuff,0x00,sizeof ( rBuff ) );
		bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );

		ret = compareArray ( wBuff,rBuff,sizeof ( USERDATA_STRU ) );

		if ( ret == 0 )
		{
			break;
		}


		if ( ret != 0 && times == 1 )
		{
			//log_d("modify record is error\r\n");
			return 3;
		}

		times--;
	}

	//дɾ������
	//ɾ������ֵΪԭ������ֵ
	memset ( header,0x00,sizeof ( ( const char* ) header ) );
	sprintf ( ( char* ) header,"%08d",index );

	//log_d("need del header value = %s,\r\n",header);

	ret = writeDelHeader ( header,mode );
	if ( ret != 0 )
	{
		//log_d("write del index error\r\n");
		return 5;//��ʾδ�ҵ�����
	}

	//ɾ��ԭ����
	ret = delSourceHeader ( index,mode );
	if ( ret != 0 )
	{
		//log_d("write del source index error\r\n");
		return 6;//��ʾδ�ҵ�����
	}


	iTime2 = xTaskGetTickCount();   /* ���½���ʱ�� */
	log_d ( "�޸ļ�¼�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );

	return 0;
}


uint8_t writeDelHeader ( uint8_t* header,uint8_t mode )
{
	uint8_t times = 3;
	uint8_t readBuff[HEAD_lEN+1] = {0};
	uint8_t ret = 0;
	uint8_t headCnt[CARD_USER_LEN] = {0};
	uint8_t temp[HEAD_lEN+1] = {0};
	uint32_t addr = 0;

	if ( mode == CARD_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR + gRecordIndex.delCardNoIndex * HEAD_lEN;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_DEL_HEAD_ADDR + gRecordIndex.delUserIdIndex * HEAD_lEN;
	}

	if ( header == NULL || strlen ( ( const char* ) header ) == 0 )
	{
		//log_d("<<<<<<<<<<<<<<cardNoHeader is empty>>>>>>>>>>>>>>\r\n");
		return 1;
	}

	log_d ( "<<<<<<<<<<<<<<write del Header = %s,addr = %d>>>>>>>>>>>>>>\r\n",header,addr );

	memset ( temp,0x00,sizeof ( temp ) );
	asc2bcd ( temp, header,CARD_NO_LEN_ASC, 0 );

	while ( times )
	{
//		ret = bsp_sf_WriteBuffer (temp, addr, HEAD_lEN);
		ret = FRAM_Write ( FM24V10_1, addr, temp,HEAD_lEN );

		//�ٶ��������Ա��Ƿ�һ��
		memset ( readBuff,0x00,sizeof ( readBuff ) );
//		bsp_sf_ReadBuffer (readBuff, addr, HEAD_lEN);
		FRAM_Read ( FM24V10_1, addr, readBuff, HEAD_lEN );

		ret = compareArray ( temp,readBuff,HEAD_lEN );

		if ( ret == 0 )
		{
			break;
		}

		if ( ret != 0 && times == 1 )
		{
			//log_d("write header error\r\n");
			return 3;
		}

		times--;
	}

	//������Ҫ����
//	if ( mode == CARD_MODE )
//	{
//		gRecordIndex.delCardNoIndex++;
//		memset ( headCnt,0x00,sizeof ( headCnt ) );
//		sprintf ( ( char* ) headCnt,"%08d",gRecordIndex.delCardNoIndex );
//		ef_set_env_blob ( "DelCardHeaderIndex",headCnt,CARD_USER_LEN );

//	}
//	else
//	{
//		gRecordIndex.delUserIdIndex++;
//		memset ( headCnt,0x00,sizeof ( headCnt ) );
//		sprintf ( ( char* ) headCnt,"%08d",gRecordIndex.delUserIdIndex );
//		ef_set_env_blob ( "DelUserHeaderIndex",headCnt,CARD_USER_LEN );
//	}

//	log_d ( "gRecordIndex.delCardNoIndex = %d,gRecordIndex.delUserIdIndex = %d\r\n",gRecordIndex.delCardNoIndex,gRecordIndex.delUserIdIndex );


	return 0;

}

static uint8_t delSourceHeader ( uint16_t index,uint8_t mode )
{
	uint8_t times = 3;
	uint8_t readBuff[HEAD_lEN+1] = {0};
	uint8_t ret = 0;
	uint8_t temp[HEAD_lEN+1] = {0};
	uint32_t addr = 0;

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR + index * HEAD_lEN;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR + index * HEAD_lEN;
	}

	memset ( temp,0xFF,sizeof ( temp ) );

	while ( times )
	{
//        ret = bsp_sf_WriteBuffer (temp, addr, HEAD_lEN);
		ret = FRAM_Write ( FM24V10_1, addr, temp,HEAD_lEN );

		//�ٶ��������Ա��Ƿ�һ��
		memset ( readBuff,0x00,sizeof ( readBuff ) );
//        bsp_sf_ReadBuffer (readBuff, addr, HEAD_lEN);
		FRAM_Read ( FM24V10_1, addr, readBuff, HEAD_lEN );

		ret = compareArray ( temp,readBuff,HEAD_lEN );

		if ( ret == 0 )
		{
			break;
		}

		if ( ret != 0 && times == 1 )
		{
			log_d ( "write header error\r\n" );
			return 3;
		}

		times--;
	}

	return 0;

}




void TestFlash ( uint8_t mode )
{
	char buff[156] = {0};
    HEADINFO_STRU tmp;
	uint32_t addr = 0;
	uint32_t data_addr = 0;
	uint16_t i = 0;
	uint32_t num = 0;

	if ( buff == NULL )
	{
		//log_d("my_malloc error\r\n");
		return ;
	}
	
	ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);
    


	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		data_addr = CARD_NO_DATA_ADDR;
		num = gRecordIndex.cardNoIndex;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
		data_addr = USER_ID_DATA_ADDR;
		num = gRecordIndex.userIdIndex;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		num = gRecordIndex.delCardNoIndex;
	}
	else if ( mode == USER_DEL_MODE )
	{
		addr = USER_DEL_HEAD_ADDR;
		num = gRecordIndex.delUserIdIndex;
	}

	for ( i=0; i<num; i++ )
	{
        memset ( &tmp,0x00,sizeof ( HEADINFO_STRU ) );
		memset ( buff,0x00,sizeof ( buff ) );
		
		FRAM_Read ( FM24V10_1, addr+i*sizeof ( HEADINFO_STRU ), &tmp, sizeof ( HEADINFO_STRU ) );
		log_d("the %d card id = %x,the flash addr = %d\r\n",i,tmp.headData.id,tmp.flashAddr);


	}

	for ( i=0; i<num; i++ )
	{
		memset ( buff,0x00,sizeof ( buff ) );
		bsp_sf_ReadBuffer ( ( uint8_t* ) buff, data_addr+i * ( sizeof ( USERDATA_STRU ) ), sizeof ( USERDATA_STRU ) );
		printf ( "the %d data ====================== \r\n",i );
		dbh ( "data", buff, ( sizeof ( USERDATA_STRU ) ) );

	}

}

//ret success ��ǰID��FLASH�е�����;fail NO_FIND_HEAD
int readHead(HEADINFO_STRU *head,uint8_t mode)
{
	uint8_t i = 0;
	uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint16_t loop = 0;
	uint32_t address = 0;
	uint32_t curIndex = 0;
	int ret = 0;

    //512��ͷ��1ҳ�Ŀռ�
    HEADINFO_STRU * sectorBuff = (HEADINFO_STRU*)mymalloc(SRAMIN,sizeof(HEADINFO_STRU)*HEAD_NUM_SECTOR);    
    
    if (sectorBuff == NULL)
    {
       myfree(SRAMIN,sectorBuff);
       log_d("my_malloc error\r\n");
       return NO_FIND_HEAD;
    }

	if ( head == NULL )
	{
	    myfree(SRAMIN,sectorBuff);
		return NO_FIND_HEAD;
	}	

    log_d("2 want find head = %x\r\n",head->headData.id);
	

    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);
    

	if ( mode == CARD_MODE )
	{
		address = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == USER_MODE )
	{
		address = USER_ID_HEAD_ADDR;
		curIndex = gRecordIndex.userIdIndex;		
	}
	else if ( mode == CARD_DEL_MODE )
	{
		address = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
	else if ( mode == USER_DEL_HEAD_ADDR )
	{
		address = USER_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delUserIdIndex;
	}	
	
	multiple = curIndex / HEAD_NUM_SECTOR;
	remainder = curIndex % HEAD_NUM_SECTOR;

    //1.��ȡ��ҳ���߶�ҳ���һҳ�ĵ�ַ
    if(multiple > 0)
    {
        address += multiple * HEAD_NUM_SECTOR  * HEAD_lEN;
    }

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",address,multiple,remainder);
    
    
    //2.��ȡ���һҳ��һ�����ź����һ�����ţ�
    ret = FRAM_Read (FM24V10_1, address, sectorBuff, (remainder)* HEAD_lEN);
    
    if(ret == 0)
    {
        log_e("read fram error\r\n");
    }   

    
    log_d("head = %x,last page %x,%x\r\n",head->headData.id,sectorBuff[0].headData.id,sectorBuff[remainder-1].headData.id);

    
    if((head->headData.id >= sectorBuff[0].headData.id) && (head->headData.id <= sectorBuff[remainder-1].headData.id))
    {
        ret = Bin_Search(sectorBuff,remainder,head->headData.id);
        if(ret != NO_FIND_HEAD)
        {
            myfree(SRAMIN,sectorBuff);
            return ret;
        }
    }    
    
    for(i=0;i<multiple;i++)
    {
        address += i * HEAD_NUM_SECTOR  * HEAD_lEN;
        
        //2.��ȡ��һ�����ź����һ�����ţ�
        ret = FRAM_Read (FM24V10_1, address, sectorBuff, HEAD_NUM_SECTOR * HEAD_lEN);
        
        if(ret == 0)
        {
            log_e("read fram error\r\n");
        }  
        
        if(head->headData.id >= sectorBuff[0].headData.id && head->headData.id <= sectorBuff[HEAD_NUM_SECTOR-1].headData.id)
        {
            ret = Bin_Search(sectorBuff,HEAD_NUM_SECTOR,head->headData.id);
            if(ret != NO_FIND_HEAD)
            {
                myfree(SRAMIN,sectorBuff);
                return ret;
            }
        }
    
    }

    myfree(SRAMIN,sectorBuff);
    return NO_FIND_HEAD;

}



void sortHead(HEADINFO_STRU *head,int length)
{
    int left, right, mid;
    HEADINFO_STRU tmp;

    memset(&tmp,0x00,sizeof(tmp));
    
    for (int i = 1; i < length; i++)
    {
        /* �ҵ������е�һ���������������Ϊtmp */
        if (head[i].headData.id < head[i - 1].headData.id)
        {
            tmp = head[i];
        }
        else
        {
            continue;
        }
		/* �ҵ������е�һ���������������Ϊtmp */

		/* ���ֲ�ѯ��ʼ */
        left = 0;
        right = i - 1;
        while (left <= right)
        {
            mid = (left + right) / 2;
            if (head[mid].headData.id > tmp.headData.id)
            {
                right = mid - 1;
            }
            else
            {
                left = mid + 1;
            }
        }
		/* ���ֲ�ѯ����,��ʱa[left]>=a[i],��¼��left��ֵ */

		/* �����������б�Ҫ���������������� */
        for (int j = i; j > left; j--)
        {
            head[j] = head[j - 1];
        }
		/* �����������б�Ҫ���������������� */

		// ��leftλ�ø�ֵΪҪ�������
        head[left] = tmp;
    }
}

//return:<0,δ�ҵ���>=0��FLASH�е�����ֵ
static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target)
{
	int left = 0,right = numsSize-1,mid;
	
	while(left <= right)
	{
		mid = (left + right) / 2;//ȷ���м�Ԫ��	
		if(num[mid].headData.id > target)
		{
			right = mid-1; //mid�Ѿ���������,right��ǰ��һλ
		}
		else if(num[mid].headData.id < target)
		{
			left = mid+1;//mid�Ѿ���������,left������һλ
		}	
		else //�ж��Ƿ����
		{		    
            return num[mid].flashAddr;
		}
	}        
    return NO_FIND_HEAD;
}


void addHead(uint8_t *head,uint8_t mode)
{
    uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint32_t addr = 0;
	uint8_t ret = 0;
	uint32_t curIndex = 0;
	uint8_t i = 0;

    int32_t iTime1, iTime2;
	
    //512��ͷ��1ҳ�Ŀռ�
    HEADINFO_STRU * sectorBuff = (HEADINFO_STRU*)mymalloc(SRAMIN,sizeof(HEADINFO_STRU)*HEAD_NUM_SECTOR);    


    if(sectorBuff == NULL)
    {
        myfree(SRAMIN, sectorBuff);
        log_e("create sectorBuff is error\r\n");
        return;
    }
//    log_d("sizeof(HEADINFO_STRU) = %d\r\n",sizeof(HEADINFO_STRU));

//    memset(sectorBuff,0x00,sizeof(HEADINFO_STRU)*HEAD_NUM_SECTOR);

    
   iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */
   //1.���ж���ǰ�ж��ٸ�����;
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
		curIndex = gRecordIndex.userIdIndex;		
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
	else if ( mode == USER_DEL_MODE )
	{
		addr = USER_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delUserIdIndex;
	}	    
    
    multiple = curIndex / HEAD_NUM_SECTOR;
    remainder = curIndex % HEAD_NUM_SECTOR;

//    log_d("mode = %d,addr = %x,multiple = %d,remainder=%d\r\n",mode,addr,multiple,remainder);


    //����Ҫ��1��ʼ    
    if(multiple==0 && remainder==0)
    {
        //��һ����¼
        sectorBuff[0].flashAddr = curIndex;
        memcpy(sectorBuff[0].headData.sn,head,CARD_NO_LEN_BCD);  

//        log_d("add = %x,sectorBuff[0].headData.sn = %02x,%02x,%02x,%02x,addr = %d\r\n",addr,sectorBuff[0].headData.sn[0],sectorBuff[0].headData.sn[1],sectorBuff[0].headData.sn[2],sectorBuff[0].headData.sn[3],sectorBuff[0].flashAddr);
        
        //д�뵽�洢����
        ret = FRAM_Write ( FM24V10_1, addr, sectorBuff,1 * sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
        }        
    }
    else 
    {
        //2.׷�ӵ����һҳ��
        if(multiple > 0)
        {
            addr += multiple * HEAD_NUM_SECTOR  * sizeof(HEADINFO_STRU);  
        }

        //3.��ȡ���һҳ
        ret = FRAM_Read (FM24V10_1, addr, sectorBuff, (remainder)* sizeof(HEADINFO_STRU));
        if(ret == 0)
        {
            log_e("read fram error\r\n");
        }

//        log_d("FRAM_Read SUCCESS addr = %d,remainder = %d\r\n",addr,remainder);
        
//        for(i=0;i<remainder;i++)
//        {
//            log_d("add = %d,id =%x,sn = %02x,%02x,%02x,%02x,flashAddr = %d\r\n",addr,sectorBuff[i].headData.id,sectorBuff[i].headData.sn[0],sectorBuff[i].headData.sn[1],sectorBuff[i].headData.sn[2],sectorBuff[i].headData.sn[3],sectorBuff[i].flashAddr);
//        }        
        
        //4.��ֵ,׷����Ҫ��ӵĿ��ŵ����һ��λ��
        sectorBuff[remainder].flashAddr = curIndex;
        memcpy(sectorBuff[remainder].headData.sn,head,CARD_NO_LEN_BCD);
        
//        dbh("add head", sectorBuff[remainder].headData.sn, CARD_NO_LEN_BCD);  

        
        //5.����
        sortHead(sectorBuff,remainder+1);

//        log_d("<<<<<<<<<< >>>>>>>>>>\r\n");
        
//        for(i=0;i<remainder+1;i++)
//        {
//            log_d("add = %d,id =%x,sn =%02x,%02x,%02x,%02x,sectorBuff.flashAddr = %d\r\n",addr,sectorBuff[i].headData.id,sectorBuff[i].headData.sn[0],sectorBuff[i].headData.sn[1],sectorBuff[i].headData.sn[2],sectorBuff[i].headData.sn[3],sectorBuff[i].flashAddr);
//        }

        //6.д�뵽�洢����
        ret = FRAM_Write ( FM24V10_1, addr, sectorBuff,(remainder+1)* sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
        }        
    }


	if ( mode == CARD_MODE )
	{
        gRecordIndex.cardNoIndex++;
	}
	else if ( mode == USER_MODE )
	{
        gRecordIndex.userIdIndex++;		
	}
	else if ( mode == CARD_DEL_MODE )
	{
        gRecordIndex.delCardNoIndex++;		
	}
	else if ( mode == USER_DEL_MODE )
	{
        gRecordIndex.delUserIdIndex++;
	}

//    log_d("cardNoIndex = %d,userIdIndex = %d\r\n",gRecordIndex.cardNoIndex,gRecordIndex.userIdIndex);	
    optRecordIndex(&gRecordIndex,WRITE_PRARM);

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_e ( "add head�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );

    myfree(SRAMIN,sectorBuff);
  
}


