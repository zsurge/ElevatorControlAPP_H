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
#define LOG_TAG    "DataProcess"
#include "elog.h"

#include "DataProcess_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "bsp_dipSwitch.h"
#include "FloorDataProc.h"
#include "bsp_usart6.h"
#include "malloc.h"
#include "tool.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37

 
#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 7) 
#define DATAPROC_STK_SIZE 		(configMINIMAL_STACK_SIZE*12)

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
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(30); /* 设置最大等待时间为100ms */ 
    char isFind = 0; 
    uint8_t ret = 0; 
    uint8_t jsonBuf[512] = {0};
    uint8_t cardNo[8] = {0};
    int len = 0;
    
    
    USERDATA_STRU localUserData ;
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    
    
    while (1)
    {
    
        memset(&localUserData,0x00,sizeof(USERDATA_STRU));     
        memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));    
        memset(&gElevtorData,0x00,sizeof(ELEVATOR_BUFF_STRU));    

        /* 清零 */
        ptMsg->authMode = 0; //默认为刷卡
        ptMsg->dataLen = 0;
        memset(ptMsg->data,0x00,sizeof(ptMsg->data));

        /* 发送事件标志，表示任务正常运行 */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_6);  
        
        xReturn = xQueueReceive( xDataProcessQueue,    /* 消息队列的句柄 */
                                 (void *)&ptMsg,  /*这里获取的是结构体的地址 */
                                 xMaxBlockTime); /* 设置阻塞时间 */
        if(pdTRUE != xReturn)
        {
            continue;
        }
        
        
        switch (ptMsg->authMode)
        {
            case AUTH_MODE_CARD:            
                //读卡 CARD 230000000089E1E35D,23         
                memcpy(cardNo,ptMsg->data,CARD_NO_LEN);
                log_d("key = %s\r\n",cardNo);     
                
                isFind = readUserData(cardNo,CARD_MODE,&localUserData);   

                log_d("isFind = %d,rUserData.cardState = %d\r\n",isFind,localUserData.cardState);

                if(localUserData.cardState != CARD_VALID || isFind != 0)
                {
                    //未找到记录，无权限
                    log_e("not find record\r\n");
                   break;
                } 
                
                localUserData.platformType = 4;
                localUserData.authMode = ptMsg->authMode; 
                memcpy(localUserData.timeStamp,time_to_timestamp(),TIMESTAMP_LEN);
                log_d("localUserData->timeStamp = %s\r\n",localUserData.timeStamp);  
                
                //1.打包
                packetPayload(&localUserData,jsonBuf); 
                len = strlen((const char*)jsonBuf);
                
                //2.发给服务器
                len = mqttSendData(jsonBuf,len);
                log_d("send = %d\r\n",len);
                
                //3.计算电梯数据
                ret = packetToElevator(&localUserData);
                if(ret != NO_ERR)
                {
                    log_d("invalid floor\r\n");
                    break;  //无权限   
                }  
                break;
            case AUTH_MODE_QR:
                isFind = parseQrCode((uint8_t *)ptMsg->data,&localUserData);
                if(isFind != NO_ERR)
                {
                    log_d("not find record\r\n");
                    break ;  //无权限
                }
                
                localUserData.authMode = ptMsg->authMode; 
                
                //1.打包
                packetPayload(&localUserData,jsonBuf); 
                len = strlen((const char*)jsonBuf);

                //2.发给服务器
                len = mqttSendData(jsonBuf,len);
                log_d("send = %d\r\n",len);  

                //3.计算电梯数据
                ret = packetToElevator(&localUserData);
                if(ret != NO_ERR)
                {
                    log_d("invalid floor\r\n");
                    break;  //无权限   
                }
                break;
            case AUTH_MODE_REMOTE:
                //直接发送目标楼层
                log_d("send desc floor = %s,%d\r\n",ptMsg->data,ptMsg->dataLen);  
                
                localUserData.authMode = ptMsg->authMode; 
                ret = packetRemoteRequestToElevator((uint8_t *)ptMsg->data,ptMsg->dataLen);
                if(ret != NO_ERR)
                {
                    log_d("invalid floor\r\n");
                    break;
                }    
                break;
            case AUTH_MODE_UNBIND:
                //直接发送停用设备指令
                xQueueReset(xDataProcessQueue); 
                log_d("send AUTH_MODE_UNBIND floor\r\n");
                break;     
            case AUTH_MODE_BIND:
                //直接发送启动设置指令
                xQueueReset(xDataProcessQueue); 
                log_d("send AUTH_MODE_BIND floor\r\n");
                break;                
        }       
        

        vTaskDelay(100);

    }

}


