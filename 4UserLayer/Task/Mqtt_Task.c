/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Mqtt_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��28��
  ����޸�   :
  ��������   : MQTTͨѶ��������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��28��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "Mqtt_Task.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MQTT_TASK_PRIO	    (tskIDLE_PRIORITY)
#define MQTT_STK_SIZE 		(configMINIMAL_STACK_SIZE*2)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *MqttTaskName = "vMQTTTask";      //���Ź�������


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskMqtt = NULL;      //LED��


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskMQTT(void *pvParameters);

void CreateMqttTask(void *pvParameters)
{
    //����LED����
    xTaskCreate((TaskFunction_t )vTaskLed,         
                (const char*    )ledTaskName,       
                (uint16_t       )LED_STK_SIZE, 
                (void*          )pvParameters,              
                (UBaseType_t    )LED_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskLed);   

}
