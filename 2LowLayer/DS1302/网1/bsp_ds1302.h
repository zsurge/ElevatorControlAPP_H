/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_ds1302.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2020��1��14��
  ����޸�   :
  ��������   : bsp_ds1302.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��1��14��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __BSP_DS1302_H__
#define __BSP_DS1302_H__

#include "delay.h"
#include "sys.h"

/*	IO Definitions	*/
#define		SCLK			PEout(9)	//ʱ��
#define		SDA				PEout(8)	//����	
#define		RST				PEout(7)	//DS1302��λ(Ƭѡ)

#define		SCLK_Pin		GPIO_Pin_9
#define		SDA_Pin			GPIO_Pin_8
#define		RST_Pin			GPIO_Pin_7

#define 	DS1302PORT      GPIOE
#define     DS1302CLKLINE   RCC_AHB1Periph_GPIOE	

#define 	DS1302_W_ADDR 	0x80
#define 	DS1302_R_ADDR 	0x81

typedef struct 
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t week;
	uint8_t year;
} __Date;

extern __Date ModifyDate;
extern uint8_t time[7];

void bsp_ds1302_init(void); //����STM32��GPIO
uint8_t *bsp_ds1302_readtime(void);
void bsp_ds1302_mdifytime(uint8_t *descTime);





#endif /* __BSP_DS1302_H__ */

