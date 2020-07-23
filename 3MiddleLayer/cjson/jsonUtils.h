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



/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define JSON_ITEM_MAX_LEN   1024*2 
#define CARD_NO_LEN             (8)
#define USER_ID_LEN             (8)
#define FLOOR_ARRAY_LEN         (64) //ÿ����ͨ�û����10����Ȩ��
#define TIME_LEN                (10)
#define QRID_LEN                (10)
#define TIMESTAMP_LEN           (10)






/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
//#pragma pack(1)
//typedef struct
//{
//    uint8_t authMode;                               //��Ȩģʽ,ˢ��=2��QR=7
//    uint8_t defaultFloor;                           //Ĭ��¥��
//    uint8_t qrType;                                 //QR���� 1 2 3 4
//    uint8_t qrID[QRID_LEN+1];                         //QRID
//    uint8_t userId[USER_ID_LEN+1];                  //�û�ID
//    uint8_t cardNo[CARD_NO_LEN+1];                  //����
//    char accessFloor[FLOOR_ARRAY_LEN+1];           //Ȩ��¥��
//    uint8_t startTime[TIME_LEN+1];                    //��ʼ��Чʱ��
//    uint8_t endTime[TIME_LEN+1];                      //����ʱ��    
//    uint8_t qrStarttimeStamp[TIMESTAMP_LEN+1];             //��ά�뿪ʼʱ���  
//    uint8_t qrEndtimeStamp[TIMESTAMP_LEN+1];               //��ά�����ʱ���
//    uint8_t timeStamp[TIMESTAMP_LEN+1];                    //��ά��ʱ���
//}LOCAL_USER_STRU;


//typedef struct 
//{    
//    uint8_t type;                                   //��ά������
//    uint8_t defaultFloor;                           //Ĭ��¥��    
//    uint8_t qrID[QRID_LEN+1];                         //QRID
//    uint8_t startTime[TIME_LEN+1];                    //��ʼ��Чʱ��
//    uint8_t endTime[TIME_LEN+1];                      //����ʱ�� 
//    char accessFloor[FLOOR_ARRAY_LEN+1];           //Ȩ��¥��
//}QRCODE_INFO_STRU;

#pragma pack()

//extern LOCAL_USER_STRU gLoalUserData;




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
//�洢APP�����������͵�����
SYSERRORCODE_E saveUpgradeData(uint8_t *jsonBuff);


SYSERRORCODE_E getTimePacket(uint8_t *descBuf);



//��ȡJSON����
uint8_t** GetCardArray ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t *num);




#endif



