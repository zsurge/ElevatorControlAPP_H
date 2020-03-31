/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : jsonUtils.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��12��19��
  ����޸�   :
  ��������   : JSON���ݴ���
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��12��19��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __JSONUTILS_H__
#define __JSONUTILS_H__

#include "errorcode.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"

#define LOG_TAG    "jsonutils"
#include "elog.h"

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define JSON_ITEM_MAX_LEN   1024*1 
#define CARD_NO_LEN             8
#define USER_ID_LEN             4
#define FLOOR_ARRAY_LEN         16 //ÿ����ͨ�û����10����Ȩ��
#define TIME_LEN                16
#define QRID_LEN                   6
#define TIMESTAMP_LEN           10


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
#pragma pack(1)
typedef struct
{
    uint8_t authMode;                               //��Ȩģʽ,ˢ��=2��QR=7
    uint8_t defaultFloor;                           //Ĭ��¥��
    uint8_t qrID[QRID_LEN];                         //QRID
    uint8_t userId[USER_ID_LEN+1];                  //�û�ID
    uint8_t cardNo[CARD_NO_LEN+1];                  //����
    uint8_t accessFloor[FLOOR_ARRAY_LEN];           //¥��Ȩ��
    uint8_t startTime[TIME_LEN];                    //��ʼ��Чʱ��
    uint8_t endTime[TIME_LEN];                      //����ʱ��    
    uint8_t qrStarttimeStamp[TIMESTAMP_LEN];             //��ά�뿪ʼʱ���  
    uint8_t qrEndtimeStamp[TIMESTAMP_LEN];               //��ά�����ʱ���
    uint8_t timeStamp[TIMESTAMP_LEN];                    //��ά��ʱ���
}LOCAL_USER_STRU;
#pragma pack()

typedef struct 
{    
    uint8_t tagFloor;                               //Ŀ��¥��    
    uint8_t qrID[QRID_LEN];                         //QRID
    uint8_t qrStarttimeStamp[TIMESTAMP_LEN];             //��ά�뿪ʼʱ���  
    uint8_t qrEndtimeStamp[TIMESTAMP_LEN];               //��ά�����ʱ���  
    uint8_t startTime[TIME_LEN];                    //��ʼ��Чʱ��
    uint8_t endTime[TIME_LEN];                      //����ʱ�� 
}QRCODE_INFO_STRU;

extern LOCAL_USER_STRU gLoalUserData;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

//���ӻ����޸�JSON���ݰ�
SYSERRORCODE_E modifyJsonItem(const uint8_t *srcJson,const uint8_t *item,const uint8_t *value,uint8_t isSubitem,uint8_t *descJson);

//��ȡָ����Ŀ��ֵ
uint8_t* GetJsonItem ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t isSubitem);

//ͨ�ú�������ɻ��ķ������ݰ�
uint8_t* packetBaseJson(uint8_t *jsonBuff);

//���豸��Ϣ���д��
SYSERRORCODE_E PacketDeviceInfo ( const uint8_t* jsonBuff,const uint8_t* descJson);

//���APP�����������͵�����
SYSERRORCODE_E upgradeDataPacket(uint8_t *descBuf);



uint8_t packetPayload(LOCAL_USER_STRU *localUserData,uint8_t *descJson);



//����ģ����Ϣ
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff);

//����QRCODE����
uint8_t parseQrCode(uint8_t *jsonBuff,QRCODE_INFO_STRU *qrCodeInfo);



#endif



