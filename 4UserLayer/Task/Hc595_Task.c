/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Hc595_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��25��
  ����޸�   :
  ��������   : ����ܲ���
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��25��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "hc595_task.h"
#include "bsp_digitaltube.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define DISPLAY_TASK_PRIO	( tskIDLE_PRIORITY + 2)
#define DISPLAY_STK_SIZE    (configMINIMAL_STACK_SIZE)  

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskDisplay = NULL;      //�����


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
const char *HC595TaskName = "vHC595Task"; 

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void vTaskDisplay(void *pvParameters);

void CreateHc595Task(void)
{
    //�����
    xTaskCreate((TaskFunction_t )vTaskDisplay,
                (const char*    )HC595TaskName,       
                (uint16_t       )DISPLAY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )DISPLAY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskDisplay);     

}

static void vTaskDisplay(void *pvParameters)
{
    while(1)
    {
        bsp_HC595Show(1,2,3);
        vTaskDelay(300);
        bsp_HC595Show(4,5,6);
        vTaskDelay(300);
        bsp_HC595Show(7,8,9);
        vTaskDelay(300);
        bsp_HC595Show('a','b','c');
        vTaskDelay(300);
        bsp_HC595Show('d','e','f');     
        vTaskDelay(300);
        bsp_HC595Show('a',0,1);
        vTaskDelay(300);
        bsp_HC595Show('d',3,4);   
        vTaskDelay(300);
        bsp_HC595Show(1,0,1);  
        
        /* �����¼���־����ʾ������������ */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_3);     
        vTaskDelay(300);
    }  

}



