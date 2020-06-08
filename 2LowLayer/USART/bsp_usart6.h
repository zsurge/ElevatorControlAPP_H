/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_usart1.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��22��
  ����޸�   :
  ��������   : ����1��ʼ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __BSP_USART6_H
#define __BSP_USART6_H
#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"
#include "sys.h"
#include "ringbuff.h"


//���Ŷ���
/*******************************************************/
#define USART1_CLK                         RCC_APB2Periph_USART6
#define USART1_GPIO_PORT                   GPIOC
#define USART1_GPIO_CLK                    RCC_AHB1Periph_GPIOC

#define USART1_RX_PIN                      GPIO_Pin_7
#define USART1_RX_AF                       GPIO_AF_USART6
#define USART1_RX_SOURCE                   GPIO_PinSource7

#define USART1_TX_PIN                      GPIO_Pin_6
#define USART1_TX_AF                       GPIO_AF_USART6
#define USART1_TX_SOURCE                   GPIO_PinSource6


#define COM6_RXBUFFER_SIZE   256 //���ڽ��ջ���������
#define UART6_RX_LEN         COM6_RXBUFFER_SIZE //USART6 DMA���ջ���������
#define UART6_TX_LEN         COM6_RXBUFFER_SIZE //USART6 DMA���ͻ���������

#define RS485_U6_RX_EN()	    {PAout(8)=0;} //SP485����ģʽ,�͵�ƽ��Ч
#define RS485_U6_TX_EN()	    {PAout(8)=1;} //SP485����ģʽ,�ߵ�ƽ��Ч

extern uint8_t gUsart6RXBuff[COM6_RXBUFFER_SIZE];//���ջ�����
extern RingBuff_t ringbuff_handle;


void BSP_Usart6_Init(uint32_t bound);
void BSP_DMAUsart6Send(uint8_t *str,uint8_t cndtr);
uint16_t BSP_DMAUsart6Read(uint8_t *buff, uint32_t len);
    
#endif


