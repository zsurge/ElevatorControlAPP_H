/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_flash.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��23��
  ����޸�   :
  ��������   : FLASH����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��23��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#include "bsp_flash.h"

u16 FLASH_TYPE=W25Q128;	//Ĭ����W25Q128

void SPI1_Init(void);
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler);
u8 SPI1_ReadWriteByte(u8 TxData);



//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ 						  
//SPI�ڳ�ʼ��
//�������Ƕ�SPI1�ĳ�ʼ��
void SPI1_Init(void)
{	 
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOAʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//ʹ��SPI1ʱ��

    //GPIOFB3,4,5��ʼ������
    GPIO_InitStructure.GPIO_Pin = FLASH_SCK_PIN|FLASH_MISO_PIN|FLASH_MOSI_PIN;//PB3~5���ù������	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(FLASH_PORT, &GPIO_InitStructure);//��ʼ��

    GPIO_PinAFConfig(FLASH_PORT,FLASH_SCK_PINSource,GPIO_AF_SPI1); //PB3����Ϊ SPI1
    GPIO_PinAFConfig(FLASH_PORT,FLASH_MISO_PINSource,GPIO_AF_SPI1); //PB4����Ϊ SPI1
    GPIO_PinAFConfig(FLASH_PORT,FLASH_MOSI_PINSource,GPIO_AF_SPI1); //PB5����Ϊ SPI1

    //����ֻ���SPI�ڳ�ʼ��
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//��λSPI1
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//ֹͣ��λSPI1

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
    SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

    SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����

    SPI1_ReadWriteByte(0xff);//��������		
}   

//SPI1�ٶ����ú���
//SPI�ٶ�=fAPB2/��Ƶϵ��
//@ref SPI_BaudRate_Prescaler:SPI_BaudRatePrescaler_2~SPI_BaudRatePrescaler_256  
//fAPB2ʱ��һ��Ϊ84Mhz��
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//�ж���Ч��
	SPI1->CR1&=0XFFC7;//λ3-5���㣬�������ò�����
	SPI1->CR1|=SPI_BaudRatePrescaler;	//����SPI1�ٶ� 
	SPI_Cmd(SPI1,ENABLE); //ʹ��SPI1 
} 


//SPI1 ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI1_ReadWriteByte(u8 TxData)
{		 			 
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){}//�ȴ���������  

    SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ��byte  ����
    	
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){} //�ȴ�������һ��byte  

    return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����				    
}


/*****************************************************************************
 �� �� ��  : bsp_Flash_Init
 ��������  : FLASH ��ʼ��
 �������  : void
 �������  : ��
 �� �� ֵ  : void
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��24��
    ��    ��   : �Ŷ�

    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Flash_Init( void )
{
    //��ʼ��SPI FLASH��IO��
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);//ʹ��GPIOGʱ��

    //GPIOB14
    GPIO_InitStructure.GPIO_Pin = FLASH_CS_PIN;//PB14
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(FLASH_PORT, &GPIO_InitStructure);//��ʼ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//PG7
    GPIO_Init(GPIOG, &GPIO_InitStructure);//��ʼ��

    GPIO_SetBits(GPIOG,GPIO_Pin_7); //PG7���1,��ֹNRF����SPI FLASH��ͨ�� 

    
    FLASH_CS=1;			        //SPI FLASH��ѡ��
    SPI1_Init();		   			//��ʼ��SPI
    SPI1_SetSpeed(SPI_BaudRatePrescaler_4);		//����Ϊ21Mʱ��,����ģʽ 
    FLASH_TYPE = bsp_FLASH_ReadID();	            //��ȡFLASH ID.
}

//��ȡFLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
u8 bsp_FLASH_ReadSR(void)   
{  
	u8 byte=0;   
	FLASH_CS=0;                            //ʹ������   
	SPI1_ReadWriteByte(FLASH_ReadStatusReg);	//���Ͷ�ȡ״̬�Ĵ�������    
	byte=SPI1_ReadWriteByte(0Xff);        	//��ȡһ���ֽ�  
	FLASH_CS=1;                            //ȡ��Ƭѡ     
	return byte;   
} 
//дFLASH״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void bsp_FLASH_Write_SR(u8 sr)   
{   
	FLASH_CS=0;                            //ʹ������   
	SPI1_ReadWriteByte(FLASH_WriteStatusReg);   //����дȡ״̬�Ĵ�������    
	SPI1_ReadWriteByte(sr);              	//д��һ���ֽ�  
	FLASH_CS=1;                            //ȡ��Ƭѡ     	      
}   
//FLASHдʹ��	
//��WEL��λ   
void bsp_FLASH_Write_Enable(void)   
{
	FLASH_CS=0;                            //ʹ������   
    SPI1_ReadWriteByte(FLASH_WriteEnable); 	//����дʹ��  
	FLASH_CS=1;                            //ȡ��Ƭѡ     	      
} 
//FLASHд��ֹ	
//��WEL����  
void bsp_FLASH_Write_Disable(void)   
{  
	FLASH_CS=0;                            //ʹ������   
    SPI1_ReadWriteByte(FLASH_WriteDisable); 	//����д��ָֹ��    
	FLASH_CS=1;                            //ȡ��Ƭѡ     	      
} 		
//��ȡоƬID
//����ֵ����:				   
//0XEF13,��ʾоƬ�ͺ�ΪW25Q80  
//0XEF14,��ʾоƬ�ͺ�ΪW25Q16    
//0XEF15,��ʾоƬ�ͺ�ΪW25Q32  
//0XEF16,��ʾоƬ�ͺ�ΪW25Q64 
//0XEF17,��ʾоƬ�ͺ�ΪW25Q128 	  
u16 bsp_FLASH_ReadID(void)
{
	u16 Temp = 0;	  
	FLASH_CS=0;				    
	SPI1_ReadWriteByte(0x90);//���Ͷ�ȡID����	    
	SPI1_ReadWriteByte(0x00); 	    
	SPI1_ReadWriteByte(0x00); 	    
	SPI1_ReadWriteByte(0x00); 	 			   
	Temp|=SPI1_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI1_ReadWriteByte(0xFF);	 
	FLASH_CS=1;				    
	return Temp;
}   		    
//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void bsp_FLASH_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;   										    
	FLASH_CS=0;                            	//ʹ������   
    SPI1_ReadWriteByte(FLASH_ReadData);   		//���Ͷ�ȡ����   
    SPI1_ReadWriteByte((u8)((ReadAddr)>>16));	//����24bit��ַ    
    SPI1_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPI1_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI1_ReadWriteByte(0XFF);   	//ѭ������  
    }
	FLASH_CS=1;  				    	      
}  
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void bsp_FLASH_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    bsp_FLASH_Write_Enable();                  //SET WEL 
	FLASH_CS=0;                            //ʹ������   
    SPI1_ReadWriteByte(FLASH_PageProgram);      //����дҳ����   
    SPI1_ReadWriteByte((u8)((WriteAddr)>>16)); //����24bit��ַ    
    SPI1_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPI1_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI1_ReadWriteByte(pBuffer[i]);//ѭ��д��  
	FLASH_CS=1;                 			//ȡ��Ƭѡ 
	bsp_FLASH_Wait_Busy();						//�ȴ�д�����
} 
//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void bsp_FLASH_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�
	while(1)
	{	   
		bsp_FLASH_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//д�������
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
			else pageremain=NumByteToWrite; 	  //����256���ֽ���
		}
	};	    
} 
//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)						
//NumByteToWrite:Ҫд����ֽ���(���65535) 
#if FLASH_USE_MALLOC==0 
u8 FLASH_BUFFER[4096];		 
#endif
void bsp_FLASH_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * FLASH_BUF;	
#if	FLASH_USE_MALLOC==1	//��̬�ڴ����
	FLASH_BUF=mymalloc(SRAMIN,4096);//�����ڴ�
#else
   	FLASH_BUF=FLASH_BUFFER; 
#endif     
 	secpos=WriteAddr/4096;//������ַ  
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
	while(1) 
	{	
		bsp_FLASH_Read(FLASH_BUF,secpos*4096,4096);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(FLASH_BUF[secoff+i]!=0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			bsp_FLASH_Erase_Sector(secpos);	//�����������
			for(i=0;i<secremain;i++)	  	//����
			{
				FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			bsp_FLASH_Write_NoCheck(FLASH_BUF,secpos*4096,4096);//д����������  

		}else bsp_FLASH_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumByteToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		   	NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
			else secremain=NumByteToWrite;			//��һ����������д����
		}	 
	};	
#if	FLASH_USE_MALLOC==1		 
	myfree(SRAMIN,FLASH_BUF);	//�ͷ��ڴ�
#endif	
}
//��������оƬ		  
//�ȴ�ʱ�䳬��...
void bsp_FLASH_Erase_Chip(void)   
{                                   
    bsp_FLASH_Write_Enable();                  //SET WEL 
    bsp_FLASH_Wait_Busy();   
  	FLASH_CS=0;                            //ʹ������   
    SPI1_ReadWriteByte(FLASH_ChipErase);		//����Ƭ��������  
	FLASH_CS=1;                            //ȡ��Ƭѡ     	      
	bsp_FLASH_Wait_Busy();   				   	//�ȴ�оƬ��������
}   
//����һ������
//Dst_Addr:������ַ ����ʵ����������
//����һ��ɽ��������ʱ��:150ms
void bsp_FLASH_Erase_Sector(u32 Dst_Addr)   
{  
	//����falsh�������,������    	
 	Dst_Addr *= 4096;
    bsp_FLASH_Write_Enable();                  //SET WEL 	 
    bsp_FLASH_Wait_Busy();   
  	FLASH_CS=0;                            //ʹ������   
    SPI1_ReadWriteByte(FLASH_SectorErase);  	//������������ָ�� 
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>16));  //����24bit��ַ    
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI1_ReadWriteByte((u8)Dst_Addr);  
	FLASH_CS=1;                       		//ȡ��Ƭѡ     	      
    bsp_FLASH_Wait_Busy();   				   	//�ȴ��������
}  
//�ȴ�����
void bsp_FLASH_Wait_Busy(void)   
{   
	while((bsp_FLASH_ReadSR()&0x01)==0x01);   // �ȴ�BUSYλ���
}  
//�������ģʽ
void bsp_FLASH_PowerDown(void)   
{ 
  	FLASH_CS=0;                            //ʹ������   
    SPI1_ReadWriteByte(FLASH_PowerDown);     //���͵�������  
	FLASH_CS=1;                            //ȡ��Ƭѡ     	      
    delay_us(3);                            //�ȴ�TPD  
}   
//����
void bsp_FLASH_WAKEUP(void)   
{  
  	FLASH_CS=0;                            	//ʹ������   
    SPI1_ReadWriteByte(FLASH_ReleasePowerDown);	//  send FLASH_PowerDown command 0xAB    
	FLASH_CS=1;                            	//ȡ��Ƭѡ     	      
    delay_us(3);                               	//�ȴ�TRES1
}   





