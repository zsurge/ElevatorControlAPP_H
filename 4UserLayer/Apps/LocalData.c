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
#include "ini.h"


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

//USERDATA_STRU gUserDataStru = {0};

HEADINFO_STRU gSectorBuff[512] = {0};

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static uint8_t checkFlashSpace ( uint8_t mode );

//static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target);
static int Bin_Search_addr(uint32_t startAddr,int numsSize,int target);


uint8_t writeUserData ( USERDATA_STRU *userData,uint8_t mode )
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t isFull = 0;
	uint8_t crc=0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = 0;
    uint8_t head[8] = {0};
    int tmpIndex = 0;
	HEADINFO_STRU tmpHead;

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

    memcpy(tmpHead.headData.sn,head,CARD_NO_LEN_BCD);
    tmpIndex = readHead(&tmpHead,mode);

    //���Ż�����USERID�Ѵ���
    if(tmpIndex != NO_FIND_HEAD)
    {
        return 4;
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


//add 2020.12.24 ���ҿ��ţ����У���ֱ�Ӹ��ǵ�ǰ���Ŷ�Ӧ��FLASH�������ޣ����½�
int modifyCardData(USERDATA_STRU *userData)
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t crc=0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = CARD_NO_DATA_ADDR;	
	
    uint8_t head[4] = {0};
    int tmpIndex = 0;
    uint32_t flashIndex = 0;
	HEADINFO_STRU tmpHead;
    
	int32_t iTime1, iTime2;

	iTime1 = xTaskGetTickCount();	/* ���¿�ʼʱ�� */

    asc2bcd ( head, userData->cardNo,CARD_NO_LEN_ASC, 1 );	
    
    memcpy(tmpHead.headData.sn,head,CARD_NO_LEN_BCD);
    
    tmpIndex = readHead(&tmpHead,CARD_MODE);

    //�����Ѵ���,ֱ�Ӹ�д��Ӧ��FLASH����
    if(tmpIndex != NO_FIND_HEAD)
    {
        //��дFLASH
        addr = CARD_NO_HEAD_ADDR;
        addr += tmpIndex  * sizeof(HEADINFO_STRU);
        
        tmpHead.headData.id = 0;    
        tmpHead.flashAddr = 0;
        ret = FRAM_Read (FM24V10_1, addr, &tmpHead,sizeof(HEADINFO_STRU)); 
    	if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }  
    	//��ȡFLASH����
        flashIndex = tmpHead.flashAddr;	  

        log_d("delHead head =%x,delHead flashIndex = %d\r\n",tmpHead.headData.id,tmpHead.flashAddr); 
        
        addr = CARD_NO_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );

        bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );

//        dbh("modifyCardData", rBuff, sizeof(USERDATA_STRU));  
    }
    else
    {
        //���¼ӿ��ţ���дFLASH
        //д��ͷ
    	addHead(head,CARD_MODE);

        //����ǰFLASH������
    	ClearRecordIndex();
        optRecordIndex(&gRecordIndex,READ_PRARM);
        
    	addr = CARD_NO_DATA_ADDR + (gRecordIndex.cardNoIndex-1) * ( sizeof ( USERDATA_STRU ) );
    }

	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

	//copy to buff
	memcpy ( wBuff, userData, sizeof ( USERDATA_STRU )-1 );

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



char readUserData ( uint8_t* header,uint8_t mode,USERDATA_STRU* userData )
{
	uint8_t rBuff[255] = {0};
	uint8_t crc = 0;
	uint8_t ret = 0;
	int index = 0;
	uint32_t addr = 0;
	HEADINFO_STRU head;
	uint32_t flashIndex = 0;

    int32_t iTime1, iTime2;

	if ( header == NULL )
	{
		//log_d("invalid head\r\n");
		return 1; //��ʾ����Ĳ���
	}
	
	asc2bcd (head.headData.sn, header,CARD_NO_LEN_ASC, 1 );
	
	
    iTime1 = xTaskGetTickCount();	/* ���¿�ʼʱ�� */

    log_d("1 want find head = %x\r\n",head.headData.id);

    //������head�����кţ�Ҫ��֪��FLASH������������Ҫ��ȡ
	index = readHead(&head,mode);
	
    iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d("readUserData�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1);	

	if(index == NO_FIND_HEAD)
	{
        log_d("no find head\r\n");
        return NO_FIND_HEAD;
	}

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
	}
	
    head.flashAddr = 0;
    head.headData.id = 0;	
    ret = FRAM_Read (FM24V10_1, addr+index*CARD_USER_LEN, &head,CARD_USER_LEN);    
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD; 
    }

    flashIndex = head.flashAddr;

    log_d("readUserData head = %x,readUserData flashIndex = %d\r\n",head.headData.id,flashIndex);

	if ( mode == CARD_MODE )
	{
        addr = CARD_NO_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );
	}
	else if(mode == USER_MODE)
	{
		addr = USER_ID_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );
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





uint8_t delUserData ( uint8_t* header,uint8_t mode )
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = 0;
	
    uint8_t headBcd[4] = {0};
    int flashIndex = 0;
	
	
	

	int32_t iTime1, iTime2;
	//log_d("sizeof(USERDATA_STRU) = %d\r\n",sizeof(USERDATA_STRU));

	iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */

    asc2bcd (headBcd, header,CARD_NO_LEN_ASC, 1 );

	//��ȡFLASH����
    flashIndex = delHead(headBcd,mode);	

	if ( flashIndex == NO_FIND_HEAD)
	{
		return 6;//��ʾδ�ҵ�����
	}	
	
	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );
	}

	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

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
	
	iTime2 = xTaskGetTickCount();   /* ���½���ʱ�� */
	log_d ( "�޸ļ�¼�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );

	return 0;
}

int delHead(uint8_t *headBuff,uint8_t mode)
{
	int ret = 0;
	uint8_t multiple = 0;
	uint16_t remainder = 0;
    HEADINFO_STRU tmpCard;
    uint32_t addr = 0;    
    int num = 0;
    uint32_t flashIndex = 0;

    if ( headBuff == NULL )
	{
		return NO_FIND_HEAD;
	}	
	
	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
	}
	else if ( mode == USER_DEL_HEAD_ADDR )
	{
		addr = USER_DEL_HEAD_ADDR;
	}	

    memcpy(tmpCard.headData.sn,headBuff,sizeof(tmpCard.headData.sn));

    //1.����Ҫɾ���ĶԵ�����
    ret = readHead(&tmpCard,mode);

    if(ret == NO_FIND_HEAD)
    {
        return NO_FIND_HEAD;
    }

    //2.����Ҫɾ�����ŵĵ�ַ
    multiple = ret / HEAD_NUM_SECTOR;
	remainder = ret % HEAD_NUM_SECTOR;	

	log_d("multiple = %d,remainder=%d\r\n",multiple,remainder);
	
    addr += ret  * sizeof(HEADINFO_STRU);
    
    tmpCard.headData.id = 0;    
    tmpCard.flashAddr = 0;
    ret = FRAM_Read (FM24V10_1, addr, &tmpCard,sizeof(HEADINFO_STRU)); 
	if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD; 
    }  
	//��ȡFLASH����
    flashIndex = tmpCard.flashAddr;	  

    log_d("delHead head =%x,delHead flashIndex = %d\r\n",tmpCard.headData.id,tmpCard.flashAddr);

    //3.��Ҫɾ����ͷ��ȫ0    
    tmpCard.headData.id = 0;    
    tmpCard.flashAddr = 0;
    ret = FRAM_Write ( FM24V10_1, addr, &tmpCard,1* sizeof(HEADINFO_STRU));    
    if(ret == 0)
    {
        log_e("write fram error\r\n");
        return NO_FIND_HEAD;
    } 


	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		
        ClearRecordIndex();
        optRecordIndex(&gRecordIndex,READ_PRARM);        
        num = gRecordIndex.cardNoIndex % HEAD_NUM_SECTOR;		
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
        ClearRecordIndex();
        optRecordIndex(&gRecordIndex,READ_PRARM);        
        num = gRecordIndex.userIdIndex % HEAD_NUM_SECTOR;		
	}
    	
    //4.����һҳ��������
    if(multiple >= 1)
    {
        addr += (multiple-1) * HEAD_NUM_SECTOR  * sizeof(HEADINFO_STRU);
        num = HEAD_NUM_SECTOR;
    }

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));  
    log_d("del add = %x,num = %d\r\n",addr,num);
    
    //��һҳ����
    ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, num* sizeof(HEADINFO_STRU));
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD;
    }
    
//    for(int i=0;i<num;i++)
//    {
//        log_d("befare head id =%x,flashAddr = %d\r\n",gSectorBuff[i].headData.id,gSectorBuff[i].flashAddr);
//    }  
    
    
    //����
    //qSortCard(gSectorBuff,num);  
    sortHead(gSectorBuff,num);  
    //д������
    ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,num* sizeof(HEADINFO_STRU));
    if(ret == 0)
    {
        log_e("write fram error\r\n");
        return NO_FIND_HEAD;
    }      

//    for(int i=0;i<num;i++)
//    {
//        log_d("after head id =%x,flashAddr = %d\r\n",gSectorBuff[i].headData.id,gSectorBuff[i].flashAddr);
//    }  


    log_d("qSortCard success\r\n");
    

    
    return flashIndex;
  
}



uint8_t writeDelHeader ( uint8_t* header,uint8_t mode )
{
	uint8_t times = 3;
	uint8_t readBuff[HEAD_lEN+1] = {0};
	uint8_t ret = 0;
	
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
	
	uint32_t address = 0;
	uint32_t curIndex = 0;
	int ret = 0;
    int32_t iTime1, iTime2;

    //512��ͷ��1ҳ�Ŀռ�
    HEADINFO_STRU firstData,lastData;
    
	if ( head == NULL )
	{
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
    address += multiple * HEAD_NUM_SECTOR  * HEAD_lEN;

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",address,multiple,remainder);
    
    
    //2.��ȡ���һҳ��һ�����ź����һ�����ţ�
    firstData.headData.id = 0;
    lastData.headData.id = 0;   

    iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */
    
    ret = FRAM_Read (FM24V10_1, address, &firstData,CARD_USER_LEN);
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD; 
    }
    
    ret = FRAM_Read (FM24V10_1, address+(remainder-1)* CARD_USER_LEN, &lastData,CARD_USER_LEN); 
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD;
    }  
    log_d("head = %x,last page %x,%x\r\n",head->headData.id,firstData.headData.id,lastData.headData.id);


    
    if((head->headData.id >= firstData.headData.id) && (head->headData.id <= lastData.headData.id))
    {
//        ret = Bin_Search(sectorBuff,remainder,head->headData.id);
        ret = Bin_Search_addr(address,remainder,head->headData.id); 
        
        if(ret != NO_FIND_HEAD)
        {
            //ֱ�ӷ��ص�ǰĿ��ֵ��FLASH����
            iTime2 = xTaskGetTickCount();   /* ���½���ʱ�� */
            log_d ( "find it��use %d ms,head addr = %d\r\n",iTime2 - iTime1,multiple*HEAD_NUM_SECTOR+ret);              
            return multiple*HEAD_NUM_SECTOR+ret;
        }
    }    
    
    for(i=0;i<multiple;i++)
    {
        address = CARD_NO_HEAD_ADDR;//���㿪ʼ��;
        address += i * HEAD_NUM_SECTOR  * CARD_USER_LEN;
        
        
        //2.��ȡ��һ�����ź����һ�����ţ�
        firstData.headData.id = 0;
        lastData.headData.id = 0;   
        
        ret = FRAM_Read (FM24V10_1, address, &firstData,CARD_USER_LEN);
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }
        
        ret = FRAM_Read (FM24V10_1, address+(HEAD_NUM_SECTOR-1)* CARD_USER_LEN, &lastData,CARD_USER_LEN); 
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD;
        }  
        log_d("head = %x,last page %x,%x\r\n",head->headData.id,firstData.headData.id,lastData.headData.id);

        
        if(head->headData.id >= firstData.headData.id && head->headData.id <= lastData.headData.id)
        {
//            ret = Bin_Search(sectorBuff,HEAD_NUM_SECTOR,head->headData.id);
            ret = Bin_Search_addr(address,HEAD_NUM_SECTOR,head->headData.id);       
            if(ret != NO_FIND_HEAD)
            {
            	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
            	log_d ( "find it��use %d ms,index = %d\r\n",iTime2 - iTime1,i*HEAD_NUM_SECTOR+ret);      
                
                return i*HEAD_NUM_SECTOR+ret;
            }

        }
    
    }
    
	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_d ( "read all Head��use %d ms\r\n",iTime2 - iTime1 );    

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


void manualSortCard(void)
{
    uint8_t multiple = 0;
	uint16_t remainder = 0;
	int ret = 0;

	int i = 0;
	uint32_t addr = CARD_NO_HEAD_ADDR;

    int32_t iTime1, iTime2;   
       
   //1.���ж���ǰ�ж��ٸ�����;
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);
    
	addr = CARD_NO_HEAD_ADDR;    
    multiple = gRecordIndex.cardNoIndex / HEAD_NUM_SECTOR;
    remainder = gRecordIndex.cardNoIndex % HEAD_NUM_SECTOR;

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));
    iTime1 = xTaskGetTickCount();   /* ���¿�ʼʱ�� */
    if(remainder != 0)
    {
        //2.�������һҳ��ַ
        addr += multiple * HEAD_NUM_SECTOR  * sizeof(HEADINFO_STRU);    
        
        //3.��ȡ���һҳ
        ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, remainder* sizeof(HEADINFO_STRU));
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return ;
        } 
        
        //5.����        
        sortHead(gSectorBuff,remainder);   
        
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,remainder* sizeof(HEADINFO_STRU));        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
            return ;
        }          
    }    

    
    for(i=0;i<multiple;i++)
    {
        addr = CARD_NO_HEAD_ADDR;//���㿪ʼ��;
        addr += i * HEAD_NUM_SECTOR  * CARD_USER_LEN; 
        memset(gSectorBuff,0x00,sizeof(gSectorBuff));
        
        //3.����ǰҳ      
        ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, HEAD_NUM_SECTOR* sizeof(HEADINFO_STRU));
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return ;
        }         
        //����
        sortHead(gSectorBuff,HEAD_NUM_SECTOR); 
        //д������
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,HEAD_NUM_SECTOR* sizeof(HEADINFO_STRU));        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
            return ;
        }        
     }

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_e( "sort all card success��use time: %dms\r\n",iTime2 - iTime1 );  	
}


//return:<0,δ�ҵ���>=0��FLASH�е�����ֵ
//static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target)
//{
//	int left = 0,right = numsSize-1,mid;
//	
//	while(left <= right)
//	{
//		mid = (left + right) / 2;//ȷ���м�Ԫ��	
//		if(num[mid].headData.id > target)
//		{
//			right = mid-1; //mid�Ѿ���������,right��ǰ��һλ
//		}
//		else if(num[mid].headData.id < target)
//		{
//			left = mid+1;//mid�Ѿ���������,left������һλ
//		}	
//		else //�ж��Ƿ����
//		{		    
//            return num[mid].flashAddr;
//		}
//	}        
//    return NO_FIND_HEAD;
//}

//���ز��ҵ�ֵ��FLASH����
static int Bin_Search_addr(uint32_t startAddr,int numsSize,int target)
{
	int low = 0,high = numsSize-1,mid;
	HEADINFO_STRU tmpData;
	uint8_t ret = 0;
	    
	while(low <= high)
	{
		mid = low + ((high - low) >> 1); //��ȡ�м�ֵ

        ret = FRAM_Read (FM24V10_1, startAddr+(mid)*CARD_USER_LEN, &tmpData, (1)* CARD_USER_LEN);
      
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }  
        
		if(tmpData.headData.id == target)
		{
		    //return tmpData.flashAddr;
		    return mid;
		}
		else if(tmpData.headData.id < target)
		{
		    low = mid+1;//mid�Ѿ���������,low������һλ
		}
		else
		{
		    high = mid-1; //mid�Ѿ���������,right��ǰ��һλ
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
	

    int32_t iTime1, iTime2;	
    
    
    
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

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));

    //����Ҫ��1��ʼ    
    if(multiple==0 && remainder==0)
    {
        //��һ����¼��ֵ
        gSectorBuff[0].flashAddr = curIndex;
        memcpy(gSectorBuff[0].headData.sn,head,CARD_NO_LEN_BCD);
        
        
        //д�뵽�洢����
        ret = FRAM_Write ( FM24V10_1, addr,  gSectorBuff,1 * sizeof(HEADINFO_STRU));
        
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
        
        memset(gSectorBuff,0x00,sizeof(gSectorBuff));
        
        //3.��ȡ���һҳ
        ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, (remainder)* sizeof(HEADINFO_STRU));
        if(ret == 0)
        {
            log_e("read fram error\r\n");
        }
        
        //4.��ֵ,׷����Ҫ��ӵĿ��ŵ����һ��λ��
        gSectorBuff[remainder].flashAddr = curIndex;
        memcpy(gSectorBuff[remainder].headData.sn,head,CARD_NO_LEN_BCD);

        log_d("gSectorBuff[remainder].headData.id = %x gSectorBuff[remainder].flashAddr = %d\r\n",gSectorBuff[remainder].headData.id,gSectorBuff[remainder].flashAddr);

       //5.����
        sortHead(gSectorBuff,remainder+1);

        //6.д�뵽�洢����
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,(remainder+1)* sizeof(HEADINFO_STRU));
        
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

    log_d("cardNoIndex = %d,userIdIndex = %d\r\n",gRecordIndex.cardNoIndex,gRecordIndex.userIdIndex);	
    optRecordIndex(&gRecordIndex,WRITE_PRARM);

	iTime2 = xTaskGetTickCount();	/* ���½���ʱ�� */
	log_e ( "add head�ɹ�����ʱ: %dms\r\n",iTime2 - iTime1 );

  
}


