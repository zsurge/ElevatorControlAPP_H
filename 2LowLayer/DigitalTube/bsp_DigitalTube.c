/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_DigitalTube.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��2��
  ����޸�   :
  ��������   : ����74HC595�����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��2��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "delay.h"
#include "bsp_digitaltube.h"
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
static uint8_t Conver(uint8_t bit);

 void bsp_HC595Init(void)
{

    GPIO_InitTypeDef  GPIO_InitStructure; 

    RCC_AHB1PeriphClockCmd(HC595_GPIO_CLK, ENABLE);//ʹ��GPIOA,GPIOCʱ�� 

    GPIO_InitStructure.GPIO_Pin = HC595_CLK_PIN|HC595_CS_PIN|HC595_DATA_PIN; //KEY0 KEY1 KEY2��Ӧ����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//25M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//
    GPIO_Init(HC595_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4
	
	HC595_CLK_H();
	HC595_DATA_H();
	HC595_CS_H();

} 
 
 void bsp_HC595Send(uint16_t data)
 {
     u8 j = 0;
     for (j = 8; j > 0; j--)
     {
       if(data & 0x80)
              HC595_DATA_H();
           else
               HC595_DATA_L();
           
       HC595_CLK_L();              //�����ط�����λ
           delay_ms(1);
       data <<= 1;
       HC595_CLK_H();      
     }
     
//       bsp_HC595Load();
 }
 
 void bsp_HC595Load(void)
 {
     HC595_CS_L();
     delay_ms(1);
     HC595_CS_H();
 }


static uint8_t Conver(uint8_t bit)
{
    
    //��������ܶ�Ӧ�ı���
    //0~9 A~F     
    uint8_t table[]={ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};
    uint8_t index = 0;


    switch(bit)
    {  
        case 'A':
        case 'a':
            index = 10;
            break;
        case 'B':
        case 'b':
            index = 11;
            break;
        case 'C':
        case 'c':
            index = 12;
            break;
        case 'D':
        case 'd':
            index = 13;
            break;
        case 'E':
        case 'e':
            index = 14;
            break;
        case 'F':
        case 'f':
            index = 15;
            break;
        default:
            //if(bit >= 0 && bit <= 9) ������unsigned char�͵Ĳ������������Ǻ�>=0,���Բ���Ҫ��0ֵ�Ա�
			if(bit <= 9)
            {
                index = bit;
            }
            else
            {
                //�������ݴ�������Ĭ��Ϊ0
                index = 0;
            }
            break;
    }

    return table[index];
}
 
 void bsp_HC595Show(uint8_t bit1, uint8_t bit2,uint8_t bit3)
 {
	bsp_HC595Send(Conver(bit3)); 
    bsp_HC595Send(Conver(bit2)); 
    bsp_HC595Send(Conver(bit1)); 
    bsp_HC595Load();
 }
 
 
 void bsp_HC595RowOut(uint32_t Data)
 {
     bsp_HC595Send(Data >> 16);
     bsp_HC595Send(Data >> 8);
     bsp_HC595Send(Data >> 0);
     bsp_HC595Load();
 }

 


