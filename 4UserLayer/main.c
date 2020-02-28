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
#define LED_TASK_PRIO	    ( tskIDLE_PRIORITY)
#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define READER_TASK_PRIO	( tskIDLE_PRIORITY + 1)
#define QR_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define KEY_TASK_PRIO	    ( tskIDLE_PRIORITY + 2)
#define DISPLAY_TASK_PRIO	( tskIDLE_PRIORITY + 2)
#define MQTT_TASK_PRIO	    ( tskIDLE_PRIORITY + 3)
#define COMM_TASK_PRIO		( tskIDLE_PRIORITY + 4)
#define START_TASK_PRIO		( tskIDLE_PRIORITY + 5)

#define LED_STK_SIZE 		(256)
#define COMM_STK_SIZE 		(1024*1)
#define START_STK_SIZE 	    (512)
#define QR_STK_SIZE 		(512)
#define READER_STK_SIZE     (512)
#define HANDSHAKE_STK_SIZE  (256)
#define KEY_STK_SIZE        (1024*1)
#define MQTT_STK_SIZE        (1024*2)
#define DISPLAY_STK_SIZE     (512)

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
//������
static TaskHandle_t xHandleTaskLed = NULL;      //LED��
static TaskHandle_t xHandleTaskComm = NULL;      //������ͨѶ
static TaskHandle_t xHandleTaskReader = NULL;   //Τ��������
static TaskHandle_t xHandleTaskQr = NULL;       //��ά���ͷ
static TaskHandle_t xHandleTaskStart = NULL;    //���Ź�
static TaskHandle_t xHandleTaskHandShake = NULL;    // ����
static TaskHandle_t xHandleTaskKey = NULL;      //����
static TaskHandle_t xHandleTaskMqtt = NULL;      //MQTT ����
static TaskHandle_t xHandleTaskDisplay = NULL;      //�����

//�¼����
EventGroupHandle_t xCreatedEventGroup = NULL;
SemaphoreHandle_t gxMutex = NULL;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

//������
static void vTaskLed(void *pvParameters);
static void vTaskKey(void *pvParameters);
static void vTaskReader(void *pvParameters);
static void vTaskQR(void *pvParameters);
static void vTaskStart(void *pvParameters);
//���Ϳ�������
static void vTaskHandShake(void *pvParameters);
static void vTaskMqttTest(void *pvParameters);
static void vTaskDisplay(void *pvParameters);
static void vTaskComm(void *pvParameters);



static void AppTaskCreate(void);
static void AppObjCreate (void);
static void App_Printf(char *format, ...);

static void EasyLogInit(void);
void check_msg_queue(void);
static void DisplayDevInfo (void);

static void DisplayDevInfo(void)
{
	printf("Softversion :%s\r\n",gDevinfo.SoftwareVersion);
    printf("HardwareVersion :%s\r\n", gDevinfo.HardwareVersion);
	printf("Model :%s\r\n", gDevinfo.Model);
	printf("ProductBatch :%s\r\n", gDevinfo.ProductBatch);	    
	printf("BulidDate :%s\r\n", gDevinfo.BulidDate);
	printf("DevSn :%s\r\n", gDevinfo.GetSn());
    printf("Devip :%s\r\n", gDevinfo.GetIP());
}

int main(void)
{   
    //Ӳ����ʼ��
    bsp_Init();  

    EasyLogInit();  
    
	/* ��������ͨ�Ż��� */
	AppObjCreate();

	/* �������� */
	AppTaskCreate();
    
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

    StartEthernet();   



    //������ͨѶ
    xTaskCreate((TaskFunction_t )vTaskComm,
                (const char*    )"vTaskComm",       
                (uint16_t       )COMM_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )COMM_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskComm);              

  
 

    //MQTTͨѶ mqtt_thread
    xTaskCreate((TaskFunction_t )vTaskMqttTest,//vTaskMqttTest, mqtt_thread    
                (const char*    )"vMqttTest",   
                (uint16_t       )MQTT_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )MQTT_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskMqtt); 


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
    
    xTransQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                              (UBaseType_t ) sizeof(READER_BUFF_T *));/* ��Ϣ�Ĵ�С */
    if(xTransQueue == NULL)
    {
        App_Printf("����xTransQueue��Ϣ����ʧ��!\r\n");
    }
    else
    {
        App_Printf("create queue success!\r\n");
    }


}



/*
*********************************************************************************************************
*	�� �� ��: vTaskStart
*	����˵��: �������񣬵ȴ����������¼���־������
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 4  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* ����ӳ�100ms */   
    
	/* 
	  ��ʼִ����������������ǰʹ�ܶ������Ź���
	  ����LSI��32��Ƶ�����溯��������Χ0-0xFFF���ֱ������Сֵ1ms�����ֵ4095ms
	  �������õ���4s�����4s��û��ι����ϵͳ��λ��

      LSI��Ƶ����Ϊ [4,8,16,32,64,128,256],
      ��Ӧ�����ʱ��Ϊ��Ϊ��409.6 819.2 1638.4 3276.8 6553.6 13107.226214.4ms

      ���㹫ʽΪ Tout=(4 * 2^PR * ��RL+1��)/40 ms
      ��Сʱ�䵥λ��1/40 ms
      ���Ƿ�Ƶϵ�� Prescaler��4-256����1*Prescaler/40 ms
      ����RLֵ(0-FFF)��1*Prescaler*��RL+1��/40 ms

      Ŀǰ����ʹ�õ���128��Ƶ�����13.1��
	*/
	bsp_InitIwdg(4095);
	
	/* ��ӡϵͳ����״̬������鿴ϵͳ�Ƿ�λ */
	App_Printf("=====================================================\r\n");
	App_Printf("ϵͳ����ִ��\r\n");
	App_Printf("=====================================================\r\n");
	
    while(1)
    {   
        
		/* �ȴ������������¼���־ */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* �¼���־���� */
							         TASK_BIT_ALL,       /* �ȴ�TASK_BIT_ALL������ */
							         pdTRUE,             /* �˳�ǰTASK_BIT_ALL�������������TASK_BIT_ALL�������òű�ʾ���˳���*/
							         pdTRUE,             /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
							         xTicksToWait); 	 /* �ȴ��ӳ�ʱ�� */
		
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



static void vTaskComm(void *pvParameters)
{
    uint8_t recvLen = 0;
    uint8_t buf[5+1] = {0};
    uint8_t crc = 0;    
    uint8_t sendBuf[64] = {0};
    
    READER_BUFF_T *ptMsg;
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* �������ȴ�ʱ��Ϊ200ms */  
    
    //��ȡ��ǰ�豸��ID
    uint16_t readID = bsp_dipswitch_read();

    log_d("current dev addr =%d\r\n",readID);
    
    while (1)
    {
//        recvLen = RS485_Recv(COM5,buf,MAX_CMD_LEN);
//        
//        //�ж����ݵ���Ч��
//        if(recvLen != MAX_CMD_LEN || buf[0] != CMD_STX || buf[1]<1 || buf[1]>4)
//        {
//            vTaskDelay(500); 
//            continue;
//        }

//        crc= xorCRC(buf,3);
//        
//        if(crc != buf[3])
//        {
//            vTaskDelay(500); 
//            continue;
//        }
        
//        if(buf[1] == readID)
        {
            xReturn = xQueueReceive( xTransQueue,    /* ��Ϣ���еľ�� */
                                     (void *)&ptMsg,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                     xMaxBlockTime); /* ��������ʱ�� */
            if(pdTRUE == xReturn)
            {
                //��Ϣ���ճɹ������ͽ��յ�����Ϣ
                packetSendBuf(ptMsg,sendBuf);     
            }
            else
            {
                //����Ĭ�����ݰ�
                packetDefaultSendBuf(sendBuf); //���  
            }

            RS485_SendBuf(COM4,sendBuf,MAX_SEND_LEN);

        }



		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
        vTaskDelay(300);
    }

}



//LED������ 
static void vTaskLed(void *pvParameters)
{  
    uint8_t i = 0;
    BEEP = 1;
    vTaskDelay(300);
    BEEP = 0;
    
    while(1)
    {  
        if(i == 250)
        {
            i = 100;
        }
        i+=20;
        bsp_SetTIMOutPWM(GPIOG, GPIO_Pin_8, TIM1, 1, 100, ((i) * 10000) /255);
        
		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);  
        vTaskDelay(100); 
    }
}   

static void vTaskKey(void *pvParameters)
{
    
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];

    float sin_value = 0.0;

    uint16_t crc_value = 0;

    uint8_t cm4[] = { 0x02,0x7B,0x22,0x63,0x6D,0x64,0x22,0x3A,0x22,0x75,0x70,0x64,0x61,0x74,0x65,0x22,0x2C,0x22,0x76,0x61,0x6C,0x75,0x65,0x22,0x3A,0x7B,0x22,0x75,0x70,0x64,0x61,0x74,0x65,0x22,0x3A,0x22,0x41,0x37,0x22,0x7D,0x2C,0x22,0x64,0x61,0x74,0x61,0x22,0x3A,0x22,0x30,0x30,0x22,0x7D,0x03 };

    uint32_t g_memsize;
    
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
					App_Printf("=================================================\r\n");
					App_Printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
					vTaskList((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);
                    
					App_Printf("\r\n������       ���м���         ʹ����\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);          

                    g_memsize = xPortGetFreeHeapSize();
                    printf("ϵͳ��ǰ�ڴ��СΪ %d �ֽڣ���ʼ�����ڴ�\n",g_memsize);
                    
					break;				
				/* K2�����£���ӡ���ڲ������� */
				case KEY_RR_PRES:                 
                    check_msg_queue();
                    
//                    ef_print_env();

//                    bsp_ds1302_mdifytime("2020-01-17 09:24:15");
                    
                    log_d("read gpio = %02x\r\n",bsp_dipswitch_read());
//                    testSplit();

                      ee_test();
//			        
					break;
				case KEY_LL_PRES:   
                    log_i("KEY_DOWN_K3\r\n");
//                    ef_env_set_default();
//                    calcRunTime();           
                    log_d("current time =%s\r\n",bsp_ds1302_readtime());

                    ef_set_env_blob("sn_flag","0000",4);

					break;
				case KEY_OK_PRES:    
//                    test_env();
                    log_w("KEY_DOWN_K4\r\n");
                    crc_value = CRC16_Modbus(cm4, 54);
                    log_v("hi = %02x, lo = %02x\r\n", crc_value>>8, crc_value & 0xff);

                    ef_set_env_blob("3867", "89E1E35D;10;10;2019-12-29;2029-12-31",strlen("89E1E35D;10;10;2019-12-29;2029-12-31")); 
                    ef_set_env_blob("3896", "89E1E35D;8;8;2020-01-03;2029-12-31",strlen("89E1E35D;8;8;2020-01-03;2029-12-31")); 
                    ef_set_env_blob("89E1E35D", "3867;8,9,10,11,12;9;2019-12-29;2029-12-31",strlen("3867;8,9,10,11,12;9;2019-12-29;2029-12-31"));                    
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


static void vTaskMqttTest(void *pvParameters)
{
    mqtt_thread();

    while(1)
    {
        vTaskDelay(1000);        
    }
    
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



static void vTaskReader(void *pvParameters)
{ 
    uint32_t CardID = 0;
    uint8_t dat[4] = {0};
    uint8_t asc[9] = {0};
    uint8_t tmp[26] ={ 0x43,0x41,0x52,0x44,0x20,0x32,0x33,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x38,0x39,0x30,0x30,0x30,0x30,0x30,0x30,0x0d,0x0a };
    READER_BUFF_T *ptReader; 
	/* ��ʼ���ṹ��ָ�� */
	ptReader = &gReaderMsg;
	
	/* ���� */
    ptReader->dataLen = 0;
    ptReader->authMode = AUTH_MODE_CARD;
    memset(ptReader->data,0x00,sizeof(ptReader->data)); 

    while(1)
    {
        CardID = bsp_WeiGenScanf();

        if(CardID != 0)
        {
            memset(dat,0x00,sizeof(dat));            
            
//			dat[0] = CardID>>24;
			dat[0] = CardID>>16;
			dat[1] = CardID>>8;
			dat[2] = CardID&0XFF;    

            dbh("card id",(char *)dat,3);
            
            bcd2asc(asc, dat, 6, 0);
            log_d("asc = %s\r\n",asc);
            
            memcpy(tmp+17,asc,6);
            log_d("tmp = %s\r\n",tmp);
            
            ptReader->dataLen = 25;
            memcpy(ptReader->data,tmp,ptReader->dataLen);

			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
			if(xQueueSend(xTransQueue,              /* ��Ϣ���о�� */
						 (void *) &ptReader,   /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
						 (TickType_t)50) != pdPASS )
			{
                DBG("the queue is full!\r\n");                             
            } 
            else
            {
                dbh("WGREADER",(char *)dat,4);
            }          

          
        }
        
    	/* �����¼���־����ʾ������������ */        
    	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);       
        
        vTaskDelay(100);        
    }

}   


static void vTaskQR(void *pvParameters)
{ 
    uint8_t recv_buf[255] = {0};
    uint16_t len = 0; 

    READER_BUFF_T *ptQR; 
 	/* ��ʼ���ṹ��ָ�� */
	ptQR = &gReaderMsg;
	
	/* ���� */
    ptQR->authMode = AUTH_MODE_CARD; //Ĭ��Ϊˢ��
    ptQR->dataLen = 0;
    memset(ptQR->data,0x00,sizeof(ptQR->data)); 
    
    while(1)
    {   
           memset(recv_buf,0x00,sizeof(recv_buf));
           len = RS485_RecvAtTime(COM5,recv_buf,sizeof(recv_buf),800);
           if(recv_buf[len-1] == 0x00 && len > 0)
           {
                len -= 1; //���ﲻ֪��Ϊʲô�����һ��0x00
           }
           
           if(len > 0  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
           {
                log_d("reader = %s\r\n",recv_buf);      

                //�ж���ˢ������QR
                //if(strstr_t(recv_buf,"CARD") == NULL)
                if(len > 50)
                {
                    //QR
                    ptQR->authMode = AUTH_MODE_QR;
                }                               

                ptQR->dataLen = len;                
                memcpy(ptQR->data,recv_buf,len);

    			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    			if(xQueueSend(xTransQueue,              /* ��Ϣ���о�� */
    						 (void *) &ptQR,   /* ����ָ�����recv_buf�ĵ�ַ */
    						 (TickType_t)50) != pdPASS )
    			{
                    DBG("the queue is full!\r\n");                
                    xQueueReset(xTransQueue);
                } 
                else
                {
                    dbh("QR",(char *)recv_buf,len);
                }                
           }
    

		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
        vTaskDelay(300);        
    }
}   


static void vTaskHandShake(void *pvParameters)
{
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[12] = {0};
    uint8_t bcdbuf[6] = {0};
    

    
    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
    assert_param(c_old_boot_times);
    i_boot_times = atol(c_old_boot_times);
    
    /* boot count +1 */
    i_boot_times ++;

    /* interger to string */
    sprintf(c_new_boot_times,"%012ld", i_boot_times);
    
    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);    

    asc2bcd(bcdbuf,(uint8_t *)c_new_boot_times , 12, 0);

    log_d("local time = %s\r\n",bsp_ds1302_readtime());
    
    vTaskDelete( NULL ); //ɾ���Լ�
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


//��ѯMessage_Queue�����е��ܶ���������ʣ���������
void check_msg_queue(void)
{
    
	u8 msgq_remain_size;	//��Ϣ����ʣ���С
    u8 msgq_total_size;     //��Ϣ�����ܴ�С
    
    taskENTER_CRITICAL();   //�����ٽ���
    msgq_remain_size=uxQueueSpacesAvailable(xTransQueue);//�õ�����ʣ���С
    msgq_total_size=uxQueueMessagesWaiting(xTransQueue)+uxQueueSpacesAvailable(xTransQueue);//�õ������ܴ�С���ܴ�С=ʹ��+ʣ��ġ�
	printf("Total Size = %d, Remain Size = %d\r\n",msgq_total_size,msgq_remain_size);	//��ʾDATA_Msg��Ϣ�����ܵĴ�С

    taskEXIT_CRITICAL();    //�˳��ٽ���
}

