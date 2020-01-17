/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   RTC����
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "bsp_rtc.h"
#include "stdio.h"

/**
  * @brief  ����ʱ�������
  * @param  ��
  * @retval ��
  */
void RTC_TimeAndDate_Set(char *defalutTime)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

    uint8_t tmp[5] = {0};


    //"2020-01-08 08:30:00"

	// ��ʼ������
    memset(tmp,0x00,sizeof(tmp));
    memcpy(tmp,defalutTime+2,2);	
	RTC_DateStructure.RTC_Year = atoi(tmp);		
    
    memset(tmp,0x00,sizeof(tmp));
    memcpy(tmp,defalutTime+5,2);  
	RTC_DateStructure.RTC_Month = atoi(tmp);

    memcpy(tmp,defalutTime+8,2);  
	RTC_DateStructure.RTC_Date = atoi(tmp);

	RTC_SetDate ( RTC_Format_BINorBCD, &RTC_DateStructure );
	RTC_WriteBackupRegister ( RTC_BKP_DRX, RTC_BKP_DATA );    

	// ��ʼ��ʱ��
	RTC_TimeStructure.RTC_H12 = RTC_H12_AMorPM;
    
    memcpy(tmp,defalutTime+11,2);  
	RTC_TimeStructure.RTC_Hours = atoi(tmp);

    memcpy(tmp,defalutTime+14,2);  
	RTC_TimeStructure.RTC_Minutes = atoi(tmp);

    memcpy(tmp,defalutTime+17,2);  
	RTC_TimeStructure.RTC_Seconds = atoi(tmp);

    
	RTC_SetTime ( RTC_Format_BINorBCD, &RTC_TimeStructure );
	RTC_WriteBackupRegister ( RTC_BKP_DRX, RTC_BKP_DATA );
}

/**
  * @brief  ��ʾʱ�������
  * @param  ��
  * @retval ��
  */
void RTC_TimeAndDate_Show ( void )
{
	uint8_t Rtctmp=0;
	char LCDTemp[100];
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;


	while ( 1 )
	{
		// ��ȡ����
		RTC_GetTime ( RTC_Format_BIN, &RTC_TimeStructure );
		RTC_GetDate ( RTC_Format_BIN, &RTC_DateStructure );

		// ÿ���ӡһ��
		if ( Rtctmp != RTC_TimeStructure.RTC_Seconds )
		{

			// ��ӡ����
			printf ( "The Date :  Y:20%0.2d - M:%0.2d - D:%0.2d - W:%0.2d\r\n",
			         RTC_DateStructure.RTC_Year,
			         RTC_DateStructure.RTC_Month,
			         RTC_DateStructure.RTC_Date,
			         RTC_DateStructure.RTC_WeekDay );

			//Һ����ʾ����
			//�Ȱ�Ҫ��ʾ��������sprintf����ת��Ϊ�ַ�����Ȼ�������Һ����ʾ������ʾ
			sprintf ( LCDTemp,"The Date:Y:20%0.2d-M:%0.2d-D:%0.2d-W:%0.2d",
			          RTC_DateStructure.RTC_Year,
			          RTC_DateStructure.RTC_Month,
			          RTC_DateStructure.RTC_Date,
			          RTC_DateStructure.RTC_WeekDay );


			// ��ӡʱ��
			printf ( "The Time :  %0.2d:%0.2d:%0.2d \r\n\r\n",
			         RTC_TimeStructure.RTC_Hours,
			         RTC_TimeStructure.RTC_Minutes,
			         RTC_TimeStructure.RTC_Seconds );

			//Һ����ʾʱ��
			sprintf ( LCDTemp,"The Time :  %0.2d:%0.2d:%0.2d",
			          RTC_TimeStructure.RTC_Hours,
			          RTC_TimeStructure.RTC_Minutes,
			          RTC_TimeStructure.RTC_Seconds );

			( void ) RTC->DR;
		}
		Rtctmp = RTC_TimeStructure.RTC_Seconds;
	}
}

/**
  * @brief  RTC���ã�ѡ��RTCʱ��Դ������RTC_CLK�ķ�Ƶϵ��
  * @param  ��
  * @retval ��
  */
void RTC_CLK_Config ( void )
{
	RTC_InitTypeDef RTC_InitStructure;

	/*ʹ�� PWR ʱ��*/
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_PWR, ENABLE );
	/* PWR_CR:DBF��1��ʹ��RTC��RTC���ݼĴ����ͱ���SRAM�ķ��� */
	PWR_BackupAccessCmd ( ENABLE );

#if defined (RTC_CLOCK_SOURCE_LSI)
	/* ʹ��LSI��ΪRTCʱ��Դ�������
	 * Ĭ��ѡ��LSE��ΪRTC��ʱ��Դ
	 */
	/* ʹ��LSI */
	RCC_LSICmd ( ENABLE );
	/* �ȴ�LSI�ȶ� */
	while ( RCC_GetFlagStatus ( RCC_FLAG_LSIRDY ) == RESET )
	{
	}
	/* ѡ��LSI��ΪRTC��ʱ��Դ */
	RCC_RTCCLKConfig ( RCC_RTCCLKSource_LSI );

#elif defined (RTC_CLOCK_SOURCE_LSE)

	/* ʹ��LSE */
	RCC_LSEConfig ( RCC_LSE_ON );
	/* �ȴ�LSE�ȶ� */
	while ( RCC_GetFlagStatus ( RCC_FLAG_LSERDY ) == RESET )
	{
	}
	/* ѡ��LSE��ΪRTC��ʱ��Դ */
	RCC_RTCCLKConfig ( RCC_RTCCLKSource_LSE );

#endif /* RTC_CLOCK_SOURCE_LSI */

	/* ʹ��RTCʱ�� */
	RCC_RTCCLKCmd ( ENABLE );

	/* �ȴ� RTC APB �Ĵ���ͬ�� */
	RTC_WaitForSynchro();

	/*=====================��ʼ��ͬ��/�첽Ԥ��Ƶ����ֵ======================*/
	/* ����������ʱ��ck_spare = LSE/[(255+1)*(127+1)] = 1HZ */

	/* �����첽Ԥ��Ƶ����ֵ */
	RTC_InitStructure.RTC_AsynchPrediv = ASYNCHPREDIV;
	/* ����ͬ��Ԥ��Ƶ����ֵ */
	RTC_InitStructure.RTC_SynchPrediv = SYNCHPREDIV;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	/* ��RTC_InitStructure�����ݳ�ʼ��RTC�Ĵ��� */
	if ( RTC_Init ( &RTC_InitStructure ) == ERROR )
	{
		printf ( "\n\r RTC ʱ�ӳ�ʼ��ʧ�� \r\n" );
	}
}

/**
  * @brief  RTC���ã�ѡ��RTCʱ��Դ������RTC_CLK�ķ�Ƶϵ��
  * @param  ��
  * @retval ��
  */
#define LSE_STARTUP_TIMEOUT     ((uint16_t)0x05000)
void RTC_CLK_Config_Backup ( void )
{
	__IO uint16_t StartUpCounter = 0;
	FlagStatus LSEStatus = RESET;
	RTC_InitTypeDef RTC_InitStructure;

	/* ʹ�� PWR ʱ�� */
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_PWR, ENABLE );
	/* PWR_CR:DBF��1��ʹ��RTC��RTC���ݼĴ����ͱ���SRAM�ķ��� */
	PWR_BackupAccessCmd ( ENABLE );

	/*=========================ѡ��RTCʱ��Դ==============================*/
	/* Ĭ��ʹ��LSE�����LSE��������ʹ��LSI */
	/* ʹ��LSE */
	RCC_LSEConfig ( RCC_LSE_ON );

	/* �ȴ�LSE�����ȶ��������ʱ���˳� */
	do
	{
		LSEStatus = RCC_GetFlagStatus ( RCC_FLAG_LSERDY );
		StartUpCounter++;
	}
	while ( ( LSEStatus == RESET ) && ( StartUpCounter != LSE_STARTUP_TIMEOUT ) );


	if ( LSEStatus == SET )
	{
		printf ( "\n\r LSE �����ɹ� \r\n" );
		/* ѡ��LSE��ΪRTC��ʱ��Դ */
		RCC_RTCCLKConfig ( RCC_RTCCLKSource_LSE );
	}
	else
	{
		printf ( "\n\r LSE ���ϣ�תΪʹ��LSI \r\n" );

		/* ʹ��LSI */
		RCC_LSICmd ( ENABLE );
		/* �ȴ�LSI�ȶ� */
		while ( RCC_GetFlagStatus ( RCC_FLAG_LSIRDY ) == RESET )
		{
		}

		printf ( "\n\r LSI �����ɹ� \r\n" );
		/* ѡ��LSI��ΪRTC��ʱ��Դ */
		RCC_RTCCLKConfig ( RCC_RTCCLKSource_LSI );
	}

	/* ʹ�� RTC ʱ�� */
	RCC_RTCCLKCmd ( ENABLE );
	/* �ȴ� RTC APB �Ĵ���ͬ�� */
	RTC_WaitForSynchro();

	/*=====================��ʼ��ͬ��/�첽Ԥ��Ƶ����ֵ======================*/
	/* ����������ʱ��ck_spare = LSE/[(255+1)*(127+1)] = 1HZ */

	/* �����첽Ԥ��Ƶ����ֵΪ127 */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	/* ����ͬ��Ԥ��Ƶ����ֵΪ255 */
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	/* ��RTC_InitStructure�����ݳ�ʼ��RTC�Ĵ��� */
	if ( RTC_Init ( &RTC_InitStructure ) == ERROR )
	{
		printf ( "\n\r RTC ʱ�ӳ�ʼ��ʧ�� \r\n" );
	}
}


void bsp_rtc_init(void)
{
    /* RTC���ã�ѡ��ʱ��Դ������RTC_CLK�ķ�Ƶϵ�� */
    RTC_CLK_Config();
    
    if (RTC_ReadBackupRegister(RTC_BKP_DRX) != RTC_BKP_DATA)
    {
      /* ����ʱ������� */
          RTC_TimeAndDate_Set(RTC_DEFAULT_TIEM);
    }
    else
    {
      /* ����Ƿ��Դ��λ */
      if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
      {
        printf("\r\n ������Դ��λ....\r\n");
      }
      /* ����Ƿ��ⲿ��λ */
      else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
      {
        printf("\r\n �����ⲿ��λ....\r\n");
      }
    
      printf("\r\n ����Ҫ��������RTC....\r\n");
      
      /* ʹ�� PWR ʱ�� */
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
      /* PWR_CR:DBF��1��ʹ��RTC��RTC���ݼĴ����ͱ���SRAM�ķ��� */
      PWR_BackupAccessCmd(ENABLE);
      /* �ȴ� RTC APB �Ĵ���ͬ�� */
      RTC_WaitForSynchro();   
    } 

}

uint8_t *GetLocalTime(void)
{
    static uint8_t localTime[20] = {0};
    
	uint8_t Rtctmp=0;

	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

	// ��ȡ����
	RTC_GetTime ( RTC_Format_BIN, &RTC_TimeStructure );
	RTC_GetDate ( RTC_Format_BIN, &RTC_DateStructure );

	//Һ����ʾ����
	//�Ȱ�Ҫ��ʾ��������sprintf����ת��Ϊ�ַ�����Ȼ�������Һ����ʾ������ʾ
	sprintf ( localTime,"20%0.2d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d",
	          RTC_DateStructure.RTC_Year,
	          RTC_DateStructure.RTC_Month,
	          RTC_DateStructure.RTC_Date,
	          RTC_TimeStructure.RTC_Hours,
	          RTC_TimeStructure.RTC_Minutes,
	          RTC_TimeStructure.RTC_Seconds);


    return localTime;
}


/**********************************END OF FILE*************************************/
