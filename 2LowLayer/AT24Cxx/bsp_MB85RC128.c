/**
  ******************************************************************************
  *               Copyright(C) 2016-2026 GDKY  All Rights Reserved
  *
  * @file     fm24clxx.c
  * @author   ZouZH
  * @version  V1.08
  * @date     17-March-2016
  * @brief    RAMTRON FRAM Serial Memory Driver.
  ******************************************************************************
  * @history :
  * --2014-12-01 V1.01 Add FM24V10, Change of address space
  * --2014-12-07 V1.02 Add Erase chip function.
  * --2014-12-08 V1.03 Modify the I2C timing parameters
  * --2015-01-22 V1.04 Add FM24V10_2
  * --2015-01-28 V1.05 Add FreeRTOS Mutex Semaphore
  * --2015-02-16 V1.06 Add FM24V10_3 FM24V10_4
  * --2015-06-25 V1.07 Add FM24CL64_2, FM24W256_1, FM24W256_2
  *                    Modify FM24CL64 to FM24CL64_1
  * --2016-03-17 V1.08 Modify FRAM_EraseChip() to FRAM_Erase()
  */

/* INCLUDES ------------------------------------------------------------------- */
#include "bsp_MB85RC128.h"
#include "malloc.h"
#include "tool.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

#define GPIO_PORT_SDA	GPIOF
#define GPIO_PORT_CLK	GPIOF


#define GPIO_PIN_SDA	GPIO_Pin_0
#define GPIO_PIN_CLK	GPIO_Pin_1


#define GPIO_CLK_SDA	RCC_AHB1Periph_GPIOF
#define GPIO_CLK_CLK	RCC_AHB1Periph_GPIOF


#define SDA_L()         GPIO_ResetBits(GPIO_PORT_SDA, GPIO_PIN_SDA)
#define SDA_H()         GPIO_SetBits(GPIO_PORT_SDA, GPIO_PIN_SDA)
#define SDA_READ()      GPIO_ReadInputDataBit(GPIO_PORT_SDA, GPIO_PIN_SDA)

#define SCL_L()         GPIO_ResetBits(GPIO_PORT_CLK, GPIO_PIN_CLK)
#define SCL_H()         GPIO_SetBits(GPIO_PORT_CLK, GPIO_PIN_CLK)


/**
  * @brief  memory address computation
  */
#define ADDR_1st_CYCLE(ADDR)       (uint8_t)((ADDR)& 0xFFUL)               /* 1st addressing cycle */
#define ADDR_2nd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF00UL) >> 8)      /* 2nd addressing cycle */
#define ADDR_3rd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF0000UL) >> 16)   /* 3rd addressing cycle */
#define ADDR_4th_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF000000UL) >> 24) /* 4th addressing cycle */

#define FRAMSIZE (1*1024*1024)


/* CONSTANTS  ----------------------------------------------------------------- */

#define I2C_US_NUM      4
#define I2C_FALSE       0
#define I2C_TRUE        1

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

#ifdef INC_FREERTOS_H
  static SemaphoreHandle_t hMutexFRAM = NULL;
#endif  /* INC_FREERTOS_H */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */
static uint8_t i2c_start(void);
static void    i2c_stop(void);
static void    i2c_ack(void);
static void    i2c_noack(void);
static uint8_t i2c_waitack(void);
static void    i2c_writebyte(uint8_t byte);
static uint8_t i2c_readbyte(void);
static void    i2c_delay(uint16_t us);



/**
 * @brief  FRAM初始�?
 *
 * @param  �?
 *
 * @retval �?
 */
void FRAM_Init(void)
{
  uint8_t i = 0;
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(GPIO_CLK_SDA | GPIO_CLK_CLK, ENABLE);


  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SDA;
  GPIO_Init(GPIO_PORT_SDA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  GPIO_PIN_CLK;
  GPIO_Init(GPIO_PORT_CLK, &GPIO_InitStructure);


  /* I2C总线死锁复位 */
  if (i2c_start() != I2C_TRUE)
  {
    for (i = 0; i < 9; i++)
    {
      i2c_start();
      i2c_stop();
    }
  }

#ifdef INC_FREERTOS_H
  if (hMutexFRAM == NULL)
  {
    hMutexFRAM = xSemaphoreCreateMutex();
  }
#endif /* INC_FREERTOS_H*/

}


/**
 * @brief  дFRAM
 *
 * @param  FM24xx,Ҫд�����ݵĶ�Ӧ�豸
 * @param  addr,Ҫд�����ݵĶ�Ӧ��ַ
 * @param  pvbuf,Ҫд�����ݵĶ�Ӧ����
 * @param  size,Ҫд�����ݵĶ�Ӧ���ݳ���
 *
 * @retval 1Success, 0Fail
 */

uint8_t FRAM_Write(FRAMDev_t FM24xx, uint32_t addr, const void *pvbuf, uint32_t size)
{
  uint32_t addrtmp = 0;
  uint32_t i = 0;
  uint8_t err = I2C_TRUE;
  uint8_t *p = (uint8_t *)pvbuf;

  do
  {
#ifdef INC_FREERTOS_H
    if (hMutexFRAM == NULL)
    {
      err = 0;
      break;
    }
    xSemaphoreTake(hMutexFRAM, portMAX_DELAY);
#endif /* INC_FREERTOS_H */



    err = i2c_start();  if(!err)  continue;

    switch(FM24xx)
    {
      case FM24CL04_1:
      {
        addrtmp = addr & FM24CL04_MAX_ADDR;
        if (!ADDR_2nd_CYCLE(addrtmp))  // page 1 256bytes
        {
          i2c_writebyte(FRAM_WRITE_ADDR);
        }
        else
        {
          i2c_writebyte(FRAM_WRITE_ADDR | (1 << 1));  // page 2
        }
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(addrtmp));  // data addr
        err = i2c_waitack();  if(!err)  continue;
      }break;
      case FM24CL64_1:
      case FM24CL64_2:
      {
        addrtmp = addr & FM24CL64_MAX_ADDR;
        i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24CL64_1) << 1));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(addrtmp));  // high addr
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(addrtmp));  // low addr
        err = i2c_waitack();  if(!err)  continue;
      }break;

      case FM24W256_1:
      case FM24W256_2:
      {
        addrtmp = addr & FM24W256_MAX_ADDR;
        i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24W256_1) << 1));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(addrtmp));  // high addr
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(addrtmp));  // low addr
        err = i2c_waitack();  if(!err)  continue;
      }break;

      case FM24V10_1:
      case FM24V10_2:
      case FM24V10_3:
      case FM24V10_4:
      {
        addrtmp = addr & FM24V10_MAX_ADDR;
        if (!ADDR_3rd_CYCLE(addrtmp))  // page 1 64K
        {
          i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24V10_1) << 2));
        }
        else
        {
          i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24V10_1) << 2) | (1 << 1));  // page 2 64K
        }
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(addrtmp));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(addrtmp));
        err = i2c_waitack();  if(!err)  continue;
      }break;

      default:
      {
        err = I2C_FALSE;
        continue;
      }
    }

    for (i = 0; i < size; i++)
    {
      i2c_writebyte(*(p + i));
      err = i2c_waitack();  if(!err)  continue;
    }
  }while(0);

  i2c_stop();

#ifdef INC_FREERTOS_H
  xSemaphoreGive(hMutexFRAM);
#endif  /* INC_FREERTOS_H */

  return err;
}


/**
 * @brief  ��FRAM
 *
 * @param  FM24xx,Ҫ�������ݵĶ�Ӧ�豸
 * @param  addr,Ҫ�������ݵĶ�Ӧ��ַ
 * @param  pvbuf,Ҫ�������ݵĶ�Ӧ����
 * @param  size,Ҫ�������ݵĶ�Ӧ���ݳ���
 *
 * @retval 1Success, 0Fail
 */

uint8_t FRAM_Read(FRAMDev_t FM24xx, uint32_t addr, void *pvbuf, uint32_t size)
{
  uint32_t addrtmp = 0;
  uint32_t i = 0;
  uint8_t err = I2C_TRUE;
  uint8_t *p = (uint8_t *)pvbuf;
  
  do
  {
#ifdef INC_FREERTOS_H
    if (hMutexFRAM == NULL)
    {
      err = 0;
      break;
    }
    xSemaphoreTake(hMutexFRAM, portMAX_DELAY);
#endif /* INC_FREERTOS_H */

    if(size == 0)
    {
      return err;
    }


    err = i2c_start();  if(!err)  continue;

    switch(FM24xx)
    {
      case FM24CL04_1:
      {
        addrtmp = addr & FM24CL04_MAX_ADDR;
        if (!ADDR_2nd_CYCLE(addrtmp))  // page 1 256bytes
        {
          i2c_writebyte(FRAM_WRITE_ADDR);
        }
        else
        {
          i2c_writebyte(FRAM_WRITE_ADDR | (1 << 1));  // page 2
        }
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(addrtmp));  // data addr
        err = i2c_waitack();  if(!err)  continue;

        err = i2c_start();  if(!err)  continue;
        if (!ADDR_2nd_CYCLE(addrtmp))  // page 1 256bytes
        {
          i2c_writebyte(FRAM_READ_ADDR);
        }
        else
        {
          i2c_writebyte(FRAM_READ_ADDR | (1 << 1));  // page 2
        }
        err = i2c_waitack();  if(!err)  continue;
      }break;

      case FM24CL64_1:
      case FM24CL64_2:
      {
        addrtmp = addr & FM24CL64_MAX_ADDR;
        i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24CL64_1) << 1));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(addrtmp));  // high addr
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(addrtmp));  // low addr
        err = i2c_waitack();  if(!err)  continue;

        err = i2c_start();  if(!err)  continue;
        i2c_writebyte(FRAM_READ_ADDR | ((FM24xx - FM24CL64_1) << 1));
        err = i2c_waitack();  if(!err)  continue;
      }break;

      case FM24W256_1:
      case FM24W256_2:
      {
        addrtmp = addr & FM24W256_MAX_ADDR;
        i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24W256_1) << 1));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(addrtmp));  // high addr
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(addrtmp));  // low addr
        err = i2c_waitack();  if(!err)  continue;

        err = i2c_start();  if(!err)  continue;
        i2c_writebyte(FRAM_READ_ADDR | ((FM24xx - FM24W256_1) << 1));
        err = i2c_waitack();  if(!err)  continue;
      }break;

      case FM24V10_1:
      case FM24V10_2:
      case FM24V10_3:
      case FM24V10_4:
      {
        addrtmp = addr & FM24V10_MAX_ADDR;
        if (!ADDR_3rd_CYCLE(addrtmp))  // page 1 64K
        {
          i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24V10_1) << 2));
        }
        else
        {
          i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24V10_1) << 2) | (1 << 1));  // page 2 64K
        }
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(addrtmp));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(addrtmp));
        err = i2c_waitack();  if(!err)  continue;

        err = i2c_start();  if(!err)  continue;
        if (!ADDR_3rd_CYCLE(addrtmp))  // page 1 64K
        {
          i2c_writebyte(FRAM_READ_ADDR | ((FM24xx - FM24V10_1) << 2));
        }
        else
        {
          i2c_writebyte(FRAM_READ_ADDR | ((FM24xx - FM24V10_1) << 2) | (1 << 1));  // page 2 64K
        }
        err = i2c_waitack();  if(!err)  continue;
      }break;

      default:
      {
        err = I2C_FALSE;
        continue;
      }
    }

    for (i = 0; i < size - 1; i++)
    {
      *(p+i) = i2c_readbyte();
      i2c_ack();
    }
    *(p+size - 1) = i2c_readbyte();
    i2c_noack();
  }while(0);

  i2c_stop();

#ifdef INC_FREERTOS_H
  xSemaphoreGive(hMutexFRAM);
#endif  /* INC_FREERTOS_H */

  return err;
}


/**
 * @brief  ����FRAM����
 *
 * @param  FM24xx, Ҫ�������ݵ��豸
 * @param  startAddr, Ҫ�������ݵ���ʼ��ַ
 * @param  numBytes, Ҫ�������ݵ��ֽ�����
 *
 * @retval 1Success, 0Fail
 */

uint8_t FRAM_Erase(FRAMDev_t FM24xx, uint32_t startAddr, uint32_t numBytes)
{
  uint32_t i = 0;
  uint8_t err = I2C_TRUE;

  do
  {
#ifdef INC_FREERTOS_H
    if (hMutexFRAM == NULL)
    {
      err = 0;
      break;
    }
    xSemaphoreTake(hMutexFRAM, portMAX_DELAY);
#endif /* INC_FREERTOS_H */



    err = i2c_start();  if(!err)  continue;

    switch(FM24xx)
    {
      case FM24CL04_1:
      {
        if (!ADDR_2nd_CYCLE(startAddr))  // page 1 256bytes
        {
          i2c_writebyte(FRAM_WRITE_ADDR);
        }
        else
        {
          i2c_writebyte(FRAM_WRITE_ADDR | (1 << 1));  // page 2
        }
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(startAddr));  // data addr
        err = i2c_waitack();  if(!err)  continue;
      }break;

      case FM24CL64_1:
      case FM24CL64_2:
      {
        i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24CL64_1) << 1));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(startAddr));  // high addr
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(startAddr));  // low addr
        err = i2c_waitack();  if(!err)  continue;
      }break;

      case FM24W256_1:
      case FM24W256_2:
      {
        i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24W256_1) << 1));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(startAddr));  // high addr
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(startAddr));  // low addr
        err = i2c_waitack();  if(!err)  continue;
      }break;

      case FM24V10_1:
      case FM24V10_2:
      case FM24V10_3:
      case FM24V10_4:
      {
        if (!ADDR_3rd_CYCLE(startAddr))  // page 1 64K
        {
          i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24V10_1) << 2));
        }
        else
        {
          i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx - FM24V10_1) << 2) | (1 << 1));  // page 2 64K
        }
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_2nd_CYCLE(startAddr));
        err = i2c_waitack();  if(!err)  continue;
        i2c_writebyte(ADDR_1st_CYCLE(startAddr));
        err = i2c_waitack();  if(!err)  continue;
      }break;

      default:
      {
        err = I2C_FALSE;
        continue;
      }
    }

    for (i = 0; i < numBytes; i++)
    {
      i2c_writebyte(0x00);
      err = i2c_waitack();  if(!err)  continue;
    }
    i2c_stop();

  }while(0);

  if (!err) i2c_stop();


#ifdef INC_FREERTOS_H
  xSemaphoreGive(hMutexFRAM);
#endif  /* INC_FREERTOS_H */

  return err;

}


// SCL: 280Khz/168Mhz
static void i2c_delay(uint16_t us)
{
  volatile uint32_t i = 8 * us;

  while (i--)
  {
    __NOP();
  }
}

static uint8_t i2c_start(void)
{
  i2c_delay(I2C_US_NUM * 2);

  SDA_H();
  SCL_H();

  i2c_delay(I2C_US_NUM);
  if(!SDA_READ()) return I2C_FALSE;  // SDA线为低电平则总线�?,�?�?
  SDA_L();
  i2c_delay(I2C_US_NUM);
  if(SDA_READ())  return I2C_FALSE;  // SDA线为高电平则总线出错,�?�?
  SCL_L();
  i2c_delay(I2C_US_NUM);
  return I2C_TRUE;
}

static void i2c_stop(void)
{
  i2c_delay(I2C_US_NUM);
  SCL_L();
  SDA_L();
  i2c_delay(I2C_US_NUM);
  SCL_H();
  i2c_delay(I2C_US_NUM);
  SDA_H();
  i2c_delay(I2C_US_NUM);
}

static void i2c_ack(void)
{
  i2c_delay(I2C_US_NUM);
  SCL_L();
  SDA_L();
  i2c_delay(I2C_US_NUM);
  SCL_H();
  i2c_delay(I2C_US_NUM);
  SCL_L();
  i2c_delay(I2C_US_NUM);
}

static void i2c_noack(void)
{
  i2c_delay(I2C_US_NUM);
  SCL_L();
  SDA_H();
  i2c_delay(I2C_US_NUM);
  SCL_H();
  i2c_delay(I2C_US_NUM);
  SCL_L();
  i2c_delay(I2C_US_NUM);
}


static uint8_t i2c_waitack(void)
{
  i2c_delay(I2C_US_NUM);
#if 1
  SCL_L();
  SDA_H();
  i2c_delay(I2C_US_NUM);
  SCL_H();
  i2c_delay(I2C_US_NUM);

  if(SDA_READ())
  {
    SCL_L();
    return I2C_FALSE;
  }
  SCL_L();
#else
  uint8_t tmout = 250;

  SCL_L();
  SDA_H();
  i2c_delay(I2C_US_NUM);
  SCL_H();
  i2c_delay(I2C_US_NUM);

  while(SDA_READ())
  {
    if (--tmout < 1)
    {
      SCL_L();
      return I2C_FALSE;
    }
  }
  SCL_L();
#endif

  return I2C_TRUE;
}

// MSB First
static void i2c_writebyte(uint8_t byte)
{
  uint8_t i = 0;

  for (i=0; i<8; i++)
  {
    SCL_L();
    if (byte&0x80)  SDA_H();
    else            SDA_L();
    byte <<= 1;
    i2c_delay(I2C_US_NUM);
    SCL_H();
    i2c_delay(I2C_US_NUM);
  }
  SCL_L();
}

static uint8_t i2c_readbyte(void)
{
  uint8_t i = 0;
  uint8_t recbyte = 0;

  SDA_H();
  for (i=0; i<8; i++)
  {
    recbyte <<= 1;
    SCL_L();
    i2c_delay(I2C_US_NUM);
    SCL_H();
    i2c_delay(I2C_US_NUM);
    if (SDA_READ()) recbyte |= 0x01;
  }
  SCL_L();

  return recbyte;
}





//void farm_test(void)
//{
//    int i =0;
//     int32_t iTime1, iTime2;
//    char rbuf[FRAMSIZE] = {0};
//    char wbuf[FRAMSIZE] = {0};

//    memset(rbuf,0x00,sizeof(rbuf));
//    memset(wbuf,0xBB,sizeof(wbuf));
//    
//    dbh("1 rbuf", rbuf, FRAMSIZE);
//    dbh("2 wbuf", wbuf, FRAMSIZE);
//    iTime1 =  xTaskGetTickCount();  /* ���¿�ʼʱ�� */   

//    if(FRAM_Write(FM24V10_1,0,wbuf,FRAMSIZE) == 1)
//    {		
//        
//        iTime2 =  xTaskGetTickCount();	/* ���½���ʱ�� */
//		printf("дeeprom�ɹ���\r\n");    
//    }
//    else
//    {
//		printf("дeeprom����\r\n");
//		return;
//    }
//	/* ��ӡ���ٶ� */
//	printf("д��ʱ: %dms, д�ٶ�: %dB/s\r\n", iTime2 - iTime1, (FRAMSIZE * 1000) / (iTime2 - iTime1));


//    iTime1 =  xTaskGetTickCount();  /* ���¿�ʼʱ�� */   

//      if(FRAM_Read(FM24V10_1,0,rbuf,FRAMSIZE) == 1)
//      {
//          iTime2 =  xTaskGetTickCount();	/* ���½���ʱ�� */
//          printf("��eeprom�ɹ���\r\n");    
//      }
//      else
//      {
//          printf("��eeprom����\r\n");
//          return;
//      }
//      /* ��ӡ���ٶ� */
//      printf("����ʱ: %dms, ���ٶ�: %dB/s\r\n", iTime2 - iTime1, (FRAMSIZE * 1000) / (iTime2 - iTime1));


//      dbh("A rbuf", rbuf, FRAMSIZE);
//      dbh("B wbuf", wbuf, FRAMSIZE);
//      
//}


//void eeeeee(void)
//{
//    int32_t iTime1, iTime2;
//    iTime1 =  xTaskGetTickCount();  /* ���¿�ʼʱ�� */
//    FRAM_Erase(FM24V10_1,0,FRAMSIZE);
//    iTime2 =  xTaskGetTickCount();	/* ���½���ʱ�� */
//    /* ��ӡ���ٶ� */
//    printf("������ʱ: %dms, �����ٶ�: %dB/s\r\n", iTime2 - iTime1, (FRAMSIZE * 1000) / (iTime2 - iTime1));
//       		
//}


//void farm_read(void)
//{
//     int32_t iTime1, iTime2;
//    char rbuf[FRAMSIZE] = {0};

//    memset(rbuf,0x00,sizeof(rbuf));    
//    iTime1 =  xTaskGetTickCount();  /* ���¿�ʼʱ�� */   


//      if(FRAM_Read(FM24V10_1,0,rbuf,FRAMSIZE) == 1)
//      {
//          iTime2 =  xTaskGetTickCount();	/* ���½���ʱ�� */
//          printf("��eeprom�ɹ���\r\n");    
//      }
//      else
//      {
//          printf("��eeprom����\r\n");
//          return;
//      }
//      /* ��ӡ���ٶ� */
//      printf("����ʱ: %dms, ���ٶ�: %dB/s\r\n", iTime2 - iTime1, (FRAMSIZE * 1000) / (iTime2 - iTime1));


//      dbh("A rbuf", rbuf, FRAMSIZE);
//      
//}

