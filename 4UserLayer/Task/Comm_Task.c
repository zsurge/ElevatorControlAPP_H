/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Comm_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月28日
  最近修改   :
  功能描述   : 跟电梯通讯的任务处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月28日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "Comm_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"


#define LOG_TAG    "CommTask"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37

 
#define COMM_TASK_PRIO		(tskIDLE_PRIORITY + 4) 
#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *CommTaskName = "vCommTask"; 

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskComm = NULL;  

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskComm(void *pvParameters);


void CreateCommTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskComm,         
                (const char*    )CommTaskName,       
                (uint16_t       )COMM_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )COMM_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskComm);
}

static void vTaskComm(void *pvParameters)
{
    uint8_t recvLen = 0;
    uint8_t buf[5+1] = {0};
    uint8_t crc = 0;    
    uint8_t sendBuf[64] = {0};
    
    READER_BUFF_T *ptMsg;
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* 设置最大等待时间为200ms */  
    
    //获取当前设备的ID
    uint16_t readID = bsp_dipswitch_read();

    log_d("current dev addr =%d\r\n",readID);
    
    while (1)
    {
//        recvLen = RS485_Recv(COM5,buf,MAX_CMD_LEN);
//        
//        //判定数据的有效性
//        if(recvLen != MAX_CMD_LEN || buf[0] != CMD_STX || buf[1]<1 || buf[1]>4)
//        {
//            vTaskDelay(500); 
//            continue;
//        }

//        crc= xorCRC(buf,3);
//        
//        if(crc != buf[3])
//        {
//            vTaskDelay(500); 
//            continue;
//        }
        
//        if(buf[1] == readID)
        {
            xReturn = xQueueReceive( xTransQueue,    /* 消息队列的句柄 */
                                     (void *)&ptMsg,  /*这里获取的是结构体的地址 */
                                     xMaxBlockTime); /* 设置阻塞时间 */
            if(pdTRUE == xReturn)
            {
                //消息接收成功，发送接收到的消息
                packetSendBuf(ptMsg,sendBuf);     
            }
            else
            {
                //发送默认数据包
                packetDefaultSendBuf(sendBuf); //打包  
            }

            RS485_SendBuf(COM4,sendBuf,MAX_RS485_LEN);

        }



		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
        vTaskDelay(300);
    }

}


