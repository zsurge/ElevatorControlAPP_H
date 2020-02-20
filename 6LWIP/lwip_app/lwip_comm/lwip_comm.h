#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H 

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//lwip带UCOS系统的通用驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	

//lwip控制结构体
typedef struct  
{
	uint8_t mac[6];      //MAC地址
	uint8_t remoteip[4];	//远端主机IP地址 
	uint8_t ip[4];       //本机IP地址
	uint8_t netmask[4]; 	//子网掩码
	uint8_t gateway[4]; 	//默认网关的IP地址
	
	volatile uint8_t dhcpstatus;	//dhcp状态 
                                //0,未获取DHCP地址;
                                //1,成功获取DHCP地址
                                //0XFF,获取失败.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip控制结构体

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













