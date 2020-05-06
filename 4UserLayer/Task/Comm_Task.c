/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Comm_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��28��
  ����޸�   :
  ��������   : ������ͨѶ���������ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��28��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "Comm_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "bsp_dipSwitch.h"
#include "FloorDataProc.h"


#define LOG_TAG    "CommTask"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37

 
#define COMM_TASK_PRIO		(tskIDLE_PRIORITY + 4) 
#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *CommTaskName = "vCommTask"; 

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskComm = NULL;  

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskComm(void *pvParameters);
void packetDefaultSendBuf111(uint8_t *buf);


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
    uint8_t recvLen = 0;
    uint8_t buf[128] = {0};
    uint8_t crc = 0;    
    uint8_t sendBuf[64] = {0};

    
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000); /* �������ȴ�ʱ��Ϊ200ms */  
    
    //��ȡ��ǰ�豸��ID
    uint16_t readID = bsp_dipswitch_read();    

    memset(&gReaderMsg,0x00,sizeof(gReaderMsg));
    
    /* ���� */
    ptMsg->authMode = 0; //Ĭ��Ϊˢ��
    ptMsg->dataLen = 0;
    memset(ptMsg->data,0x00,sizeof(ptMsg->data));  


    log_d("current dev addr =%d\r\n",readID);

    log_d("start vTaskComm\r\n");   
    
    while (1)
    {  

        memset(buf,0x00,sizeof(buf));
        recvLen = RS485_Recv(COM6,buf,sizeof(buf));
//        
        dbh("read buf", buf, sizeof(buf));
//        
//        //�ж����ݵ���Ч��
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
        
        //if(buf[1] == readID)
        {
            xReturn = xQueueReceive( xTransQueue,    /* ��Ϣ���еľ�� */
                                     (void *)&ptMsg,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                     xMaxBlockTime); /* ��������ʱ�� */
            if(pdTRUE == xReturn)
            {
                log_d("receve queue data\r\n");  
                log_d("<<<<<<<<<<<pQueue->authMode>>>>>>>>>>>>:%d\r\n",ptMsg->authMode);
                log_d("%s,%d\r\n",ptMsg->data,ptMsg->dataLen);
                //��Ϣ���ճɹ������ͽ��յ�����Ϣ
                packetSendBuf(ptMsg,sendBuf);     
            }
            else
            {
                //����Ĭ�����ݰ�
                packetDefaultSendBuf(sendBuf); //���  
            }
         
//            RS485_SendBuf(COM6,sendBuf,MAX_RS485_LEN);
              memset(sendBuf,0x00,sizeof(sendBuf));
              packetDefaultSendBuf111(sendBuf);
              RS485_SendBuf(COM6,sendBuf,5);
        }



		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
        vTaskDelay(500);
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
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000); /* �������ȴ�ʱ��Ϊ200ms */  
    
    //��ȡ��ǰ�豸��ID
    uint16_t readID = 0x01;//bsp_dipswitch_read();
    

    memset(&gReaderMsg,0x00,sizeof(gReaderMsg));
    /* ���� */
    ptMsg->authMode = 0; //Ĭ��Ϊˢ��
    ptMsg->dataLen = 0;
    memset(ptMsg->data,0x00,sizeof(ptMsg->data));  


    log_d("current dev addr =%d\r\n",readID);

    log_d("start vTaskComm\r\n");   
    
    while (1)
    {  

        memset(buf,0x00,sizeof(buf));
        recvLen = RS485_Recv(COM6,buf,sizeof(buf));
        
        dbh("read buf", buf, sizeof(buf));
        
        //�ж����ݵ���Ч��
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
            xReturn = xQueueReceive( xTransQueue,    /* ��Ϣ���еľ�� */
                                     (void *)&ptMsg,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                     xMaxBlockTime); /* ��������ʱ�� */
            if(pdTRUE == xReturn)
            {
                log_d("receve queue data\r\n");  
                log_d("<<<<<<<<<<<pQueue->authMode>>>>>>>>>>>>:%d\r\n",ptMsg->authMode);
                log_d("%s,%d\r\n",ptMsg->data,ptMsg->dataLen);
                //��Ϣ���ճɹ������ͽ��յ�����Ϣ
                packetSendBuf(ptMsg,sendBuf);     
            }
            else
            {
                //����Ĭ�����ݰ�
                packetDefaultSendBuf(sendBuf); //���  
            }


            dbh("sendBuf", sendBuf, MAX_RS485_LEN);
            
            RS485_SendBuf(COM6,sendBuf,MAX_RS485_LEN);

        }

		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
        vTaskDelay(500);  

    }
}

#endif


void packetDefaultSendBuf111(uint8_t *buf)
{
    uint8_t sendBuf[5] = {0};

    sendBuf[0] = 0x5A;
    sendBuf[1] = 1;
    sendBuf[2] = 0;
    sendBuf[3] = 0;
    sendBuf[4] = xorCRC(sendBuf,4);

    memcpy(buf,sendBuf,5);
}



