/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : templateprocess.h
  �� �� ��   : ����
  ��������   : 2020��1��10��
  ����޸�   
  ��������   : ģ���������ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��1��10��
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __TEMPLATEPROCESS_H_
#define __TEMPLATEPROCESS_H_

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define DEVICE_ENABLE       1        
#define DEVICE_DISABLE      0

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

//��������Ϣ
typedef struct
{
    uint8_t createorID[12];
    uint8_t createTime[20];
    uint8_t createCompanyId[12];
    uint8_t updateUserId[12];
    uint8_t updateUserName[20];    
}CREATOR_INFO_STRU;



//���ݷ�ʽ
//typedef struct {
//    uint8_t isFace;
//    uint8_t isBarCode;
//    uint8_t isCard;
//    uint8_t isAppointment;
//    uint8_t isRemote;
//} CALL_MODE_T;

//����ģʽ һ��/�߷�/�ڼ���
//typedef struct {
//    uint8_t isNormalMode;
//    uint8_t isPeakMode;
//    uint8_t isHolidayMode;
//} WORK_TYPE;


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


//

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
extern TEMPLATE_PARAM_STRU gTemplateParam;

extern uint8_t gDeviceStateFlag;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/




#endif

