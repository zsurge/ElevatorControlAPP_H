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

  FLASH����
  0-0x200000����������
  0x300000    ���bin�ļ�
  0x400000    ����Ѵ洢���ţ���ɾ�����ţ��Ѵ洢�û�ID����ɾ���û�ID��
  0x500000    ��ſ�������
  0x900000    ����û�����
  0x1300000   Ԥ��

FLASH����˼·��
����0.�ȶ�ȡ��ɾ������������Ϊ�㣬�����ɾ�����һ������ֵ��ֵ������������д���ţ�
      ��ɾ������ֵ�Լ�������ִ�в���2��
    1.��ɾ������Ϊ�㣬�Կ���Ϊ�������洢��FLASH�У���Ϊ������ÿ����һ�����ţ�����������
    2.���Ź�������Ϊ�̶����ݳ��ȣ��洢ʱ��д�������ݵ�ַ+ƫ����(����*�̶�����)
ɾ��1.���ҿ��ţ����У����¼�ÿ�����ֵ���洢����ɾ���ռ��ڣ�����ɾ������ֵ����,
      ���ޣ��򷵻�δ�ҵ���
�ģ�1.���ҿ��ţ�ȷ��λ�ã��޸���Ӧ��ֵ����д�뵽FLASH
�飺1.���ҿ��ţ�ȷ��λ�ã���������Ӧ��ֵ  

******************************************************************************/
#ifndef __LOCALDATA_H__
#define __LOCALDATA_H__

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 

#define HEAD_lEN 4                  //ÿ����¼ռ4���ֽ�
#define MAX_HEAD_RECORD     32768   //���32768����¼
#define SECTOR_SIZE         4096    //ÿ��������С


#define CARD_NO_HEAD_SIZE   (HEAD_lEN*MAX_HEAD_RECORD)  //128K
#define USER_ID_HEAD_SIZE   (CARD_NO_HEAD_SIZE)
#define CARD_DEL_HEAD_SIZE  (CARD_NO_HEAD_SIZE)
#define USER_DEL_HEAD_SIZE  (CARD_NO_HEAD_SIZE)


#define CARD_HEAD_SECTOR_NUM     (CARD_NO_HEAD_SIZE/SECTOR_SIZE) //32������
#define USER_HEAD_SECTOR_NUM     (CARD_HEAD_SECTOR_NUM)
#define CARD_HEAD_DEL_SECTOR_NUM (CARD_HEAD_SECTOR_NUM)
#define USER_HEAD_DEL_SECTOR_NUM (CARD_HEAD_SECTOR_NUM)

#define HEAD_NUM_SECTOR     (SECTOR_SIZE/HEAD_lEN) //1024��


#define CARD_NO_HEAD_ADDR   0x400000
#define USER_ID_HEAD_ADDR   (CARD_NO_HEAD_ADDR+CARD_NO_HEAD_SIZE)
#define CARD_DEL_HEAD_ADDR  (USER_ID_HEAD_ADDR+USER_ID_HEAD_SIZE)
#define USER_DEL_HEAD_ADDR  (CARD_DEL_HEAD_ADDR+CARD_DEL_HEAD_SIZE)


#define CARD_NO_DATA_ADDR   0X500000
#define USER_ID_DATA_ADDR   0X900000

#define DATA_SECTOR_NUM     ((USER_ID_DATA_ADDR-CARD_NO_DATA_ADDR)/SECTOR_SIZE)


#define CARD_USER_LEN              (8)
#define FLOOR_ARRAY_LENGTH         (64) //ÿ����ͨ�û����64��Ȩ��
#define TIME_LENGTH                (10)
#define TIMESTAMP_LENGTH           (10)
#define RESERVE_LENGTH             (5) //Ԥ���ռ� Ϊ�˶��룬����һ�����������������ֽ���

#define CARD_MODE                   1 //��ģʽ
#define USER_MODE                   2 //�û�IDģʽ
#define CARD_DEL_MODE               3 //ɾ����ģʽ
#define USER_DEL_MODE               4 //ɾ���û�IDģʽ


////���ÿ�״̬Ϊ0��ɾ����
#define CARD_DEL                    0
#define CARD_VALID                  1
#define USER_DEL                    CARD_DEL
#define USER_VALID                  CARD_VALID
#define TABLE_HEAD                  0xAA



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
extern uint16_t gDelCardHeaderIndex;    //��ɾ����������
extern uint16_t gDelUserHeaderIndex;    //��ɾ���û�ID����
extern uint16_t gCurRecordIndex;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
typedef struct HEADER
{    
    uint8_t value[HEAD_lEN];     //��ͷ��ֵ
}HEADER_STRU;

typedef enum 
{
  ISFIND_NO = 0,
  ISFIND_YES 
}ISFIND_ENUM;



//HEADER_STRU cardNoHeader,userIdHeader;

#pragma pack(1)
typedef struct USERDATA
{
    uint8_t head;                                   //����ͷ
    uint8_t authMode;                               //��Ȩģʽ,ˢ��=2��QR=7
    uint8_t defaultFloor;                           //Ĭ��¥��  
    uint8_t cardState;                              //��״̬ ��Ч/��ɾ��/������/��ʱ��    
    uint8_t userState;                              //�û�״̬ ��Ч/��ɾ��
    uint8_t userId[CARD_USER_LEN+1];                  //�û�ID
    uint8_t cardNo[CARD_USER_LEN+1];                  //����
    uint8_t accessFloor[FLOOR_ARRAY_LENGTH+1];        //¥��Ȩ��
    uint8_t startTime[TIME_LENGTH+1];                 //�˻���Чʱ��
    uint8_t endTime[TIME_LENGTH+1];                   //�˻�����ʱ��    
    uint8_t timeStamp[TIME_LENGTH+1];                 //��ά��ʱ���
    uint8_t reserve[RESERVE_LENGTH+1];                //Ԥ���ռ� 
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
uint8_t writeHeader(uint8_t* header,uint8_t mode,uint32_t *headIndex);
ISFIND_ENUM searchHeaderIndex(uint8_t* header,uint8_t mode,uint16_t *index);
uint8_t writeUserData(USERDATA_STRU userData,uint8_t mode);
uint8_t readUserData(uint8_t* header,uint8_t mode,USERDATA_STRU *userData);
uint8_t modifyUserData(USERDATA_STRU userData,uint8_t mode);
uint8_t delUserData(uint8_t *header,uint8_t mode);

uint8_t writeDelHeader(uint8_t* header,uint8_t mode);



void TestFlash(uint8_t mode);






#endif /* __LOCALDATA_H__ */
