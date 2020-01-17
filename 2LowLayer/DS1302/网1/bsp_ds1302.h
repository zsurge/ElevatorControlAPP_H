/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_ds1302.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2020年1月14日
  最近修改   :
  功能描述   : bsp_ds1302.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年1月14日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/
#ifndef __BSP_DS1302_H__
#define __BSP_DS1302_H__

#include "delay.h"
#include "sys.h"

/*	IO Definitions	*/
#define		SCLK			PEout(9)	//时钟
#define		SDA				PEout(8)	//数据	
#define		RST				PEout(7)	//DS1302复位(片选)

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

void bsp_ds1302_init(void); //配置STM32的GPIO
uint8_t *bsp_ds1302_readtime(void);
void bsp_ds1302_mdifytime(uint8_t *descTime);





#endif /* __BSP_DS1302_H__ */

