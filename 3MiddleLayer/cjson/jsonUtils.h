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
#define CARD_NO_LEN             8
#define USER_ID_LEN             4
#define FLOOR_ARRAY_LEN         16 //每个普通用户最多10个层权限
#define TIME_LEN                16

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
#pragma pack(1)
typedef struct
{
    uint8_t userId[USER_ID_LEN+1];                  //用户ID
    uint8_t cardNo[CARD_NO_LEN+1];                  //卡号
    uint8_t accessFloor[FLOOR_ARRAY_LEN];           //楼层权限
    uint8_t startTime[TIME_LEN];                    //开始有效时间
    uint8_t endTime[TIME_LEN];                      //结束时间    
    uint8_t timeStamp[16];                          //二维码时间戳
    uint8_t authMode;                               //鉴权模式,刷卡=2；QR=7
    uint8_t defaultFloor;                           //默认楼层        
    
}LOCAL_USER_STRU;
#pragma pack()

extern LOCAL_USER_STRU gLoalUserData;


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

//打包
SYSERRORCODE_E packetJson(uint8_t *buf);

//
SYSERRORCODE_E parseJson(uint8_t *json);

uint8_t packetPayload(LOCAL_USER_STRU *localUserData,uint8_t *descJson);



//保存模板信息
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff);


#endif



