/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : all_task.h
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月25日
  最近修改   :
  功能描述   : 所有任务头文件合集
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月25日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

 *----------------------------------------------*/
 //任务优先级 

#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define READER_TASK_PRIO	( tskIDLE_PRIORITY + 1)
#define QR_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define MQTT_TASK_PRIO	    ( tskIDLE_PRIORITY + 3)
#define COMM_TASK_PRIO		( tskIDLE_PRIORITY + 4)
#define START_TASK_PRIO		( tskIDLE_PRIORITY + 5)

//任务分配空间    

#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)
#define START_STK_SIZE 	    (configMINIMAL_STACK_SIZE*4)
#define QR_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)
#define READER_STK_SIZE     (configMINIMAL_STACK_SIZE*4)
#define HANDSHAKE_STK_SIZE  (configMINIMAL_STACK_SIZE*2)

#define MQTT_STK_SIZE       (configMINIMAL_STACK_SIZE*16)
  

//任务句柄

extern  TaskHandle_t xHandleTaskComm  ;      //跟主机通讯
extern  TaskHandle_t xHandleTaskReader ;   //韦根读卡器
extern  TaskHandle_t xHandleTaskQr  ;       //二维码读头
extern  TaskHandle_t xHandleTaskStart  ;    //看门狗
extern  TaskHandle_t xHandleTaskHandShake  ;    // 握手

extern  TaskHandle_t xHandleTaskMqtt  ;      //MQTT 测试



//任务函数


void vTaskReader(void *pvParameters);
void vTaskQR(void *pvParameters);
void vTaskStart(void *pvParameters);
void vTaskHandShake(void *pvParameters);
void vTaskMqttTest(void *pvParameters);

void vTaskComm(void *pvParameters);

