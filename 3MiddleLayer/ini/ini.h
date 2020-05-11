/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : ini.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月28日
  最近修改   :
  功能描述   : 对参数进行操作
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月28日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#ifndef __INI_H
#define __INI_H

#include "easyflash.h"
#include "version.h"
#include "string.h"
#include "templateprocess.h"
#include "tool.h"
#include "LocalData.h"
#include "eth_cfg.h"
#include "cJSON.h"
#include "errorcode.h"
#include "calcDevNO.h"

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


void readTemplateData(void);

void readCardAndUserIdIndex(void);

void RestoreDefaultSetting(void);

void SystemUpdate(void);

void readDevState(void);

void SaveDevState(char state);

void ReadLocalDevSn(void);


uint8_t packetPayload(USERDATA_STRU *localUserData,uint8_t *descJson);
//解析QRCODE数据
uint8_t parseQrCode(uint8_t *jsonBuff,USERDATA_STRU *qrCodeInfo);


#endif
