/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : templateprocess.h
  版 本 号   : 初稿
  生成日期   : 2020年1月10日
  最近修改   
  功能描述   : 模版参数处理头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年1月10日
    修改内容   : 创建文件

******************************************************************************/
#ifndef __TEMPLATEPROCESS_H_
#define __TEMPLATEPROCESS_H_

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

//创建人信息
typedef struct
{
    uint8_t createorID[12];
    uint8_t createTime[20];
    uint8_t createCompanyId[12];
    uint8_t updateUserId[12];
    uint8_t updateUserName[20];
    uint8_t createTime[20];
}CREATOR_INFO_STRU;



//呼梯方式
//typedef struct {
//    uint8_t isFace;
//    uint8_t isBarCode;
//    uint8_t isCard;
//    uint8_t isAppointment;
//    uint8_t isRemote;
//} CALL_MODE_T;

//工作模式 一般/高峰/节假日
//typedef struct {
//    uint8_t isNormalMode;
//    uint8_t isPeakMode;
//    uint8_t isHolidayMode;
//} WORK_TYPE;


typedef struct
{
    uint8_t channelType;//通道类型 1、智慧通道 2、梯控
    uint8_t voiceSize;
    uint8_t templateType;//模板模式类型 1、高峰模式 2、节假日模式
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
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/




#endif

