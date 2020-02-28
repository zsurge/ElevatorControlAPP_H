/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : all_task.h
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��25��
  ����޸�   :
  ��������   : ��������ͷ�ļ��ϼ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��25��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

 *----------------------------------------------*/
 //�������ȼ� 

#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define READER_TASK_PRIO	( tskIDLE_PRIORITY + 1)
#define QR_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define MQTT_TASK_PRIO	    ( tskIDLE_PRIORITY + 3)
#define COMM_TASK_PRIO		( tskIDLE_PRIORITY + 4)
#define START_TASK_PRIO		( tskIDLE_PRIORITY + 5)

//�������ռ�    

#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)
#define START_STK_SIZE 	    (configMINIMAL_STACK_SIZE*4)
#define QR_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)
#define READER_STK_SIZE     (configMINIMAL_STACK_SIZE*4)
#define HANDSHAKE_STK_SIZE  (configMINIMAL_STACK_SIZE*2)

#define MQTT_STK_SIZE       (configMINIMAL_STACK_SIZE*16)
  

//������

extern  TaskHandle_t xHandleTaskComm  ;      //������ͨѶ
extern  TaskHandle_t xHandleTaskReader ;   //Τ��������
extern  TaskHandle_t xHandleTaskQr  ;       //��ά���ͷ
extern  TaskHandle_t xHandleTaskStart  ;    //���Ź�
extern  TaskHandle_t xHandleTaskHandShake  ;    // ����

extern  TaskHandle_t xHandleTaskMqtt  ;      //MQTT ����



//������


void vTaskReader(void *pvParameters);
void vTaskQR(void *pvParameters);
void vTaskStart(void *pvParameters);
void vTaskHandShake(void *pvParameters);
void vTaskMqttTest(void *pvParameters);

void vTaskComm(void *pvParameters);

