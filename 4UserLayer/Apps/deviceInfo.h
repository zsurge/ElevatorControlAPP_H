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
 #define MQTT_TOPIC_MAX_LEN     128

 #define DEFAULT_INIVAL 0xAA55AA55
 #define DEFAULT_BASE_INIVAL 0xAA55AA55 
 #define DEFAULT_DEV_NAME "ELEVATOR"

 #define DEVICE_DISABLE 0x00
 #define DEVICE_ENABLE  0x5555AAAA

 #define WRITE_PRARM    0x01
 #define READ_PRARM     0x02


//#define DEFAULT_TEMPLATE_PARAM                                              
//{                                                                             
//    {.id = 1,
//    .modeType = "5",
//    .templateCode = "100000",
//    .templateName = "defaultName",
//    .templateStatus = 1,
//    .callingWay = "1,2,3",
//    .offlineProcessing = 2
//    }
//}


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

typedef union
{
	unsigned int iFlag;        //
	unsigned char cFlag[4];    //���Ű��ַ�
}DEVICE_SWITCH;

 
 typedef struct
 {
     DEVICE_SWITCH initFlag;   //��ʼ����־\x55\xAA\x55\xBB�϶�Ϊ������ʼ��
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

typedef struct DEVICE_ID
{
    DEVICE_SWITCH downLoadFlag;       //
    char qrSn[8];                     //��ά���·��ı���ID
    char qrSnLen;
    char deviceSn[32];                //MQTT ����ʱ��SN      
    char deviceSnLen;
}DEVICE_ID_STRU;

typedef struct UPGRADE_URL
{
    char upgradeState;//�����У�������ɣ�����ʧ��
    char url[UPGRADE_URL_MAX_LEN];//���ڴ洢�����ļ���ŵ�ַ
    char retUrl[UPGRADE_URL_MAX_LEN];//֪ͨ����������״̬������
}UPGRADE_URL_STRU;

typedef struct
{
    char publish[MQTT_TOPIC_MAX_LEN];   //����������
    char subscribe[MQTT_TOPIC_MAX_LEN]; //���ĵ�����
}MQTT_TOPIC_STRU;




typedef struct DEV_BASE_PARAM
{
    //�豸״̬
    DEVICE_SWITCH deviceState; //\x55\xAA\x55\xBB �豸����  
    
    //SN
    DEVICE_ID_STRU deviceCode;

    //��������
    UPGRADE_URL_STRU upgradeInfo;   

    //MQTT����
    MQTT_TOPIC_STRU mqttTopic;    
}DEV_BASE_PARAM_STRU;

///////////////////////FLASH���///////////////////////////////////////////

//�洢��ͷ��һЩ����ֵ
typedef struct RECORDINDEX
{
    volatile uint32_t cardNoIndex;      //��ǰ�Ѵ洢�˶��ٸ�����
    volatile uint32_t userIdIndex;      //��ǰ�Ѵ洢�˶��ٸ��û���
    volatile uint32_t delCardNoIndex;   //��ǰ�Ѵ洢�˶��ٸ���ɾ���Ŀ���
    volatile uint32_t delUserIdIndex;   //��ǰ�Ѵ洢�˶��ٸ���ɾ�����û���
}RECORDINDEX_STRU;



/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
extern TEMPLATE_PARAM_STRU gtemplateParam;
extern DEV_BASE_PARAM_STRU gDevBaseParam;
extern RECORDINDEX_STRU gRecordIndex;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

#endif

