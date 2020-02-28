/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Watchdog_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��27��
  ����޸�   :
  ��������   : ���Ź�����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��27��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "Watchdog_Task.h"
#include "tool.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define WATCHDOG_TASK_PRIO		( tskIDLE_PRIORITY + 5)
#define WATCHDOG_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *WatchDogTaskName = "vWatchDogTask";  

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskWatchDog = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskWatchDog(void *pvParameters);

void CreateWatchDogTask(void)
{
    //���Ź�
    xTaskCreate((TaskFunction_t )vTaskWatchDog,     
                (const char*    )WatchDogTaskName,   
                (uint16_t       )WATCHDOG_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )WATCHDOG_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskWatchDog);
}



static void vTaskWatchDog(void *pvParameters)
{
    EventBits_t uxBits;
    const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* ����ӳ�1000ms */   
    
    /* 
      ��ʼִ����������������ǰʹ�ܶ������Ź���
      ����LSI��32��Ƶ�����溯��������Χ0-0xFFF���ֱ������Сֵ1ms�����ֵ4095ms
      �������õ���4s�����4s��û��ι����ϵͳ��λ��
    */
    bsp_InitIwdg(4000);
    
    /* ��ӡϵͳ����״̬������鿴ϵͳ�Ƿ�λ */
    DBG("=====================================================\r\n");
    DBG("ϵͳ����ִ��\r\n");
    DBG("=====================================================\r\n");
    
    while(1)
    {   
        
        /* �ȴ������������¼���־ */
        uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* �¼���־���� */
                                     TASK_BIT_ALL,       /* �ȴ�TASK_BIT_ALL������ */
                                     pdTRUE,             /* �˳�ǰTASK_BIT_ALL�������������TASK_BIT_ALL�������òű�ʾ���˳���*/
                                     pdTRUE,             /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
                                     xTicksToWait);      /* �ȴ��ӳ�ʱ�� */
        
        if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
        {            
            IWDG_Feed(); //ι��           
        }
        else
        {
            /* ������ÿxTicksToWait����һ�� */
            /* ͨ������uxBits�򵥵Ŀ����ڴ˴�����Ǹ�������û�з������б�־ */   
        }
    }
}




