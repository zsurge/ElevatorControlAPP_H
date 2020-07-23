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


static void test(void);

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37

 
#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 6) 
#define DATAPROC_STK_SIZE 		(configMINIMAL_STACK_SIZE*12)

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

    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100); /* �������ȴ�ʱ��Ϊ100ms */ 
   
    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));    

    /* ���� */
    ptMsg->authMode = 0; //Ĭ��Ϊˢ��
    ptMsg->dataLen = 0;
    memset(ptMsg->data,0x00,sizeof(ptMsg->data));
    
    while (1)
    {
        xReturn = xQueueReceive( xDataProcessQueue,    /* ��Ϣ���еľ�� */
                                 (void *)&ptMsg,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                 xMaxBlockTime); /* ��������ʱ�� */
        if(pdTRUE == xReturn)
        {
            //��Ϣ���ճɹ������ͽ��յ�����Ϣ
            packetSendBuf(ptMsg); 
            log_d("exec packetSendBuf end\r\n");
        }
        
        
        /* �����¼���־����ʾ������������ */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_6);  
        vTaskDelay(100);

    }

}


static void test(void)
{
    int i = 0; 
    char buf[4096] = {0};

    memset(buf,0x00,sizeof(buf));

   for(i=0;i<4096;i++)
   {
    buf[i] = i;
   }

    dbh("buf", buf, 4096);
}

 
