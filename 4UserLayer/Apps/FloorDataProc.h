/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : FloorDataProc.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月23日
  最近修改   :
  功能描述   : 电梯控制器的指令处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月23日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/
#ifndef __FLOORDATAPROC_H_
#define __FLOORDATAPROC_H_


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "errorcode.h"
#include "LocalData.h"
#include "bsp_uart_fifo.h"
#include "tool.h"
#include "bsp_dipSwitch.h"
#include "easyflash.h"
#include "cmdhandle.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "jsonUtils.h"
#include "bsp_ds1302.h"
#include "ini.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_CMD_LEN 5
#define MAX_SEND_LEN 37
#define CMD_STX     0x5A


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

void packetDefaultSendBuf(uint8_t *buf);
void packetSendBuf(READER_BUFF_STRU *pQueue,uint8_t *buf);


SYSERRORCODE_E authRemote(READER_BUFF_STRU *pQueue,USERDATA_STRU *localUserData);



#endif


