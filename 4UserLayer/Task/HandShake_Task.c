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
//    uint32_t i_boot_times = NULL;
//    char *c_old_boot_times, c_new_boot_times[12] = {0};
//    uint8_t bcdbuf[6] = {0};  


//    log_d("start vTaskHandShake\r\n");
//    
//    /* get the boot count number from Env */
//    c_old_boot_times = ef_get_env("boot_times");

//    i_boot_times = atol(c_old_boot_times);
//    
//    /* boot count +1 */
//    i_boot_times ++;

//    /* interger to string */
//    sprintf(c_new_boot_times,"%012ld", i_boot_times);
//    
//    /* set and store the boot count number to Env */
//    ef_set_env("boot_times", c_new_boot_times);    

//    asc2bcd(bcdbuf,(uint8_t *)c_new_boot_times , 12, 0);

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
    
//    vTaskDelay(500);
//    
//    vTaskDelete( NULL ); //ɾ���Լ�

    while(1)
    {  
        bsp_HC595Show('A',0,1);
        vTaskDelay(800);
        bsp_HC595Show(0,0,2);
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




