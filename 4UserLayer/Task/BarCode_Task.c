/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : BarCode_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月27日
  最近修改   :
  功能描述   : 条码处理任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月27日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
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
 * 宏定义                                       *
 *----------------------------------------------*/
#define BARCODE_TASK_PRIO		(tskIDLE_PRIORITY + 1)
#define BARCODE_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *BarCodeTaskName = "vBarCodeTask";  

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskBarCode = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskBarCode(void *pvParameters);


static int compareDate(uint8_t *date1,uint8_t *date2);
static int compareTime(const char *currentTime);




void CreateBarCodeTask(void)
{
    //读取条码数据并处理
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
 	/* 初始化结构体指针 */
	ptQR = &gReaderMsg;

    log_d("start vTaskBarCode\r\n");
    while(1)
    {   
        /* 清零 */
        ptQR->authMode = 0; 
        ptQR->dataLen = 0;
        memset(ptQR->data,0x00,sizeof(ptQR->data)); 

        memset(recv_buf,0x00,sizeof(recv_buf));           
        len = RS485_RecvAtTime(COM5,recv_buf,sizeof(recv_buf),800);

        if(recv_buf[len-1] == 0x00 && len > 1)
        {
            len -= 1; //这里不知道为什么会多了一个0x00
        }
       
        if(len > 0  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
        {
            log_d("reader = %s\r\n",recv_buf);      

            //判定是刷卡还是QR
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

            //添加模版是否启用的判定
            if(gTemplateParam.templateStatus == 0)
            {
                continue;//模板没有启用，不对电梯进行控制
            }


            //读取当前时间
            memcpy(localTime,bsp_ds1302_readtime(),8);
            
            //判定有效期
            //if(compareDate(localTime,gTemplateParam.peakInfo[0].endTime) < 0)
            
            //判定有效期内不受控时间段
            cmpTimeFlag = compareTime(localTime);

            if(cmpTimeFlag == -1
)
            {
                //不在受控时间段
            }
            else if(cmpTimeFlag == 0)
            {
                //没有指定受控时间段
            }
            else
            {
                //在受控时间段内
            }
                
            
            //添加呼梯方式的判定
            
            
                
            //添加有效日期的判定
            //添加不受控时段的判定

        	/* 使用消息队列实现指针变量的传递 */
        	if(xQueueSend(xTransQueue,              /* 消息队列句柄 */
        				 (void *) &ptQR,   /* 发送指针变量recv_buf的地址 */
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


	/* 发送事件标志，表示任务正常运行 */        
	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
    vTaskDelay(300);        
    }
}

/*
该算法总体思想是计算给定日期到 0年3月1日的天数，然后相减，获取天数的间隔。

m1 = (month_start + 9) % 12; 用于判断日期是否大于3月（2月是判断闰年的标识），还用于纪录到3月的间隔月数。

y1 = year_start - m1/10; 如果是1月和2月，则不包括当前年（因为是计算到0年3月1日的天数）。

d1 = 365*y1 + y1/4 - y1/100 + y1/400 + (m1*306 + 5)/10 + (day_start - 1);

? ? 其中 365*y1 是不算闰年多出那一天的天数，

? ? y1/4 - y1/100 + y1/400 ?是加所有闰年多出的那一天，

(m2*306 + 5)/10?用于计算到当前月到3月1日间的天数，306=365-31-28（1月和2月），5是全年中不是31天月份的个数

(day_start - 1)?用于计算当前日到1日的间隔天数。
————————————————
原文链接：https://blog.csdn.net/a_ran/article/details/43601699
*/



//返回date1和date2中间相关的天数，若date2小于date1，则返回负数
//只能计算2000~2099年
static int compareDate(uint8_t *date1,uint8_t *date2)
{
    int y2, m2, d2;
	int y1, m1, d1;
    int year_start,month_start,day_start;
    int year_end,month_end,day_end;
    char buff[4] = {0};

    if(strlen((const char*)date2) < 9)
    {
        return -1;//错误的日期
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

//判定当前时间段，是否在time123时间段内，若是返回1，不是返回-1，若没有指定时间段则返回0
static int compareTime(const char *currentTime)
{
    int ret = -1;
    int localTime = 0;
    char buff[4] = {0};

    int begin1,begin2,begin3,end1,end2,end3;

    if(strlen(gTemplateParam.hoildayMode[0].startTime)==0 && strlen(gTemplateParam.hoildayMode[1].startTime)==0 && strlen(gTemplateParam.hoildayMode[2].startTime)==0)
    {
        //没有指定时间段
        ret = 0;
        return ret;
    }

    //把时间转换为分钟
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

            
