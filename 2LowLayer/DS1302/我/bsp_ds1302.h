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


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "sys.h"
#include "delay.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define DS1302_PORT GPIOE
#define DS1302_SCK GPIO_Pin_9 //DS1302_SCK
#define DS1302_DIO GPIO_Pin_8 //DS1302_DIO
#define DS1302_CE GPIO_Pin_7 //DS1302_CE


typedef enum DS1302_REGADDR
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
}DS1302_REGADDR_ENUM;

typedef enum DS1302_MODE
{
    DS1302_WrClockBurst = 0xbe, //дʱ��ͻ��ģʽ
    DS1302_RdRamBurst = 0xbf, //��ʱ��ͻ��ģʽ
    DS1302_WrRAMBurst = 0xfe, //дRAMͻ��ģʽ
    DS1302_RdRAMBurst = 0xff, //��RAMͻ��ģʽ       
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







//�Ĵ���IO�ڲ���״̬
#define Clr_Sclk() (GPIO_ResetBits(DS1302_PORT, DS1302_SCK))
#define Set_Sclk() (GPIO_SetBits(DS1302_PORT, DS1302_SCK))


#define Clr_Dio() (GPIO_ResetBits(DS1302_PORT, DS1302_DIO))
#define Set_Dio() (GPIO_SetBits(DS1302_PORT, DS1302_DIO))


#define Di_Ss() (GPIO_ResetBits(DS1302_PORT, DS1302_CE))
#define En_Ss() (GPIO_SetBits(DS1302_PORT, DS1302_CE))


#define Read_Dio() (GPIO_ReadInputDataBit(DS1302_PORT, DS1302_DIO))

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
extern TIME_TYPEDEF_STRU gTIME_T;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void bsp_ds1302_init(void); //����STM32��GPIO
uint8_t *bsp_ds1302_readtime(void);
void bsp_ds1302_mdifytime(uint8_t *descTime);


#endif /* __BSP_DS1302_H__ */

