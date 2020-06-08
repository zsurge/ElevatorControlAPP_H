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
#include "bsp_usart6.h"
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"

uint8_t gUsart6RXBuff[COM6_RXBUFFER_SIZE];//���ջ�����

//����1����DMA����
RingBuff_t ringbuff_handle;


uint8_t Uart6_Tx[UART6_TX_LEN] = {0};               //����1����DMA����     
uint8_t Uart6_Rx[UART6_RX_LEN] = {0};               //����1����DMA���� 

static void BSP_DMAUsar6Rx_Init(void);
static void BSP_DMAUsar6Tx_Init(void);

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

	RCC_AHB1PeriphClockCmd(USART1_GPIO_CLK,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(USART1_CLK,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(USART1_GPIO_PORT,USART1_TX_SOURCE,USART1_TX_AF); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(USART1_GPIO_PORT,USART1_RX_SOURCE,USART1_RX_AF); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN | USART1_RX_PIN; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(USART1_GPIO_PORT,&GPIO_InitStructure); //��ʼ��PA9��PA10

    //DE1
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	/* ��GPIOʱ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* ���������� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO������ٶ� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);   		

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = BaudRate;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART6, &USART_InitStructure); //��ʼ������1    


    //����USART6
    USART_Cmd(USART6, ENABLE);
    

	USART_ClearFlag(USART6, USART_FLAG_TC); //���������ɱ�־	
	while(USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);	//�ȴ�����֡������ɺ������㷢����ɱ�־
	USART_ClearFlag(USART6, USART_FLAG_TC);	//���������ɱ�־
    
    //����USART6���߿����ж�
	USART_ITConfig(USART6,USART_IT_TC,DISABLE);  
	USART_ITConfig(USART6,USART_IT_RXNE,DISABLE); 
	USART_ITConfig(USART6,USART_IT_TXE,DISABLE); 
	USART_ITConfig(USART6,USART_IT_IDLE,ENABLE); 



	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =4;		//�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���


	
    //���ڽ���DMA����
    BSP_DMAUsar6Rx_Init();
    //���ڷ���DMA����
    BSP_DMAUsar6Tx_Init();

    USART_DMACmd(USART6,USART_DMAReq_Tx,ENABLE);  //����DMA��ʽ����   
    USART_DMACmd(USART6,USART_DMAReq_Rx,ENABLE); //����DMA��ʽ����

    
    Create_RingBuff(&ringbuff_handle,  gUsart6RXBuff,sizeof(gUsart6RXBuff));
}

void USART6_IRQHandler(void)
{    
    uint32_t rxBuffLen = 0;
    uint32_t ulReturn;
    /* �����ٽ�Σ��ٽ�ο���Ƕ�� */
    ulReturn = taskENTER_CRITICAL_FROM_ISR();
    
    if(USART_GetITStatus(USART6, USART_IT_IDLE) != RESET)  
    {
      //1.���USART6��������ж�
        USART6->SR;  
        USART6->DR;   //��USART_IT_IDLE��־  
      //2.�洢�յ����������ݡ����ȡ���־λ
        DMA_Cmd(DMA2_Stream1,DISABLE); //ʹ��������1 ͨ��5
        
        DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);//�����־λ
    
        rxBuffLen = UART6_RX_LEN - DMA_GetCurrDataCounter(DMA2_Stream1);        //����������ݰ�����      
        
        Write_RingBuff(&ringbuff_handle,Uart6_Rx,rxBuffLen);  
        
        DMA_SetCurrDataCounter(DMA2_Stream1,UART6_RX_LEN);                      //���ô������ݳ���
        DMA_Cmd(DMA2_Stream1,ENABLE);                                           //��DMA 
    } 

//    if(USART_GetITStatus(USART6, USART_IT_TC) != RESET)
//    {
//        USART_ClearFlag(USART6,USART_IT_TC);
//        
//        //�رշ�������ж�
//        USART_ITConfig(USART6,USART_IT_TC,DISABLE);
//        
//        RS485_U6_RX_EN();
//    }

    if(USART_GetITStatus(USART6, USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(USART6, USART_IT_TC);   //���USART1��������жϱ�־
        DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | 
                      DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6);//�����־λ 
                      
        RS485_U6_RX_EN();       //�л�ΪRS485����ģʽ 
    }    

  /* �˳��ٽ�� */
  taskEXIT_CRITICAL_FROM_ISR( ulReturn );	    

}

//USART1����DMA����
static void BSP_DMAUsar6Rx_Init(void)
{
    DMA_InitTypeDef   DMA_InitStructure;
    u16 mid_u16RetryCnt = 0;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);                        //����DMAʱ��  
    
    DMA_DeInit(DMA2_Stream1);
    while ((DMA_GetCmdStatus(DMA2_Stream1) != DISABLE) && (mid_u16RetryCnt++ < 500));
    
    DMA_InitStructure.DMA_Channel = DMA_Channel_5;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART6->DR);         //�����ַ      
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Uart6_Rx;                 //�ڴ��ַ      
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                     //dma���䷽����      
    DMA_InitStructure.DMA_BufferSize = UART6_RX_LEN;                            //����DMA�ڴ���ʱ�������ĳ���     
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //����DMA���������ģʽ��һ������        
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //����DMA���ڴ����ģʽ      
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     //���������ֳ�         
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;             //�ڴ������ֳ�      
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               //����DMA�Ĵ���ģʽ      
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                     //����DMA�����ȼ���     
    
 	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
 	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
 	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
 	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    
    DMA_Init(DMA2_Stream1,&DMA_InitStructure);	  
    DMA_Cmd(DMA2_Stream1,ENABLE);                                               //ʹ��������1 ͨ��5
}
 
//USART6����DMA����
static void BSP_DMAUsar6Tx_Init(void)
{
    DMA_InitTypeDef   DMA_InitStructure;  
    NVIC_InitTypeDef NVIC_InitStructure;
    u16 mid_u16RetryCnt = 0;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);                        //����DMAʱ��  
    
    DMA_DeInit(DMA2_Stream6);
    while ((DMA_GetCmdStatus(DMA2_Stream6) != DISABLE) && (mid_u16RetryCnt++ < 500));
    
    DMA_InitStructure.DMA_Channel = DMA_Channel_5;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART6->DR);         //DMA�������ַ
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Uart6_Tx;                 //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                     //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
	DMA_InitStructure.DMA_BufferSize = UART6_TX_LEN;                            //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;             //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               //����������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                       //DMAͨ�� xӵ�������ȼ� 
    
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;    
	


    //DMA�����ж�����
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    DMA_ITConfig(DMA2_Stream6,DMA_IT_TC,ENABLE);    
    
	DMA_Init(DMA2_Stream6, &DMA_InitStructure);                                 //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART6_Tx_DMA_Channel����ʶ�ļĴ���   
    DMA_Cmd(DMA2_Stream6,DISABLE);
}
//USART2 DMA����ָ�����ȵ�����
//str��Ҫ���͵������׵�ַ
//cndtr:���ݴ����� 
void BSP_DMAUsart6Send(uint8_t *str,uint8_t cndtr)
{
//    RS485_U6_TX_EN();
//    memcpy(Uart6_Tx, str, cndtr);
//    USART_DMACmd(USART6,USART_DMAReq_Tx,ENABLE);                                //ʹ�ܴ���6��DMA����
//	DMA_Cmd(DMA2_Stream6, DISABLE );                                            //�ر�USART6 TX DMA2 ��ָʾ��ͨ��      
// 	DMA_SetCurrDataCounter(DMA2_Stream6,cndtr);                                 //DMAͨ����DMA����Ĵ�С
// 	DMA_Cmd(DMA2_Stream6, ENABLE);

    u16 l_u16RetryCnt = 0;    
    RS485_U6_TX_EN();
    memcpy(Uart6_Tx, str, cndtr);
    DMA_Cmd(DMA2_Stream6, DISABLE);                      //�ر�DMA����           
    while ((DMA_GetCmdStatus(DMA2_Stream6) != DISABLE) && (l_u16RetryCnt++ < 500));	//�ȴ�DMA������	
    DMA_SetCurrDataCounter(DMA2_Stream6, cndtr);  //���ݴ����� 	 
    DMA_Cmd(DMA2_Stream6, ENABLE);                      	//����DMA����    	
}

//DMA2�жϺ�����
void DMA2_Stream6_IRQHandler(void)
{
//    if(DMA_GetITStatus(DMA2_Stream6,DMA_IT_TCIF6) != RESET) 
//	{
//		//�����־λ
//		DMA_ClearFlag(DMA2_Stream6,DMA_FLAG_TCIF6);
//		//�ر�DMA
//		DMA_Cmd(DMA2_Stream6,DISABLE);
//		//�򿪷�������ж�,������������ֽ�
//		USART_ITConfig(USART6,USART_IT_TC,ENABLE);
//		
//		RS485_U6_RX_EN();		//�л�ΪRS485����ģʽ				
//	}

    if(DMA_GetFlagStatus(DMA2_Stream6, DMA_FLAG_TCIF6) != RESET)	//DMA�������  
    {   
		DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | 
					  DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6); 	//�����־λ			
		
		while(!USART_GetFlagStatus(USART6, USART_FLAG_TC));	//�ȴ�USART1������ɱ�־TC��1
		USART_ClearFlag(USART6, USART_FLAG_TC); 	//���������ɱ�־

		DMA_Cmd(DMA2_Stream6,DISABLE);
        USART_ITConfig(USART6,USART_IT_TC,ENABLE);		 
		RS485_U6_RX_EN();		//�л�ΪRS485����ģʽ		
    } 

}


uint16_t BSP_DMAUsart6Read(uint8_t *buffter, uint32_t len)
{
    return Read_RingBuff(&ringbuff_handle, buffter, len);
}




