/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Key_Task.h
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月25日
  最近修改   :
  功能描述   : Key_Task.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月25日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
#ifndef __KEY_TASK_H__
#define __KEY_TASK_H__

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
extern  TaskHandle_t xHandleTaskKey  ;      //按键

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

void CreateKeyTask(void *pvParameters);




#endif /* __KEY_TASK_H__ */
