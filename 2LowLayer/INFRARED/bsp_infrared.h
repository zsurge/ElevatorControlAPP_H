/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_infrared.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��28��
  ����޸�   :
  ��������   : ��ȡ���⴫����״̬
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��28��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __bsp_INFRARED_
#define __bsp_INFRARED_

#include "delay.h"

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
//LED�˿ڶ���

#define ERR_INFRARED    (-1)
    
#define RCC_ALL_SENSOR     (RCC_AHB1Periph_GPIOG|RCC_AHB1Periph_GPIOF)
    
#define GPIO_PORT_SENSOR14    GPIOG
#define GPIO_PORT_SENSOR58    GPIOF



    
#define GPIO_PIN_SENSOR1    GPIO_Pin_3
#define GPIO_PIN_SENSOR2    GPIO_Pin_2
#define GPIO_PIN_SENSOR3    GPIO_Pin_1
#define GPIO_PIN_SENSOR4    GPIO_Pin_0

#define GPIO_PIN_SENSOR5    GPIO_Pin_15
#define GPIO_PIN_SENSOR6    GPIO_Pin_14
#define GPIO_PIN_SENSOR7    GPIO_Pin_13
#define GPIO_PIN_SENSOR8    GPIO_Pin_12





#define GETS1 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR1) 
#define GETS2 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR2)
#define GETS3 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR3) 
#define GETS4 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR4)

#define GETS5 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR58,GPIO_PIN_SENSOR5) 
#define GETS6 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR58,GPIO_PIN_SENSOR6)
#define GETS7 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR58,GPIO_PIN_SENSOR7) 
#define GETS8 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR58,GPIO_PIN_SENSOR8)


#define SENSORMAXBUFFSIZE 128

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

void bsp_infrared_init(void);

int16_t bsp_infrared_scan(void);

void bsp_GetSensorStatus(uint8_t *dat);




#endif





