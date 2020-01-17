#ifndef __RTC_H__
#define __RTC_H__

#include "stm32f4xx.h"


// ʱ��Դ�궨��
//#define RTC_CLOCK_SOURCE_LSE      
#define RTC_CLOCK_SOURCE_LSI

// �첽��Ƶ����
#define ASYNCHPREDIV         0X7F
// ͬ����Ƶ����
#define SYNCHPREDIV          0XFF



// ʱ��궨��
#define RTC_H12_AMorPM			 RTC_H12_AM  
//Ĭ��ʱ��Ϊ 2020��1��8������8��30��30
#define RTC_DEFAULT_TIEM        "2020-01-08 08:30:00"

//#define HOURS                12          // 0~23
//#define MINUTES              00          // 0~59
//#define SECONDS              00          // 0~59

//  //���ں궨��
//#define WEEKDAY              5          // 1~7
//#define DATE                 03         // 1~31
//#define MONTH                01         // 1~12
//#define YEAR                 20         // 0~99

// ʱ���ʽ�궨��
#define RTC_Format_BINorBCD  RTC_Format_BIN

// ������Ĵ����궨��
#define RTC_BKP_DRX          RTC_BKP_DR0
// д�뵽���ݼĴ��������ݺ궨��
#define RTC_BKP_DATA         0X32F2
 

void bsp_rtc_init(void);

void RTC_CLK_Config(void);
void RTC_TimeAndDate_Set(char *defalutTime);
void RTC_TimeAndDate_Show(void);

uint8_t *GetLocalTime(void);


#endif // __RTC_H__
