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
#include "templateprocess.h"
#include "stdlib.h"
#include "bsp_ds1302.h"


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


static int compareDate(uint8_t *date1,uint8_t *date2);
static int compareTime(const char *currentTime);




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
    uint8_t localTime[9] = {0};

    int cmpTimeFlag = -1;
    int cmpDateFlag = -1;
    
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
            if(strstr_t((const char*)recv_buf,(const char*)"CARD") == NULL)
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

            //���ģ���Ƿ����õ��ж�
            if(gTemplateParam.templateStatus == 0)
            {
                continue;//ģ��û�����ã����Ե��ݽ��п���
            }


            //��ȡ��ǰʱ��
            memcpy(localTime,bsp_ds1302_readtime(),8);
            
            //�ж���Ч��
            //if(compareDate(localTime,gTemplateParam.peakInfo[0].endTime) < 0)
            
            //�ж���Ч���ڲ��ܿ�ʱ���
            cmpTimeFlag = compareTime(localTime);

            if(cmpTimeFlag == -1
)
            {
                //�����ܿ�ʱ���
            }
            else if(cmpTimeFlag == 0)
            {
                //û��ָ���ܿ�ʱ���
            }
            else
            {
                //���ܿ�ʱ�����
            }
                
            
            //��Ӻ��ݷ�ʽ���ж�
            
            
                
            //�����Ч���ڵ��ж�
            //��Ӳ��ܿ�ʱ�ε��ж�

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

/*
���㷨����˼���Ǽ���������ڵ� 0��3��1�յ�������Ȼ���������ȡ�����ļ����

m1 = (month_start + 9) % 12; �����ж������Ƿ����3�£�2�����ж�����ı�ʶ���������ڼ�¼��3�µļ��������

y1 = year_start - m1/10; �����1�º�2�£��򲻰�����ǰ�꣨��Ϊ�Ǽ��㵽0��3��1�յ���������

d1 = 365*y1 + y1/4 - y1/100 + y1/400 + (m1*306 + 5)/10 + (day_start - 1);

? ? ���� 365*y1 �ǲ�����������һ���������

? ? y1/4 - y1/100 + y1/400 ?�Ǽ���������������һ�죬

(m2*306 + 5)/10?���ڼ��㵽��ǰ�µ�3��1�ռ��������306=365-31-28��1�º�2�£���5��ȫ���в���31���·ݵĸ���

(day_start - 1)?���ڼ��㵱ǰ�յ�1�յļ��������
��������������������������������
ԭ�����ӣ�https://blog.csdn.net/a_ran/article/details/43601699
*/



//����date1��date2�м���ص���������date2С��date1���򷵻ظ���
//ֻ�ܼ���2000~2099��
static int compareDate(uint8_t *date1,uint8_t *date2)
{
    int y2, m2, d2;
	int y1, m1, d1;
    int year_start,month_start,day_start;
    int year_end,month_end,day_end;
    char buff[4] = {0};

    if(strlen((const char*)date2) < 9)
    {
        return -1;//���������
    }

    year_start = 2000 + atoi((const char *)date1[6]);
    month_start = atoi((const char *)date1[5]);
    day_start = atoi((const char *)date1[4]);

    //2020-03-01      
    memcpy(buff,date2,4);
    year_end = atoi(buff);

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date2+5,2);
    month_end = atoi(buff);
        
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date2+8,2);        
    day_end = atoi(buff);

	m1 = (month_start + 9) % 12;
	y1 = year_start - m1/10;
	d1 = 365*y1 + y1/4 - y1/100 + y1/400 + (m1*306 + 5)/10 + (day_start - 1);
 
	m2 = (month_end + 9) % 12;
	y2 = year_end - m2/10;
	d2 = 365*y2 + y2/4 - y2/100 + y2/400 + (m2*306 + 5)/10 + (day_end - 1);
	
	return (d2 - d1);    

}

//�ж���ǰʱ��Σ��Ƿ���time123ʱ����ڣ����Ƿ���1�����Ƿ���-1����û��ָ��ʱ����򷵻�0
static int compareTime(const char *currentTime)
{
    int ret = -1;
    int localTime = 0;
    char buff[4] = {0};

    int begin1,begin2,begin3,end1,end2,end3;

    if(strlen(gTemplateParam.hoildayMode[0].startTime)==0 && strlen(gTemplateParam.hoildayMode[1].startTime)==0 && strlen(gTemplateParam.hoildayMode[2].startTime)==0)
    {
        //û��ָ��ʱ���
        ret = 0;
        return ret;
    }

    //��ʱ��ת��Ϊ����
    localTime = atoi(currentTime[2])*60 + atoi(currentTime[1]);

    //17:30
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[0].startTime,2);
    begin1 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[0].startTime+3,2);
    begin1 += atoi(buff);   


    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[1].startTime,2);
    begin2 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[1].startTime+3,2);
    begin2 += atoi(buff);   

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[2].startTime,2);
    begin3 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[2].startTime+3,2);
    begin3 += atoi(buff);   

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[0].endTime,2);
    end1 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[0].endTime+3,2);
    end1 += atoi(buff);   
    
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[1].endTime,2);
    end2 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[1].endTime+3,2);
    end2 += atoi(buff);   
    
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[2].endTime,2);
    end3 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.hoildayMode[2].endTime+3,2);
    end3 += atoi(buff);       


    if(localTime>begin1 && localTime<end1)
    {
        ret = 1
    }

    
    if(localTime>begin2 && localTime<end2)
    {
        ret = 1
    }

    
    if(localTime>begin3 && localTime<end3)
    {
        ret = 1
    }


    return ret;
}

            
