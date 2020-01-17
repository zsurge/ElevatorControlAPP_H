/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_ds1302.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2020年1月14日
  最近修改   :
  功能描述   : 时钟模块驱动
  函数列表   :
  修改历史   :
  1.日    期   : 2020年1月14日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stdio.h"
#include "bsp_ds1302.h"
#include "tool.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
uint8_t u8time[8];
uint8_t rtc_init[8] =
{
	58, //秒 0
	18, //分 1
	10, //小时2
	15,//日 3
	1,//月 4
	3,//周
	20,//2016年 6
	0,//写效许 7
};


/*************************************************************************************************************************
* 函数 : void DS1302_GPIO_Init(void)
* 功能 : 配置STM32的GPIO和SPI接口，用于连接 DS1302
* 参数 : 无
* 返回 : 无
* 依赖 : GPIO库函数
* 作者 : li_qcxy@126.com
* 时间 : 2016-12-9
* 最后修改时间 : 2017-1-4修改驱动
* 说明 : DS1302_DIO配置为开漏模式，此模式下能够实现真正的双向IO口
*************************************************************************************************************************/
void DS1302_GPIO_Init ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOE, ENABLE ); //使能PA,PD端口时钟
	
	GPIO_InitStructure.GPIO_Pin = DS1302_SCK|DS1302_DIO|DS1302_CE; //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz; //IO口速度为50MHz
	GPIO_Init ( DS1302_PORT, &GPIO_InitStructure ); //根据设定参数初始化GPIOA.8
}


void Dio_In ( void )

{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOE, ENABLE );//使能PA,PD端口时钟
	GPIO_InitStructure.GPIO_Pin = DS1302_DIO ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init ( DS1302_PORT, &GPIO_InitStructure );
}

void DIO_Out ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOE, ENABLE ); //使能PA,PD端口时钟

	GPIO_InitStructure.GPIO_Pin = DS1302_DIO ; //LED0-->PA.8 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; //开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init ( DS1302_PORT, &GPIO_InitStructure ); //根据设定参数初始化GPIOA.8
}
/*************************************************************************************************************************
*函数     : void DS1302_WriteByte(uint8_t data)
*功能     : 写一个Byte到DS1302
*参数     : data:要写入的Byte
*返回     : 无
*依赖 : 底层宏定义
* 作者 : li_qcxy@126.com
* 时间 : 2016-12-09
* 最后修改时间:
*说明     : 写一个字节的数据SCK上升沿写数据
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
*函数     : uint8_t DS1302_ReadByte(uint8_t addr)
*功能     : 从DS1302SPI总线上读取一个字节
*参数     : addr:寄存器地址
*返回     : 读取到的数据
*依赖 : 底层宏定义
* 作者 : li_qcxy@126.com
* 时间 : 2016-12-09
* 最后修改时间:
*说明     : 读一个字节的数据 SCK下降沿读数据
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
	//读数据
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
*函数     : void DS1302_WriteData(uint8_t addr,uint8_t data)
*功能     : 向指定寄存器写入一个字节的数据
*参数     : addr:寄存器地址;data:需要写入的数据
*返回     : 无
*依赖 : 底层宏定义
* 作者 : li_qcxy@126.com
* 时间 : 2016-12-09
* 最后修改时间:
*说明     :
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
*函数     : void DS1302_ReadTime(uint8_t addr,uint8_t time[8])
*功能     : 处理数据并通过串口打印
*参数     : read:要写入的Byte
*返回     : 无
*依赖 : 底层宏定义
* 作者 : li_qcxy@126.com
* 时间 : 2016-12-10
* 最后修改时间:
*说明     :
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
	//读数据
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
*函数     : void DS1302_Settime(uint8_t *Buffer)
*功能     : 对时
*参数     : 要写入年 月 日 时 分 秒
*返回     : 无
*依赖 : 底层宏定义
* 作者 : li_qcxy@126.com
* 时间 : 2016-12-10
* 最后修改时间:
*说明     :
*************************************************************************************************************************/
void DS1302_Settime ( uint8_t addr,uint8_t time[8] )
{
	char i,j,ge;
	int Coda=0;


	Coda=addr;
	En_Ss();
	for ( i=0; i<8; i++ ) //时间地址
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

	for ( i=0; i<8; i++ ) //时间数据
	{
		ge=time[i]%10;//个位数部分
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


