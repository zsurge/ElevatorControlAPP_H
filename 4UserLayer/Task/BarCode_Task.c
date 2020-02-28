/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : BarCode_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��27��
  ����޸�   :
  ��������   : ���봦������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��27��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "MsgParse_Task.h"
#include "bsp_uart_fifo.h"
#include "comm.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define BARCODE_TASK_PRIO		( tskIDLE_PRIORITY + 1)
#define BARCODE_STK_SIZE 		(configMINIMAL_STACK_SIZE*9)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *BarCodeTaskName = "vBarCodeTask";  

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskBarCode = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskBarCode(void *pvParameters);

void CreateBarCodeTask(void)
{
    //��androidͨѶ�������ݽ���
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

        /* �����¼���־����ʾ������������ */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
        vTaskDelay(500);        
    }
}

