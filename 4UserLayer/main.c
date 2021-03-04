/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��9��
  ����޸�   :
  ��������   : ������ģ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��9��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "def.h"

#define LOG_TAG    "main"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
//�������ȼ� 
#define APPCREATE_TASK_PRIO		(tskIDLE_PRIORITY)
#define APPCREATE_STK_SIZE 		(configMINIMAL_STACK_SIZE*16)

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
const char *AppCreateTaskName = "vAppCreateTask"; 

//������
static TaskHandle_t xHandleTaskAppCreate = NULL;     

SemaphoreHandle_t gxMutex = NULL;
EventGroupHandle_t xCreatedEventGroup = NULL;
QueueHandle_t xTransDataQueue = NULL; 
QueueHandle_t xDataProcessQueue = NULL;
SemaphoreHandle_t CountSem_Handle = NULL;




/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void AppTaskCreate(void);
static void AppObjCreate (void);
static void App_Printf(char *format, ...);
static void EasyLogInit(void);


int main(void)
{   
    //Ӳ����ʼ��
    bsp_Init();  

    EasyLogInit();  
    
	/* ��������ͨ�Ż��� */
	AppObjCreate();


	
    
    //����AppTaskCreate����
    xTaskCreate((TaskFunction_t )AppTaskCreate,     
                (const char*    )AppCreateTaskName,   
                (uint16_t       )APPCREATE_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )APPCREATE_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskAppCreate);   

    
    /* �������ȣ���ʼִ������ */
    vTaskStartScheduler();


    
    
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
    //�����ٽ���
    taskENTER_CRITICAL();    

    //������ʼ��
    StartEthernet();   

    //����
    CreateHandShakeTask();

    //LED��
    /* BEGIN: Deleted by  , 2021/2/2   ���ⵥ��:012 */
    /* CreateLedTask();  */
    /* END: Deleted by  , 2021/2/2 */

    //������ͨѶ
    CreateCommTask();//1

    //�������ʾ
//    CreateHc595Task();//3

    //���ݴ���
    CreateDataProcessTask();//6
    
//    //����
//    CreateKeyTask();

    //����һ������
    CreateBarCodeTask();//4

    //MQTTͨѶ
    CreateMqttTask();//5

    //���Ź�
    CreateWatchDogTask();

    //ɾ������
    vTaskDelete(xHandleTaskAppCreate); //ɾ��AppTaskCreate����

    //�˳��ٽ���
    taskEXIT_CRITICAL();   

}




/*
*********************************************************************************************************
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨ�Ż���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* �����¼���־�� */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
        App_Printf("�����¼���־��ʧ��\r\n");
    }

	/* ���������ź��� */
    gxMutex = xSemaphoreCreateMutex();
	
	if(gxMutex == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
        App_Printf("���������ź���ʧ��\r\n");
    }    

    //����Ϣ���У����ˢ������ά������

    xDataProcessQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                              (UBaseType_t ) sizeof(READER_BUFF_STRU *));/* ��Ϣ�Ĵ�С */
    if(xDataProcessQueue == NULL)
    {
        App_Printf("���� xDataProcessQueue ��Ϣ����ʧ��!\r\n");
    }
    
    
    xTransDataQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                              (UBaseType_t ) sizeof(char *));/* ��Ϣ�Ĵ�С */
    if(xTransDataQueue == NULL)
    {
        App_Printf("���� xTransDataQueue ��Ϣ����ʧ��!\r\n");
    }
   

    /*  ���� CountSem */
    CountSem_Handle = xSemaphoreCreateCounting(2,2);
    if (NULL != CountSem_Handle)
        App_Printf("CountSem_Handle  �����ź��������ɹ�!\r\n");

}


/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void  App_Printf(char *format, ...)
{
    char  buf_str[512 + 1];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* �����ź��� */
	xSemaphoreTake(gxMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(gxMutex);
}


static void EasyLogInit(void)
{
    /* initialize EasyLogger */
     elog_init();
     /* set EasyLogger log format */
     elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
     elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_TIME);
     elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_TIME);

     
     /* start EasyLogger */
     elog_start();  
}




