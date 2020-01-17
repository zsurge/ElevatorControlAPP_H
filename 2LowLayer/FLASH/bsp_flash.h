/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_flash.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��23��
  ����޸�   :
  ��������   : �ⲿFLASH��д����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��23��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H
#include "sys.h"  
#include "delay.h"

#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17

//GPIOA define
#define FLASH_PORT			GPIOB
#define FLASH_CS_PIN		GPIO_Pin_12
#define FLASH_SCK_PIN		GPIO_Pin_13
#define FLASH_MISO_PIN		GPIO_Pin_14
#define FLASH_MOSI_PIN		GPIO_Pin_15

#define FLASH_CS_PINSource		GPIO_PinSource12
#define FLASH_SCK_PINSource		GPIO_PinSource13
#define FLASH_MISO_PINSource	GPIO_PinSource14
#define FLASH_MOSI_PINSource	GPIO_PinSource15

#define FLASH_WriteEnable		0x06 
#define FLASH_WriteDisable		0x04 
#define FLASH_ReadStatusReg		0x05 
#define FLASH_WriteStatusReg	0x01 
#define FLASH_ReadData			0x03 
#define FLASH_FastReadData		0x0B 
#define FLASH_FastReadDual		0x3B 
#define FLASH_PageProgram		0x02 
#define FLASH_BlockErase		0xD8 
#define FLASH_SectorErase		0x20 
#define FLASH_ChipErase			0xC7 
#define FLASH_PowerDown			0xB9 
#define FLASH_ReleasePowerDown	0xAB 
#define FLASH_DeviceID			0xAB 
#define FLASH_ManufactDeviceID	0x90 
#define FLASH_JedecDeviceID		0x9F 


extern u16 FLASH_TYPE;					//����W25QXXоƬ�ͺ�		   

#define	FLASH_CS 			PBout(12)  	//W25QXX��Ƭѡ�ź�

#define FLASH_USE_MALLOC	0			//�����Ƿ�ʹ�ö�̬�ڴ����

void bsp_Flash_Init( void );

u16  bsp_FLASH_ReadID(void);  	    		//��ȡFLASH ID
u8   bsp_FLASH_ReadSR(void);        		//��ȡ״̬�Ĵ��� 
void bsp_FLASH_Write_SR(u8 sr);  			//д״̬�Ĵ���
void bsp_FLASH_Write_Enable(void);  		//дʹ�� 
void bsp_FLASH_Write_Disable(void);		//д����
void bsp_FLASH_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void bsp_FLASH_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //��ȡflash
void bsp_FLASH_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//д��flash
void bsp_FLASH_Erase_Chip(void);    	  	//��Ƭ����
void bsp_FLASH_Erase_Sector(u32 Dst_Addr);	//��������
void bsp_FLASH_Wait_Busy(void);           	//�ȴ�����
void bsp_FLASH_PowerDown(void);        	//�������ģʽ
void bsp_FLASH_WAKEUP(void);				//����







#endif



