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

//�Ĵ�����ַ
typedef enum
{
	DS1302_RdSec = 0x81, //BIT7: 0��������; 1:�͹���ģʽ
	DS1302_RdMin = 0x83,
	DS1302_RdHour = 0x85, //BIT5: 0����Ϊ12ʱ; 1:24ʱ
	DS1302_RdDate = 0x87,
	DS1302_RdMonth = 0x89,
	DS1302_RdWeek = 0x8b,
	DS1302_RdYear = 0x8d,
	DS1302_RdProtect = 0x8f, //������
	DS1302_RdTrickleCharge = 0x91,
	DS1302_RdClockBurst = 0xbf,
	DS1302_WrSec = 0x80, //BIT7: 0��������; 1:�͹���ģʽ
	DS1302_WrMin = 0x82,
	DS1302_WrHour = 0x84, //BIT5: 0����Ϊ12ʱ; 1:24ʱ
	DS1302_WrDate = 0x86,
	DS1302_WrMonth = 0x88,
	DS1302_WrWeek = 0x8a,
	DS1302_WrYear = 0x8c,
	DS1302_WrProtect = 0x8e, //д����
	DS1302_WrTrickleCharge = 0x90,
} DS1302_RegAddr;


typedef enum
{
	DS1302_WrClockBurst = 0xbe, //дʱ��ͻ��ģʽ
	DS1302_RdRamBurst = 0xbf, //��ʱ��ͻ��ģʽ
	DS1302_WrRAMBurst = 0xfe, //дRAMͻ��ģʽ
	DS1302_RdRAMBurst = 0xff, //��RAMͻ��ģʽ
} DS1302_MODE;


//����ʱ��ṹ��
typedef struct
{
	unsigned char sec;
	unsigned char min;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned char year;
} TIME_TypeDef;


//*****************DS1302��������*******************
//���Ӧ��IO��������ֲʱ�޸�
#define DS1302_PORT GPIOE
#define DS1302_SCK GPIO_Pin_9 //DS1302_SCK
#define DS1302_DIO GPIO_Pin_8 //DS1302_DIO
#define DS1302_CE GPIO_Pin_7 //DS1302_CE


//�Ĵ���IO�ڲ���״̬
#define Clr_Sclk() (GPIO_ResetBits(DS1302_PORT, DS1302_SCK))
#define Set_Sclk() (GPIO_SetBits(DS1302_PORT, DS1302_SCK))


#define Clr_Dio() (GPIO_ResetBits(DS1302_PORT, DS1302_DIO))
#define Set_Dio() (GPIO_SetBits(DS1302_PORT, DS1302_DIO))


#define Di_Ss() (GPIO_ResetBits(DS1302_PORT, DS1302_CE))
#define En_Ss() (GPIO_SetBits(DS1302_PORT, DS1302_CE))


#define Read_Dio() (GPIO_ReadInputDataBit(DS1302_PORT, DS1302_DIO))


extern uint8_t rtc_init[8];
extern uint8_t u8time[8];
/**********************************************��������**************************************************/
void DS1302_GPIO_Init ( void ); //����STM32��GPIO��SPI�ӿ�
void DS1302_Settime ( uint8_t addr,uint8_t time[8] ); //����ʱ��
void DS1302_ReadTime ( uint8_t addr,uint8_t time[8] ); //��ȡDS1302ʱ��





#endif /* __BSP_DS1302_H__ */

