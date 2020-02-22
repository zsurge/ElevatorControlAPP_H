#ifndef __MQTTCLIENT_H
#define __MQTTCLIENT_H
#include "stm32f4xx.h"

enum QoS { QOS0, QOS1, QOS2 };
#define MSG_MAX_LEN    300
#define MSG_TOPIC_LEN  50

#define  KEEPLIVE_TIME 50

//数据交互结构体
typedef struct __MQTTMessage
{
    u32 qos;
    u8 retained;
    u8 dup;
    u16 id;
	  u8 type;
    void *payload;
    s32 payloadlen;
}MQTTMessage;

//用户接收消息结构体
typedef struct __MQTT_MSG
{
	  u8  msgqos;                 //消息质量
		u8  msg[MSG_MAX_LEN];       //消息
	  u32 msglenth;               //消息长度
	  u8  topic[MSG_TOPIC_LEN];   //主题    
	  u16 packetid;               //消息ID
	  u8  valid;                  //标明消息是否有效
}MQTT_USER_MSG;

//发送消息结构体
typedef struct
{
    s8 topic[MSG_TOPIC_LEN];
    s8 qos;
    s8 retained;

    u8 msg[MSG_MAX_LEN];
    u8 msglen;
} mqtt_recv_msg_t, *p_mqtt_recv_msg_t, mqtt_send_msg_t, *p_mqtt_send_msg_t;



void mqtt_thread( void *pvParameters);

/************************************************************************
** 函数名称: my_mqtt_send_pingreq								
** 函数功能: 发送MQTT心跳包
** 入口参数: 无
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注: 
************************************************************************/
s32 my_mqtt_send_pingreq(s32 sock);

/************************************************************************
** 函数名称: MQTTClientInit								
** 函数功能: 初始化客户端并登录服务器
** 入口参数: s32 sock:网络描述符
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注: 
************************************************************************/
s32 MQTTClientInit(s32 sock);

/************************************************************************
** 函数名称: MQTTSubscribe								
** 函数功能: 订阅消息
** 入口参数: s32 sock：套接字
**           s8 *topic：主题
**           enum QoS pos：消息质量
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注: 
************************************************************************/
s32 MQTTSubscribe(s32 sock,s8 *topic,enum QoS pos);

/************************************************************************
** 函数名称: UserMsgCtl						
** 函数功能: 用户消息处理函数
** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
** 出口参数: 无
** 备    注: 
************************************************************************/
void UserMsgCtl(MQTT_USER_MSG  *msg);

/************************************************************************
** 函数名称: GetNextPackID						
** 函数功能: 产生下一个数据包ID
** 入口参数: 无
** 出口参数: u16 packetid:产生的ID
** 备    注: 
************************************************************************/
u16 GetNextPackID(void);

/************************************************************************
** 函数名称: mqtt_msg_publish						
** 函数功能: 用户推送消息
** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注: 
************************************************************************/
s32 MQTTMsgPublish(s32 sock, s8 *topic, s8 qos, s8 retained,u8* msg,u32 msg_len);

/************************************************************************
** 函数名称: ReadPacketTimeout					
** 函数功能: 阻塞读取MQTT数据
** 入口参数: s32 sock:网络描述符
**           u8 *buf:数据缓存区
**           s32 buflen:缓冲区大小
**           u32 timeout:超时时间--0-表示直接查询，没有数据立即返回
** 出口参数: -1：错误,其他--包类型
** 备    注: 
************************************************************************/
s32 ReadPacketTimeout(s32 sock,u8 *buf,s32 buflen,u32 timeout);

/************************************************************************
** 函数名称: mqtt_pktype_ctl						
** 函数功能: 根据包类型进行处理
** 入口参数: u8 packtype:包类型
** 出口参数: 无
** 备    注: 
************************************************************************/
void mqtt_pktype_ctl(u8 packtype,u8 *buf,u32 buflen);

/************************************************************************
** 函数名称: WaitForPacket					
** 函数功能: 等待特定的数据包
** 入口参数: s32 sock:网络描述符
**           u8 packettype:包类型
**           u8 times:等待次数
** 出口参数: >=0:等到了特定的包 <0:没有等到特定的包
** 备    注: 
************************************************************************/
s32 WaitForPacket(s32 sock,u8 packettype,u8 times);

#endif



