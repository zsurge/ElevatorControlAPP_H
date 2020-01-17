/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : key.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��25��
  ����޸�   :
  ��������   : ��������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��25��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __bsp_KEY_H
#define __bsp_KEY_H	 

#include "sys.h" 
#include "delay.h"

#if 0

/* �����ڶ�Ӧ��RCCʱ�� */
#define RCC_ALL_KEY     (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE)

#define GPIO_PORT_KEY       GPIOE

#define GPIO_PIN_KEY_SET    GPIO_Pin_2
#define GPIO_PIN_KEY_RR     GPIO_Pin_3
#define GPIO_PIN_KEY_LL     GPIO_Pin_4

#define GPIO_PORT_KEY_OK    GPIOA
#define GPIO_PIN_KEY_OK     GPIO_Pin_0


#define KEY_SET     PEin(2)   	//PE2
#define KEY_RR 		PEin(3)		//PE3 
#define KEY_LL 		PEin(4)		//PE4
#define KEY_OK		PAin(0)		//PA0 

//#define KEY_SET 	GPIO_ReadInputDataBit(GPIO_PORT_KEY,GPIO_PIN_KEY_SET)   //PA4
//#define KEY_RR 		GPIO_ReadInputDataBit(GPIO_PORT_KEY,GPIO_PIN_KEY_RR)	//PA0  
//#define KEY_LL 		GPIO_ReadInputDataBit(GPIO_PORT_KEY,GPIO_PIN_KEY_LL)    //PA1
//#define KEY_OK 	    GPIO_ReadInputDataBit(GPIO_PORT_KEY_OK,GPIO_PIN_KEY_OK)	//PC3 


#define KEY_NONE        0   //�ް���
#define KEY_SET_PRES 	1	//KEY0����
#define KEY_RR_PRES	    2	//KEY1����
#define KEY_LL_PRES	    3	//KEY2����
#define KEY_OK_PRES     4	//KEY3����


void bsp_key_Init(void);	//IO��ʼ��
u8 bsp_key_Scan(u8);  		//����ɨ�躯��		
#endif



/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
#define KEY0 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) //PE4
#define KEY1 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)	//PE3 
#define KEY2 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) //PE2
#define WK_UP 	    GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)	//PA0



#define KEY_NONE        0   //�ް���
#define KEY_SET_PRES 	1	//KEY0����
#define KEY_RR_PRES	    2	//KEY1����
#define KEY_LL_PRES	    3	//KEY2����
#define KEY_OK_PRES     4	//KEY3����



#define KEY0_PRES 	1
#define KEY1_PRES	2
#define KEY2_PRES	3
#define WKUP_PRES   4

void bsp_key_Init(void);	//IO��ʼ��
uint8_t bsp_Key_Scan(u8);  		//����ɨ�躯��	


#endif

