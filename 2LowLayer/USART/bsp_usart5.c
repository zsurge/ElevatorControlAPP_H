/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_Usart3.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��22��
  ����޸�   :
  ��������   : ����3 IAP����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

#include "bsp_usart5.h"
#include "bsp_time.h"

volatile uint8_t USART5RecvBuf[USART5MAXBUFFSIZE] = {0};
volatile uint16_t RecvTop5 = 0;
volatile uint16_t RecvEnd5 = 0;





/*****************************************************************************
 �� �� ��  : bsp_Usart5Init
 ��������  : ���ڳ�ʼ��
 �������  : uint32_t BaudRate  ������
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart3Init (uint32_t BaudRate);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart5_Init (uint32_t BaudRate)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


	RCC_APB1PeriphClockCmd(USART5_CLK,ENABLE); //ʹ��GPIOAʱ��
	RCC_AHB1PeriphClockCmd(USART5_GPIO_RX_CLK|USART5_GPIO_TX_CLK,ENABLE);//ʹ��Usart5ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(USART5_GPIO_TX_PORT,USART5_TX_SOURCE,USART5_TX_AF); //GPIOB10����ΪUSART3
	GPIO_PinAFConfig(USART5_GPIO_RX_PORT,USART5_RX_SOURCE,USART5_RX_AF); //GPIOB11����ΪUSART3
	
	//Usart2�˿�����
	GPIO_InitStructure.GPIO_Pin = USART5_RX_PIN; //GPIOB10��GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(USART5_GPIO_RX_PORT,&GPIO_InitStructure); //��ʼ��PB10��PB11

	GPIO_InitStructure.GPIO_Pin = USART5_TX_PIN; //GPIOB10��GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(USART5_GPIO_TX_PORT,&GPIO_InitStructure); //��ʼ��PB10��PB11


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* ���������� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO������ٶ� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD, &GPIO_InitStructure);   


   //USART3 ��ʼ������
	USART_InitStructure.USART_BaudRate = BaudRate;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(UART5, &USART_InitStructure); //��ʼ������2    

	//USART3 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;//����3�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//��������ж�    
	
	USART_Cmd(UART5, ENABLE);  //ʹ�ܴ���1   


	RS485_U5_RX_EN();

}

void UART5_IRQHandler (void)
{
	uint8_t temp = 0;

	//Receive data register not empty flag
	if (USART_GetITStatus (UART5, USART_IT_RXNE) != RESET)
	{
		temp = USART_ReceiveData (UART5);		//get received data

		if ( (RecvEnd5 == (RecvTop5 - 1) ) || ( (RecvTop5 == 0) && (RecvEnd5 == (USART5MAXBUFFSIZE - 1) ) ) )
		{
			//�������
			RecvTop5 = 0;
			RecvEnd5 = 0;
		}
		else
		{
			USART5RecvBuf[RecvEnd5] = temp;
			RecvEnd5++;

			if (RecvEnd5 >= USART5MAXBUFFSIZE) RecvEnd5 = 0;
		}

		/* Clear the UART1 Recvive interrupt */
		USART_ClearITPendingBit (UART5, USART_IT_RXNE);
	}
	else if (USART_GetITStatus (UART5, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE) != RESET)
	{
		USART_GetITStatus (UART5, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE);
		USART_ReceiveData (UART5);
	}
}


void bsp_Usart5_SendOne (const uint8_t dat)
{
    RS485_U5_TX_EN();
    
	USART_SendData (UART5, dat);

	while (USART_GetFlagStatus (UART5, USART_FLAG_TXE) == RESET) {}

	USART_ClearFlag (UART5, USART_FLAG_TXE);

    RS485_U5_RX_EN();
}



/*****************************************************************************
 �� �� ��  : bsp_Usart3SendString
 ��������  : �����ַ���
 �������  : const uint8_t *Buff  �����ַ���
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart3SendString (const uint8_t *Buff);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart5_SendString (const uint8_t *Buff)
{
	while (*Buff != 0)
	{
		bsp_Usart5_SendOne (*Buff);
		Buff++;
	}    
}


/*****************************************************************************
 �� �� ��  : bsp_Usart3SendData
 ��������  : ����ָ�������ַ�
 �������  : const uint8_t *Buff  �������ݵ�buff
             uint16_t SendSize  �������ݵĳ���  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart3SendData (const uint8_t *Buff, uint16_t SendSize);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart5_SendData (const uint8_t *Buff, uint16_t SendSize)
{
	while (SendSize != 0)
	{
		bsp_Usart5_SendOne (*Buff);
		Buff++;
		SendSize--;
	}

}



/*****************************************************************************
 �� �� ��  : bsp_Usart3RecvReset
 ��������  : ��մ��ڽ��ջ���
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart3RecvReset (void);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart5_RecvReset (void)
{
	RecvTop5 = 0;
	RecvEnd5 = 0;

	memset((void *)USART5RecvBuf, 0, USART5MAXBUFFSIZE); 
}



/*****************************************************************************
 �� �� ��  : bsp_Usart3RecvOne
 ��������  : ��ȡ1���ֽ�
 �������  : uint8_t *Str  �������ֽڴ洢��Str[0]
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
uint8_t bsp_Usart3RecvOne (uint8_t *Str);
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t bsp_Usart5_RecvOne (uint8_t *Str)
{
	if (RecvTop5 == RecvEnd5) return 0;//read nothing

	*Str = USART5RecvBuf[RecvTop5];
	RecvTop5++;

	if (RecvTop5 >= USART5MAXBUFFSIZE) RecvTop5 = 0;

	return 1;//read one

}




/*****************************************************************************
 �� �� ��  : dev_Usart2Read
 ��������  : ����������
 �������  : unsigned char *buf  
             int len             
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
int dev_Usart2Read(unsigned char *buf, int len)
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t bsp_Usart5_Read(uint8_t *Buff, uint16_t len)
{

	uint16_t RecvLen = 0;
	uint8_t tmp[1] = {0};

	if (len == 0 || Buff == NULL) return 0;

	while (len--)
	{
		if (bsp_Usart5_RecvOne (tmp) == 1)
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= len) return RecvLen;
	}

	return RecvLen;
    
}



