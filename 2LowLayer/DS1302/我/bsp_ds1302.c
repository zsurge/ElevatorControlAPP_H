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
TIME_TYPEDEF_STRU gTIME_T;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void bsp_ds1302_settime(void);


                                    
/*************************************************************************************************************************
* ���� : void bsp_ds1302_init(void)
* ���� : ����STM32��GPIO��SPI�ӿڣ��������� DS1302
* ���� : ��
* ���� : ��
* ���� : GPIO�⺯��
* ���� : 
* ʱ�� : 
* ����޸�ʱ�� : 2017-1-4�޸�����
* ˵�� : DS1302_DIO����Ϊ��©ģʽ����ģʽ���ܹ�ʵ��������˫��IO��
*************************************************************************************************************************/
void bsp_ds1302_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure; 
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //ʹ��PB�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = DS1302_SCK|DS1302_CE; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //IO���ٶ�Ϊ50MHz
    GPIO_Init(DS1302_PORT, &GPIO_InitStructure); //�����趨������ʼ��GPIOB.8        

	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//��©���
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
    GPIO_Init(DS1302_PORT, &GPIO_InitStructure); //�����趨������ʼ��GPIOB.8        
}
 
void  DIO_Out(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 
    GPIO_InitStructure.GPIO_Pin = DS1302_DIO ; //LED0-->PA.8 �˿�����
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//��©���
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //IO���ٶ�Ϊ50MHz
    GPIO_Init(DS1302_PORT, &GPIO_InitStructure); //�����趨������ʼ��GPIOB.8        
}
/*************************************************************************************************************************
*����         : void DS1302_WriteByte(u8 data)
*����         : дһ��Byte��DS1302
*����         : data:Ҫд���Byte
*����         : ��
*���� : �ײ�궨��
* ���� : 
* ʱ�� :
* ����޸�ʱ��: 
*˵��         : дһ���ֽڵ�����SCK������д����
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
*����         : u8 DS1302_ReadByte(u8 addr)
*����         : ��DS1302SPI�����϶�ȡһ���ֽ�
*����         : addr:�Ĵ�����ַ
*����         : ��ȡ��������
*���� : �ײ�궨��
* ���� : 
* ʱ�� : 
* ����޸�ʱ��: 
*˵��         : ��һ���ֽڵ����� SCK�½��ض�����
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
   Dio_In();   //������
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
*����         : void DS1302_WriteData(u8 addr,u8 data)
*����         : ��ָ���Ĵ���д��һ���ֽڵ�����
*����         : addr:�Ĵ�����ַ;data:��Ҫд�������
*����         : ��
*���� : �ײ�궨��
* ���� : 
* ʱ�� : 
* ����޸�ʱ��: 
*˵��         : 
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

//����ת��

unsigned char BCDToInt(unsigned char bcd) //BCDתʮ����
{
  return (0xff & (bcd>>4))*10 +(0xf & bcd);
}

unsigned char IntToBCD(unsigned char intdat) //ʮ����תBCD
{
    return (((intdat/10)<<4) + ((intdat%10)&0x0f));
}

///*************************************************************************************************************************
//*����         : 
//*����         :     2000��~2099�������㷨 
//*����         : 
//*����         : ��
//*���� : �ײ�궨��
//* ���� :
//* ʱ�� : 
//* ����޸�ʱ��: 
//*˵��         : 
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


//��ʾʵʱʱ��
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
    gTIME_T.week = WeekDay(gTIME_T.year,gTIME_T.month,gTIME_T.date );//�Զ���������
    init_time[5] = IntToBCD(gTIME_T.week );
    init_time[4] = IntToBCD(gTIME_T.month);
    init_time[3] = IntToBCD(gTIME_T.date);
    init_time[2] = IntToBCD(gTIME_T.hour);
    init_time[1] = IntToBCD(gTIME_T.min);
    init_time[0] = IntToBCD(gTIME_T.sec);

    dbh("set time", init_time, 8);

    DS1302_WriteData(DS1302_WrProtect,0x00); //���д����
    for(i=0;i<8;i++)
    {
        DS1302_WriteData(DS1302_WrSec+(i*2),init_time[i]);
    }   
    DS1302_WriteData(DS1302_WrProtect,0x80); //ʹ��д����
    
}

void bsp_ds1302_mdifytime(uint8_t *descTime)
{
    //"2020-01-08 08:30:00"

    uint8_t tmp[5] = {0};

	// ��ʼ������
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





