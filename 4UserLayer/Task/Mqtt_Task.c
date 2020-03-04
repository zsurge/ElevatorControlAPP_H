/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Mqtt_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月28日
  最近修改   :
  功能描述   : MQTT通讯处理任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月28日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "Mqtt_Task.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MQTT_TASK_PRIO	    (tskIDLE_PRIORITY + 3)
#define MQTT_STK_SIZE 		(configMINIMAL_STACK_SIZE*16)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *MqttTaskName = "vMQTTTask";      


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskMqtt = NULL;      //MQTT句柄


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskMQTT(void *pvParameters);

void CreateMqttTask(void)
{
    //创建LED任务
    xTaskCreate((TaskFunction_t )vTaskMQTT,         
                (const char*    )MqttTaskName,       
                (uint16_t       )MQTT_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )MQTT_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskMqtt);  
}

static void vTaskMQTT(void *pvParameters)
{
    printf("start vLwipComTask\r\n");
    mqtt_thread();
}


