/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_Wiegand.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��29��
  ����޸�   :
  ��������   : Τ��������ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��29��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __BSP_WIEGAND_H
#define __BSP_WIEGAND_H


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "sys.h"
#include "delay.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define WG_TIMEOUT   	4

#if 0

#define WG1_RCC         RCC_AHB1Periph_GPIOE
#define WG1_GPIO_PORT   GPIOE
#define WG1_IN_D0       GPIO_Pin_10//Τ���ӿ�
#define WG1_IN_D1       GPIO_Pin_11//Τ���ӿ�

#define WG1_PortSource   EXTI_PortSourceGPIOE   

#define WG1_IN_D0_PinSource EXTI_PinSource10
#define WG1_IN_D1_PinSource EXTI_PinSource11


#define WG1_IN_D0_EXTI  EXTI_Line10//Τ���ӿ�
#define WG1_IN_D1_EXTI  EXTI_Line11//Τ���ӿ�

#else
#define WG1_RCC         RCC_AHB1Periph_GPIOB
#define WG1_GPIO_PORT   GPIOB
#define WG1_IN_D0       GPIO_Pin_6//Τ���ӿ�
#define WG1_IN_D1       GPIO_Pin_7//Τ���ӿ�

#define WG1_PortSource   EXTI_PortSourceGPIOB

#define WG1_IN_D0_PinSource EXTI_PinSource6
#define WG1_IN_D1_PinSource EXTI_PinSource7


#define WG1_IN_D0_EXTI  EXTI_Line6//Τ���ӿ�
#define WG1_IN_D1_EXTI  EXTI_Line7//Τ���ӿ�

#endif

#define WG_IN_D0  GPIO_ReadInputDataBit(WG1_GPIO_PORT,WG1_IN_D0) 
#define WG_IN_D1  GPIO_ReadInputDataBit(WG1_GPIO_PORT,WG1_IN_D1) 




/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
typedef struct
{
  volatile uint8_t WG_Rx_Len; //���յ�λ����
  volatile uint8_t WG_Rx_End; //���ս���
  volatile uint8_t WG_Bit;     //λֵ
  volatile uint8_t WG_Rx_Bit;  //�յ�λ
  volatile uint8_t END_TIME_C_EN;  //���ս�����ʱʹ��
  volatile uint32_t WG_Rx_Data;   //��������
  volatile uint8_t End_TIME;      //���ս���ʱ��
}WG_RX_T;   //Τ�����սṹ��


extern WG_RX_T WG_Rx_Str;   //Τ�����սṹ��



/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void WeigenInD0(void);
void WeigenInD1(void);

void bsp_WiegandInit(void);
uint32_t bsp_WeiGenScanf(void);




#endif



