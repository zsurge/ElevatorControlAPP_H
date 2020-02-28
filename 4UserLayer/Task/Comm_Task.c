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


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define COMM_STK_SIZE 		(1024*1)
#define COMM_TASK_PRIO		( tskIDLE_PRIORITY + 4)
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


void CreateCommTask(void)
{

}

static void vTaskComm(void *pvParameters)
{
    uint8_t recvLen = 0;
    uint8_t buf[5+1] = {0};
    uint8_t crc = 0;    
    uint8_t sendBuf[64] = {0};
    
    READER_BUFF_T *ptMsg;
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* �������ȴ�ʱ��Ϊ200ms */  
    
    //��ȡ��ǰ�豸��ID
    uint16_t readID = bsp_dipswitch_read();

    log_d("current dev addr =%d\r\n",readID);
    
    while (1)
    {
//        recvLen = RS485_Recv(COM5,buf,MAX_CMD_LEN);
//        
//        //�ж����ݵ���Ч��
//        if(recvLen != MAX_CMD_LEN || buf[0] != CMD_STX || buf[1]<1 || buf[1]>4)
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
                //��Ϣ���ճɹ������ͽ��յ�����Ϣ
                packetSendBuf(ptMsg,sendBuf);     
            }
            else
            {
                //����Ĭ�����ݰ�
                packetDefaultSendBuf(sendBuf); //���  
            }

            RS485_SendBuf(COM4,sendBuf,MAX_SEND_LEN);

        }



		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
        vTaskDelay(300);
    }

}


