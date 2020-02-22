#include "mqttclient.h"
#include "transport.h"
#include "MQTTPacket.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "sockets.h"

#define HOST_NAME "m2m.eclipse.org"//"192.168.110.109"
#define HOST_PORT 1883

//�����û���Ϣ�ṹ��
MQTT_USER_MSG  mqtt_user_msg;

void deliverMessage ( MQTTString* TopicName,MQTTMessage* msg,MQTT_USER_MSG* mqtt_user_msg );

/************************************************************************
** ��������: mqtt_thread
** ��������: MQTT����
** ��ڲ���: void *pvParameters���������
** ���ڲ���: ��
** ��    ע:
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
	//������������
	printf ( "MQTT>>1.��ʼ������������...\n" );
	printf ( "��������%s,�˿ںţ�%0d\n",HOST_NAME,HOST_PORT );
	while ( 1 )
	{
		//���ӷ�����
		mysock = transport_open ( ( s8* ) HOST_NAME,HOST_PORT );
		//������ӷ������ɹ�
		if ( mysock >= 0 )
		{
			printf ( "MQTT>>���ӷ������ɹ���\n" );
			break;
		}
		printf ( "MQTT>>���ӷ�����ʧ�ܣ���Ϣ3�����������ӣ�\n" );
		//��Ϣ3��
		vTaskDelay ( 3000/portTICK_RATE_MS );
	}

	//��ʼ��MQTT�ͻ���
	printf ( "MQTT>>2.��ʼ��MQTT�ͻ���,�����ӿͻ��˵�������...\n" );
	//��¼������
	sessionPresent = MQTTClientInit ( mysock );
	if ( sessionPresent < 0 )
	{
		printf ( "MQTT>>�ͻ������ӷ�����ʧ��...\n" );
		goto MQTT_reconnect;
	}

	//������Ϣ
	printf ( "MQTT>>3.��ʼ������Ϣ...\n" );
	if ( sessionPresent != 1 )
	{
		//������Ϣ
		if ( MQTTSubscribe ( mysock, ( s8* ) "mymqttsubtest",QOS0 ) < 0 )
		{
			//����������
			printf ( "MQTT>>�ͻ��˶�����Ϣʧ��...\n" );
			goto MQTT_reconnect;
		}
	}

	//��ȡ��ǰ�δ���Ϊ��������ʼʱ��
	curtick = xTaskGetTickCount();

	//����ѭ��
	printf ( "MQTT>>4.��ʼѭ�����ն��ĵ���Ϣ...\n" );
	while ( 1 )
	{
		//���������ݽ���
		no_mqtt_msg_exchange = 1;

		//������Ϣ
		FD_ZERO ( &readfd );
		FD_SET ( mysock,&readfd );

		//�ȴ��ɶ��¼�
		select ( mysock+1,&readfd,NULL,NULL,&tv );

		//�ж�MQTT�������Ƿ�������
		if ( FD_ISSET ( mysock,&readfd ) != 0 )
		{
			//��ȡ���ݰ�--ע���������Ϊ0��������
			type = ReadPacketTimeout ( mysock,buf,buflen,0 );
			if ( type != -1 )
			{
				mqtt_pktype_ctl ( type,buf,buflen );
				//���������ݽ���
				no_mqtt_msg_exchange = 0;
				//��ȡ��ǰ�δ���Ϊ��������ʼʱ��
				curtick = xTaskGetTickCount();
			}
		}

		//ÿ��5�������һ����Ϣ
		if ( ( xTaskGetTickCount() - pubtick ) > ( 5000 ) )
		{
			pubtick = xTaskGetTickCount();

			//������Ϣ
			ret = MQTTMsgPublish ( mysock, ( s8* ) "DongLuTest",0, 0, ( u8* ) "This is my first MQTT programme!",strlen ( "This is my first MQTT programme!" ) );
			if ( ret >= 0 )
			{
				//���������ݽ���
				no_mqtt_msg_exchange = 0;
				//��ȡ��ǰ�δ���Ϊ��������ʼʱ��
				curtick = xTaskGetTickCount();
			}

		}

		//������ҪĿ���Ƕ�ʱ�����������PING��������
		if ( ( xTaskGetTickCount() - curtick ) > ( KEEPLIVE_TIME/2*1000 ) )
		{
			curtick = xTaskGetTickCount();
			//�ж��Ƿ������ݽ���
			if ( no_mqtt_msg_exchange == 0 )
			{
				//��������ݽ�������ξͲ���Ҫ����PING��Ϣ
				continue;
			}

			if ( my_mqtt_send_pingreq ( mysock ) < 0 )
			{
				//����������
				printf ( "MQTT>>����pingʧ��....\n" );
				goto MQTT_reconnect;
			}

			//�����ɹ�
			printf ( "MQTT>>����ping��Ϊ�����ɹ�....\n" );
			//���������ݽ���
			no_mqtt_msg_exchange = 0;
		}
	}

MQTT_reconnect:
	//�ر�����
	transport_close();
	//�������ӷ�����
	goto MQTT_START;
}


/************************************************************************
** ��������: my_mqtt_send_pingreq
** ��������: ����MQTT������
** ��ڲ���: ��
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
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

	//�ȴ��ɶ��¼�
	if ( select ( sock+1,&readfd,NULL,NULL,&tv ) == 0 )
	{
		return -1;
	}

	//�пɶ��¼�
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
** ��������: MQTTClientInit
** ��������: ��ʼ���ͻ��˲���¼������
** ��ڲ���: s32 sock:����������
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
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

	//����MQTT�ͻ������Ӳ���
	connectData.willFlag = 0;
	//MQTT�汾
	connectData.MQTTVersion = 4;
	//�ͻ���ID--����Ψһ
	connectData.clientID.cstring = "MIMITOTO";
	//������
	connectData.keepAliveInterval = KEEPLIVE_TIME;
	//�û���
	connectData.username.cstring = NULL;
	//�û�����
	connectData.password.cstring = NULL;
	//����Ự
	connectData.cleansession = 1;

	//���л�������Ϣ
	len = MQTTSerialize_connect ( buf, buflen, &connectData );
	//����TCP����
	if ( transport_sendPacketBuffer ( buf, len ) < 0 )
	{
		return -1;
	}

	//�ȴ��ɶ��¼�--�ȴ���ʱ
	if ( select ( sock+1,&readfd,NULL,NULL,&tv ) == 0 )
	{
		return -2;
	}
	//�пɶ��¼�--û�пɶ��¼�
	if ( FD_ISSET ( sock,&readfd ) == 0 )
	{
		return -3;
	}

	if ( MQTTPacket_read ( buf, buflen, transport_getdata ) != CONNACK )
	{
		return -4;
	}
	//������ӻ�Ӧ��
	if ( MQTTDeserialize_connack ( &sessionPresent, &connack_rc, buf, buflen ) != 1 || connack_rc != 0 )
	{
		return -5;
	}

	if ( sessionPresent == 1 )
	{
		return 1;    //����Ҫ���¶���--�������Ѿ���ס�˿ͻ��˵�״̬
	}
	else
	{
		return 0;    //��Ҫ���¶���
	}
}


/************************************************************************
** ��������: MQTTSubscribe
** ��������: ������Ϣ
** ��ڲ���: s32 sock���׽���
**           s8 *topic������
**           enum QoS pos����Ϣ����
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
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

	//��������
	topicString.cstring = ( char* ) topic;
	//��������
	req_qos = pos;

	//���л�������Ϣ
	len = MQTTSerialize_subscribe ( buf, buflen, 0, PacketID++, 1, &topicString, &req_qos );
	//����TCP����
	if ( transport_sendPacketBuffer ( buf, len ) < 0 )
	{
		return -1;
	}

	//�ȴ��ɶ��¼�--�ȴ���ʱ
	if ( select ( sock+1,&readfd,NULL,NULL,&tv ) == 0 )
	{
		return -2;
	}
	//�пɶ��¼�--û�пɶ��¼�
	if ( FD_ISSET ( sock,&readfd ) == 0 )
	{
		return -3;
	}

	//�ȴ����ķ���--δ�յ����ķ���
	if ( MQTTPacket_read ( buf, buflen, transport_getdata ) != SUBACK )
	{
		return -4;
	}

	//���Ļ�Ӧ��
	if ( MQTTDeserialize_suback ( &packetidbk,1, &conutbk, &qosbk, buf, buflen ) != 1 )
	{
		return -5;
	}

	//��ⷵ�����ݵ���ȷ��
	if ( ( qosbk == 0x80 ) || ( packetidbk != ( PacketID-1 ) ) )
	{
		return -6;
	}

	//���ĳɹ�
	return 0;
}


/************************************************************************
** ��������: UserMsgCtl
** ��������: �û���Ϣ������
** ��ڲ���: MQTT_USER_MSG  *msg����Ϣ�ṹ��ָ��
** ���ڲ���: ��
** ��    ע:
************************************************************************/
void UserMsgCtl ( MQTT_USER_MSG*  msg )
{
	//���ﴦ������ֻ�Ǵ�ӡ���û���������������Լ��Ĵ���ʽ
	printf ( "MQTT>>****�յ��ͻ����Լ����ĵ���Ϣ����****\n" );
	//���غ�����Ϣ
	switch ( msg->msgqos )
	{
		case 0:
			printf ( "MQTT>>��Ϣ������QoS0\n" );
			break;
		case 1:
			printf ( "MQTT>>��Ϣ������QoS1\n" );
			break;
		case 2:
			printf ( "MQTT>>��Ϣ������QoS2\n" );
			break;
		default:
			printf ( "MQTT>>�������Ϣ����\n" );
			break;
	}
	printf ( "MQTT>>��Ϣ���⣺%s\n",msg->topic );
	printf ( "MQTT>>��Ϣ���ݣ�%s\n",msg->msg );
	printf ( "MQTT>>��Ϣ���ȣ�%d\n",msg->msglenth );

	//���������������
	msg->valid  = 0;
}

/************************************************************************
** ��������: GetNextPackID
** ��������: ������һ�����ݰ�ID
** ��ڲ���: ��
** ���ڲ���: u16 packetid:������ID
** ��    ע:
************************************************************************/
u16 GetNextPackID ( void )
{
	static u16 pubpacketid = 0;
	return pubpacketid++;
}

/************************************************************************
** ��������: mqtt_msg_publish
** ��������: �û�������Ϣ
** ��ڲ���: MQTT_USER_MSG  *msg����Ϣ�ṹ��ָ��
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
************************************************************************/
s32 MQTTMsgPublish ( s32 sock, s8* topic, s8 qos, s8 retained,u8* msg,u32 msg_len )
{
	u8 buf[MSG_MAX_LEN];
	s32 buflen = sizeof ( buf ),len;
	MQTTString topicString = MQTTString_initializer;
	u16 packid = 0,packetidbk;

	//�������
	topicString.cstring = ( char* ) topic;

	//������ݰ�ID
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

	//������Ϣ
	len = MQTTSerialize_publish ( buf, buflen, 0, qos, retained, packid, topicString, ( unsigned char* ) msg, msg_len );
	if ( len <= 0 )
	{
		return -1;
	}
	if ( transport_sendPacketBuffer ( buf, len ) < 0 )
	{
		return -2;
	}

	//�����ȼ�0������Ҫ����
	if ( qos == QOS0 )
	{
		return 0;
	}

	//�ȼ�1
	if ( qos == QOS1 )
	{
		//�ȴ�PUBACK
		if ( WaitForPacket ( sock,PUBACK,5 ) < 0 )
		{
			return -3;
		}
		return 1;

	}
	//�ȼ�2
	if ( qos == QOS2 )
	{
		//�ȴ�PUBREC
		if ( WaitForPacket ( sock,PUBREC,5 ) < 0 )
		{
			return -3;
		}
		//����PUBREL
		len = MQTTSerialize_pubrel ( buf, buflen,0, packetidbk );
		if ( len == 0 )
		{
			return -4;
		}
		if ( transport_sendPacketBuffer ( buf, len ) < 0 )
		{
			return -6;
		}
		//�ȴ�PUBCOMP
		if ( WaitForPacket ( sock,PUBREC,5 ) < 0 )
		{
			return -7;
		}
		return 2;
	}
	//�ȼ�����
	return -8;
}

/************************************************************************
** ��������: ReadPacketTimeout
** ��������: ������ȡMQTT����
** ��ڲ���: s32 sock:����������
**           u8 *buf:���ݻ�����
**           s32 buflen:��������С
**           u32 timeout:��ʱʱ��--0-��ʾֱ�Ӳ�ѯ��û��������������
** ���ڲ���: -1������,����--������
** ��    ע:
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

		//�ȴ��ɶ��¼�--�ȴ���ʱ
		if ( select ( sock+1,&readfd,NULL,NULL,&tv ) == 0 )
		{
			return -1;
		}
		//�пɶ��¼�--û�пɶ��¼�
		if ( FD_ISSET ( sock,&readfd ) == 0 )
		{
			return -1;
		}
	}
	//��ȡTCP/IP�¼�
	return MQTTPacket_read ( buf, buflen, transport_getdata );
}


/************************************************************************
** ��������: deliverMessage
** ��������: ���ܷ�������������Ϣ
** ��ڲ���: MQTTMessage *msg:MQTT��Ϣ�ṹ��
**           MQTT_USER_MSG *mqtt_user_msg:�û����ܽṹ��
**           MQTTString  *TopicName:����
** ���ڲ���: ��
** ��    ע:
************************************************************************/
void deliverMessage ( MQTTString*  TopicName,MQTTMessage* msg,MQTT_USER_MSG* mqtt_user_msg )
{
	//��Ϣ����
	mqtt_user_msg->msgqos = msg->qos;
	//������Ϣ
	memcpy ( mqtt_user_msg->msg,msg->payload,msg->payloadlen );
	mqtt_user_msg->msg[msg->payloadlen] = 0;
	//������Ϣ����
	mqtt_user_msg->msglenth = msg->payloadlen;
	//��Ϣ����
	memcpy ( ( char* ) mqtt_user_msg->topic,TopicName->lenstring.data,TopicName->lenstring.len );
	mqtt_user_msg->topic[TopicName->lenstring.len] = 0;
	//��ϢID
	mqtt_user_msg->packetid = msg->id;
	//������Ϣ�Ϸ�
	mqtt_user_msg->valid = 1;
}


/************************************************************************
** ��������: mqtt_pktype_ctl
** ��������: ���ݰ����ͽ��д���
** ��ڲ���: u8 packtype:������
** ���ڲ���: ��
** ��    ע:
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
			//����PUBLISH��Ϣ
			if ( MQTTDeserialize_publish ( &msg.dup, ( int* ) &msg.qos, &msg.retained, &msg.id, &receivedTopic, ( unsigned char** ) &msg.payload, &msg.payloadlen, buf, buflen ) != 1 )
			{
				return;
			}
			//������Ϣ
			deliverMessage ( &receivedTopic,&msg,&mqtt_user_msg );

			//��Ϣ������ͬ������ͬ
			if ( msg.qos == QOS0 )
			{
				//QOS0-����ҪACK
				//ֱ�Ӵ�������
				UserMsgCtl ( &mqtt_user_msg );
				return;
			}
			//����PUBACK��Ϣ
			if ( msg.qos == QOS1 )
			{
				len =MQTTSerialize_puback ( buf,buflen,mqtt_user_msg.packetid );
				if ( len == 0 )
				{
					return;
				}
				//���ͷ���
				if ( transport_sendPacketBuffer ( buf,len ) <0 )
				{
					return;
				}
				//���غ�����Ϣ
				UserMsgCtl ( &mqtt_user_msg );
				return;
			}

			//��������2,ֻ��Ҫ����PUBREC�Ϳ�����
			if ( msg.qos == QOS2 )
			{
				len = MQTTSerialize_ack ( buf, buflen, PUBREC, 0, mqtt_user_msg.packetid );
				if ( len == 0 )
				{
					return;
				}
				//���ͷ���
				transport_sendPacketBuffer ( buf,len );
			}
			break;
		case  PUBREL:
			//���������ݣ������ID��ͬ�ſ���
			rc = MQTTDeserialize_ack ( &msg.type,&msg.dup, &msg.id, buf,buflen );
			if ( ( rc != 1 ) || ( msg.type != PUBREL ) || ( msg.id != mqtt_user_msg.packetid ) )
			{
				return ;
			}
			//�յ�PUBREL����Ҫ������������
			if ( mqtt_user_msg.valid == 1 )
			{
				//���غ�����Ϣ
				UserMsgCtl ( &mqtt_user_msg );
			}
			//���л�PUBCMP��Ϣ
			len = MQTTSerialize_pubcomp ( buf,buflen,msg.id );
			if ( len == 0 )
			{
				return;
			}
			//���ͷ���--PUBCOMP
			transport_sendPacketBuffer ( buf,len );
			break;
		case   PUBACK://�ȼ�1�ͻ����������ݺ󣬷���������
			break;
		case   PUBREC://�ȼ�2�ͻ����������ݺ󣬷���������
			break;
		case   PUBCOMP://�ȼ�2�ͻ�������PUBREL�󣬷���������
			break;
		default:
			break;
	}
}

/************************************************************************
** ��������: WaitForPacket
** ��������: �ȴ��ض������ݰ�
** ��ڲ���: s32 sock:����������
**           u8 packettype:������
**           u8 times:�ȴ�����
** ���ڲ���: >=0:�ȵ����ض��İ� <0:û�еȵ��ض��İ�
** ��    ע:
************************************************************************/
s32 WaitForPacket ( s32 sock,u8 packettype,u8 times )
{
	s32 type;
	u8 buf[MSG_MAX_LEN];
	u8 n = 0;
	s32 buflen = sizeof ( buf );
	do
	{
		//��ȡ���ݰ�
		type = ReadPacketTimeout ( sock,buf,buflen,2 );
		if ( type != -1 )
		{
			mqtt_pktype_ctl ( type,buf,buflen );
		}
		n++;
	}
	while ( ( type != packettype ) && ( n < times ) );
	//�յ������İ�
	if ( type == packettype )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
