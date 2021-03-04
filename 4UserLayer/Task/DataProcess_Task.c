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



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_RS485_LEN 37

 
#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 7) 
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
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(30); /* �������ȴ�ʱ��Ϊ100ms */ 
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

        /* ���� */
        ptMsg->authMode = 0; //Ĭ��Ϊˢ��
        ptMsg->dataLen = 0;
        memset(ptMsg->data,0x00,sizeof(ptMsg->data));
    
        xReturn = xQueueReceive( xDataProcessQueue,    /* ��Ϣ���еľ�� */
                                 (void *)&ptMsg,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                 xMaxBlockTime); /* ��������ʱ�� */
        if(pdTRUE != xReturn)
        {
            continue;
        }
        
        
        switch (ptMsg->authMode)
        {
            case AUTH_MODE_CARD:            
                //���� CARD 230000000089E1E35D,23         
                memcpy(cardNo,ptMsg->data,CARD_NO_LEN);
                log_d("key = %s\r\n",cardNo);     
                
                isFind = readUserData(cardNo,CARD_MODE,&localUserData);   

                log_d("isFind = %d,rUserData.cardState = %d\r\n",isFind,localUserData.cardState);

                if(localUserData.cardState != CARD_VALID || isFind != 0)
                {
                    //δ�ҵ���¼����Ȩ��
                    log_e("not find record\r\n");
                   break;
                } 
                
                localUserData.platformType = 4;
                localUserData.authMode = ptMsg->authMode; 
                memcpy(localUserData.timeStamp,time_to_timestamp(),TIMESTAMP_LEN);
                log_d("localUserData->timeStamp = %s\r\n",localUserData.timeStamp);  
                
                //1.���
                packetPayload(&localUserData,jsonBuf); 
                len = strlen((const char*)jsonBuf);
                
                //2.����������
                len = mqttSendData(jsonBuf,len);
                log_d("send = %d\r\n",len);
                
                //3.�����������
                ret = packetToElevator(&localUserData);
                if(ret != NO_ERR)
                {
                    log_d("invalid floor\r\n");
                    break;  //��Ȩ��   
                }  
                break;
            case AUTH_MODE_QR:
                isFind = parseQrCode((uint8_t *)ptMsg->data,&localUserData);
                if(isFind != NO_ERR)
                {
                    log_d("not find record\r\n");
                    break ;  //��Ȩ��
                }
                
                localUserData.authMode = ptMsg->authMode; 
                
                //1.���
                packetPayload(&localUserData,jsonBuf); 
                len = strlen((const char*)jsonBuf);

                //2.����������
                len = mqttSendData(jsonBuf,len);
                log_d("send = %d\r\n",len);  

                //3.�����������
                ret = packetToElevator(&localUserData);
                if(ret != NO_ERR)
                {
                    log_d("invalid floor\r\n");
                    break;  //��Ȩ��   
                }
                break;
            case AUTH_MODE_REMOTE:
                //ֱ�ӷ���Ŀ��¥��
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
                //ֱ�ӷ���ͣ���豸ָ��
                xQueueReset(xDataProcessQueue); 
                log_d("send AUTH_MODE_UNBIND floor\r\n");
                break;     
            case AUTH_MODE_BIND:
                //ֱ�ӷ�����������ָ��
                xQueueReset(xDataProcessQueue); 
                log_d("send AUTH_MODE_BIND floor\r\n");
                break;                
        }
        
        
        /* �����¼���־����ʾ������������ */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_6);  
        vTaskDelay(100);

    }

}


