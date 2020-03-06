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
#include "bsp_uart_fifo.h"
#include "CmdHandle.h"
#include "tool.h"

#define LOG_TAG    "BarCode"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define BARCODE_TASK_PRIO		(tskIDLE_PRIORITY + 1)
#define BARCODE_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)

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
    //��ȡ�������ݲ�����
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
 	/* ��ʼ���ṹ��ָ�� */
	ptQR = &gReaderMsg;

    log_d("start vTaskBarCode\r\n");
    while(1)
    {   
        /* ���� */
        ptQR->authMode = 0; 
        ptQR->dataLen = 0;
        memset(ptQR->data,0x00,sizeof(ptQR->data)); 

        memset(recv_buf,0x00,sizeof(recv_buf));           
        len = RS485_RecvAtTime(COM5,recv_buf,sizeof(recv_buf),800);

        if(recv_buf[len-1] == 0x00 && len > 1)
        {
            len -= 1; //���ﲻ֪��Ϊʲô�����һ��0x00
        }
       
        if(len > 0  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
        {
            log_d("reader = %s\r\n",recv_buf);      

            //�ж���ˢ������QR
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

        	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
        	if(xQueueSend(xTransQueue,              /* ��Ϣ���о�� */
        				 (void *) &ptQR,   /* ����ָ�����recv_buf�ĵ�ַ */
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


	/* �����¼���־����ʾ������������ */        
	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
    vTaskDelay(300);        
    }
}


