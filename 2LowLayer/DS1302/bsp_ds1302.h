#ifndef __BSP_DS1302_H__
#define __BSP_DS1302_H__


#include "sys.h"
#include "delay.h"



/* �������� ------------------------------------------------------------------*/
/* �궨�� --------------------------------------------------------------------*/

#define DS1302_PORT GPIOE
#define DS1302CLK	GPIO_Pin_9
#define DS1302DAT 	GPIO_Pin_8
#define DS1302RST 	GPIO_Pin_7    



/* ���� ----------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/




void bsp_ds1302_init(void);
void bsp_ds1302_mdifytime(u8 *descTime);
char *bsp_ds1302_readtime(void);

char* time_to_timestamp(void);
void  timestamp_to_time(unsigned int timestamp);





#endif

