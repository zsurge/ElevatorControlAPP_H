/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : DataProcess_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��5��15��
  ����޸�   :
  ��������   : ��ˢ��/QR/Զ���͹��������ݽ��д���
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��5��15��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37

 
#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 6) 
#define DATAPROC_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *dataProcTaskName = "vDataProcTask"; 

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskDataProc = NULL;  


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
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
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100); /* �������ȴ�ʱ��Ϊ100ms */ 
   
    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));    

    /* ���� */
    ptMsg->authMode = 0; //Ĭ��Ϊˢ��
    ptMsg->dataLen = 0;
    memset(ptMsg->data,0x00,sizeof(ptMsg->data));

    //memset(sendBuf->data,0x00,sizeof(sendBuf->data));
    
    while (1)
    {
        xReturn = xQueueReceive( xDataProcessQueue,    /* ��Ϣ���еľ�� */
                                 (void *)&ptMsg,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                 xMaxBlockTime); /* ��������ʱ�� */
        if(pdTRUE == xReturn)
        {
            //��Ϣ���ճɹ������ͽ��յ�����Ϣ
            packetSendBuf(ptMsg); 
            //packetSendBuf(ptMsg,sendBuf->data); 

            log_d("exec packetSendBuf end\r\n");

    
//            dbh("packetToElevator", (char *)sendBuf, MAX_SEND_LEN);

//            /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
//            if(xQueueSend(xTransDataQueue,              /* ��Ϣ���о�� */
//            			 (void *) &sendBuf,   /* ����ָ�����recv_buf�ĵ�ַ */
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

        
        /* �����¼���־����ʾ������������ */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_6);  
        vTaskDelay(100);

    }

}


 
