/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月9日
  最近修改   :
  功能描述   : 主程序模块
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月9日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "def.h"

#define LOG_TAG    "main"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
//任务优先级 
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
 * 模块级变量                                   *
 *----------------------------------------------*/
//任务句柄
static TaskHandle_t xHandleTaskLed = NULL;      //LED灯
static TaskHandle_t xHandleTaskComm = NULL;      //跟主机通讯
static TaskHandle_t xHandleTaskReader = NULL;   //韦根读卡器
static TaskHandle_t xHandleTaskQr = NULL;       //二维码读头
static TaskHandle_t xHandleTaskStart = NULL;    //看门狗
static TaskHandle_t xHandleTaskHandShake = NULL;    // 握手
static TaskHandle_t xHandleTaskKey = NULL;      //按键
static TaskHandle_t xHandleTaskMqtt = NULL;      //MQTT 测试
static TaskHandle_t xHandleTaskDisplay = NULL;      //数码管

//事件句柄
EventGroupHandle_t xCreatedEventGroup = NULL;
SemaphoreHandle_t gxMutex = NULL;


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

//任务函数
static void vTaskLed(void *pvParameters);
static void vTaskKey(void *pvParameters);
static void vTaskReader(void *pvParameters);
static void vTaskQR(void *pvParameters);
static void vTaskStart(void *pvParameters);
//上送开机次数
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
    //硬件初始化
    bsp_Init();  

    EasyLogInit();  
    
	/* 创建任务通信机制 */
	AppObjCreate();

	/* 创建任务 */
	AppTaskCreate();
    
    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();
    
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{

    StartEthernet();   



    //跟电梯通讯
    xTaskCreate((TaskFunction_t )vTaskComm,
                (const char*    )"vTaskComm",       
                (uint16_t       )COMM_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )COMM_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskComm);              

  
 

    //MQTT通讯 mqtt_thread
    xTaskCreate((TaskFunction_t )vTaskMqttTest,//vTaskMqttTest, mqtt_thread    
                (const char*    )"vMqttTest",   
                (uint16_t       )MQTT_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )MQTT_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskMqtt); 


}


/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* 创建事件标志组 */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
        App_Printf("创建事件标志组失败\r\n");
    }

	/* 创建互斥信号量 */
    gxMutex = xSemaphoreCreateMutex();
	
	if(gxMutex == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
        App_Printf("创建互斥信号量失败\r\n");
    }    

    //创消息队列，存放刷卡及二维码数据
    
    xTransQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                              (UBaseType_t ) sizeof(READER_BUFF_T *));/* 消息的大小 */
    if(xTransQueue == NULL)
    {
        App_Printf("创建xTransQueue消息队列失败!\r\n");
    }
    else
    {
        App_Printf("create queue success!\r\n");
    }


}



/*
*********************************************************************************************************
*	函 数 名: vTaskStart
*	功能说明: 启动任务，等待所有任务发事件标志过来。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 4  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* 最大延迟100ms */   
    
	/* 
	  开始执行启动任务主函数前使能独立看门狗。
	  设置LSI是32分频，下面函数参数范围0-0xFFF，分别代表最小值1ms和最大值4095ms
	  下面设置的是4s，如果4s内没有喂狗，系统复位。

      LSI分频可以为 [4,8,16,32,64,128,256],
      对应的最大时间为别为：409.6 819.2 1638.4 3276.8 6553.6 13107.226214.4ms

      计算公式为 Tout=(4 * 2^PR * （RL+1）)/40 ms
      最小时间单位：1/40 ms
      考虑分频系数 Prescaler（4-256）：1*Prescaler/40 ms
      考虑RL值(0-FFF)：1*Prescaler*（RL+1）/40 ms

      目前我们使用的是128分频，最大13.1秒
	*/
	bsp_InitIwdg(4095);
	
	/* 打印系统开机状态，方便查看系统是否复位 */
	App_Printf("=====================================================\r\n");
	App_Printf("系统开机执行\r\n");
	App_Printf("=====================================================\r\n");
	
    while(1)
    {   
        
		/* 等待所有任务发来事件标志 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* 事件标志组句柄 */
							         TASK_BIT_ALL,       /* 等待TASK_BIT_ALL被设置 */
							         pdTRUE,             /* 退出前TASK_BIT_ALL被清除，这里是TASK_BIT_ALL都被设置才表示“退出”*/
							         pdTRUE,             /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
							         xTicksToWait); 	 /* 等待延迟时间 */
		
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{  
		    IWDG_Feed(); //喂狗	
		}
	    else
		{
			/* 基本是每xTicksToWait进来一次 */
			/* 通过变量uxBits简单的可以在此处检测那个任务长期没有发来运行标志 */
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
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* 设置最大等待时间为200ms */  
    
    //获取当前设备的ID
    uint16_t readID = bsp_dipswitch_read();

    log_d("current dev addr =%d\r\n",readID);
    
    while (1)
    {
//        recvLen = RS485_Recv(COM5,buf,MAX_CMD_LEN);
//        
//        //判定数据的有效性
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
            xReturn = xQueueReceive( xTransQueue,    /* 消息队列的句柄 */
                                     (void *)&ptMsg,  /*这里获取的是结构体的地址 */
                                     xMaxBlockTime); /* 设置阻塞时间 */
            if(pdTRUE == xReturn)
            {
                //消息接收成功，发送接收到的消息
                packetSendBuf(ptMsg,sendBuf);     
            }
            else
            {
                //发送默认数据包
                packetDefaultSendBuf(sendBuf); //打包  
            }

            RS485_SendBuf(COM4,sendBuf,MAX_SEND_LEN);

        }



		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
        vTaskDelay(300);
    }

}



//LED任务函数 
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
        
		/* 发送事件标志，表示任务正常运行 */        
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
				/* K1键按下 打印任务执行情况 */
				case KEY_SET_PRES:	             
					App_Printf("=================================================\r\n");
					App_Printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);
                    
					App_Printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);          

                    g_memsize = xPortGetFreeHeapSize();
                    printf("系统当前内存大小为 %d 字节，开始申请内存\n",g_memsize);
                    
					break;				
				/* K2键按下，打印串口操作命令 */
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
				
				/* 其他的键值不处理 */
				default:   
				log_e("KEY_default\r\n");
					break;
			}
		}

        /* 发送事件标志，表示任务正常运行 */
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

        
        /* 发送事件标志，表示任务正常运行 */
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
	/* 初始化结构体指针 */
	ptReader = &gReaderMsg;
	
	/* 清零 */
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

			/* 使用消息队列实现指针变量的传递 */
			if(xQueueSend(xTransQueue,              /* 消息队列句柄 */
						 (void *) &ptReader,   /* 发送结构体指针变量ptReader的地址 */
						 (TickType_t)50) != pdPASS )
			{
                DBG("the queue is full!\r\n");                             
            } 
            else
            {
                dbh("WGREADER",(char *)dat,4);
            }          

          
        }
        
    	/* 发送事件标志，表示任务正常运行 */        
    	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);       
        
        vTaskDelay(100);        
    }

}   


static void vTaskQR(void *pvParameters)
{ 
    uint8_t recv_buf[255] = {0};
    uint16_t len = 0; 

    READER_BUFF_T *ptQR; 
 	/* 初始化结构体指针 */
	ptQR = &gReaderMsg;
	
	/* 清零 */
    ptQR->authMode = AUTH_MODE_CARD; //默认为刷卡
    ptQR->dataLen = 0;
    memset(ptQR->data,0x00,sizeof(ptQR->data)); 
    
    while(1)
    {   
           memset(recv_buf,0x00,sizeof(recv_buf));
           len = RS485_RecvAtTime(COM5,recv_buf,sizeof(recv_buf),800);
           if(recv_buf[len-1] == 0x00 && len > 0)
           {
                len -= 1; //这里不知道为什么会多了一个0x00
           }
           
           if(len > 0  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
           {
                log_d("reader = %s\r\n",recv_buf);      

                //判定是刷卡还是QR
                //if(strstr_t(recv_buf,"CARD") == NULL)
                if(len > 50)
                {
                    //QR
                    ptQR->authMode = AUTH_MODE_QR;
                }                               

                ptQR->dataLen = len;                
                memcpy(ptQR->data,recv_buf,len);

    			/* 使用消息队列实现指针变量的传递 */
    			if(xQueueSend(xTransQueue,              /* 消息队列句柄 */
    						 (void *) &ptQR,   /* 发送指针变量recv_buf的地址 */
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
    

		/* 发送事件标志，表示任务正常运行 */        
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
    
    vTaskDelete( NULL ); //删除自己
}



/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
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

	/* 互斥信号量 */
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


//查询Message_Queue队列中的总队列数量和剩余队列数量
void check_msg_queue(void)
{
    
	u8 msgq_remain_size;	//消息队列剩余大小
    u8 msgq_total_size;     //消息队列总大小
    
    taskENTER_CRITICAL();   //进入临界区
    msgq_remain_size=uxQueueSpacesAvailable(xTransQueue);//得到队列剩余大小
    msgq_total_size=uxQueueMessagesWaiting(xTransQueue)+uxQueueSpacesAvailable(xTransQueue);//得到队列总大小，总大小=使用+剩余的。
	printf("Total Size = %d, Remain Size = %d\r\n",msgq_total_size,msgq_remain_size);	//显示DATA_Msg消息队列总的大小

    taskEXIT_CRITICAL();    //退出临界区
}

