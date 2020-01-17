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

uint8_t time[7] = {0x50, 0x30, 0x15, 0x05, 0x06, 0x02, 0x18};
//                 second,minute,hour,date,month,week,year

__Date ModifyDate;

GPIO_InitTypeDef  GPIO;


void DS1302_RST(void)//令SCK=0；RST=1
{
    RST  = 0;//停止数据传送
    SCLK = 0;//时钟总线清零
    RST  = 1;//数据操作
}

//写一个字节
void DS1302_Write_Byte(uint8_t dat)
{
    uint8_t i;
    RCC_AHB1PeriphClockCmd(DS1302CLKLINE, ENABLE);
    GPIO.GPIO_Pin   = SDA_Pin;
    GPIO.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(DS1302PORT, &GPIO);
    
    for(i = 0; i < 8; i ++)
    {
        SDA  = dat & 0x01;
        SCLK = 1;
        SCLK = 0;
        dat >>= 1;
    }
}

//读一个字节
//对于数据输出：开始的8 个SCLK 周期，输入一个读命令字节，数据字节在后8 个SCLK周期的下降沿输出
//注意，第一个数据字节的第一个下降沿发生后，命令字的最后一位被写入
//（Note that the first data bit to be transmitted occurs on the first falling edge after the last bit of the command byte is written. ），
//也就是命令字节最后一个字节写入后产生的下降沿使得第一个数据位被读取
uint8_t DS1302_Read_Byte(void)
{
    uint8_t i, dat = 0;//用dat存储读到的数据
    
    RCC_AHB1PeriphClockCmd(DS1302CLKLINE, ENABLE);
    GPIO.GPIO_Pin   = SDA_Pin;
    GPIO.GPIO_Mode  = GPIO_Mode_IN;

    GPIO_Init(DS1302PORT, &GPIO);
    
    for(i = 0; i < 8; i ++)
    {
        dat >>= 1;  
        if(BIT_ADDR(GPIOA_IDR_Addr,1) == 1)
          dat |= 0x80;//因为从低位开始读
        SCLK = 1;
        SCLK = 0;   //由上面解释可以知道：我们首先应该判断    if(SDA==1)，然后进行SCK=1，SCK=0
    }
    return dat;
}

//清除写保护
void DS1302_CLR_WP(void)
{
    DS1302_RST();
    RST = 1;
    DS1302_Write_Byte(0x8E);//写入地址
    DS1302_Write_Byte(0x00);//写入数据
    //写完后
    SDA = 0;
    RST = 0;
}

//设置写保护
void DS1302_SET_WP(void)
{
    DS1302_RST();
    RST = 1;
    DS1302_Write_Byte(0x8E);//写入地址
    DS1302_Write_Byte(0x80);//写入数据
    //写完后
    SDA = 0;
    RST = 0;
}

//写入ds1302
void DS1302_Write(uint8_t addr, uint8_t dat)
{
    DS1302_RST();
    RST = 1;
    DS1302_Write_Byte(addr);//写入地址
    DS1302_Write_Byte(dat);//写入数据
    //写完后
    RST = 0;
    SDA = 0;
}

//从ds1302读数据
uint8_t DS1302_Read(uint8_t addr)
{
    uint8_t temp = 0;//存储读出来的数据
    DS1302_RST();
    RST = 1;
    DS1302_Write_Byte(addr);//写入地址
    temp = DS1302_Read_Byte();//写入数据
    //写完后
    RST = 0;
    SDA = 0;
    return temp;
}

//void delay(uint16_t n)
//{ 
//  while(n--);
//}


/**
 * 设定时钟数据
 其中涉及二进制和BCD转换  因为ds1302存BCD ，但是机器只认识二进制
 例如：存入ds1302为50  0101 0000  那么我们写入的是ds1302_write（80）
*/
//void set_time(uint8_t *timedata)

void bsp_ds1302_init(void)
{
    uint8_t i;
    uint8_t temp = 0;
    RCC_APB2PeriphClockCmd(DS1302CLKLINE, ENABLE);
    //Register IO 
    GPIO.GPIO_Pin   = SCLK_Pin | RST_Pin | SDA_Pin;
    GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO.GPIO_OType = GPIO_OType_PP;
    
    GPIO_Init(DS1302PORT, &GPIO);
    

    if (DS1302_Read(0xcf) != 0xaa)
    {
        DS1302_CLR_WP();
        temp = DS1302_W_ADDR;//写入起始位置
        for(i = 0; i < 7; i ++)
        {
            DS1302_Write(temp, time[i]);
            temp += 2;
        }
    //  DS1302_Write(0x90, 0xa5);
        DS1302_Write(0xce, 0xaa);
        DS1302_SET_WP();
    }
}

/**
 * 读时钟数据（BCD格式）
*/
uint8_t *bsp_ds1302_readtime(void)
{
   static uint8_t buf[8] = {0};
   uint8_t i, temp;
   temp = DS1302_R_ADDR;//读起始位置
   for(i = 0; i < 7; i ++)
   {
     buf[i] = DS1302_Read(temp);
     temp += 2;

     printf("%d\t:",buf[i]);
   }

    printf("\r\n");
    dbh("bsp_ds1302_readtime", buf, 8);
   
   return buf;
}

void bsp_ds1302_mdifytime(uint8_t *descTime)
{
    uint8_t tmp[5] = {0};

	// 初始化日期
    memset(tmp,0x00,sizeof(tmp));
    memcpy(tmp,descTime+2,2);	
	ModifyDate.year = atoi(tmp);		
    
    memset(tmp,0x00,sizeof(tmp));
    memcpy(tmp,descTime+5,2);  
	ModifyDate.month = atoi(tmp);

    memset(tmp,0x00,sizeof(tmp));
    memcpy(tmp,descTime+8,2);  
	ModifyDate.day = atoi(tmp);

    memset(tmp,0x00,sizeof(tmp));    
    memcpy(tmp,descTime+11,2);  
	ModifyDate.hour = atoi(tmp);

    memset(tmp,0x00,sizeof(tmp));    
    memcpy(tmp,descTime+14,2);  
	ModifyDate.minute = atoi(tmp);

    memset(tmp,0x00,sizeof(tmp));    
    memcpy(tmp,descTime+17,2);  
	ModifyDate.second = atoi(tmp);

    ModifyDate.week = 2;

    printf("Time: %d/%d/%d %d:%d:%d\r\n",ModifyDate.year,ModifyDate.month,ModifyDate.day,ModifyDate.hour,ModifyDate.minute,ModifyDate.second);

    
    DS1302_CLR_WP();
    DS1302_Write(0x80, ModifyDate.second);
    DS1302_Write(0x82, ModifyDate.minute);
    DS1302_Write(0x84, ModifyDate.hour);
    DS1302_Write(0x86, ModifyDate.day);
    DS1302_Write(0x88, ModifyDate.month);
    DS1302_Write(0x8A, ModifyDate.week);
    DS1302_Write(0x8C, ModifyDate.year);
    DS1302_Write(0xce, 0xaa);
    DS1302_SET_WP();
}

