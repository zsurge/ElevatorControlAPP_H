/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : stmflash.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��3��
  ����޸�   :
  ��������   : оƬ�ڲ�FLASH����
  �����б�   :
              GetSector     //���ݵ�ַ����������
              STM_FLASH_DisableWriteProtection  //
              STM_FLASH_Erase
              STM_FLASH_GetWriteProtectionStatus
              STM_FLASH_Init
              STM_FLASH_Write
  �޸���ʷ   :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stmflash.h"


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
 
//Private Function
static uint32_t GetSector(uint32_t Address);

/*****************************************************************************
 �� �� ��  : STM_FLASH_Init
 ��������  : Ƭ��FLASH��ʼ��������
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�

    �޸�����   : �����ɺ���

*****************************************************************************/
void STM_FLASH_Init(void)
{ 
  FLASH_Unlock(); 

  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
}

/*****************************************************************************
 �� �� ��  : STM_FLASH_Erase
 ��������  : ���������û���flash����
 �������  : StartSector �û�APP��ʼ��ַ
 �������  : ��
 �� �� ֵ  : 0:�����ɹ�
             1:��������
 
 �޸���ʷ      :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�

    �޸�����   : �����ɺ���

*****************************************************************************/  
uint32_t STM_FLASH_Erase(uint32_t StartSector)
{
  uint32_t UserStartSector = FLASH_Sector_1, i = 0;

  /* Get the sector where start the user flash area */
  UserStartSector = GetSector(StartSector);

  for(i = UserStartSector; i <= FLASH_Sector_11; i += 8)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE)
    {
      /* Error occurred while page erase */
      return (1);
    }
  }
  
  return (0);
}

//This function writes a data buffer in flash (data are 32-bit aligned).
//After writing data buffer, the flash content is checked.
//FlashAddress: start address for writing data buffer
//Data: pointer on data buffer
//DataLength: length of data buffer (unit is 32-bit word)   
//return:
//      0=Data successfully written to Flash memory
//      1=Error occurred while writing data in Flash memory
//      2=Written Data in flash memory is different from expected one



/*****************************************************************************
 �� �� ��  : STM_FLASH_Write
 ��������  : FLASHд�����(������32λ�����)
 �������  : __IO uint32_t* FlashAddress   Ҫд�����ʼ��ַ
             uint32_t* Data              Ҫд�������ָ��
             uint32_t DataLength         Ҫд�����ݵĳ���(unit is 32-bit word)   
 �������  : ��
 �� �� ֵ  : 0=����д��ɹ�
             1=д������ʱ��������
             2=д���������Ԥ�ڵĲ�ͬ
 
 �޸���ʷ      :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�

    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t STM_FLASH_Write(__IO uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
  uint32_t i = 0;

  for (i = 0; (i < DataLength) && (*FlashAddress <= (USER_FLASH_END_ADDRESS-4)); i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    if (FLASH_ProgramWord(*FlashAddress, *(uint32_t*)(Data+i)) == FLASH_COMPLETE)
    {
     /* Check the written value */
      if (*(uint32_t*)*FlashAddress != *(uint32_t*)(Data+i))
      {
        /* Flash content doesn't match SRAM content */
        return(2);
      }
      /* Increment FLASH destination address */
      *FlashAddress += 4;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return (1);
    }
  }
  return (0);
}




/*****************************************************************************
 �� �� ��  : STM_FLASH_GetWriteProtectionStatus
 ��������  : �����û�flash�����д����״̬
 �������  : void
 �������  : ��
 �� �� ֵ  : 0=�û�flash������û��д��������
             1=�û�flash�����ڵ�ĳЩ������д����
            
 �޸���ʷ      :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�

    �޸�����   : �����ɺ���

*****************************************************************************/
uint16_t STM_FLASH_GetWriteProtectionStatus(void)
{
  uint32_t UserStartSector = FLASH_Sector_1;

  /* Get the sector where start the user flash area */
  UserStartSector = GetSector(APPLICATION_ADDRESS);

  /* Check if there are write protected sectors inside the user flash area */
  if ((FLASH_OB_GetWRP() >> (UserStartSector/8)) == (0xFFF >> (UserStartSector/8)))
  { /* No write protected sectors inside the user flash area */
    return 1;
  }
  else
  { /* Some sectors inside the user flash area are write protected */
    return 0;
  }
}

/*****************************************************************************
 �� �� ��  : STM_FLASH_DisableWriteProtection
 ��������  : �����û�flash�����д������
 �������  : void  
 �������  : 
 �� �� ֵ  :    1:д�����ɹ�����
             2:����:Flashдδ����ʧ��
 
 �޸���ʷ      :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t STM_FLASH_DisableWriteProtection(void)
{
  __IO uint32_t UserStartSector = FLASH_Sector_1, UserWrpSectors = OB_WRP_Sector_1;

  /* Get the sector where start the user flash area */
  UserStartSector = GetSector(APPLICATION_ADDRESS);

  /* Mark all sectors inside the user flash area as non protected */  
  UserWrpSectors = 0xFFF-((1 << (UserStartSector/8))-1);
   
  /* Unlock the Option Bytes */
  FLASH_OB_Unlock();

  /* Disable the write protection for all sectors inside the user flash area */
  FLASH_OB_WRPConfig(UserWrpSectors, DISABLE);

  /* Start the Option Bytes programming process. */  
  if (FLASH_OB_Launch() != FLASH_COMPLETE)
  {
    /* Error: Flash write unprotection failed */
    return (2);
  }

  /* Write Protection successfully disabled */
  return (1);
}


/*****************************************************************************
 �� �� ��  : GetSector
 ��������  : ��ȡָ����ַ��������
 �������  : uint32_t Address  
 �������  : 
 �� �� ֵ  : ������
 
 �޸���ʷ      :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�   
    �޸�����   : �����ɺ���

*****************************************************************************/
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }
    return sector;
}







