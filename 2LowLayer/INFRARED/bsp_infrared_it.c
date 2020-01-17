/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_infrared_it.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月24日
  最近修改   :
  功能描述   : 红外传感器中断驱动,PA4/PA5与PC4/PC5重合，暂时无法使用中断方法获取状态
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月24日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "bsp_infrared_it.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
__IO int16_t g_infraredIRQ = -1;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void infrared_gpioinit(void);



static void infrared_gpioinit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_ALL_SENSOR, ENABLE );	//使能GPIOF时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SENSOR1|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		    //上拉
	GPIO_Init ( GPIO_PORT_SENSOR14, &GPIO_InitStructure );		//初始化GPIO

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SENSOR5 |GPIO_PIN_SENSOR6;	    //LED0和LED1对应IO口
	GPIO_Init ( GPIO_PORT_SENSOR56, &GPIO_InitStructure );		//初始化GPIO

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SENSOR7 |GPIO_PIN_SENSOR8;	    //LED0和LED1对应IO口
	GPIO_Init ( GPIO_PORT_SENSOR78, &GPIO_InitStructure );		//初始化GPIO    

//	GPIO_SetBits ( GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR1|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR4);
//	GPIO_SetBits ( GPIO_PORT_SENSOR56,GPIO_PIN_SENSOR5 |GPIO_PIN_SENSOR6 );
//	GPIO_SetBits ( GPIO_PORT_SENSOR78,GPIO_PIN_SENSOR7 |GPIO_PIN_SENSOR8 );    
}

void bsp_InitInfraredExit(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

    /* 初始化GPIO */
    infrared_gpioinit();
	
	/* 使能SYSCFG时钟 */ //使用外部中断，此时钟必须打开
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* S1 连接 EXTI Line4 到 PA4 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4); 

    /* S2连接 EXTI Line5 到 PA5 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);

    /* S3连接 EXTI Line6 到 PA6 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource6);

    /* S4连接 EXTI Line7 到 PA7 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);

    /* 连接 EXTI Line8 到 PC4 引脚 */
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8);

    /* 连接 EXTI Line9 到 PC5 引脚 */
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource9);    

    /* 连接 EXTI Line0 到 PB0 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0);

    /* 连接 EXTI Line1 到 PB1 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);    

    /* 配置 EXTI LineXXX */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line4 | EXTI_Line5 \
	                              |EXTI_Line6| EXTI_Line7| EXTI_Line8|EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);


	/* 中断优先级配置 最低优先级 这里一定要分开的设置中断，不能够合并到一个里面设置 */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);    
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
}

void EXTI9_5_IRQHandler(void)
{

	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{	
        if(0 == GETS2)
        {
            g_infraredIRQ = 2;
            printf("s2 active\r\n"); 
        }
        else
        {
            g_infraredIRQ = 12;
            printf("s2 leave\r\n");
        }

		EXTI_ClearITPendingBit(EXTI_Line5); /* 清除中断标志位 */
	}

    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
	{	
        if(0 == GETS3)
        {
            g_infraredIRQ = 3;
            printf("s3 active\r\n"); 
        }
        else
        {
            g_infraredIRQ = 13;
            printf("s3 leave\r\n");
        }

		EXTI_ClearITPendingBit(EXTI_Line6); /* 清除中断标志位 */
	}

	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{	
        if(0 == GETS4)
        {
            g_infraredIRQ = 4;
            printf("s4 active\r\n"); 
        }
        else
        {
            g_infraredIRQ = 14;
            printf("s4 leave\r\n");
        }

		EXTI_ClearITPendingBit(EXTI_Line7); /* 清除中断标志位 */
	}

//    if(EXTI_GetITStatus(EXTI_Line8) != RESET)
//	{	
//        if(0 == GETS5)
//        {
//            g_infraredIRQ = 5;
//            printf("s5 active\r\n"); 
//        }
//        else
//        {
//            g_infraredIRQ = 15;
//            printf("s5 leave\r\n");
//        }

//		EXTI_ClearITPendingBit(EXTI_Line8); /* 清除中断标志位 */
//	}        

//    if(EXTI_GetITStatus(EXTI_Line9) != RESET)
//	{	
//        if(0 == GETS6)
//        {
//            g_infraredIRQ = 6;
//            printf("s6 active\r\n"); 
//        }
//        else
//        {
//            g_infraredIRQ = 16;
//            printf("s6 leave\r\n");
//        }

//		EXTI_ClearITPendingBit(EXTI_Line9); /* 清除中断标志位 */
//	} 	   
}

void EXTI0_IRQHandler(void)
{

	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{	
        if(0 == GETS7)
        {
            g_infraredIRQ = 7;
            printf("s7 active\r\n"); 
        }
        else
        {
            g_infraredIRQ = 17;
            printf("s7 leave\r\n");
        }

		EXTI_ClearITPendingBit(EXTI_Line0); /* 清除中断标志位 */

	}
			   
}

void EXTI1_IRQHandler(void)
{

	if(EXTI_GetITStatus(EXTI_Line1) != RESET)
	{	
        if(0 == GETS8)
        {
            g_infraredIRQ = 8;
            printf("s8 active\r\n"); 
        }
        else
        {
            g_infraredIRQ = 18;
            printf("s8 leave\r\n");
        }

		EXTI_ClearITPendingBit(EXTI_Line1); /* 清除中断标志位 */
	}
			   
}


void EXTI4_IRQHandler(void)
{


	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{	
//        if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4) == 0)
//        {
            if(0 == GETS1)
            {
           		g_infraredIRQ = 1;
                printf("s1 active\r\n"); 
            }
            else
            {
        		g_infraredIRQ = 11;
                printf("s1 leave\r\n");
            }
//        }

//        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4) == 0)
//        {
//            if(0 == GETS5)
//            {
//                g_infraredIRQ = 5;
//                printf("s5 active\r\n"); 
//            }
//            else
//            {
//                g_infraredIRQ = 15;
//                printf("s5 leave\r\n");
//            }

//        }        
        

		EXTI_ClearITPendingBit(EXTI_Line4); /* 清除中断标志位 */

	}
			   
}



int16_t bsp_InfraredExitScan(void)
{
    static int16_t ret = 0;
    
    switch (g_infraredIRQ)
    {       
        case 1:
            ret |= (0x01<<0);
            break;
        case 2:
            ret |= (0x01<<1);
            break;
        case 3:
            ret |= (0x01<<2);
            break;
        case 4:
            ret |= (0x01<<3);
            break;
        case 5:
            ret |= (0x01<<4);
            break;
        case 6:
            ret |= (0x01<<5);
            break;
        case 7:
            ret |= (0x01<<6);
            break;
        case 8:
            ret |= (0x01<<7);
            break;
        case 11:
            ret &= ~(0x01<<0);
            break;
        case 12:
            ret &= ~(0x01<<1);
            break;
        case 13:
            ret &= ~(0x01<<2);
            break;
        case 14:
            ret &= ~(0x01<<3);
            break;
        case 15:
            ret &= ~(0x01<<4);
            break;
        case 16:
            ret &= ~(0x01<<5);
            break;
        case 17:
            ret &= ~(0x01<<6);
            break;
        case 18:
            ret &= ~(0x01<<7);
            break;        
        default:   
            ret = -1;
            break;        
    }

    g_infraredIRQ = -1;
                printf("ret = %02x\r\n",ret);
    return ret;
}




