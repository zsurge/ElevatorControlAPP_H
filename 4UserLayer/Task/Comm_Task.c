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
#define LOG_TAG    "CommTask"
#include "elog.h"

#include "Comm_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "bsp_dipSwitch.h"
#include "FloorDataProc.h"
#include "bsp_usart6.h"
#include "malloc.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37
#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55



#define STEP1   0
#define STEP2   10
#define STEP3   20
#define STEP4   30

typedef struct FROMHOST
{
    uint8_t rxStatus;                   //接收状态
    uint8_t rxCRC;                      //校验值
    uint8_t rxBuff[16];                 //接收字节数
    uint16_t rxCnt;                     //接收字节数    
}FROMHOST_STRU;

 
#define COMM_TASK_PRIO		(tskIDLE_PRIORITY + 8) 
#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)
//static uint32_t totalCnt = 0;
//static uint32_t validCnt = 0;
//static uint32_t sendCnt = 0;

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
static uint8_t deal_Serial_Parse(void);

static FROMHOST_STRU rxFromHost;


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
    TickType_t xLastWakeTime;
    ELEVATOR_BUFF_STRU *sendBuf = &gElevtorData;

//	//改为0A是，默认只去-2楼和1楼
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51 };
//	//改为1F是，默认只去2,1,-2,-3 都不控制
      uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x1B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40 };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5A };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53 };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = {"AA5555555555555555555555555555555BB\r\n"};

    uint32_t i = 0;
    
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(4); /* 设置最大等待时间为200ms */  
    
    //获取当前设备的ID
    uint16_t readID = 0x01;//bsp_dipswitch_read();    

    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));    

    /* 清零 */
    ptMsg->authMode = 0; //默认为刷卡
    ptMsg->dataLen = 0;
    memset(ptMsg->data,0x00,sizeof(ptMsg->data));
    memset(sendBuf->data,0x00,sizeof(sendBuf->data));   

    xLastWakeTime = xTaskGetTickCount();

    memset(&rxFromHost,0x00,sizeof(rxFromHost));
    
    while (1)
    {  
        if(deal_Serial_Parse() == FINISHED)
        {
            xReturn = xQueueReceive( xTransDataQueue,    /* 消息队列的句柄 */
                                     (void *)&sendBuf,  /*这里获取的是结构体的地址 */
                                     0); /* 设置阻塞时间 */
            if(pdTRUE == xReturn)
            {
                //消息接收成功，发送接收到的消息
                dbh("sendBuf->data", sendBuf->data, MAX_RS485_LEN);
            }
            else
            {
                //发送默认数据包
                memcpy(sendBuf->data,defaultBuff,MAX_RS485_LEN);
            }

            RS485_SendBuf(COM6,(uint8_t *)sendBuf->data,MAX_RS485_LEN); 
        }


		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
//        vTaskDelay(10);
        vTaskDelayUntil(&xLastWakeTime, 2);  
    }

}




static uint8_t deal_Serial_Parse(void)
{
    uint8_t ch = 0;
    
    while(RS485_Recv(COM6,&ch,1))
    {
       switch (rxFromHost.rxStatus)
        {                
            case STEP1:
                if(0x5A == ch) /*接收包头*/
                {
                    rxFromHost.rxBuff[0] = ch;
                    rxFromHost.rxCRC = ch;
                    rxFromHost.rxCnt = 1;
                    rxFromHost.rxStatus = STEP2;
                }

                break;
           case STEP2:
                if(0x01 == ch) //判定第二个字节是否是需要的字节，若多梯联动时需读取拨码开关的值
                {
                    rxFromHost.rxBuff[1] = ch;
                    rxFromHost.rxCRC ^= ch;
                    rxFromHost.rxCnt = 2;
                    rxFromHost.rxStatus = STEP3;                
                }
                else
                {                
                   memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));                   
                }
                break;           
            default:      /* 接收整个数据包 */
            
                rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;
                rxFromHost.rxCRC ^= ch;
                
                if(rxFromHost.rxCnt >= 5)
                {
                
                    if(rxFromHost.rxCRC == 0)
                    { 
                        memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
                        return FINISHED;                         
                    }  
                    memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
                } 
             
                break;
         }
         
    }   

    return UNFINISHED;
}


