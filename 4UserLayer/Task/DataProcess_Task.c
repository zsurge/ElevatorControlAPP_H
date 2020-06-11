/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : DataProcess_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年5月15日
  最近修改   :
  功能描述   : 对刷卡/QR/远程送过来的数据进行处理
  函数列表   :
  修改历史   :
  1.日    期   : 2020年5月15日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "DataProcess_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "bsp_dipSwitch.h"
#include "FloorDataProc.h"
#include "bsp_usart6.h"
#include "malloc.h"

#define LOG_TAG    "DataProcess"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37

 
#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 6) 
#define DATAPROC_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *dataProcTaskName = "vDataProcTask"; 

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskDataProc = NULL;  


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
 
static void vTaskDataProcess(void *pvParameters);
//static READER_BUFF_STRU gtmpReaderMsg;



void CreateDataProcessTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskDataProcess,         
                (const char*    )dataProcTaskName,       
                (uint16_t       )DATAPROC_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )DATAPROC_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskDataProc);
}


static void vTaskDataProcess(void *pvParameters)
{
    uint16_t recvLen = 0;
    uint8_t buf[5] = {0};
    uint8_t crc = 0;    
    uint8_t sendBuf[38] = {0};
//    ELEVATOR_BUFF_STRU *sendBuf = &gElevtorData;

    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5A };

    uint32_t i = 0;

    
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100); /* 设置最大等待时间为100ms */ 
   
    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));    

    /* 清零 */
    ptMsg->authMode = 0; //默认为刷卡
    ptMsg->dataLen = 0;
    memset(ptMsg->data,0x00,sizeof(ptMsg->data));

    //memset(sendBuf->data,0x00,sizeof(sendBuf->data));
    
    while (1)
    {
        xReturn = xQueueReceive( xDataProcessQueue,    /* 消息队列的句柄 */
                                 (void *)&ptMsg,  /*这里获取的是结构体的地址 */
                                 xMaxBlockTime); /* 设置阻塞时间 */
        if(pdTRUE == xReturn)
        {
            //消息接收成功，发送接收到的消息
            packetSendBuf(ptMsg); 
            //packetSendBuf(ptMsg,sendBuf->data); 

            log_d("exec packetSendBuf end\r\n");

    
//            dbh("packetToElevator", (char *)sendBuf, MAX_SEND_LEN);

//            /* 使用消息队列实现指针变量的传递 */
//            if(xQueueSend(xTransDataQueue,              /* 消息队列句柄 */
//            			 (void *) &sendBuf,   /* 发送指针变量recv_buf的地址 */
//            			 (TickType_t)10) != pdPASS )
//            {
//                log_d("the queue is full!\r\n");                
//                xQueueReset(xTransDataQueue);
//            } 
//            else
//            {
//               //dbh("vTaskDataProcess send buf", sendBuf->data, MAX_RS485_LEN);
//                printf("1.%02x,%02x\r\n",sendBuf->data[11],sendBuf->data[36]);
//            }
        }

        
        /* 发送事件标志，表示任务正常运行 */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_6);  
        vTaskDelay(100);

    }

}


 
