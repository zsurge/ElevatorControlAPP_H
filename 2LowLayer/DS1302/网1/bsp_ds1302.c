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

uint8_t time[7] = {0x50, 0x30, 0x15, 0x05, 0x06, 0x02, 0x18};
//                 second,minute,hour,date,month,week,year

__Date ModifyDate;

GPIO_InitTypeDef  GPIO;


void DS1302_RST(void)//��SCK=0��RST=1
{
    RST  = 0;//ֹͣ���ݴ���
    SCLK = 0;//ʱ����������
    RST  = 1;//���ݲ���
}

//дһ���ֽ�
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

//��һ���ֽ�
//���������������ʼ��8 ��SCLK ���ڣ�����һ���������ֽڣ������ֽ��ں�8 ��SCLK���ڵ��½������
//ע�⣬��һ�������ֽڵĵ�һ���½��ط����������ֵ����һλ��д��
//��Note that the first data bit to be transmitted occurs on the first falling edge after the last bit of the command byte is written. ����
//Ҳ���������ֽ����һ���ֽ�д���������½���ʹ�õ�һ������λ����ȡ
uint8_t DS1302_Read_Byte(void)
{
    uint8_t i, dat = 0;//��dat�洢����������
    
    RCC_AHB1PeriphClockCmd(DS1302CLKLINE, ENABLE);
    GPIO.GPIO_Pin   = SDA_Pin;
    GPIO.GPIO_Mode  = GPIO_Mode_IN;

    GPIO_Init(DS1302PORT, &GPIO);
    
    for(i = 0; i < 8; i ++)
    {
        dat >>= 1;  
        if(BIT_ADDR(GPIOA_IDR_Addr,1) == 1)
          dat |= 0x80;//��Ϊ�ӵ�λ��ʼ��
        SCLK = 1;
        SCLK = 0;   //��������Ϳ���֪������������Ӧ���ж�    if(SDA==1)��Ȼ�����SCK=1��SCK=0
    }
    return dat;
}

//���д����
void DS1302_CLR_WP(void)
{
    DS1302_RST();
    RST = 1;
    DS1302_Write_Byte(0x8E);//д���ַ
    DS1302_Write_Byte(0x00);//д������
    //д���
    SDA = 0;
    RST = 0;
}

//����д����
void DS1302_SET_WP(void)
{
    DS1302_RST();
    RST = 1;
    DS1302_Write_Byte(0x8E);//д���ַ
    DS1302_Write_Byte(0x80);//д������
    //д���
    SDA = 0;
    RST = 0;
}

//д��ds1302
void DS1302_Write(uint8_t addr, uint8_t dat)
{
    DS1302_RST();
    RST = 1;
    DS1302_Write_Byte(addr);//д���ַ
    DS1302_Write_Byte(dat);//д������
    //д���
    RST = 0;
    SDA = 0;
}

//��ds1302������
uint8_t DS1302_Read(uint8_t addr)
{
    uint8_t temp = 0;//�洢������������
    DS1302_RST();
    RST = 1;
    DS1302_Write_Byte(addr);//д���ַ
    temp = DS1302_Read_Byte();//д������
    //д���
    RST = 0;
    SDA = 0;
    return temp;
}

//void delay(uint16_t n)
//{ 
//  while(n--);
//}


/**
 * �趨ʱ������
 �����漰�����ƺ�BCDת��  ��Ϊds1302��BCD �����ǻ���ֻ��ʶ������
 ���磺����ds1302Ϊ50  0101 0000  ��ô����д�����ds1302_write��80��
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
        temp = DS1302_W_ADDR;//д����ʼλ��
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
 * ��ʱ�����ݣ�BCD��ʽ��
*/
uint8_t *bsp_ds1302_readtime(void)
{
   static uint8_t buf[8] = {0};
   uint8_t i, temp;
   temp = DS1302_R_ADDR;//����ʼλ��
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

	// ��ʼ������
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

