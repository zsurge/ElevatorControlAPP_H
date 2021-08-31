/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : HandShake_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��26��
  ����޸�   :
  ��������   : ����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��26��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "handshake_task.h"
#include "easyflash.h"
#include "stdlib.h"
#include "tool.h"
#include "bsp_ds1302.h"
#include "ini.h"
#include "bsp_digitaltube.h"
#include "bsp_led.h"


#define LOG_TAG    "handShake"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define HANDSHAKE_STK_SIZE  (configMINIMAL_STACK_SIZE*2)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *handShakeTaskName = "vHandShakeTask";      //���Ź�������


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskHandShake = NULL;      //LED��


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskHandShake(void *pvParameters);
static void DisplayDevInfo (void);


static void vTaskHandShake(void *pvParameters)
{

    //��ȡ����ʱ��
    log_d("bsp_ds1302_readtime= %s\r\n",bsp_ds1302_readtime());

    //��ȡģ������
//    readTemplateData();
//    readDevState();
//    clearTemplateFRAM();

//    eraseUserDataAll();

    initRecordIndex();
    
    initDevBaseParam();

    initTemplateParam();
    
    DisplayDevInfo();   


    while(1)
    {  
        bsp_HC595Show('A',1,1);
        vTaskDelay(800);
        bsp_HC595Show(0,0,1);
        vTaskDelay(800);

        LEDERROR = !LEDERROR;
		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);  
      
    }    
}

void CreateHandShakeTask(void)
{  
    //��android����
    xTaskCreate((TaskFunction_t )vTaskHandShake,
                (const char*    )handShakeTaskName,       
                (uint16_t       )HANDSHAKE_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )HANDSHAKE_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskHandShake);  
                

}


static void DisplayDevInfo(void)
{
    printf("\r\n==========Version==========\r\n");
	printf("Softversion :%s\r\n",gDevinfo.SoftwareVersion);
    printf("HardwareVersion :%s\r\n", gDevinfo.HardwareVersion);
	printf("Model :%s\r\n", gDevinfo.Model);
	printf("ProductBatch :%s\r\n", gDevinfo.ProductBatch);	    
	printf("BulidDate :%s\r\n", gDevinfo.BulidDate);
	printf("DevSn :%s\r\n", gDevinfo.GetSn());
    printf("Devip :%s\r\n", gDevinfo.GetIP());
	printf("DevID :%s\r\n", gDevBaseParam.deviceCode.qrSn);
}




