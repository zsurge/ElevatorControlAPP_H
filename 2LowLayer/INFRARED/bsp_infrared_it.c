/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_infrared_it.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��24��
  ����޸�   :
  ��������   : ���⴫�����ж�����,PA4/PA5��PC4/PC5�غϣ���ʱ�޷�ʹ���жϷ�����ȡ״̬
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��24��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "bsp_infrared_it.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
__IO int16_t g_infraredIRQ = -1;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void infrared_gpioinit(void);



static void infrared_gpioinit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_ALL_SENSOR, ENABLE );	//ʹ��GPIOFʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SENSOR1|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		    //����
	GPIO_Init ( GPIO_PORT_SENSOR14, &GPIO_InitStructure );		//��ʼ��GPIO

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SENSOR5 |GPIO_PIN_SENSOR6;	    //LED0��LED1��ӦIO��
	GPIO_Init ( GPIO_PORT_SENSOR56, &GPIO_InitStructure );		//��ʼ��GPIO

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SENSOR7 |GPIO_PIN_SENSOR8;	    //LED0��LED1��ӦIO��
	GPIO_Init ( GPIO_PORT_SENSOR78, &GPIO_InitStructure );		//��ʼ��GPIO    

//	GPIO_SetBits ( GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR1|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR4);
//	GPIO_SetBits ( GPIO_PORT_SENSOR56,GPIO_PIN_SENSOR5 |GPIO_PIN_SENSOR6 );
//	GPIO_SetBits ( GPIO_PORT_SENSOR78,GPIO_PIN_SENSOR7 |GPIO_PIN_SENSOR8 );    
}

void bsp_InitInfraredExit(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

    /* ��ʼ��GPIO */
    infrared_gpioinit();
	
	/* ʹ��SYSCFGʱ�� */ //ʹ���ⲿ�жϣ���ʱ�ӱ����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* S1 ���� EXTI Line4 �� PA4 ���� */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4); 

    /* S2���� EXTI Line5 �� PA5 ���� */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);

    /* S3���� EXTI Line6 �� PA6 ���� */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource6);

    /* S4���� EXTI Line7 �� PA7 ���� */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);

    /* ���� EXTI Line8 �� PC4 ���� */
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8);

    /* ���� EXTI Line9 �� PC5 ���� */
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource9);    

    /* ���� EXTI Line0 �� PB0 ���� */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0);

    /* ���� EXTI Line1 �� PB1 ���� */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);    

    /* ���� EXTI LineXXX */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line4 | EXTI_Line5 \
	                              |EXTI_Line6| EXTI_Line7| EXTI_Line8|EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);


	/* �ж����ȼ����� ������ȼ� ����һ��Ҫ�ֿ��������жϣ����ܹ��ϲ���һ���������� */
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

		EXTI_ClearITPendingBit(EXTI_Line5); /* ����жϱ�־λ */
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

		EXTI_ClearITPendingBit(EXTI_Line6); /* ����жϱ�־λ */
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

		EXTI_ClearITPendingBit(EXTI_Line7); /* ����жϱ�־λ */
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

//		EXTI_ClearITPendingBit(EXTI_Line8); /* ����жϱ�־λ */
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

//		EXTI_ClearITPendingBit(EXTI_Line9); /* ����жϱ�־λ */
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

		EXTI_ClearITPendingBit(EXTI_Line0); /* ����жϱ�־λ */

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

		EXTI_ClearITPendingBit(EXTI_Line1); /* ����жϱ�־λ */
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
        

		EXTI_ClearITPendingBit(EXTI_Line4); /* ����жϱ�־λ */

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




