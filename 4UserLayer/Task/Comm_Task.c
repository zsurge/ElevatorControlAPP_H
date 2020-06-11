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
#include "bsp_usart6.h"
#include "malloc.h"

#define LOG_TAG    "CommTask"
#include "elog.h"

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

#if 1
static void vTaskComm(void *pvParameters)
{
    TickType_t xLastWakeTime;
    ELEVATOR_BUFF_STRU *sendBuf = &gElevtorData;

	//��Ϊ0A�ǣ�Ĭ��ֻȥ-2¥��1¥
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51 };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5A };
//    uint8_t defaultBuff[MAX_RS485_LEN+1] = { 0x5A,0x01,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53 };
    uint8_t defaultBuff[MAX_RS485_LEN+1] = {"AA5555555555555555555555555555555BB\r\n"};

    uint32_t i = 0;
    
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(4); /* �������ȴ�ʱ��Ϊ200ms */  
    
    //��ȡ��ǰ�豸��ID
    uint16_t readID = bsp_dipswitch_read();    

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
            }
            else
            {
                //����Ĭ�����ݰ�
                memcpy(sendBuf->data,defaultBuff,MAX_RS485_LEN);
            }

            RS485_SendBuf(COM6,sendBuf->data,MAX_RS485_LEN); 
        }


		/* �����¼���־����ʾ������������ */        
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
            xReturn = xQueueReceive( xDataProcessQueue,    /* ��Ϣ���еľ�� */
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
                
                   memset(&rxFromHost,0x00,sizeof(rxFromHost));                   
                }
                break;           
            default:      /* �����������ݰ� */
            
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


