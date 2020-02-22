#include "mqttclient.h"
#include "transport.h"
#include "MQTTPacket.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "sockets.h"

#define HOST_NAME "m2m.eclipse.org"//"192.168.110.109"
#define HOST_PORT 1883

//定义用户消息结构体
MQTT_USER_MSG  mqtt_user_msg;

void deliverMessage ( MQTTString* TopicName,MQTTMessage* msg,MQTT_USER_MSG* mqtt_user_msg );

/************************************************************************
** 函数名称: mqtt_thread
** 函数功能: MQTT任务
** 入口参数: void *pvParameters：任务参数
** 出口参数: 无
** 备    注:
************************************************************************/
void mqtt_thread ( void* pvParameters )
{
	s32 mysock = 0;
	u32 curtick,pubtick;
	u8 no_mqtt_msg_exchange = 1;
	s32 sessionPresent = 0;

	u8 buf[MSG_MAX_LEN];
	s32 buflen = sizeof ( buf ),type,ret;


	fd_set readfd;
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;



MQTT_START:
	//创建网络连接
	printf ( "MQTT>>1.开始创建网络连接...\n" );
	printf ( "服务器：%s,端口号：%0d\n",HOST_NAME,HOST_PORT );
	while ( 1 )
	{
		//连接服务器
		mysock = transport_open ( ( s8* ) HOST_NAME,HOST_PORT );
		//如果连接服务器成功
		if ( mysock >= 0 )
		{
			printf ( "MQTT>>连接服务器成功！\n" );
			break;
		}
		printf ( "MQTT>>连接服务器失败，休息3秒钟重新连接！\n" );
		//休息3秒
		vTaskDelay ( 3000/portTICK_RATE_MS );
	}

	//初始化MQTT客户端
	printf ( "MQTT>>2.初始化MQTT客户端,并连接客户端到服务器...\n" );
	//登录服务器
	sessionPresent = MQTTClientInit ( mysock );
	if ( sessionPresent < 0 )
	{
		printf ( "MQTT>>客户端连接服务器失败...\n" );
		goto MQTT_reconnect;
	}

	//订阅消息
	printf ( "MQTT>>3.开始订阅消息...\n" );
	if ( sessionPresent != 1 )
	{
		//订阅消息
		if ( MQTTSubscribe ( mysock, ( s8* ) "mymqttsubtest",QOS0 ) < 0 )
		{
			//重连服务器
			printf ( "MQTT>>客户端订阅消息失败...\n" );
			goto MQTT_reconnect;
		}
	}

	//获取当前滴答，作为心跳包起始时间
	curtick = xTaskGetTickCount();

	//无线循环
	printf ( "MQTT>>4.开始循环接收订阅的消息...\n" );
	while ( 1 )
	{
		//表明无数据交换
		no_mqtt_msg_exchange = 1;

		//推送消息
		FD_ZERO ( &readfd );
		FD_SET ( mysock,&readfd );

		//等待可读事件
		select ( mysock+1,&readfd,NULL,NULL,&tv );

		//判断MQTT服务器是否有数据
		if ( FD_ISSET ( mysock,&readfd ) != 0 )
		{
			//读取数据包--注意这里参数为0，不阻塞
			type = ReadPacketTimeout ( mysock,buf,buflen,0 );
			if ( type != -1 )
			{
				mqtt_pktype_ctl ( type,buf,buflen );
				//表明有数据交换
				no_mqtt_msg_exchange = 0;
				//获取当前滴答，作为心跳包起始时间
				curtick = xTaskGetTickCount();
			}
		}

		//每隔5秒就推送一次消息
		if ( ( xTaskGetTickCount() - pubtick ) > ( 5000 ) )
		{
			pubtick = xTaskGetTickCount();

			//发布消息
			ret = MQTTMsgPublish ( mysock, ( s8* ) "DongLuTest",0, 0, ( u8* ) "This is my first MQTT programme!",strlen ( "This is my first MQTT programme!" ) );
			if ( ret >= 0 )
			{
				//表明有数据交换
				no_mqtt_msg_exchange = 0;
				//获取当前滴答，作为心跳包起始时间
				curtick = xTaskGetTickCount();
			}

		}

		//这里主要目的是定时向服务器发送PING保活命令
		if ( ( xTaskGetTickCount() - curtick ) > ( KEEPLIVE_TIME/2*1000 ) )
		{
			curtick = xTaskGetTickCount();
			//判断是否有数据交换
			if ( no_mqtt_msg_exchange == 0 )
			{
				//如果有数据交换，这次就不需要发送PING消息
				continue;
			}

			if ( my_mqtt_send_pingreq ( mysock ) < 0 )
			{
				//重连服务器
				printf ( "MQTT>>发送ping失败....\n" );
				goto MQTT_reconnect;
			}

			//心跳成功
			printf ( "MQTT>>发送ping作为心跳成功....\n" );
			//表明有数据交换
			no_mqtt_msg_exchange = 0;
		}
	}

MQTT_reconnect:
	//关闭链接
	transport_close();
	//重新链接服务器
	goto MQTT_START;
}


/************************************************************************
** 函数名称: my_mqtt_send_pingreq
** 函数功能: 发送MQTT心跳包
** 入口参数: 无
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注:
************************************************************************/
s32 my_mqtt_send_pingreq ( s32 sock )
{
	s32 len;
	u8 buf[200];
	s32 buflen = sizeof ( buf );
	fd_set readfd;
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	FD_ZERO ( &readfd );
	FD_SET ( sock,&readfd );

	len = MQTTSerialize_pingreq ( buf, buflen );
	transport_sendPacketBuffer ( buf, len );

	//等待可读事件
	if ( select ( sock+1,&readfd,NULL,NULL,&tv ) == 0 )
	{
		return -1;
	}

	//有可读事件
	if ( FD_ISSET ( sock,&readfd ) == 0 )
	{
		return -2;
	}

	if ( MQTTPacket_read ( buf, buflen, transport_getdata ) != PINGRESP )
	{
		return -3;
	}

	return 0;

}

/************************************************************************
** 函数名称: MQTTClientInit
** 函数功能: 初始化客户端并登录服务器
** 入口参数: s32 sock:网络描述符
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注:
************************************************************************/
s32 MQTTClientInit ( s32 sock )
{
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	u8 buf[100];
	s32 buflen = sizeof ( buf );
	s32 len;
	u8 sessionPresent,connack_rc;

	fd_set readfd;
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	FD_ZERO ( &readfd );
	FD_SET ( sock,&readfd );

	//创建MQTT客户端连接参数
	connectData.willFlag = 0;
	//MQTT版本
	connectData.MQTTVersion = 4;
	//客户端ID--必须唯一
	connectData.clientID.cstring = "MIMITOTO";
	//保活间隔
	connectData.keepAliveInterval = KEEPLIVE_TIME;
	//用户名
	connectData.username.cstring = NULL;
	//用户密码
	connectData.password.cstring = NULL;
	//清除会话
	connectData.cleansession = 1;

	//串行化连接消息
	len = MQTTSerialize_connect ( buf, buflen, &connectData );
	//发送TCP数据
	if ( transport_sendPacketBuffer ( buf, len ) < 0 )
	{
		return -1;
	}

	//等待可读事件--等待超时
	if ( select ( sock+1,&readfd,NULL,NULL,&tv ) == 0 )
	{
		return -2;
	}
	//有可读事件--没有可读事件
	if ( FD_ISSET ( sock,&readfd ) == 0 )
	{
		return -3;
	}

	if ( MQTTPacket_read ( buf, buflen, transport_getdata ) != CONNACK )
	{
		return -4;
	}
	//拆解连接回应包
	if ( MQTTDeserialize_connack ( &sessionPresent, &connack_rc, buf, buflen ) != 1 || connack_rc != 0 )
	{
		return -5;
	}

	if ( sessionPresent == 1 )
	{
		return 1;    //不需要重新订阅--服务器已经记住了客户端的状态
	}
	else
	{
		return 0;    //需要重新订阅
	}
}


/************************************************************************
** 函数名称: MQTTSubscribe
** 函数功能: 订阅消息
** 入口参数: s32 sock：套接字
**           s8 *topic：主题
**           enum QoS pos：消息质量
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注:
************************************************************************/
s32 MQTTSubscribe ( s32 sock,s8* topic,enum QoS pos )
{
	static u32 PacketID = 0;
	u16 packetidbk = 0;
	s32 conutbk = 0;
	u8 buf[100];
	s32 buflen = sizeof ( buf );
	MQTTString topicString = MQTTString_initializer;
	s32 len;
	s32 req_qos,qosbk;

	fd_set readfd;
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	FD_ZERO ( &readfd );
	FD_SET ( sock,&readfd );

	//复制主题
	topicString.cstring = ( char* ) topic;
	//订阅质量
	req_qos = pos;

	//串行化订阅消息
	len = MQTTSerialize_subscribe ( buf, buflen, 0, PacketID++, 1, &topicString, &req_qos );
	//发送TCP数据
	if ( transport_sendPacketBuffer ( buf, len ) < 0 )
	{
		return -1;
	}

	//等待可读事件--等待超时
	if ( select ( sock+1,&readfd,NULL,NULL,&tv ) == 0 )
	{
		return -2;
	}
	//有可读事件--没有可读事件
	if ( FD_ISSET ( sock,&readfd ) == 0 )
	{
		return -3;
	}

	//等待订阅返回--未收到订阅返回
	if ( MQTTPacket_read ( buf, buflen, transport_getdata ) != SUBACK )
	{
		return -4;
	}

	//拆订阅回应包
	if ( MQTTDeserialize_suback ( &packetidbk,1, &conutbk, &qosbk, buf, buflen ) != 1 )
	{
		return -5;
	}

	//检测返回数据的正确性
	if ( ( qosbk == 0x80 ) || ( packetidbk != ( PacketID-1 ) ) )
	{
		return -6;
	}

	//订阅成功
	return 0;
}


/************************************************************************
** 函数名称: UserMsgCtl
** 函数功能: 用户消息处理函数
** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
** 出口参数: 无
** 备    注:
************************************************************************/
void UserMsgCtl ( MQTT_USER_MSG*  msg )
{
	//这里处理数据只是打印，用户可以在这里添加自己的处理方式
	printf ( "MQTT>>****收到客户端自己订阅的消息！！****\n" );
	//返回后处理消息
	switch ( msg->msgqos )
	{
		case 0:
			printf ( "MQTT>>消息质量：QoS0\n" );
			break;
		case 1:
			printf ( "MQTT>>消息质量：QoS1\n" );
			break;
		case 2:
			printf ( "MQTT>>消息质量：QoS2\n" );
			break;
		default:
			printf ( "MQTT>>错误的消息质量\n" );
			break;
	}
	printf ( "MQTT>>消息主题：%s\n",msg->topic );
	printf ( "MQTT>>消息类容：%s\n",msg->msg );
	printf ( "MQTT>>消息长度：%d\n",msg->msglenth );

	//处理完后销毁数据
	msg->valid  = 0;
}

/************************************************************************
** 函数名称: GetNextPackID
** 函数功能: 产生下一个数据包ID
** 入口参数: 无
** 出口参数: u16 packetid:产生的ID
** 备    注:
************************************************************************/
u16 GetNextPackID ( void )
{
	static u16 pubpacketid = 0;
	return pubpacketid++;
}

/************************************************************************
** 函数名称: mqtt_msg_publish
** 函数功能: 用户推送消息
** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注:
************************************************************************/
s32 MQTTMsgPublish ( s32 sock, s8* topic, s8 qos, s8 retained,u8* msg,u32 msg_len )
{
	u8 buf[MSG_MAX_LEN];
	s32 buflen = sizeof ( buf ),len;
	MQTTString topicString = MQTTString_initializer;
	u16 packid = 0,packetidbk;

	//填充主题
	topicString.cstring = ( char* ) topic;

	//填充数据包ID
	if ( ( qos == QOS1 ) || ( qos == QOS2 ) )
	{
		packid = GetNextPackID();
	}
	else
	{
		qos = QOS0;
		retained = 0;
		packid = 0;
	}

	//推送消息
	len = MQTTSerialize_publish ( buf, buflen, 0, qos, retained, packid, topicString, ( unsigned char* ) msg, msg_len );
	if ( len <= 0 )
	{
		return -1;
	}
	if ( transport_sendPacketBuffer ( buf, len ) < 0 )
	{
		return -2;
	}

	//质量等级0，不需要返回
	if ( qos == QOS0 )
	{
		return 0;
	}

	//等级1
	if ( qos == QOS1 )
	{
		//等待PUBACK
		if ( WaitForPacket ( sock,PUBACK,5 ) < 0 )
		{
			return -3;
		}
		return 1;

	}
	//等级2
	if ( qos == QOS2 )
	{
		//等待PUBREC
		if ( WaitForPacket ( sock,PUBREC,5 ) < 0 )
		{
			return -3;
		}
		//发送PUBREL
		len = MQTTSerialize_pubrel ( buf, buflen,0, packetidbk );
		if ( len == 0 )
		{
			return -4;
		}
		if ( transport_sendPacketBuffer ( buf, len ) < 0 )
		{
			return -6;
		}
		//等待PUBCOMP
		if ( WaitForPacket ( sock,PUBREC,5 ) < 0 )
		{
			return -7;
		}
		return 2;
	}
	//等级错误
	return -8;
}

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
s32 ReadPacketTimeout ( s32 sock,u8* buf,s32 buflen,u32 timeout )
{
	fd_set readfd;
	struct timeval tv;
	if ( timeout != 0 )
	{
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		FD_ZERO ( &readfd );
		FD_SET ( sock,&readfd );

		//等待可读事件--等待超时
		if ( select ( sock+1,&readfd,NULL,NULL,&tv ) == 0 )
		{
			return -1;
		}
		//有可读事件--没有可读事件
		if ( FD_ISSET ( sock,&readfd ) == 0 )
		{
			return -1;
		}
	}
	//读取TCP/IP事件
	return MQTTPacket_read ( buf, buflen, transport_getdata );
}


/************************************************************************
** 函数名称: deliverMessage
** 函数功能: 接受服务器发来的消息
** 入口参数: MQTTMessage *msg:MQTT消息结构体
**           MQTT_USER_MSG *mqtt_user_msg:用户接受结构体
**           MQTTString  *TopicName:主题
** 出口参数: 无
** 备    注:
************************************************************************/
void deliverMessage ( MQTTString*  TopicName,MQTTMessage* msg,MQTT_USER_MSG* mqtt_user_msg )
{
	//消息质量
	mqtt_user_msg->msgqos = msg->qos;
	//保存消息
	memcpy ( mqtt_user_msg->msg,msg->payload,msg->payloadlen );
	mqtt_user_msg->msg[msg->payloadlen] = 0;
	//保存消息长度
	mqtt_user_msg->msglenth = msg->payloadlen;
	//消息主题
	memcpy ( ( char* ) mqtt_user_msg->topic,TopicName->lenstring.data,TopicName->lenstring.len );
	mqtt_user_msg->topic[TopicName->lenstring.len] = 0;
	//消息ID
	mqtt_user_msg->packetid = msg->id;
	//标明消息合法
	mqtt_user_msg->valid = 1;
}


/************************************************************************
** 函数名称: mqtt_pktype_ctl
** 函数功能: 根据包类型进行处理
** 入口参数: u8 packtype:包类型
** 出口参数: 无
** 备    注:
************************************************************************/
void mqtt_pktype_ctl ( u8 packtype,u8* buf,u32 buflen )
{
	MQTTMessage msg;
	s32 rc;
	MQTTString receivedTopic;
	u32 len;
	switch ( packtype )
	{
		case PUBLISH:
			//拆析PUBLISH消息
			if ( MQTTDeserialize_publish ( &msg.dup, ( int* ) &msg.qos, &msg.retained, &msg.id, &receivedTopic, ( unsigned char** ) &msg.payload, &msg.payloadlen, buf, buflen ) != 1 )
			{
				return;
			}
			//接受消息
			deliverMessage ( &receivedTopic,&msg,&mqtt_user_msg );

			//消息质量不同，处理不同
			if ( msg.qos == QOS0 )
			{
				//QOS0-不需要ACK
				//直接处理数据
				UserMsgCtl ( &mqtt_user_msg );
				return;
			}
			//发送PUBACK消息
			if ( msg.qos == QOS1 )
			{
				len =MQTTSerialize_puback ( buf,buflen,mqtt_user_msg.packetid );
				if ( len == 0 )
				{
					return;
				}
				//发送返回
				if ( transport_sendPacketBuffer ( buf,len ) <0 )
				{
					return;
				}
				//返回后处理消息
				UserMsgCtl ( &mqtt_user_msg );
				return;
			}

			//对于质量2,只需要发送PUBREC就可以了
			if ( msg.qos == QOS2 )
			{
				len = MQTTSerialize_ack ( buf, buflen, PUBREC, 0, mqtt_user_msg.packetid );
				if ( len == 0 )
				{
					return;
				}
				//发送返回
				transport_sendPacketBuffer ( buf,len );
			}
			break;
		case  PUBREL:
			//解析包数据，必须包ID相同才可以
			rc = MQTTDeserialize_ack ( &msg.type,&msg.dup, &msg.id, buf,buflen );
			if ( ( rc != 1 ) || ( msg.type != PUBREL ) || ( msg.id != mqtt_user_msg.packetid ) )
			{
				return ;
			}
			//收到PUBREL，需要处理并抛弃数据
			if ( mqtt_user_msg.valid == 1 )
			{
				//返回后处理消息
				UserMsgCtl ( &mqtt_user_msg );
			}
			//串行化PUBCMP消息
			len = MQTTSerialize_pubcomp ( buf,buflen,msg.id );
			if ( len == 0 )
			{
				return;
			}
			//发送返回--PUBCOMP
			transport_sendPacketBuffer ( buf,len );
			break;
		case   PUBACK://等级1客户端推送数据后，服务器返回
			break;
		case   PUBREC://等级2客户端推送数据后，服务器返回
			break;
		case   PUBCOMP://等级2客户端推送PUBREL后，服务器返回
			break;
		default:
			break;
	}
}

/************************************************************************
** 函数名称: WaitForPacket
** 函数功能: 等待特定的数据包
** 入口参数: s32 sock:网络描述符
**           u8 packettype:包类型
**           u8 times:等待次数
** 出口参数: >=0:等到了特定的包 <0:没有等到特定的包
** 备    注:
************************************************************************/
s32 WaitForPacket ( s32 sock,u8 packettype,u8 times )
{
	s32 type;
	u8 buf[MSG_MAX_LEN];
	u8 n = 0;
	s32 buflen = sizeof ( buf );
	do
	{
		//读取数据包
		type = ReadPacketTimeout ( sock,buf,buflen,2 );
		if ( type != -1 )
		{
			mqtt_pktype_ctl ( type,buf,buflen );
		}
		n++;
	}
	while ( ( type != packettype ) && ( n < times ) );
	//收到期望的包
	if ( type == packettype )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
