/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Key_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月25日
  最近修改   :
  功能描述   : 处理按键的任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月25日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "key_task.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define KEY_STK_SIZE        (configMINIMAL_STACK_SIZE*8)
#define KEY_TASK_PRIO	    ( tskIDLE_PRIORITY + 2)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *keyTaskName = "vKeyTask";     

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskKey = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskKey(void *pvParameters);


void CreateKeyTask(void *pvParameters)
{
    //按键
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


