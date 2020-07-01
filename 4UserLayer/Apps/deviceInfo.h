/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : deviceInfo.h
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年6月28日
  最近修改   :
  功能描述   : 设备各种参数设置及默认值
  函数列表   :
  修改历史   :
  1.日    期   : 2020年6月28日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
#ifndef __DEVICEINFO_H__
#define __DEVICEINFO_H__
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 #define UPGRADE_URL_MAX_LEN    300

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
////////////////////模板信息////////////////////////////////////////
 //创建人信息
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
     uint8_t channelType;//通道类型 1、智慧通道 2、梯控
     uint8_t voiceSize;
     uint8_t templateType;//模板模式类型 1、高峰模式 2、节假日模式
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
 
//升级状态
typedef enum UPGRADE_STATUS
{
    UPGRADING = 0,
    UPGRADE_COMPLETE,
    UPGRADE_FAILED
}UPGRADE_STATUS_ENUM;

//设备状态
typedef enum DEVICE_STATUS
{
    DEVICE_DISABLE = 0,
    DEVICE_ENABLE
}DEVICE_STATUS_ENUM;

typedef struct DEVICE_ID
{
    char downLoadFlag;       //0x00未下载SN，使用MAC做为默认SN；0x01，已下载SN
    char qrSn[8];            //二维码下发的本机ID
    char deviceSn[32];       //MQTT 订阅时的SN      
}DEVICE_ID_STRU;

typedef struct UPGRADE_URL
{
    char upgradeState;//升级中，升级完成，升级失败
    char url[UPGRADE_URL_MAX_LEN];//用于存储升级文件存放地址
    char retUrl[UPGRADE_URL_MAX_LEN];//通知服务器升级状态的数据
}UPGRADE_URL_STRU;



/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
extern DEVICE_ID_STRU gDeviceId;
extern TEMPLATE_PARAM_STRU gTemplateParam;

extern uint8_t gDeviceStateFlag;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

#endif

