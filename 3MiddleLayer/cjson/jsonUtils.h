/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : jsonUtils.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月19日
  最近修改   :
  功能描述   : JSON数据处理
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月19日
    作    者   : 张舵
    修改内容   : 创建文件

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
 * 包含头文件                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define JSON_ITEM_MAX_LEN   1024*1 
#define CARD_NO_LEN             (8)
#define USER_ID_LEN             (4)
#define FLOOR_ARRAY_LEN         (16) //每个普通用户最多10个层权限
#define TIME_LEN                (10)
#define QRID_LEN                (10)
#define TIMESTAMP_LEN           (10)

//设备在线状态
#define ON_LINE                 1
#define OFF_LINE                (-1)

//呼梯状态：1、成功 0/2失败，3 QR设备已禁用  
#define CALL_OK                 1
#define CALL_NG                 2
#define CALL_ERR                0
#define QR_DISABLE              3

//进出方向 1、进 2、出
#define DIRECTION_IN            1
#define DIRECTION_OUT           2


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
#pragma pack(1)
typedef struct
{
    uint8_t authMode;                               //鉴权模式,刷卡=2；QR=7
    uint8_t defaultFloor;                           //默认楼层
    uint8_t qrType;                                 //QR类型 1 2 3 4
    uint8_t qrID[QRID_LEN+1];                         //QRID
    uint8_t userId[USER_ID_LEN+1];                  //用户ID
    uint8_t cardNo[CARD_NO_LEN+1];                  //卡号
    uint8_t accessFloor[FLOOR_ARRAY_LEN+1];           //楼层权限
    uint8_t startTime[TIME_LEN+1];                    //开始有效时间
    uint8_t endTime[TIME_LEN+1];                      //结束时间    
    uint8_t qrStarttimeStamp[TIMESTAMP_LEN+1];             //二维码开始时间戳  
    uint8_t qrEndtimeStamp[TIMESTAMP_LEN+1];               //二维码结束时间戳
    uint8_t timeStamp[TIMESTAMP_LEN+1];                    //二维码时间戳
}LOCAL_USER_STRU;


typedef struct 
{    
    uint8_t tagFloor;                               //目标楼层    
    uint8_t type;                                   //二维码类型
    uint8_t qrID[QRID_LEN+1];                         //QRID
    uint8_t qrStarttimeStamp[TIMESTAMP_LEN+1];        //二维码开始时间戳  
    uint8_t qrEndtimeStamp[TIMESTAMP_LEN+1];          //二维码结束时间戳  
    uint8_t startTime[TIME_LEN+1];                    //开始有效时间
    uint8_t endTime[TIME_LEN+1];                      //结束时间 
    uint16_t openNum;                               //允许呼梯次数    
}QRCODE_INFO_STRU;

#pragma pack()

//extern LOCAL_USER_STRU gLoalUserData;




/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

//增加或者修改JSON数据包
SYSERRORCODE_E modifyJsonItem(const uint8_t *srcJson,const uint8_t *item,const uint8_t *value,uint8_t isSubitem,uint8_t *descJson);

//获取指定项目的值
uint8_t* GetJsonItem ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t isSubitem);

//通用函数，组成基的返回数据包
uint8_t* packetBaseJson(uint8_t *jsonBuff);

//对设备信息进行打包
SYSERRORCODE_E PacketDeviceInfo ( const uint8_t* jsonBuff,const uint8_t* descJson);

//打包APP升级后需上送的数据
SYSERRORCODE_E upgradeDataPacket(uint8_t *descBuf);



uint8_t packetPayload(LOCAL_USER_STRU *localUserData,uint8_t *descJson);



//保存模板信息
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff);

//解析QRCODE数据
uint8_t parseQrCode(uint8_t *jsonBuff,QRCODE_INFO_STRU *qrCodeInfo);






#endif



