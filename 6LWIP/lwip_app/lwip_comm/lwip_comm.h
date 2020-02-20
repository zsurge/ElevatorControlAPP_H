#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H 

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//lwip��UCOSϵͳ��ͨ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	

//lwip���ƽṹ��
typedef struct  
{
	uint8_t mac[6];      //MAC��ַ
	uint8_t remoteip[4];	//Զ������IP��ַ 
	uint8_t ip[4];       //����IP��ַ
	uint8_t netmask[4]; 	//��������
	uint8_t gateway[4]; 	//Ĭ�����ص�IP��ַ
	
	volatile uint8_t dhcpstatus;	//dhcp״̬ 
                                //0,δ��ȡDHCP��ַ;
                                //1,�ɹ���ȡDHCP��ַ
                                //0XFF,��ȡʧ��.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip���ƽṹ��

extern struct netif lwip_netif;


typedef enum
{ 
	Link_Down = 0,
	Link_Up = 1
}LinkState_TypeDef;


void lwip_pkt_handle(void);
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
uint8_t lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);
uint8_t lwip_comm_init(void);

void StartEthernet(void);
void SetGB_LinkState(LinkState_TypeDef linkstate);
LinkState_TypeDef GetGB_LinkState(void);


#endif













