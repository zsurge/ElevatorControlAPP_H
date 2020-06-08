/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Key_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��25��
  ����޸�   :
  ��������   : ������������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��25��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "key_task.h"
#include "bsp_key.h"
#include "bsp_dipSwitch.h"
#include "bsp_ds1302.h"
#include "easyflash.h"
#include "tool.h"
#include "bsp_beep.h"
#include "localdata.h"

#define LOG_TAG    "keyTask"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define KEY_STK_SIZE        (configMINIMAL_STACK_SIZE*4)
#define KEY_TASK_PRIO	    ( tskIDLE_PRIORITY + 3)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *keyTaskName = "vKeyTask";     

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskKey = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskKey(void *pvParameters);
static void check_msg_queue(void);


void CreateKeyTask(void)
{
    //����
    xTaskCreate((TaskFunction_t )vTaskKey,         
                (const char*    )keyTaskName,       
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )KEY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskKey); 
}

static void vTaskKey(void *pvParameters)
{
    
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[1024];


    uint32_t g_memsize;

    log_d("start vTaskKey\r\n");
    while(1)
    {
        ucKeyCode = bsp_Key_Scan(0);      
		
		if (ucKeyCode != KEY_NONE)
		{
            //dbg("ucKeyCode = %d\r\n",ucKeyCode);
              
			switch (ucKeyCode)
			{
				/* K1������ ��ӡ����ִ����� */
				case KEY_SET_PRES:	             
					printf("=================================================\r\n");
					printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
                    
					printf("\r\n������       ���м���         ʹ����\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);          

                    g_memsize = xPortGetFreeHeapSize();
                    printf("ϵͳ��ǰ�ڴ��СΪ %d �ֽڣ���ʼ�����ڴ�\n",g_memsize);
                    
					break;				
				/* K2�����£���ӡ���ڲ������� */
				case KEY_RR_PRES:                 
                    check_msg_queue();                    

//                    bsp_ds1302_mdifytime("2020-01-17 09:24:15");
                    
                    log_d("read gpio = %02x\r\n",bsp_dipswitch_read());
//                    testSplit();
//                    eraseUserDataAll();

//                      ee_test();
//			        
					break;
				case KEY_LL_PRES:   
                    log_i("KEY_DOWN_K3\r\n");
//                    ef_env_set_default();
//                    calcRunTime();       
//                    bsp_ds1302_mdifytime("2020-03-31 10:18:20");

                    log_d("bsp_ds1302_readtime = %s\r\n",bsp_ds1302_readtime());
//                    ef_set_env_blob("device_sn","88888888",8); 
                    time_to_timestamp();
//                    ef_print_env();

//                    searchHeaderIndex("00012926",USER_MODE,&index);


					break;
				case KEY_OK_PRES:    
//                    test_env();
                    log_w("KEY_DOWN_K4\r\n");
                    ef_set_env_blob("sn_flag","0000",4); 
//                    ef_set_env_blob("remote_sn","7A13DCC67054F72CC07F",20);
//                ef_set_env_blob("remote_sn","823545AE9B2345B08FD8",20);
                    
					break;                
				
				/* �����ļ�ֵ������ */
				default:   
				log_e("KEY_default\r\n");
					break;
			}
		}

        /* �����¼���־����ʾ������������ */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
		
		vTaskDelay(20);
	}   

}


//��ѯMessage_Queue�����е��ܶ���������ʣ���������
void check_msg_queue(void)
{
    
	u8 msgq_remain_size;	//��Ϣ����ʣ���С
    u8 msgq_total_size;     //��Ϣ�����ܴ�С
    
    taskENTER_CRITICAL();   //�����ٽ���
    msgq_remain_size=uxQueueSpacesAvailable(xDataProcessQueue);//�õ�����ʣ���С
    msgq_total_size=uxQueueMessagesWaiting(xDataProcessQueue)+uxQueueSpacesAvailable(xDataProcessQueue);//�õ������ܴ�С���ܴ�С=ʹ��+ʣ��ġ�
	printf("Total Size = %d, Remain Size = %d\r\n",msgq_total_size,msgq_remain_size);	//��ʾDATA_Msg��Ϣ�����ܵĴ�С

    taskEXIT_CRITICAL();    //�˳��ٽ���
}




