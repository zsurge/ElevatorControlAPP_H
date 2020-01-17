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
TIME_TYPEDEF_STRU gTIME_T;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void bsp_ds1302_settime(void);


                                    
/*************************************************************************************************************************
* 函数 : void bsp_ds1302_init(void)
* 功能 : 配置STM32的GPIO和SPI接口，用于连接 DS1302
* 参数 : 无
* 返回 : 无
* 依赖 : GPIO库函数
* 作者 : 
* 时间 : 
* 最后修改时间 : 2017-1-4修改驱动
* 说明 : DS1302_DIO配置为开漏模式，此模式下能够实现真正的双向IO口
*************************************************************************************************************************/
void bsp_ds1302_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure; 
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //使能PB端口时钟

    GPIO_InitStructure.GPIO_Pin = DS1302_SCK|DS1302_CE; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //IO口速度为50MHz
    GPIO_Init(DS1302_PORT, &GPIO_InitStructure); //根据设定参数初始化GPIOB.8        

	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出
	GPIO_InitStructure.GPIO_Pin = DS1302_DIO;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(DS1302_PORT, &GPIO_InitStructure);    
}



void Dio_In(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Pin = DS1302_DIO ;  
    GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;
    GPIO_Init(DS1302_PORT, &GPIO_InitStructure); //根据设定参数初始化GPIOB.8        
}
 
void  DIO_Out(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
    GPIO_InitStructure.GPIO_Pin = DS1302_DIO ; //LED0-->PA.8 端口配置
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //IO口速度为50MHz
    GPIO_Init(DS1302_PORT, &GPIO_InitStructure); //根据设定参数初始化GPIOB.8        
}
/*************************************************************************************************************************
*函数         : void DS1302_WriteByte(u8 data)
*功能         : 写一个Byte到DS1302
*参数         : data:要写入的Byte
*返回         : 无
*依赖 : 底层宏定义
* 作者 : 
* 时间 :
* 最后修改时间: 
*说明         : 写一个字节的数据SCK上升沿写数据
*************************************************************************************************************************/
void DS1302_WriteByte(unsigned char addr,unsigned char data)
{
    char  i=0;
    int Coda=0;
    
    Coda=(data<<8)|addr;
    En_Ss();
    for(i=0;i<16;i++)
    {  
        if((Coda&0x01)!=0)
                Set_Dio( );
        else
            Clr_Dio();
        
        Set_Sclk( );
        Coda=Coda>>1; 
        Clr_Sclk( );
    }
    Di_Ss( );
} 




/*************************************************************************************************************************
*函数         : u8 DS1302_ReadByte(u8 addr)
*功能         : 从DS1302SPI总线上读取一个字节
*参数         : addr:寄存器地址
*返回         : 读取到的数据
*依赖 : 底层宏定义
* 作者 : 
* 时间 : 
* 最后修改时间: 
*说明         : 读一个字节的数据 SCK下降沿读数据
*************************************************************************************************************************/
unsigned char DS1302_ReadByte(unsigned char addr)
{
    char i,Coda;
    
  Coda=addr;
  En_Ss( );
  for(i=0;i<8;i++)
  { 
            if((Coda&0x01)!=0)
        Set_Dio( );
      else
        Clr_Dio( );
      Set_Sclk( );
          Coda=Coda>>1;
        Clr_Sclk( );
   }
  /************************************/
   Dio_In();   //读数据
   Coda=0;
   for(i=0;i<8;i++)
   {   
            if( Read_Dio()!=0)
        Coda|=(1<<i);
            
        Set_Sclk( );
        Clr_Sclk( );
    }


   Di_Ss();
   DIO_Out();              
   return Coda ;
}




/*************************************************************************************************************************
*函数         : void DS1302_WriteData(u8 addr,u8 data)
*功能         : 向指定寄存器写入一个字节的数据
*参数         : addr:寄存器地址;data:需要写入的数据
*返回         : 无
*依赖 : 底层宏定义
* 作者 : 
* 时间 : 
* 最后修改时间: 
*说明         : 
*************************************************************************************************************************/
void DS1302_WriteData(unsigned char addr,unsigned char data)
{
    Di_Ss();
    Clr_Sclk();
    delay_us(1);
    En_Ss();
    delay_us(2);
    DS1302_WriteByte(addr,data);
    Di_Ss();
    Clr_Sclk();
    delay_us(1);
}

//码制转换

unsigned char BCDToInt(unsigned char bcd) //BCD转十进制
{
  return (0xff & (bcd>>4))*10 +(0xf & bcd);
}

unsigned char IntToBCD(unsigned char intdat) //十进制转BCD
{
    return (((intdat/10)<<4) + ((intdat%10)&0x0f));
}

///*************************************************************************************************************************
//*函数         : 
//*功能         :     2000年~2099年星期算法 
//*参数         : 
//*返回         : 无
//*依赖 : 底层宏定义
//* 作者 :
//* 时间 : 
//* 最后修改时间: 
//*说明         : 
//*************************************************************************************************************************/
unsigned char WeekDay(unsigned char y, unsigned char m, unsigned char d) 
{ 
  unsigned char A;
  if (m==1||m==2)
  {
  m+=12;
  y--; 
  }
  A= (d+2*m+3* (m+1) /5+y+y/4-y/100+y/400+1) %7;
  return A;

}


//显示实时时间
uint8_t *bsp_ds1302_readtime(void)
{
    uint8_t i=0;
    static uint8_t init_time[8] = {0};

    memset(init_time,0x00,sizeof(init_time));    
    
    for(i=0;i<8;i++)
    {
        init_time[i] = DS1302_ReadByte(DS1302_RdSec+(i*2));
    }     

    gTIME_T.year =  BCDToInt(init_time[6]);
    gTIME_T.week =  BCDToInt(init_time[5]);
    gTIME_T.month = BCDToInt(init_time[4]);
    gTIME_T.date =  BCDToInt(init_time[3]);
    gTIME_T.hour =  BCDToInt(init_time[2]);
    gTIME_T.min =   BCDToInt(init_time[1]);
    gTIME_T.sec =   BCDToInt(init_time[0]);

    printf("Time: %d/%d/%d %d:%d:%d\r\n",gTIME_T.year,gTIME_T.month,gTIME_T.date,gTIME_T.hour,gTIME_T.min,gTIME_T.sec);
    return init_time;
}

static void bsp_ds1302_settime(void)
{
    unsigned char i=0;
    unsigned   char init_time[8] = {0};    

    init_time[6] = IntToBCD(gTIME_T.year);
    gTIME_T.week = WeekDay(gTIME_T.year,gTIME_T.month,gTIME_T.date );//自动计算星期
    init_time[5] = IntToBCD(gTIME_T.week );
    init_time[4] = IntToBCD(gTIME_T.month);
    init_time[3] = IntToBCD(gTIME_T.date);
    init_time[2] = IntToBCD(gTIME_T.hour);
    init_time[1] = IntToBCD(gTIME_T.min);
    init_time[0] = IntToBCD(gTIME_T.sec);

    dbh("set time", init_time, 8);

    DS1302_WriteData(DS1302_WrProtect,0x00); //解除写保护
    for(i=0;i<8;i++)
    {
        DS1302_WriteData(DS1302_WrSec+(i*2),init_time[i]);
    }   
    DS1302_WriteData(DS1302_WrProtect,0x80); //使能写保护
    
}

void bsp_ds1302_mdifytime(uint8_t *descTime)
{
    //"2020-01-08 08:30:00"

    uint8_t tmp[5] = {0};

	// 初始化日期
    memset(tmp,0x00,sizeof(tmp));
    memcpy(tmp,descTime+2,2);	
	gTIME_T.year = atoi(tmp);		
    
    memset(tmp,0x00,sizeof(tmp));
    memcpy(tmp,descTime+5,2);  
	gTIME_T.month = atoi(tmp);

    memset(tmp,0x00,sizeof(tmp));
    memcpy(tmp,descTime+8,2);  
	gTIME_T.date = atoi(tmp);

    memset(tmp,0x00,sizeof(tmp));    
    memcpy(tmp,descTime+11,2);  
	gTIME_T.hour = atoi(tmp);

    memset(tmp,0x00,sizeof(tmp));    
    memcpy(tmp,descTime+14,2);  
	gTIME_T.min = atoi(tmp);

    memset(tmp,0x00,sizeof(tmp));    
    memcpy(tmp,descTime+17,2);  
	gTIME_T.sec = atoi(tmp);

    printf("bsp_ds1302_mdifytime: %d/%d/%d %d:%d:%d\r\n",gTIME_T.year,gTIME_T.month,gTIME_T.date,gTIME_T.hour,gTIME_T.min,gTIME_T.sec);

    bsp_ds1302_settime();
}





