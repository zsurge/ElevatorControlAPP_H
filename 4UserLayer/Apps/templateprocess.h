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
    uint8_t createTime[20];
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
}SPEC_TEMPLATLE_DATA_STRU;


typedef struct 
{
    uint8_t callingWay[8];
    uint8_t beginTime[20];
    uint8_t endTime[20];
    uint8_t outsideTime[20];
    uint8_t outsideTime[20];          
}SPEC_TEMPLATELE_SET_STRU;


typedef struct
{
    uint8_t templateCode[20];
    uint8_t templateName[50];
    uint8_t templateStatus[8];
    uint8_t callingWay[8];
    uint8_t offlineProcessing[8];
    SPEC_TEMPLATLE_DATA_STRU peakMode[3];
    SPEC_TEMPLATLE_DATA_STRU hoildayMode[3];
    SPEC_TEMPLATELE_SET_STRU peakInfo[1];
    SPEC_TEMPLATELE_SET_STRU hoildayInfo[1];
    CREATOR_INFO_STRU creatorInfo;
}TMEPLATE_T;
    

//

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/




#endif

