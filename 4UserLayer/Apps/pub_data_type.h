/******************************************************************************

                  Copyright @1998 - 2021 BSG

 ******************************************************************************
�� �� ��   : pub_data_type.h
 					
@author  
@date 2021��2��20��
  ����޸�   :
@file pub_data_type.h
@brief ���һЩ�������������ݶ���
  �����б�   :
�޸���ʷ   :
@date 2021��2��20��
@author  
�޸�����   : �����ļ�

******************************************************************************/
#ifndef __PUB_DATA_TYPE_H__
#define __PUB_DATA_TYPE_H__

#include"stdint.h"

#define CARD_NO_LEN_ASC     8       //����ASC�볤��
#define CARD_NO_LEN_BCD     (CARD_NO_LEN_ASC/2) //����BCD�볤��
#define HEAD_lEN 8                  //ÿ����¼ռ8���ֽ�,4�ֽڿ��ţ�4�ֽ�flash������
#define MAX_HEAD_RECORD     7680   //���32768����¼
#define SECTOR_SIZE         4096    //ÿ��������С

#define MAX_HEAD_DEL_CARDNO     128   //��������ɾ��128�ſ�
#define MAX_HEAD_DEL_USERID     128   //��������ɾ��128���û�ID



#define CARD_NO_HEAD_SIZE   (HEAD_lEN*MAX_HEAD_RECORD)  //60K
#define USER_ID_HEAD_SIZE   (CARD_NO_HEAD_SIZE)
#define CARD_DEL_HEAD_SIZE  (HEAD_lEN*MAX_HEAD_DEL_CARDNO)   //1K
#define USER_DEL_HEAD_SIZE  (HEAD_lEN*MAX_HEAD_DEL_USERID)


#define CARD_HEAD_SECTOR_NUM     (CARD_NO_HEAD_SIZE/SECTOR_SIZE) //15������
#define USER_HEAD_SECTOR_NUM     (CARD_HEAD_SECTOR_NUM)

#define HEAD_NUM_SECTOR     (SECTOR_SIZE/HEAD_lEN) //ÿ�������洢512������/�û�ID

//��Ϊ�洢������
#define CARD_NO_HEAD_ADDR   0x0000
#define USER_ID_HEAD_ADDR   (CARD_NO_HEAD_ADDR+CARD_NO_HEAD_SIZE)
#define CARD_DEL_HEAD_ADDR  (USER_ID_HEAD_ADDR+USER_ID_HEAD_SIZE)
#define USER_DEL_HEAD_ADDR  (CARD_DEL_HEAD_ADDR+CARD_DEL_HEAD_SIZE)


//���������洢���ȼ���ַ
#define DEVICE_BASE_PARAM_SIZE (896)
#define DEVICE_BASE_PARAM_ADDR (USER_DEL_HEAD_ADDR+USER_DEL_HEAD_SIZE)


//��ͷ�������洢���ȼ���ַ
#define RECORD_INDEX_SIZE   (128)
#define RECORD_INDEX_ADDR   (DEVICE_BASE_PARAM_ADDR+DEVICE_BASE_PARAM_SIZE)

//�����Ĵ洢��ַ
#define DEVICE_TEMPLATE_PARAM_SIZE   (1024*2)
#define DEVICE_TEMPLATE_PARAM_ADDR  (RECORD_INDEX_ADDR+RECORD_INDEX_SIZE) //�����洢����4K�ռ�



#define CARD_NO_DATA_ADDR   0X500000
#define USER_ID_DATA_ADDR   0X900000

#define DATA_SECTOR_NUM     ((USER_ID_DATA_ADDR-CARD_NO_DATA_ADDR)/SECTOR_SIZE)


#define CARD_USER_LEN              (8)
#define FLOOR_ARRAY_LENGTH         (64) //ÿ����ͨ�û����64��Ȩ��
#define TIME_LENGTH                (10)
#define TIMESTAMP_LENGTH           (10)
#define RESERVE_LENGTH             (4) //Ԥ���ռ� Ϊ�˶��룬����һ�����������������ֽ���


#pragma pack(1)
typedef struct USERDATA
{
    uint8_t head;                                   //����ͷ
    uint8_t authMode;                               //��Ȩģʽ,ˢ��=2��QR=7
    uint8_t defaultFloor;                           //Ĭ��¥��  
    uint8_t cardState;                              //��״̬ ��Ч/��ɾ��/������/��ʱ��    
    uint8_t userState;                              //�û�״̬ ��Ч/��ɾ��
    uint8_t platformType;                           //ƽ̨���ͣ�������ƽ̨����˼��ƽ̨��
    uint8_t userId[CARD_USER_LEN+1];                  //�û�ID
    uint8_t cardNo[CARD_USER_LEN+1];                  //����
    char accessFloor[FLOOR_ARRAY_LENGTH+1];        //¥��Ȩ��
    uint8_t startTime[TIME_LENGTH+1];                 //�˻���Чʱ��
    uint8_t endTime[TIME_LENGTH+1];                   //�˻�����ʱ��    
    uint8_t timeStamp[TIME_LENGTH+1];                 //��ά��ʱ���
    uint8_t reserve[RESERVE_LENGTH+1];                //Ԥ���ռ� 
    uint8_t crc;                                    //У��ֵ head~reseve
}USERDATA_STRU;
#pragma pack()


#endif



