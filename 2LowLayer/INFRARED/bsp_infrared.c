/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_infrared.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��28��
  ����޸�   :
  ��������   : ������⴫�����ź�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��28��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "bsp_infrared.h"
#include "stdio.h"
#include "string.h"
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
void bsp_infrared_init ( void )
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_ALL_SENSOR, ENABLE );	//ʹ��GPIOFʱ��


	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SENSOR1|GPIO_PIN_SENSOR2|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		    //����
	GPIO_Init ( GPIO_PORT_SENSOR14, &GPIO_InitStructure );		//��ʼ��GPIO

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SENSOR5 |GPIO_PIN_SENSOR6|GPIO_PIN_SENSOR7 |GPIO_PIN_SENSOR8;;	    //LED0��LED1��ӦIO��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		    //����
	GPIO_Init ( GPIO_PORT_SENSOR58, &GPIO_InitStructure );		//��ʼ��GPIO



	GPIO_SetBits ( GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR1|GPIO_PIN_SENSOR2|GPIO_PIN_SENSOR3|GPIO_PIN_SENSOR4);
	GPIO_SetBits ( GPIO_PORT_SENSOR58,GPIO_PIN_SENSOR5 |GPIO_PIN_SENSOR6|GPIO_PIN_SENSOR7 |GPIO_PIN_SENSOR8 );



    
}

int16_t bsp_infrared_scan ( void )
{
	static u8 key_up=1;//�������ɿ���־
    static u8 key_value[3] = {0};
	
	static u8 key1_up=0;
	static u8 key2_up=0;
	static u8 key3_up=0;
	static u8 key4_up=0;
	static u8 key5_up=0;
	static u8 key6_up=0;
	static u8 key7_up=0;
	static u8 key8_up=0;
    
	u8 mode = 0;
    
	if ( mode )
	{
		key_up=1;    //֧������
	}

	if ( key_up && ( GETS1==0||GETS2==0 || GETS3==0||GETS4==0 || GETS5==0||GETS6==0 || GETS7==0||GETS8==0 ) )
	{
		delay_ms ( 5 ); //ȥ����

//        printf("--------infrared active--------\r\n");
		key_up=0;
        
		if ( GETS1==0 && key1_up==0)
		{
            key1_up = 1;            
			return key_value[0] |= (0x01<<0);
		}
		else if ( GETS2==0 && key2_up==0)
		{
            key2_up = 1;
			return key_value[0] |= (0x01<<1);
		}
		else if ( GETS3==0 && key3_up==0)
		{
            key3_up = 1;
			return key_value[0] |= (0x01<<2);
		}
		else if ( GETS4==0 && key4_up==0)
		{
            key4_up = 1;
			return key_value[0] |= (0x01<<3);
		}
		else if ( GETS5==0 && key5_up==0)
		{
            key5_up = 1;
			return key_value[0] |= (0x01<<4);
		}
		else if ( GETS6==0 && key6_up==0)
		{
            key6_up = 1;
			return key_value[0] |= (0x01<<5);
		}
		else if ( GETS7==0 && key7_up==0)
		{
            key7_up = 1;
			return key_value[0] |= (0x01<<6);
		}
		else if ( GETS8==0 && key8_up==0)
		{
            key8_up = 1;
			return key_value[0] |= (0x01<<7);
		}

	}
	else if ((key_up==0) && (GETS1==1 || GETS2==1 || GETS3==1 || GETS4==1 || GETS5==1 || GETS6==1 || GETS7==1 || GETS8==1))
	{        
        delay_ms ( 5 ); //ȥ����       
//        printf("========infrared leave========\r\n");
		key_up=1;     
        
		if ( GETS1==1 && key1_up==1)
		{
            key1_up = 0;
			return key_value[0] &= ~(0x01<<0);
		}
		else if ( GETS2==1 && key2_up==1)
		{
            key2_up = 0;
			return key_value[0] &= ~(0x01<<1);
		}
		else if ( GETS3==1 && key3_up==1)
		{
            key3_up = 0;
			return key_value[0] &= ~(0x01<<2);
		}
		else if ( GETS4==1 && key4_up==1)
		{
            key4_up = 0;
			return key_value[0] &= ~(0x01<<3);
		}
		else if ( GETS5==1 && key5_up==1)
		{
            key5_up = 0;
			return key_value[0] &= ~(0x01<<4);
		}
		else if ( GETS6==1 && key6_up==1)
		{
            key6_up = 0;
			return key_value[0] &= ~(0x01<<5);
		}
		else if ( GETS7==1 && key7_up==1)
		{
            key7_up = 0;
			return key_value[0] &= ~(0x01<<6);
		}
		else if ( GETS8==1 && key8_up==1)
		{
            key8_up = 0;
			return key_value[0] &= ~(0x01<<7);
		}        
	}

	return ERR_INFRARED;// �ް�������
}





void bsp_GetSensorStatus(uint8_t *dat)
{
    static uint8_t code = 0;

    uint8_t buf[6] = {0};
    
    if(GETS1 == 0)
    {
        code |= (0x01<<0);
    }
    else
    {
        code &= ~(0x01<<0);
    }

    if(GETS2 == 0)
    {
        code |= (0x01<<1);
    }
    else
    {
        code &= ~(0x01<<1);
    }
    
    if(GETS3 == 0)
    {
        code |= (0x01<<2);
    }
    else
    {
        code &= ~(0x01<<2);
    }
    

    if(GETS4 == 0)
    {
        code |= (0x01<<3);
    }
    else
    {
        code &= ~(0x01<<3);
    }
    

    if(GETS5 == 0)
    {
        code |= (0x01<<4);
    }
    else
    {
        code &= ~(0x01<<4);
    }

    if(GETS6 == 0)
    {
        code |= (0x01<<5);
    }
    else
    {
        code &= ~(0x01<<5);
    }
    
    if(GETS7 == 0)
    {
        code |= (0x01<<6);
    }
    else
    {
        code &= ~(0x01<<6);
    }
    

    if(GETS8 == 0)
    {
        code |= (0x01<<7);
    }    
    else
    {
        code &= ~(0x01<<7);
    }

    memset(buf,0x00,sizeof(buf));
    sprintf((char *)buf,"%06X",code);
    memcpy(dat,buf,6);

}




