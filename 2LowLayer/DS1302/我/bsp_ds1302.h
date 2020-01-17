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


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "sys.h"
#include "delay.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define DS1302_PORT GPIOE
#define DS1302_SCK GPIO_Pin_9 //DS1302_SCK
#define DS1302_DIO GPIO_Pin_8 //DS1302_DIO
#define DS1302_CE GPIO_Pin_7 //DS1302_CE


typedef enum DS1302_REGADDR
{
    DS1302_RdSec = 0x81, //BIT7: 0正常工作; 1:低功耗模式
    DS1302_RdMin = 0x83,
    DS1302_RdHour = 0x85, //BIT5: 0运行为12时; 1:24时
    DS1302_RdDate = 0x87,
    DS1302_RdMonth = 0x89,
    DS1302_RdWeek = 0x8b,
    DS1302_RdYear = 0x8d,
    DS1302_RdProtect = 0x8f, //读保护
    DS1302_RdTrickleCharge = 0x91,
    DS1302_RdClockBurst = 0xbf,
    
    DS1302_WrSec = 0x80, //BIT7: 0正常工作; 1:低功耗模式
    DS1302_WrMin = 0x82,
    DS1302_WrHour = 0x84, //BIT5: 0运行为12时; 1:24时
    DS1302_WrDate = 0x86,
    DS1302_WrMonth = 0x88,
    DS1302_WrWeek = 0x8a,
    DS1302_WrYear = 0x8c,
    DS1302_WrProtect = 0x8e, //写保护
    DS1302_WrTrickleCharge = 0x90,           
}DS1302_REGADDR_ENUM;

typedef enum DS1302_MODE
{
    DS1302_WrClockBurst = 0xbe, //写时钟突发模式
    DS1302_RdRamBurst = 0xbf, //读时钟突发模式
    DS1302_WrRAMBurst = 0xfe, //写RAM突发模式
    DS1302_RdRAMBurst = 0xff, //读RAM突发模式       
}DS1302_MODE_ENUM;


typedef struct TIME_TYPEDEF
{
    unsigned char sec;
    unsigned char min;
    unsigned char hour;
    unsigned char date;
    unsigned char month;
    unsigned char week;
    unsigned char year;      
}TIME_TYPEDEF_STRU;







//寄存器IO口操作状态
#define Clr_Sclk() (GPIO_ResetBits(DS1302_PORT, DS1302_SCK))
#define Set_Sclk() (GPIO_SetBits(DS1302_PORT, DS1302_SCK))


#define Clr_Dio() (GPIO_ResetBits(DS1302_PORT, DS1302_DIO))
#define Set_Dio() (GPIO_SetBits(DS1302_PORT, DS1302_DIO))


#define Di_Ss() (GPIO_ResetBits(DS1302_PORT, DS1302_CE))
#define En_Ss() (GPIO_SetBits(DS1302_PORT, DS1302_CE))


#define Read_Dio() (GPIO_ReadInputDataBit(DS1302_PORT, DS1302_DIO))

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
extern TIME_TYPEDEF_STRU gTIME_T;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
void bsp_ds1302_init(void); //配置STM32的GPIO
uint8_t *bsp_ds1302_readtime(void);
void bsp_ds1302_mdifytime(uint8_t *descTime);


#endif /* __BSP_DS1302_H__ */

