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
#include "bsp_dipSwitch.h"
#include "FloorDataProc.h"
#include "bsp_usart6.h"
#include "malloc.h"

#define LOG_TAG    "CommTask"
#include "elog.h"

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

#if 1
static void vTaskComm(void *pvParameters)
{
    TickType_t xLastWakeTime;
    ELEVATOR_BUFF_STRU *sendBuf = &gElevtorData;

	//改为0A是，默认只去-2楼和1楼
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51 };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5A };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53 };
    uint8_t defaultBuff[MAX_RS485_LEN+1] = {"AA5555555555555555555555555555555BB\r\n"};

    uint32_t i = 0;
    
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(4); /* 设置最大等待时间为200ms */  
    
    //获取当前设备的ID
    uint16_t readID = bsp_dipswitch_read();    

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
            }
            else
            {
                //发送默认数据包
                memcpy(sendBuf->data,defaultBuff,MAX_RS485_LEN);
            }

            RS485_SendBuf(COM6,sendBuf->data,MAX_RS485_LEN); 
        }


		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
//        vTaskDelay(10);
        vTaskDelayUntil(&xLastWakeTime, 2);  
    }

}
#else

static void vTaskComm(void *pvParameters)
{
    uint8_t recvLen = 0;
    uint8_t buf[128] = {0};
    uint8_t crc = 0;    
    uint8_t sendBuf[64] = {0};

    
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000); /* 设置最大等待时间为200ms */  
    
    //获取当前设备的ID
    uint16_t readID = 0x01;//bsp_dipswitch_read();
    

    memset(&gReaderMsg,0x00,sizeof(gReaderMsg));
    /* 清零 */
    ptMsg->authMode = 0; //默认为刷卡
    ptMsg->dataLen = 0;
    memset(ptMsg->data,0x00,sizeof(ptMsg->data));  


    log_d("current dev addr =%d\r\n",readID);

    log_d("start vTaskComm\r\n");   
    
    while (1)
    {  

        memset(buf,0x00,sizeof(buf));
        recvLen = RS485_Recv(COM6,buf,sizeof(buf));
        
        dbh("read buf", buf, sizeof(buf));
        
        //判定数据的有效性
//        if(recvLen != 5 || buf[0] != 0X5a || buf[1]<1 || buf[1]>4)
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
            xReturn = xQueueReceive( xDataProcessQueue,    /* 消息队列的句柄 */
                                     (void *)&ptMsg,  /*这里获取的是结构体的地址 */
                                     xMaxBlockTime); /* 设置阻塞时间 */
            if(pdTRUE == xReturn)
            {
                log_d("receve queue data\r\n");  
                log_d("<<<<<<<<<<<pQueue->authMode>>>>>>>>>>>>:%d\r\n",ptMsg->authMode);
                log_d("%s,%d\r\n",ptMsg->data,ptMsg->dataLen);
                //消息接收成功，发送接收到的消息
                packetSendBuf(ptMsg,sendBuf);     
            }
            else
            {
                //发送默认数据包
                packetDefaultSendBuf(sendBuf); //打包  
            }


            dbh("sendBuf", sendBuf, MAX_RS485_LEN);
            
            RS485_SendBuf(COM6,sendBuf,MAX_RS485_LEN);

        }

		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
        vTaskDelay(500);  

    }
}

#endif


//static uint8_t deal_Serial_Parse(uint8_t *str)
//{
//    uint8_t buf[6] = {0};
//    uint8_t crc = 0;    
//    uint8_t stx = 0;    

//	if(getRxCnt(COM6) < 5) 
//	{
//		return 0;
//	}

//	//totalCnt++;
//    memset(buf,0x00,sizeof(buf));
//    if(RS485_Recv(COM6,&stx,1)!=1)
//    {
//      return 0;
//    }    
//    
//    if(stx != 0x5A)
//    {
//        return 0;
//    }
//    
//    buf[0] = stx;
//    
//    
//    if(RS485_Recv(COM6,buf+1,4)!=4)
//    {
//        return 0;
//    }


//    if(buf[1] != 1)
//    {
//        return 0;
//    }

//    crc= xorCRC(buf,4);

//    if(crc != buf[4])
//    {
//        return 0;
//    }
//    memcpy(str,buf,5);
//    return 1;
//}

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
                
                   memset(&rxFromHost,0x00,sizeof(rxFromHost));                   
                }
                break;           
            default:      /* 接收整个数据包 */
            
                rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;
                rxFromHost.rxCRC ^= ch;
                
                if(rxFromHost.rxCnt >= 5)
                {
                
                    if(rxFromHost.rxCRC == 0)
                    { 
                        memset(&rxFromHost,0x00,sizeof(rxFromHost));
                        return FINISHED;                         
                    }  
                    memset(&rxFromHost,0x00,sizeof(rxFromHost));
                } 
             
                break;
         }
         
    }   

    return UNFINISHED;
}


