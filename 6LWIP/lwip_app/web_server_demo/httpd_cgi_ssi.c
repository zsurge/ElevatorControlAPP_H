#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "lwip_comm.h"
#include "led.h"
#include "beep.h"
#include "adc.h"
#include "rtc.h"
#include "lcd.h"

#include <string.h>
#include <stdlib.h>
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//NETCONN API��̷�ʽ��WebServer���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   


#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define NUM_CONFIG_SSI_TAGS	(sizeof(ppcTAGs) / sizeof(char *))
	
extern short Get_Temprate(void);  //����Get_Temperate()����
extern void RTC_Get_Time(u8 *hour,u8 *min,u8 *sec,u8 *ampm); //����RTC_Get_Timer()����
extern void RTC_Get_Date(u8 *year,u8 *month,u8 *date,u8 *week); //����RTC_Get_Date()����

//����LED��CGI handler
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* BEEP_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);

static const char *ppcTAGs[]=  //SSI��Tag
{
	"t", //ADCֵ
	"w", //�¶�ֵ
	"h", //ʱ��
	"y"  //����
};


static const tCGI ppcURLs[]= //cgi����
{
	{"/leds.cgi",LEDS_CGI_Handler},
	{"/beep.cgi",BEEP_CGI_Handler},
};


//��web�ͻ��������������ʱ��,ʹ�ô˺�����CGI handler����
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop); //����iLOOP
		}
	}
	return (-1);
}


//SSIHandler����Ҫ�õ��Ĵ���ADC�ĺ���
void ADC_Handler(char *pcInsert)
{ 
    char Digit1=0, Digit2=0, Digit3=0, Digit4=0; 
    uint32_t ADCVal = 0;        

     //��ȡADC��ֵ
     ADCVal = Get_Adc_Average(5,10); //��ȡADC1_CH5�ĵ�ѹֵ
		
     
     //ת��Ϊ��ѹ ADCVval * 0.8mv
     ADCVal = (uint32_t)(ADCVal * 0.8);  
     
     Digit1= ADCVal/1000;
     Digit2= (ADCVal-(Digit1*1000))/100 ;
     Digit3= (ADCVal-((Digit1*1000)+(Digit2*100)))/10;
     Digit4= ADCVal -((Digit1*1000)+(Digit2*100)+ (Digit3*10));
        
     //׼����ӵ�html�е�����
     *pcInsert       = (char)(Digit1+0x30);
     *(pcInsert + 1) = (char)(Digit2+0x30);
     *(pcInsert + 2) = (char)(Digit3+0x30);
     *(pcInsert + 3) = (char)(Digit4+0x30);
}

//SSIHandler����Ҫ�õ��Ĵ����ڲ��¶ȴ������ĺ���
void Temperate_Handler(char *pcInsert)
{
		char Digit1=0, Digit2=0, Digit3=0, Digit4=0,Digit5=0; 
		short Temperate = 0;
		
		//��ȡ�ڲ��¶�ֵ
		Temperate = Get_Temprate(); //��ȡ�¶�ֵ �˴�������100��
	  Digit1 = Temperate / 10000;
		Digit2 = (Temperate % 10000)/1000;
    Digit3 = (Temperate % 1000)/100 ;
    Digit4 = (Temperate % 100)/10;
    Digit5 = Temperate % 10;
		//��ӵ�html�е�����
		*pcInsert 		= (char)(Digit1+0x30);
		*(pcInsert+1) = (char)(Digit2+0x30);
		*(pcInsert+2)	=	(char)(Digit3+0x30);
		*(pcInsert+3) = '.';
		*(pcInsert+4) = (char)(Digit4+0x30);
		*(pcInsert+5) = (char)(Digit5+0x30);
}

//SSIHandler����Ҫ�õ��Ĵ���RTCʱ��ĺ���
void RTCTime_Handler(char *pcInsert)
{
	u8 hour,min,sec,ampm;
	
	RTC_Get_Time(&hour,&min,&sec,&ampm);
	
	*pcInsert = 		(char)((hour/10) + 0x30);
	*(pcInsert+1) = (char)((hour%10) + 0x30);
	*(pcInsert+2) = ':';
	*(pcInsert+3) = (char)((min/10) + 0x30);
	*(pcInsert+4) = (char)((min%10) + 0x30);
	*(pcInsert+5) = ':';
	*(pcInsert+6) = (char)((sec/10) + 0x30);
	*(pcInsert+7) = (char)((sec%10) + 0x30);
}

//SSIHandler����Ҫ�õ��Ĵ���RTC���ڵĺ���
void RTCdate_Handler(char *pcInsert)
{
	u8 year,month,date,week;
	RTC_Get_Date(&year,&month,&date,&week);
	
	*pcInsert = '2';
	*(pcInsert+1) = '0';
	*(pcInsert+2) = (char)((year/10) + 0x30);
	*(pcInsert+3) = (char)((year%10) + 0x30);
	*(pcInsert+4) = '-';
	*(pcInsert+5) = (char)((month/10) + 0x30);
	*(pcInsert+6) = (char)((month%10) + 0x30);
	*(pcInsert+7) = '-';
	*(pcInsert+8) = (char)((date/10) + 0x30);
	*(pcInsert+9) = (char)((date%10) + 0x30);
	*(pcInsert+10) = ' ';
	*(pcInsert+11) = 'w';
	*(pcInsert+12) = 'e';
	*(pcInsert+13) = 'e';
	*(pcInsert+14) = 'k';
	*(pcInsert+15) = ':';
	*(pcInsert+16) = (char)(week + 0x30);
	
}
//SSI��Handler���
static u16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{
	switch(iIndex)
	{
		case 0: 
				ADC_Handler(pcInsert);
				break;
		case 1:
				Temperate_Handler(pcInsert);
				break;
		case 2:
				RTCTime_Handler(pcInsert);
				break;
		case 3:
				RTCdate_Handler(pcInsert);
				break;
	}
	return strlen(pcInsert);
}

//CGI LED���ƾ��
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  u8 i=0;  //ע������Լ���GET�Ĳ����Ķ�����ѡ��iֵ��Χ
  
	iIndex = FindCGIParameter("LED1",pcParam,iNumParams);  //�ҵ�led��������
	//ֻ��һ��CGI��� iIndex=0
	if (iIndex != -1)
	{
			LED1=1;  //�ر����е�LED1��
		for (i=0; i<iNumParams; i++) //���CGI����: example GET /leds.cgi?led=2&led=4 */
		{
		  if (strcmp(pcParam[i] , "LED1")==0)  //������"led"
		  {
			if(strcmp(pcValue[i], "LED1ON") ==0)  //�ı�LED1״̬
			  LED1=0; //��LED1
					else if(strcmp(pcValue[i],"LED1OFF") == 0)
						LED1=1; //�ر�LED1
		  }
		}
	 }
	if(LED1 == 0 && BEEP == 0) 		return "/STM32F407LED_ON_BEEP_OFF.shtml";  	//LED1��,BEEP��
	else if(LED1 == 0 && BEEP == 1) return "/STM32F407LED_ON_BEEP_ON.shtml";	//LED1��,BEEP��
	else if(LED1 == 1 && BEEP == 1) return "/STM32F407LED_OFF_BEEP_ON.shtml";	//LED1��,BEEP��
	else return "/STM32F407LED_OFF_BEEP_OFF.shtml";   							//LED1��,BEEP��					
}

//BEEP��CGI���ƾ��
const char *BEEP_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{
	u8 i=0;
	iIndex = FindCGIParameter("BEEP",pcParam,iNumParams);  //�ҵ�BEEP��������
	if(iIndex != -1) //�ҵ�BEEP������
	{
		BEEP=0;  //�ر�
		for(i = 0;i < iNumParams;i++)
		{
			if(strcmp(pcParam[i],"BEEP") == 0)  //����CGI����
			{
				if(strcmp(pcValue[i],"BEEPON") == 0) //��BEEP
					BEEP = 1;
				else if(strcmp(pcValue[i],"BEEPOFF") == 0) //�ر�BEEP
					BEEP = 0;
			}
		}
	}
	if(LED1 == 0 && BEEP == 0) 		return "/STM32F407LED_ON_BEEP_OFF.shtml";  	//LED1��,BEEP��
	else if(LED1 == 0 && BEEP == 1) return "/STM32F407LED_ON_BEEP_ON.shtml";	//LED1��,BEEP��
	else if(LED1 == 1 && BEEP == 1) return "/STM32F407LED_OFF_BEEP_ON.shtml";	//LED1��,BEEP��
	else return "/STM32F407LED_OFF_BEEP_OFF.shtml";   //LED1��,BEEP��
	
}

//SSI�����ʼ��
void httpd_ssi_init(void)
{  
	//�����ڲ��¶ȴ�������SSI���
	http_set_ssi_handler(SSIHandler,ppcTAGs,NUM_CONFIG_SSI_TAGS);
}

//CGI�����ʼ��
void httpd_cgi_init(void)
{ 
  //����CGI���LEDs control CGI) */
  http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}


