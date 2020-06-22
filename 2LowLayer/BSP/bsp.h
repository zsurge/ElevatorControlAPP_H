/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��9��
  ����޸�   :
  ��������   : ���ǵײ�����ģ�����е�h�ļ��Ļ����ļ���
            Ӧ�ó���ֻ�� #include bsp.h ���ɣ�����Ҫ#include ÿ��ģ��� h �ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��9��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __BSP_H
#define __BSP_H
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
//��׼��ͷ�ļ�
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

//�м��
#include "stmflash.h"
#include "easyflash.h"
//#include "sfud.h"
#include "malloc.h"

#include "lwip_comm.h"
#include "lwip/netif.h"
#include "lwipopts.h"
#include "cm_backtrace.h"




//������ͷ�ļ�
#include "bsp_uart_fifo.h"
#include "LAN8720.h"
#include "bsp_key.h"
#include "bsp_time.h"
#include "bsp_led.h"
#include "bsp_beep.h" 
#include "bsp_spi_flash.h"
#include "bsp_usart6.h"

#include "bsp_usart5.h"

//#include "bsp_digitaltube.h"
#include "bsp_iwdg.h"
//#include "bsp_infrared_it.h"
#include "bsp_wiegand.h"
#include "bsp_tim_pwm.h"
#include "bsp_dipSwitch.h"
#include "bsp_digitaltube.h"
#include "bsp_rtc.h"
#include "bsp_ds1302.h"

//#include "bsp_24cxx.h"
//#include "bsp_i2c.h"
#include "bsp_MB85RC128.h"

//#include "24cxx.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void bsp_Init(void);



#endif




