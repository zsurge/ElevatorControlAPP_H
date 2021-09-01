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
 * �궨��                                       *
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
    uint8_t rxStatus;                   //����״̬
    uint8_t rxCRC;                      //У��ֵ
    uint8_t rxBuff[16];                 //�����ֽ���
    uint16_t rxCnt;                     //�����ֽ���    
}FROMHOST_STRU;

 
#define COMM_TASK_PRIO		(tskIDLE_PRIORITY + 8) 
#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)
//static uint32_t totalCnt = 0;
//static uint32_t validCnt = 0;
//static uint32_t sendCnt = 0;

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

//	//��Ϊ0A�ǣ�Ĭ��ֻȥ-2¥��1¥
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51 };
//	//��Ϊ1F�ǣ�Ĭ��ֻȥ2,1,-2,-3 ��������
      uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x1B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40 };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5A };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53 };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = {"AA5555555555555555555555555555555BB\r\n"};

    uint32_t i = 0;
    
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(4); /* �������ȴ�ʱ��Ϊ200ms */  
    
    //��ȡ��ǰ�豸��ID
    uint16_t readID = 0x01;//bsp_dipswitch_read();    

    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));    

    /* ���� */
    ptMsg->authMode = 0; //Ĭ��Ϊˢ��
    ptMsg->dataLen = 0;
    memset(ptMsg->data,0x00,sizeof(ptMsg->data));
    memset(sendBuf->data,0x00,sizeof(sendBuf->data));   

    xLastWakeTime = xTaskGetTickCount();

    memset(&rxFromHost,0x00,sizeof(rxFromHost));
    
    while (1)
    {  
        if(deal_Serial_Parse() == FINISHED)
        {
            xReturn = xQueueReceive( xTransDataQueue,    /* ��Ϣ���еľ�� */
                                     (void *)&sendBuf,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                     0); /* ��������ʱ�� */
            if(pdTRUE == xReturn)
            {
                //��Ϣ���ճɹ������ͽ��յ�����Ϣ
                dbh("sendBuf->data", sendBuf->data, MAX_RS485_LEN);
            }
            else
            {
                //����Ĭ�����ݰ�
                memcpy(sendBuf->data,defaultBuff,MAX_RS485_LEN);
            }

            RS485_SendBuf(COM6,(uint8_t *)sendBuf->data,MAX_RS485_LEN); 
        }


		/* �����¼���־����ʾ������������ */        
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
                if(0x5A == ch) /*���հ�ͷ*/
                {
                    rxFromHost.rxBuff[0] = ch;
                    rxFromHost.rxCRC = ch;
                    rxFromHost.rxCnt = 1;
                    rxFromHost.rxStatus = STEP2;
                }

                break;
           case STEP2:
                if(0x01 == ch) //�ж��ڶ����ֽ��Ƿ�����Ҫ���ֽڣ�����������ʱ���ȡ���뿪�ص�ֵ
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
            default:      /* �����������ݰ� */
            
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


