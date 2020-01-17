#include "bsp_led.h" 
#include "string.h"
 

//��ʼ��PF9��PF10Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void bsp_LED_Init(void)
{    	 
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_ALL_LED, ENABLE);//ʹ��GPIOFʱ��
    
      //GPIOFerr
      GPIO_InitStructure.GPIO_Pin = GPIO_PIN_ERRORLED|GPIO_PIN_LED1|GPIO_PIN_LED2 ;   //LED1��ӦIO��
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //��ͨ���ģʽ
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //�������
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //100MHz
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
      GPIO_Init(GPIO_PORT_LED012, &GPIO_InitStructure);         //��ʼ��GPIO
    
      //GPIOF3
      GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED3 ; //LED2��LED3��ӦIO��
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //��ͨ���ģʽ
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //�������
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //100MHz
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
      GPIO_Init(GPIO_PORT_LED3, &GPIO_InitStructure);            //��ʼ��GPIO
    
      //GPIOF4
      GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED4 ;     
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //��ͨ���ģʽ
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //�������
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //100MHz
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            //����
      GPIO_Init(GPIO_PORT_LED4, &GPIO_InitStructure);         //��ʼ��GPIO        
    
    
      GPIO_SetBits(GPIO_PORT_LED3,GPIO_PIN_LED3 );            
      GPIO_SetBits(GPIO_PORT_LED012,GPIO_PIN_LED1|GPIO_PIN_LED2|GPIO_PIN_ERRORLED);
      GPIO_SetBits(GPIO_PORT_LED4,GPIO_PIN_LED4);




}


