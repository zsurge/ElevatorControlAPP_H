/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : BarCode_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月27日
  最近修改   :
  功能描述   : 条码处理任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月27日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "bsp_uart_fifo.h"
#include "CmdHandle.h"
#include "tool.h"

#define LOG_TAG    "BarCode"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define BARCODE_TASK_PRIO		(tskIDLE_PRIORITY + 1)
#define BARCODE_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *BarCodeTaskName = "vBarCodeTask";  

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskBarCode = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskBarCode(void *pvParameters);

void CreateBarCodeTask(void)
{
    //读取条码数据并处理
    xTaskCreate((TaskFunction_t )vTaskBarCode,     
                (const char*    )BarCodeTaskName,   
                (uint16_t       )BARCODE_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )BARCODE_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskBarCode);
}



static void vTaskBarCode(void *pvParameters)
{ 
    uint8_t recv_buf[255] = {0};
    uint16_t len = 0; 

    READER_BUFF_STRU *ptQR; 
 	/* 初始化结构体指针 */
	ptQR = &gReaderMsg;

    log_d("start vTaskBarCode\r\n");
    while(1)
    {   
        /* 清零 */
        ptQR->authMode = 0; 
        ptQR->dataLen = 0;
        memset(ptQR->data,0x00,sizeof(ptQR->data)); 

        memset(recv_buf,0x00,sizeof(recv_buf));           
        len = RS485_RecvAtTime(COM5,recv_buf,sizeof(recv_buf),800);

        if(recv_buf[len-1] == 0x00 && len > 1)
        {
            len -= 1; //这里不知道为什么会多了一个0x00
        }
       
        if(len > 0  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
        {
            log_d("reader = %s\r\n",recv_buf);      

            //判定是刷卡还是QR
            if(strstr_t(recv_buf,"CARD") == NULL)
            {
                //QR
                ptQR->authMode = AUTH_MODE_QR;
            }
            else
            {
                ptQR->authMode = AUTH_MODE_CARD;
            }

            if(len > QUEUE_BUF_LEN)
            {
                len = QUEUE_BUF_LEN;
            }              
            ptQR->dataLen = len;
            
            memcpy(ptQR->data,recv_buf,len);  

            log_d("<<<<<<<<<<<pQueue->authMode>>>>>>>>>>>>:%d\r\n",ptQR->authMode);

        	/* 使用消息队列实现指针变量的传递 */
        	if(xQueueSend(xTransQueue,              /* 消息队列句柄 */
        				 (void *) &ptQR,   /* 发送指针变量recv_buf的地址 */
        				 (TickType_t)100) != pdPASS )
        	{
                log_d("the queue is full!\r\n");                
                xQueueReset(xTransQueue);
            } 
            else
            {
                dbh("the queue is send success",(char *)recv_buf,len);
            }                
        }


	/* 发送事件标志，表示任务正常运行 */        
	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
    vTaskDelay(300);        
    }
}


