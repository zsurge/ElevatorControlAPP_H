/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : LocalData.h
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��3��21��
  ����޸�   :
  ��������   : LocalData.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��3��21��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __LOCALDATA_H__
#define __LOCALDATA_H__

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 


#define HEAD_lEN 4               //ÿ����¼ռ4���ֽ�
#define MAX_HEAD_RECORD     2048 //���2048����¼
#define SECTOR_SIZE         4096    //ÿ��������С
#define CARD_NO_HEAD_ADDR   0x600000
#define CARD_NO_HEAD_SIZE   (HEAD_lEN*MAX_HEAD_RECORD)
#define USER_ID_HEAD_ADDR   (CARD_NO_HEAD_ADDR+CARD_NO_HEAD_SIZE)
#define USER_ID_HEAD_SIZE   (CARD_NO_HEAD_SIZE)

#define CARD_SECTOR_NUM     (CARD_NO_HEAD_SIZE/SECTOR_SIZE)
#define USER_SECTOR_NUM     CARD_SECTOR_NUM


#define CARD_NO_DATA_ADDR   0X700000
#define USER_ID_DATA_ADDR   0X800000

#define DATA_SECTOR_NUM     ((USER_ID_DATA_ADDR-CARD_NO_DATA_ADDR)/SECTOR_SIZE)

#define CARD_USER_LEN              8
#define FLOOR_ARRAY_LENGTH         16 //ÿ����ͨ�û����10����Ȩ��
#define TIME_LENGTH                16
#define RESERVE_LENGTH             32 //Ԥ���ռ�

#define CARD_MODE                   0 //��ģʽ
#define USER_MODE                   1 //U�û�IDģʽ

#define TABLE_HEAD                 0xAA



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
extern uint16_t gCurCardHeaderIndex;    //��������
extern uint16_t gCurUserHeaderIndex;    //�û�ID����
extern uint16_t gCurRecordIndex;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
typedef struct HEADER
{    
    uint8_t value[HEAD_lEN];     //��ͷ��ֵ
}HEADER_STRU;


//HEADER_STRU cardNoHeader,userIdHeader;

#pragma pack(1)
typedef struct USERDATA
{
    uint8_t head;                                   //����ͷ
    uint8_t authMode;                               //��Ȩģʽ,ˢ��=2��QR=7
    uint8_t defaultFloor;                           //Ĭ��¥��  
    uint8_t state;                                  //�û�״̬ ��Ч/��Ч/������/��ʱ��    
    uint8_t userId[CARD_USER_LEN];                  //�û�ID
    uint8_t cardNo[CARD_USER_LEN];                  //����
    uint8_t accessFloor[FLOOR_ARRAY_LENGTH];        //¥��Ȩ��
    uint8_t startTime[TIME_LENGTH];                 //�˻���Чʱ��
    uint8_t endTime[TIME_LENGTH];                   //�˻�����ʱ��    
    uint8_t timeStamp[TIME_LENGTH];                 //��ά��ʱ���
    uint8_t reserve[RESERVE_LENGTH];                //Ԥ���ռ�
    uint8_t crc;                                    //У��ֵ head~reseve
}USERDATA_STRU;
#pragma pack()



/*
typedef struct USERSTATE
{
    uint8_t isInvalid;
    uint8_t isValid;
    uint8_t isTemporary;
    uint8_t isBlackList;
}USERSTATE_STRU;
*/

void eraseHeadSector(void);
void eraseDataSector(void);
void eraseUserDataAll(void);
uint8_t writeHeader(uint8_t* header,uint8_t mode);
uint8_t searchHeaderIndex(uint8_t* header,uint8_t mode,uint16_t *index);
uint8_t writeUserData(USERDATA_STRU userData,uint8_t mode);
uint8_t readUserData(uint8_t* header,uint8_t mode,USERDATA_STRU *userData);






#endif /* __LOCALDATA_H__ */
