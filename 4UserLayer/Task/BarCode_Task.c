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
#include "des.h"
#include "malloc.h"

#define LOG_TAG    "BarCode"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define BARCODE_TASK_PRIO		(tskIDLE_PRIORITY + 1)
#define BARCODE_STK_SIZE 		(configMINIMAL_STACK_SIZE*10)

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
static int compareTime(uint8_t *currentTime);
static void getDevData(char *src,int icFlag,int qrFlag,READER_BUFF_STRU *desc);




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


#if 0
static void vTaskBarCode(void *pvParameters)
{ 
    uint8_t recv_buf[255] = {0};
    uint8_t readyBuff[1024] = {0};
    uint16_t len = 0; 
    uint16_t offset = 0; 
    uint8_t localTime[9] = {0};

    uint8_t relieveControl[38] = {0};

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
        ptQR->state = ENABLE;
        memset(ptQR->data,0x00,sizeof(ptQR->data)); 

        memset(recv_buf,0x00,sizeof(recv_buf));           
        len = RS485_RecvAtTime(COM5,recv_buf,sizeof(recv_buf),800);
        
        memcpy(readyBuff+offset,recv_buf,len);
        offset += len;
        

//        log_d("reader = %s\r\n",recv_buf);

        
        if(recv_buf[len-1] == 0x00 && len > 1)
        {
            len -= 1; //���ﲻ֪��Ϊʲô�����һ��0x00
        }
       
        if(len > 0  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
        {
//            log_d("reader = %s\r\n",recv_buf);
              
//            dbh("reader", recv_buf, len);
            //���ģ���Ƿ����õ��ж�
            if(gTemplateParam.templateStatus == 0)
            {
                //2020-03-18 ����Ӧ�÷��͵��ݲ����ܿ��Ƶ�ָ������ǲ���continue,
                //����Ȩ��ʽ�����ӿ������ͣ����������ָ���������

                    //add code ��ָ���ܿ���ָ��                    
                    ptQR->authMode = AUTH_MODE_RELIEVECONTROL;
                    ptQR->dataLen = sizeof(relieveControl);
                    memcpy(ptQR->data,relieveControl,ptQR->dataLen);   
                    log_d("the template is disable\r\n");
            }
            else
            {
                log_d("the template is enable\r\n");
                //�ж��߷�ڼ���ģʽ�Ƿ���
                if(gTemplateParam.workMode.isPeakMode || gTemplateParam.workMode.isHolidayMode)
                {
                    //��ȡ��ǰʱ��
                    memcpy(localTime,bsp_ds1302_readtime(),8);
                    
                    //�ж��ڼ���ģ����Ч��
                    if(compareDate(localTime,gTemplateParam.peakInfo[0].endTime) < 0) //����Ч����
                    {   
                        log_d("in the peak mode valid date \r\n");
                        
                        //�ж���Ч���ڲ��ܿ�ʱ���
                        cmpTimeFlag = compareTime(localTime);

                        log_d("cmpTimeFlag =%d\r\n",cmpTimeFlag);

                        if(cmpTimeFlag == 1)
                        {
                            log_d("peak mode - >in the out of control \r\n");
                            //�ڲ��ܿ�ʱ��Σ������������ָ��
                            ptQR->authMode = AUTH_MODE_RELIEVECONTROL;
                            ptQR->dataLen = sizeof(relieveControl);
                            memcpy(ptQR->data,relieveControl,sizeof(relieveControl)); 
                        } 
                        else
                        {
                            log_d("peak mode  >in the control time \r\n");
                            getDevData((char *)recv_buf,gTemplateParam.peakCallingWay.isIcCard,gTemplateParam.peakCallingWay.isQrCode,ptQR);
                            //û��ָ�����ܿ����� �� ��ǰ��������Ч����
                            //��ӽڼ��յĺ��ݷ�ʽ���ж�
//                            if(gTemplateParam.peakCallingWay.isIcCard == 0)
//                            {
//                                //��ֵ   
//                                if(ptQR->authMode == AUTH_MODE_CARD)
//                                {
//                                    ptQR->dataLen = 0;
//                                    memset(ptQR->data,0x00,sizeof(ptQR->data)); 
//                                }
//                            }

//                            if(gTemplateParam.peakCallingWay.isQrCode == 0)
//                            {
//                                //��ֵ   
//                                if(ptQR->authMode == AUTH_MODE_QR)
//                                {
//                                    ptQR->dataLen = 0;
//                                    memset(ptQR->data,0x00,sizeof(ptQR->data)); 
//                                }                                
//                            } 
                                
                        }
                    }
                    else
                    {
                        //������Ч����
                        //�ж�ģ��ĺ��ݷ�ʽ
//                        if(gTemplateParam.templateCallingWay.isIcCard)
//                        {
//                            //��ֵ                                
//                        }

//                        if(gTemplateParam.templateCallingWay.isQrCode)
//                        {
//                            //��ֵ                                
//                        } 
                        log_d("outside peak mode the valid date \r\n");

                        getDevData((char *)recv_buf,gTemplateParam.templateCallingWay.isIcCard,gTemplateParam.templateCallingWay.isQrCode,ptQR);

                    }

                 } 
                 else
                 {
                    //�ж�ģ��ĺ��ݷ�ʽ
//                    if(gTemplateParam.templateCallingWay.isIcCard)
//                    {
//                        //��ֵ                                
//                    }

//                    if(gTemplateParam.templateCallingWay.isQrCode)
//                    {
//                        //��ֵ                                
//                    }  
                    log_d("Now it's normal operation mode \r\n");
                    getDevData((char *)recv_buf,gTemplateParam.templateCallingWay.isIcCard,gTemplateParam.templateCallingWay.isQrCode,ptQR);

                 }  

                log_d("ptQR = %s,len = %d,state = %d\r\n",ptQR->data,ptQR->dataLen,ptQR->state);
                
                if(ptQR->state)
                {
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
            }
        }


    	/* �����¼���־����ʾ������������ */        
    	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
        vTaskDelay(500);        
    }
}
#endif 

static void vTaskBarCode(void *pvParameters)
{ 
    uint8_t recv_buf[255] = {0};
    uint8_t sendBuff[512] = {0};
    uint16_t len = 0; 
    uint16_t offset = 0; 
    uint8_t localTime[20] = {0};

    uint8_t relieveControl[38] = {0};

    int cmpTimeFlag = -1;

    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));
    READER_BUFF_STRU *ptQR = &gReaderMsg; 


    log_d("start vTaskBarCode\r\n");
    while(1)
    {   
        /* ���� */
        ptQR->authMode = 0; 
        ptQR->dataLen = 0;
        ptQR->state = ENABLE;
        memset(ptQR->data,0x00,sizeof(ptQR->data)); 

        memset(recv_buf,0x00,sizeof(recv_buf));           
        len = RS485_RecvAtTime(COM5,recv_buf,sizeof(recv_buf),800);

//        log_d("RS485_RecvAtTime = %d\r\n",len);
        if(len>255)
            len = 255;
         
        memcpy(sendBuff+offset,recv_buf,len);
        offset += len; 
        
        if(offset > 10  && sendBuff[offset-1] == 0x0A && sendBuff[offset-2] == 0x0D && gDeviceStateFlag == DEVICE_ENABLE)
        {
            comClearRxFifo(COM5);
            log_d("sendbuff = %s\r\n",sendBuff);
            //���ģ���Ƿ����õ��ж�
            if(gTemplateParam.templateStatus == 0)
            {
                //2020-03-18 ����Ӧ�÷��͵��ݲ����ܿ��Ƶ�ָ������ǲ���continue,
                //����Ȩ��ʽ�����ӿ������ͣ����������ָ���������

                    //add code ��ָ���ܿ���ָ��                    
                    ptQR->authMode = AUTH_MODE_RELIEVECONTROL;
                    ptQR->dataLen = sizeof(relieveControl);
                    memcpy(ptQR->data,relieveControl,ptQR->dataLen);   
                    log_d("the template is disable\r\n");
            }
            else
            {
                log_d("the template is enable\r\n");
                //�ж��߷�ڼ���ģʽ�Ƿ���
                if(gTemplateParam.workMode.isPeakMode || gTemplateParam.workMode.isHolidayMode)
                {
                    //��ȡ��ǰʱ��
                    strcpy((char*)localTime,(const char*)bsp_ds1302_readtime());
                    
                    //�ж��ڼ���ģ����Ч��
                    if(compareDate(localTime,gTemplateParam.peakInfo[0].endTime) < 0) //����Ч����
                    {   
                        log_d("in the peak mode valid date \r\n");
                        
                        //�ж���Ч���ڲ��ܿ�ʱ���
                        cmpTimeFlag = compareTime(localTime);

                        log_d("cmpTimeFlag =%d\r\n",cmpTimeFlag);

                        if(cmpTimeFlag == 1)
                        {
                            log_d("peak mode - >in the out of control \r\n");
                            //�ڲ��ܿ�ʱ��Σ������������ָ��
                            ptQR->authMode = AUTH_MODE_RELIEVECONTROL;
                            ptQR->dataLen = sizeof(relieveControl);
                            memcpy(ptQR->data,relieveControl,sizeof(relieveControl)); 
                        } 
                        else
                        {
                            log_d("peak mode  >in the control time \r\n");
                            getDevData((char *)sendBuff,gTemplateParam.peakCallingWay.isIcCard,gTemplateParam.peakCallingWay.isQrCode,ptQR);
                            //û��ָ�����ܿ����� �� ��ǰ��������Ч����
                            //��ӽڼ��յĺ��ݷ�ʽ���ж�
                                
                        }
                    }
                    else
                    {
                        //������Ч����
                        //�ж�ģ��ĺ��ݷ�ʽ
                        log_d("outside peak mode the valid date \r\n");

                        getDevData((char *)sendBuff,gTemplateParam.templateCallingWay.isIcCard,gTemplateParam.templateCallingWay.isQrCode,ptQR);

                    }

                 } 
                 else
                 {
                    //�ж�ģ��ĺ��ݷ�ʽ
                    log_d("Now it's normal operation mode \r\n");
                    getDevData((char *)sendBuff,gTemplateParam.templateCallingWay.isIcCard,gTemplateParam.templateCallingWay.isQrCode,ptQR);

                 }  

                log_d("ptQR = %s,len = %d,state = %d\r\n",ptQR->data,ptQR->dataLen,ptQR->state);

                if(ptQR->state)
                {
                	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
                	if(xQueueSend(xTransQueue,              /* ��Ϣ���о�� */
                				 (void *) &ptQR,   /* ����ָ�����recv_buf�ĵ�ַ */
                				 (TickType_t)1000) != pdPASS )
                	{
                        log_d("the queue is full!\r\n");                
                        xQueueReset(xTransQueue);
                    } 
//                    else
//                    {
//                        dbh("barcode task the queue is send success",(char *)ptQR->data,ptQR->dataLen/2);
//                    }   
                }
            }

            memset(sendBuff,0x00,sizeof(sendBuff));
            offset = 0;
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

    memcpy(buff,date1,4);
    year_start = atoi(buff);

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date1+5,2);    
    month_start = atoi(buff);

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date1+8,2);      
    day_start = atoi(buff);   

    //2020-03-01 18:42:25  
    memset(buff,0x00,sizeof(buff));
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

//�ж���ǰʱ��Σ��Ƿ���time123ʱ����ڣ����ڷ��أ�1�����Ƿ���-1����û��ָ��ʱ����򷵻�0
static int compareTime(uint8_t *currentTime)
{
    int ret = -1;
    int localTime = 0;
    char buff[4] = {0};

    int begin1,begin2,begin3,end1,end2,end3;

    if(strlen((const char*)gTemplateParam.holidayMode[0].startTime)==0 && strlen((const char*)gTemplateParam.holidayMode[1].startTime)==0 && strlen((const char*)gTemplateParam.holidayMode[2].startTime)==0)
    {
        //û��ָ��ʱ���
        ret = 0;
        return ret;
    }    
    //��ʱ��ת��Ϊ����
    //2020-03-01 18:42:25  
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,currentTime+11,2);    
    localTime = atoi(buff)*60;

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,currentTime+14,2);    
    localTime += atoi(buff);    
    
    //17:30
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[0].startTime,2);
    begin1 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[0].startTime+3,2);
    begin1 += atoi(buff); 

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[1].startTime,2);
    begin2 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[1].startTime+3,2);
    begin2 += atoi(buff);   

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[2].startTime,2);
    begin3 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[2].startTime+3,2);
    begin3 += atoi(buff);   

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[0].endTime,2);
    end1 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[0].endTime+3,2);
    end1 += atoi(buff);   
    
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[1].endTime,2);
    end2 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[1].endTime+3,2);
    end2 += atoi(buff);   
    
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[2].endTime,2);
    end3 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gTemplateParam.holidayMode[2].endTime+3,2);
    end3 += atoi(buff);       


    if(localTime>begin1 && localTime<end1)
    {
        ret = 1;
    }

    
    if(localTime>begin2 && localTime<end2)
    {
        ret = 1;
    }

    
    if(localTime>begin3 && localTime<end3)
    {
        ret = 1;
    }


    return ret;
}


static void getDevData(char *src,int icFlag,int qrFlag,READER_BUFF_STRU *desc)
{
    READER_BUFF_STRU readerBuff = {0}; 
    uint8_t key[16] ={ 0x82,0x5d,0x82,0xd8,0xd5,0x2f,0xdf,0x85,0x28,0xa2,0xb5,0xd8,0x88,0x88,0x88,0x88 }; 
    uint8_t bcdBuff[512] = {0};
    memset(&readerBuff,0x00,sizeof(READER_BUFF_STRU));   

    //Ĭ����֧�����͵�
    readerBuff.state = ENABLE;

    //ȥ��0D0A
    if(strlen(src)-2 >QUEUE_BUF_LEN)
    {
        readerBuff.dataLen = QUEUE_BUF_LEN;
    }
    else
    {
        readerBuff.dataLen = strlen(src)-2;
    }
    
    //�ж���ˢ������QR
    if(strstr_t((const char*)src,(const char*)"CARD") == NULL)
    {
        //QR
        readerBuff.authMode = AUTH_MODE_QR;   
        asc2bcd(bcdBuff, (uint8_t *)src, readerBuff.dataLen, 0);
        Des3_2(key, bcdBuff, readerBuff.dataLen/2, readerBuff.data, 1);
    }
    else
    {
        readerBuff.authMode = AUTH_MODE_CARD;        
        memcpy(readerBuff.data,src,readerBuff.dataLen);
    }

    log_d("readerBuff.data = %s\r\n",readerBuff.data);
    
    if(icFlag == 0)
    {
        //��ֵ   
        if(readerBuff.authMode == AUTH_MODE_CARD)
        {
            readerBuff.state = DISABLE;
            readerBuff.dataLen = 0;
            memset(readerBuff.data,0x00,sizeof(readerBuff.data)); 
            log_d("no support IC card\r\n");
        }
    }
    
    if(qrFlag == 0)
    {
        //��ֵ   
        if(readerBuff.authMode == AUTH_MODE_QR)
        {
            readerBuff.state = DISABLE;
            readerBuff.dataLen = 0;
            memset(readerBuff.data,0x00,sizeof(readerBuff.data)); 
            log_d("no support QR code\r\n");
        }                              
    } 
    
    *desc = readerBuff;    
}

            
