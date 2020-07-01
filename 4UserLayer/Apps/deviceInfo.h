/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : deviceInfo.h
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��6��28��
  ����޸�   :
  ��������   : �豸���ֲ������ü�Ĭ��ֵ
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��6��28��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __DEVICEINFO_H__
#define __DEVICEINFO_H__
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 #define UPGRADE_URL_MAX_LEN    300

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
////////////////////ģ����Ϣ////////////////////////////////////////
 //��������Ϣ
 typedef struct
 {
     uint8_t createorID[12];
     uint8_t createTime[20];
     uint8_t createCompanyId[12];
     uint8_t updateUserId[12];
     uint8_t updateUserName[20];    
 }CREATOR_INFO_STRU;
 
 typedef struct
 {
     uint8_t channelType;//ͨ������ 1���ǻ�ͨ�� 2���ݿ�
     uint8_t voiceSize;
     uint8_t templateType;//ģ��ģʽ���� 1���߷�ģʽ 2���ڼ���ģʽ
     uint8_t startTime[20];
     uint8_t endTime[20];
 }TEMPLATE_BASE_DATA_STRU;
 
 typedef struct 
 {
     uint8_t isFace;
     uint8_t isQrCode;
     uint8_t isIcCard;          
 }CALLINGWAY_STRU;
 
 typedef struct
 {
     uint8_t isPeakMode;
     uint8_t isHolidayMode;
     uint8_t isNormalMode;          
 }WORKMODE_STRU;
 
 typedef struct 
 {
     uint8_t callingWay[8];
     uint8_t beginTime[20];
     uint8_t endTime[20];
     uint8_t outsideTimeMode[20];
     uint8_t outsideTimeData[20];          
 }TEMPLATE_SET_DATA_STRU;
 
 typedef struct
 {
     uint8_t id;
     uint8_t modeType[8];
     uint8_t templateCode[20];
     uint8_t templateName[50];
     uint8_t templateStatus;
     uint8_t callingWay[8];
     uint8_t offlineProcessing;
     uint8_t peakModeCnt;
     WORKMODE_STRU workMode;
     CALLINGWAY_STRU templateCallingWay;
     CALLINGWAY_STRU peakCallingWay;
     TEMPLATE_BASE_DATA_STRU peakMode[3];
     TEMPLATE_BASE_DATA_STRU holidayMode[3];
     TEMPLATE_SET_DATA_STRU peakInfo[1];
     TEMPLATE_SET_DATA_STRU hoildayInfo[1];
     CREATOR_INFO_STRU creatorInfo;     
 }TEMPLATE_PARAM_STRU;
//////////////////////////////////////////////////////////////////
 
//����״̬
typedef enum UPGRADE_STATUS
{
    UPGRADING = 0,
    UPGRADE_COMPLETE,
    UPGRADE_FAILED
}UPGRADE_STATUS_ENUM;

//�豸״̬
typedef enum DEVICE_STATUS
{
    DEVICE_DISABLE = 0,
    DEVICE_ENABLE
}DEVICE_STATUS_ENUM;

typedef struct DEVICE_ID
{
    char downLoadFlag;       //0x00δ����SN��ʹ��MAC��ΪĬ��SN��0x01��������SN
    char qrSn[8];            //��ά���·��ı���ID
    char deviceSn[32];       //MQTT ����ʱ��SN      
}DEVICE_ID_STRU;

typedef struct UPGRADE_URL
{
    char upgradeState;//�����У�������ɣ�����ʧ��
    char url[UPGRADE_URL_MAX_LEN];//���ڴ洢�����ļ���ŵ�ַ
    char retUrl[UPGRADE_URL_MAX_LEN];//֪ͨ����������״̬������
}UPGRADE_URL_STRU;



/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
extern DEVICE_ID_STRU gDeviceId;
extern TEMPLATE_PARAM_STRU gTemplateParam;

extern uint8_t gDeviceStateFlag;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

#endif

