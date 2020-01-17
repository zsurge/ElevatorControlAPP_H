/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_dipSwitch.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��12��23��
  ����޸�   :
  ��������   : ���뿪������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��12��23��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "bsp_dipSwitch.h"
#include "tool.h"
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
void bsp_dipswitch_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOG, ENABLE );	//ʹ��GPIOGʱ��


	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_DIP0|GPIO_PIN_DIP1|GPIO_PIN_DIP2|GPIO_PIN_DIP3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		    //����
	GPIO_Init ( GPIO_PORT_DIPSWITCH, &GPIO_InitStructure );		//��ʼ��GPIO

	GPIO_SetBits ( GPIO_PORT_DIPSWITCH,GPIO_PIN_DIP0|GPIO_PIN_DIP1|GPIO_PIN_DIP2|GPIO_PIN_DIP3);

}

//���뿪�أ�����ONʱ����bit��1
int16_t bsp_dipswitch_read(void)
{
    static int16_t value = 0;

    if(DIP0 == 1)
    {
        CLRBIT(value, 0);
    }
    else
    {
        SETBIT(value, 0);
    }

    if(DIP1 == 1)
    {
        CLRBIT(value, 1);
    }
    else
    {
        SETBIT(value, 1);
    }

    if(DIP2 == 1)
    {
        CLRBIT(value, 2);
    }
    else
    {
        SETBIT(value, 2);
    }

    if(DIP3 == 1)
    {
        CLRBIT(value, 3);
    }
    else
    {
        SETBIT(value, 3);
    }    

    return value;
}

