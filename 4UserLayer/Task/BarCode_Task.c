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
#include "MsgParse_Task.h"
#include "bsp_uart_fifo.h"
#include "comm.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define BARCODE_TASK_PRIO		( tskIDLE_PRIORITY + 1)
#define BARCODE_STK_SIZE 		(configMINIMAL_STACK_SIZE*9)

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
    //跟android通讯串口数据解析
    xTaskCreate((TaskFunction_t )vTaskBarCode,     
                (const char*    )BarCodeTaskName,   
                (uint16_t       )BARCODE_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )BARCODE_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskBarCode);
}



static void vTaskBarCode(void *pvParameters)
{ 
    uint8_t recv_buf[256] = {0};    
    uint16_t len = 0;  
    
//    uint32_t FunState = 0;
//    char *QrCodeState;

//    QrCodeState = ef_get_env("QRSTATE");
//    assert_param(QrCodeState);
//    FunState = atol(QrCodeState);
    
    while(1)
    {
//       if(FunState != 0x00)
       {
           memset(recv_buf,0x00,sizeof(recv_buf));
           len = comRecvBuff(COM3,recv_buf,sizeof(recv_buf));
           
//           dbh("QR HEX", recv_buf, len);

           if(len > 2  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
           {

                SendAsciiCodeToHost(QRREADER,NO_ERR,recv_buf);
           }
           else
           {
                comClearRxFifo(COM3);
           }

       }

        /* 发送事件标志，表示任务正常运行 */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
        vTaskDelay(500);        
    }
}

