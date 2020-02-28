/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Mqtt_Task.h
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月28日
  最近修改   :
  功能描述   : Mqtt_Task.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月28日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
#ifndef __MQTT_TASK_H__
#define __MQTT_TASK_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "pub_options.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
extern TaskHandle_t xHandleTaskMqtt;      //MQTT处理流程
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
void CreateMqttTask(void);









#endif /* __MQTT_TASK_H__ */
