#ifndef __BSP_LED_H
#define __BSP_LED_H
#include "sys.h"


//LED�˿ڶ���
#define RCC_ALL_LED     (RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG|RCC_AHB1Periph_GPIOD)

#define GPIO_PORT_LED3          GPIOD
#define GPIO_PORT_LED4          GPIOF
#define GPIO_PORT_LED012        GPIOG

#define GPIO_PIN_ERRORLED   GPIO_Pin_8
#define GPIO_PIN_LED1       GPIO_Pin_12
#define GPIO_PIN_LED2       GPIO_Pin_9
#define GPIO_PIN_LED3       GPIO_Pin_4
#define GPIO_PIN_LED4       GPIO_Pin_9


//LED�˿ڶ���
#define LEDERROR PGout(8)	
#define LED1 PGout(12)	
#define LED2 PDout(9)	 
#define LED3 PDout(4)	 
#define LED4 PFout(9)	 





void bsp_LED_Init(void);//��ʼ��		 	



//void bsp_LedToggle(uint8_t _no);
#endif
