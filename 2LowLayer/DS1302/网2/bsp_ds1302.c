/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_ds1302.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2020��1��14��
  ����޸�   :
  ��������   : ʱ��ģ������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��1��14��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stdio.h"
#include "bsp_ds1302.h"
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
uint8_t u8time[8];
uint8_t rtc_init[8] =
{
	58, //�� 0
	18, //�� 1
	10, //Сʱ2
	15,//�� 3
	1,//�� 4
	3,//��
	20,//2016�� 6
	0,//дЧ�� 7
};


/*************************************************************************************************************************
* ���� : void DS1302_GPIO_Init(void)
* ���� : ����STM32��GPIO��SPI�ӿڣ��������� DS1302
* ���� : ��
* ���� : ��
* ���� : GPIO�⺯��
* ���� : li_qcxy@126.com
* ʱ�� : 2016-12-9
* ����޸�ʱ�� : 2017-1-4�޸�����
* ˵�� : DS1302_DIO����Ϊ��©ģʽ����ģʽ���ܹ�ʵ��������˫��IO��
*************************************************************************************************************************/
void DS1302_GPIO_Init ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOE, ENABLE ); //ʹ��PA,PD�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = DS1302_SCK|DS1302_DIO|DS1302_CE; //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz; //IO���ٶ�Ϊ50MHz
	GPIO_Init ( DS1302_PORT, &GPIO_InitStructure ); //�����趨������ʼ��GPIOA.8
}


void Dio_In ( void )

{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOE, ENABLE );//ʹ��PA,PD�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin = DS1302_DIO ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO���ٶ�Ϊ50MHz
	GPIO_Init ( DS1302_PORT, &GPIO_InitStructure );
}

void DIO_Out ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOE, ENABLE ); //ʹ��PA,PD�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = DS1302_DIO ; //LED0-->PA.8 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; //��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO���ٶ�Ϊ50MHz
	GPIO_Init ( DS1302_PORT, &GPIO_InitStructure ); //�����趨������ʼ��GPIOA.8
}
/*************************************************************************************************************************
*����     : void DS1302_WriteByte(uint8_t data)
*����     : дһ��Byte��DS1302
*����     : data:Ҫд���Byte
*����     : ��
*���� : �ײ�궨��
* ���� : li_qcxy@126.com
* ʱ�� : 2016-12-09
* ����޸�ʱ��:
*˵��     : дһ���ֽڵ�����SCK������д����
*************************************************************************************************************************/
void DS1302_WriteByte ( uint8_t addr,uint8_t data )
{
	char i=0;
	int Coda=0;
	Coda= ( data<<8 ) |addr;
	En_Ss( );
	for ( i=0; i<16; i++ )
	{
		if ( ( Coda&0x01 ) !=0 )
		{
			Set_Dio( );
		}
		else
		{
			Clr_Dio();
		}
		Set_Sclk( );
		Coda=Coda>>1;

		Clr_Sclk( );

	}
	Di_Ss( );
}




/*************************************************************************************************************************
*����     : uint8_t DS1302_ReadByte(uint8_t addr)
*����     : ��DS1302SPI�����϶�ȡһ���ֽ�
*����     : addr:�Ĵ�����ַ
*����     : ��ȡ��������
*���� : �ײ�궨��
* ���� : li_qcxy@126.com
* ʱ�� : 2016-12-09
* ����޸�ʱ��:
*˵��     : ��һ���ֽڵ����� SCK�½��ض�����
*************************************************************************************************************************/
uint8_t DS1302_ReadByte ( uint8_t addr )
{
	char i,Coda;
	Coda=addr;
	En_Ss( );
	for ( i=0; i<8; i++ )
	{
		if ( ( Coda&0x01 ) !=0 )
		{
			Set_Dio( );
		}
		else
		{
			Clr_Dio( );
		}
		Set_Sclk( );
		Coda=Coda>>1;
		Clr_Sclk( );

	}
	/************************************/
	Dio_In();
	//������
	Coda=0;
	for ( i=0; i<8; i++ )
	{
		if ( Read_Dio() !=0 )
		{
			Coda|= ( 1<<i );
		}
		Set_Sclk( );
		Clr_Sclk( );

	}


	Di_Ss();
	DIO_Out();

	return Coda ;
}




/*************************************************************************************************************************
*����     : void DS1302_WriteData(uint8_t addr,uint8_t data)
*����     : ��ָ���Ĵ���д��һ���ֽڵ�����
*����     : addr:�Ĵ�����ַ;data:��Ҫд�������
*����     : ��
*���� : �ײ�궨��
* ���� : li_qcxy@126.com
* ʱ�� : 2016-12-09
* ����޸�ʱ��:
*˵��     :
*************************************************************************************************************************/
void DS1302_WriteData ( uint8_t addr,uint8_t data )
{
	Di_Ss();
	Clr_Sclk();
	delay_us ( 1 );
	En_Ss();
	delay_us ( 2 );
	DS1302_WriteByte ( addr,data );
	Di_Ss();
	Clr_Sclk();
	delay_us ( 1 );
}




/*************************************************************************************************************************
*����     : void DS1302_ReadTime(uint8_t addr,uint8_t time[8])
*����     : �������ݲ�ͨ�����ڴ�ӡ
*����     : read:Ҫд���Byte
*����     : ��
*���� : �ײ�궨��
* ���� : li_qcxy@126.com
* ʱ�� : 2016-12-10
* ����޸�ʱ��:
*˵��     :
*************************************************************************************************************************/
void DS1302_ReadTime ( uint8_t addr,uint8_t time[8] )
{
	char i,j,Coda;
	uint8_t temp;
	Coda=addr;
	En_Ss();
	for ( i=0; i<8; i++ )
	{

		if ( ( Coda&0x01 ) !=0 )
		{
			Set_Dio();
		}
		else
		{
			Clr_Dio();
		}

		Set_Sclk();
		Coda=Coda>>1;
		Clr_Sclk();

	}
	Dio_In();
	//������
	for ( i=0; i<8; i++ )
	{
		time[i]=0;
		for ( j=0; j<8; j++ )
		{
			if ( Read_Dio() !=0 )
			{
				time[i]|= ( 1<<j );
			}
			Set_Sclk();
			Clr_Sclk();

		}
		temp = time[i] / 16;
		time[i] = temp * 10 + time[i] % 16;

	}
	Di_Ss();
	DIO_Out();

}




/*************************************************************************************************************************
*����     : void DS1302_Settime(uint8_t *Buffer)
*����     : ��ʱ
*����     : Ҫд���� �� �� ʱ �� ��
*����     : ��
*���� : �ײ�궨��
* ���� : li_qcxy@126.com
* ʱ�� : 2016-12-10
* ����޸�ʱ��:
*˵��     :
*************************************************************************************************************************/
void DS1302_Settime ( uint8_t addr,uint8_t time[8] )
{
	char i,j,ge;
	int Coda=0;


	Coda=addr;
	En_Ss();
	for ( i=0; i<8; i++ ) //ʱ���ַ
	{
		if ( ( Coda&0x01 ) !=0 )
		{
			Set_Dio();
		}
		else
		{
			Clr_Dio();
		}
		Set_Sclk();
		Coda=Coda>>1;
		Clr_Sclk();
	}

	for ( i=0; i<8; i++ ) //ʱ������
	{
		ge=time[i]%10;//��λ������
		time[i]= ( time[i]/10 ) *16+ge;
		Coda=time[i];
		for ( j=0; j<8; j++ )
		{
			if ( ( Coda&0x01 ) !=0 )
			{
				Set_Dio();
			}
			else
			{
				Clr_Dio();
			}

			Set_Sclk();
			Coda=Coda>>1;
			Clr_Sclk();


		}
	}
	Di_Ss();
}


