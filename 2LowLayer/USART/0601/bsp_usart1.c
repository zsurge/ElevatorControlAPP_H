/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_USART6.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��22��
  ����޸�   :
  ��������   : ����1��ʼ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#include "stm32f4xx_conf.h"
#include "bsp_usart1.h"


volatile uint8_t USART6RecvBuf[USART1MAXBUFFSIZE] = {0};
volatile uint8_t USART6SendBuf[USART1MAXBUFFSIZE] = {0};

volatile uint16_t RecvTop6 = 0;
volatile uint16_t RecvEnd6 = 0;


u16 UART6_ReceiveSize = 0;



static void Usart6_DMA2_TX_Config(void);
static void Usart6_DMA2_RX_Config(void);



/*****************************************************************************
 �� �� ��  : bsp_Usart1Init
 ��������  : ���ڳ�ʼ��
 �������  : uint32_t BaudRate  ������
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart1Init (uint32_t BaudRate);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart6_Init (uint32_t BaudRate)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


    USART_DeInit(USART6);
    
    //ʹ��GPIOAʱ��
	RCC_AHB1PeriphClockCmd(USART6_GPIO_CLK,ENABLE); 

	//ʹ��USART1ʱ��
	RCC_APB2PeriphClockCmd(USART6_CLK,ENABLE);
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(USART6_GPIO_PORT,USART6_TX_SOURCE,USART6_TX_AF); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(USART6_GPIO_PORT,USART6_RX_SOURCE,USART6_RX_AF); //GPIOA10����ΪUSART1
	
	//USART6�˿�����
	GPIO_InitStructure.GPIO_Pin = USART6_TX_PIN | USART6_RX_PIN; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(USART6_GPIO_PORT,&GPIO_InitStructure); //��ʼ��PA9��PA10

    //DE6
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	/* ��GPIOʱ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* ���������� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO������ٶ� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);   		

   //USART6 ��ʼ������
	USART_InitStructure.USART_BaudRate = BaudRate;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART6, &USART_InitStructure); //��ʼ������1    


	//Usart6 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    //modify 2020.05.29	
//	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);//��������ж�
//	USART_Cmd(USART6, ENABLE);  //ʹ�ܴ���1  
//	USART_ClearFlag(USART6, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */

    USART_ITConfig(USART6, USART_IT_RXNE, DISABLE);
    USART_ITConfig(USART6, USART_IT_TC,   DISABLE);
    USART_ITConfig(USART6, USART_IT_TXE,  DISABLE);  

    
    USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);//���������ж� 
    
    USART_DMACmd(USART6, USART_DMAReq_Tx, ENABLE);  
    USART_DMACmd(USART6, USART_DMAReq_Rx, ENABLE);
    
    Usart6_DMA2_TX_Config();   
    Usart6_DMA2_RX_Config();


    USART_Cmd(USART6, ENABLE);//ʹ�ܴ���6
    RS485_U6_RX_EN();
}


static void Usart6_DMA2_TX_Config(void)  
{  
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2ʱ��ʹ�� 
    
    //reset 
    DMA_DeInit(DMA2_Stream6); //���ʹ�ô���6�� DMA2 ͨ��5��������6

    while(DMA_GetCmdStatus(DMA2_Stream6) != DISABLE);//�ȴ�DMA������ 
    
    
    //DMA configuration  
    DMA_InitStructure.DMA_Channel = DMA_Channel_5; //ͨ��ѡ��
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART6->DR; //DMA�����ַ
    DMA_InitStructure.DMA_BufferSize = USART1MAXBUFFSIZE;//1;   //���ݴ���������ʼ��ʱ�ȶ�Ϊ1
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART6SendBuf; //DMA �洢��0��ַ     
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; //�洢��������ģʽ    
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//���������ģʽ   
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�洢������ģʽ 
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //�������ݳ���:8λ 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //�洢�����ݳ���:8λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;  // ʹ����ͨģʽ    
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //�е����ȼ�    
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;  //�洢��ͻ�����δ���
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; //����ͻ�����δ���
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold =DMA_FIFOThreshold_Full;
    /* 3. ����DMA */
    DMA_Init(DMA2_Stream6, &DMA_InitStructure);

    /* 6. ����DMA�ж����ȼ� */
    NVIC_InitStructure.NVIC_IRQChannel                   = DMA2_Stream6_IRQn;           
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;          
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);  

    DMA_Cmd(DMA2_Stream6,ENABLE);
     
//    DMA_ITConfig(DMA2_Stream6,DMA_IT_TC,ENABLE);
}

static void Usart6_DMA2_RX_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1.ʹ��DMA2ʱ�� */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    /* 2.����ʹ��DMA�������� */
    DMA_DeInit(DMA2_Stream1); 

    while(DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);//�ȴ�DMA������  

    DMA_InitStructure.DMA_Channel             = DMA_Channel_5;               /* ����DMAͨ�� */
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)(&(USART6->DR));   /* Դ */
    DMA_InitStructure.DMA_Memory0BaseAddr     = (uint32_t)USART6RecvBuf;             /* Ŀ�� */
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory;    /* ���� */
    DMA_InitStructure.DMA_BufferSize          = USART1MAXBUFFSIZE;                    /* ���� */                  
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;    /* �����ַ�Ƿ����� */
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;         /* �ڴ��ַ�Ƿ����� */
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_MemoryDataSize_Byte;      /* Ŀ�����ݴ��� */
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;      /* Դ���ݿ�� */
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;              /* ���δ���ģʽ/ѭ������ģʽ */
    DMA_InitStructure.DMA_Priority            = DMA_Priority_VeryHigh;        /* DMA���ȼ� */
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;          /* FIFOģʽ/ֱ��ģʽ */
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; /* FIFO��С */
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;       /* ���δ��� */
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single;

    /* 3. ����DMA */
    DMA_Init(DMA2_Stream1, &DMA_InitStructure);

    //DMA NVIC  
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  

    // ���DMA���б�־
    DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1);

    DMA_ITConfig(DMA2_Stream1,DMA_IT_TC, ENABLE);
    
    DMA_Cmd(DMA2_Stream1, ENABLE);  //����DMA���� 

}



//����һ��DMA����
//DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:���ݴ�����  
void DmaSendDataProc(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
{
    RS485_U6_TX_EN();
    
	DMA_Cmd(DMA_Streamx, DISABLE);                      //�ر�DMA���� 
	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//ȷ��DMA���Ա�����  
		
	DMA_SetCurrDataCounter(DMA_Streamx,ndtr);          //���ݴ�����  
 
	DMA_Cmd(DMA_Streamx, ENABLE);                      //����DMA���� 
}

///////////////////////////////////////////////////////////////////////////
//����1
//���͵��ֽ�
 void SendByteInfoProc(u8 nSendInfo)
{
	u8 *pBuf = NULL;
	//ָ���ͻ�����
	pBuf = USART6SendBuf;
	*pBuf++ = nSendInfo;
 
	DmaSendDataProc(DMA2_Stream6,1); //��ʼһ��DMA���䣡	  
}
//���Ͷ��ֽ�
void SendBytesInfoProc(u8* pSendInfo, u16 nSendCount)
{
	u16 i = 0;
	u8 *pBuf = NULL;
	//ָ���ͻ�����
	pBuf = USART6SendBuf;
 
	for (i=0; i<nSendCount; i++)
	{
		*pBuf++ = pSendInfo[i];
	}
	//DMA���ͷ�ʽ
	DmaSendDataProc(DMA2_Stream6,nSendCount); //��ʼһ��DMA���䣡	  
}


//��������ж�
void DMA2_Stream6_IRQHandler(void)
{
	//�����־
	if(DMA_GetFlagStatus(DMA2_Stream6,DMA_FLAG_TCIF6)!=RESET)//�ȴ�DMA2_Steam7�������
	{ 
		DMA_ClearFlag(DMA2_Stream6,DMA_FLAG_TCIF6);//���DMA2_Steam7������ɱ�־
		RS485_U6_RX_EN();
	}
}


//��������ж�
void DMA2_Stream1_IRQHandler(void)
{
	//�����־
	if(DMA_GetFlagStatus(DMA2_Stream1,DMA_FLAG_TCIF1)!=RESET)//�ȴ�DMA2_Steam7�������
	{ 
		DMA_Cmd(DMA2_Stream1, DISABLE); //�ر�DMA,��ֹ�������������

 
		UART6_ReceiveSize =USART1MAXBUFFSIZE - DMA_GetCurrDataCounter(DMA2_Stream1);
		if(UART6_ReceiveSize !=0)
		{
//			OSSemPost((DMAReceiveSize_Sem);
		}
		
		DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);//���DMA2_Steam7������ɱ�־
		DMA_SetCurrDataCounter(DMA2_Stream1, USART1MAXBUFFSIZE);
		DMA_Cmd(DMA2_Stream1, ENABLE);     //��DMA,
	}
}


void USART6_IRQHandler (void)
{

    u16 data;
	if(USART_GetITStatus(USART6,USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(DMA2_Stream1, DISABLE); //�ر�DMA,��ֹ�������������
 
		data = USART6->SR;
		data = USART6->DR;

		printf("UART6_ReceiveSize = %d\r\n",UART6_ReceiveSize);
		UART6_ReceiveSize =USART1MAXBUFFSIZE - DMA_GetCurrDataCounter(DMA2_Stream1);
		if(UART6_ReceiveSize !=0)
		{
//			OSSemPost(DMAReceiveSize_Sem);
		}
		DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);//���DMA2_Steam7������ɱ�־
		DMA_SetCurrDataCounter(DMA2_Stream1, USART1MAXBUFFSIZE);
		DMA_Cmd(DMA2_Stream1, ENABLE);     //��DMA,
 
	}




//	uint8_t temp = 0;

//    if (USART_GetFlagStatus(USART6, USART_FLAG_PE) != RESET)
//    {
//        USART_ClearFlag(USART6, USART_FLAG_PE);    
//        USART_ReceiveData(USART6);
//    }

//    if (USART_GetFlagStatus(USART6, USART_FLAG_ORE) != RESET)
//    {
//        USART_ClearFlag(USART6, USART_FLAG_ORE);    
//        USART_ReceiveData(USART6);
//    }
//     
//     if (USART_GetFlagStatus(USART6, USART_FLAG_FE) != RESET)
//    {
//        USART_ClearFlag(USART6, USART_FLAG_FE);    
//        USART_ReceiveData(USART6);
//    }	


//    //Receive data register not empty flag
//	if (USART_GetITStatus (USART6, USART_IT_RXNE) != RESET)
//	{

//        USART_ClearFlag(USART6, USART_FLAG_RXNE);
//		/* Clear the UART1 Recvive interrupt */
//		USART_ClearITPendingBit (USART6, USART_IT_RXNE);
//		
//		temp = USART_ReceiveData (USART6);		//get received data

//		if ( (RecvEnd6 == (RecvTop6 - 1) ) || ( (RecvTop6 == 0) && (RecvEnd6 == (USART1MAXBUFFSIZE - 1) ) ) )
//		{
//			//�������
//			RecvTop6 = 0;
//			RecvEnd6 = 0;
//		}
//		else
//		{
//			USART6RecvBuf[RecvEnd6] = temp;
//			RecvEnd6++;

//			if (RecvEnd6 >= USART1MAXBUFFSIZE) RecvEnd6 = 0;
//		}		
//	}



//	if (USART_GetITStatus (USART1, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE) != RESET)
//	{
//		USART_GetITStatus (USART1, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE);
//		USART_ReceiveData (USART1);
//	}
}


void bsp_Usart6_SendOne (const uint8_t dat)
{	
    while (USART_GetFlagStatus (USART6, USART_FLAG_TXE) == RESET) {}
    USART_SendData (USART6, dat);
    while (USART_GetFlagStatus (USART6, USART_FLAG_TC) == RESET) {}
}



/*****************************************************************************
 �� �� ��  : bsp_Usart1SendString
 ��������  : �����ַ���
 �������  : const uint8_t *Buff  �����ַ���
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart1SendString (const uint8_t *Buff);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart6_SendString (const uint8_t *Buff)
{
	while (*Buff != 0)
	{
		bsp_Usart6_SendOne (*Buff);
		Buff++;
	}    
}


/*****************************************************************************
 �� �� ��  : bsp_Usart1SendData
 ��������  : ����ָ�������ַ�
 �������  : const uint8_t *Buff  �������ݵ�buff
             uint16_t SendSize  �������ݵĳ���  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart1SendData (const uint8_t *Buff, uint16_t SendSize);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart6_SendData (const uint8_t *Buff, uint16_t SendSize)
{

    RS485_U6_TX_EN();
	while (SendSize != 0)
	{
		bsp_Usart6_SendOne (*Buff);
		Buff++;
		SendSize--;
	}

	RS485_U6_RX_EN();

}



/*****************************************************************************
 �� �� ��  : bsp_Usart1RecvReset
 ��������  : ��մ��ڽ��ջ���
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart1RecvReset (void);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart6_RecvReset (void)
{
	RecvTop6 = 0;
	RecvEnd6 = 0;

	memset((void *)USART6RecvBuf, 0, USART1MAXBUFFSIZE); 
}



/*****************************************************************************
 �� �� ��  : bsp_Usart1RecvOne
 ��������  : ��ȡ1���ֽ�
 �������  : uint8_t *Str  �������ֽڴ洢��Str[0]
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
uint8_t bsp_Usart1RecvOne (uint8_t *Str);
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t bsp_Usart6_RecvOne (uint8_t *Str)
{
	if (RecvTop6 == RecvEnd6) return 0;//read nothing

	*Str = USART6RecvBuf[RecvTop6];
	RecvTop6++;

	if (RecvTop6 >= USART1MAXBUFFSIZE) RecvTop6 = 0;

	return 1;//read one

}



/*****************************************************************************
 �� �� ��  : dev_Usart1Read
 ��������  : ����������
 �������  : unsigned char *buf  
             int len             
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
int dev_Usart1Read(unsigned char *buf, int len)
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t bsp_Usart6_Read(uint8_t *Buff, uint16_t len)
{

	uint16_t RecvLen = 0;
	uint8_t tmp[1] = {0};

	if (len == 0 || Buff == NULL) return 0;

	while (len--)
	{
		if (bsp_Usart6_RecvOne (tmp) == 1)
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= len) return RecvLen;
	}

	return RecvLen;
    
}






