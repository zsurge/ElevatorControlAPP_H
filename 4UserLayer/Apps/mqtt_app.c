/**********************************************************************************************************
** �ļ���		:mqtt_app.c
** ����			:maxlicheng<licheng.chn@outlook.com>
** ����github	:https://github.com/maxlicheng
** ���߲���		:https://www.maxlicheng.com/
** ��������		:2018-08-08
** ����			:mqtt�������
************************************************************************************************************/
#define LOG_TAG    "MQTTAPP"
#include "elog.h"

#include "mqtt_app.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "FreeRTOS.h"
#include "task.h"
#include "malloc.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"
#include "ini.h"
#include "cmdhandle.h"
#include "eth_cfg.h"
#include "pub_options.h"
#include "bsp_beep.h"
#include "jsonUtils.h"
#include "bsp_ds1302.h"

#define send_duration	180	//�����������ڣ�ms��



static void ackUp ( void );

//static void showTask ( void );

//static void showTask ( void )
//{
//	uint8_t pcWriteBuffer[1024] = {0};

//	printf ( "=================================================\r\n" );
//	printf ( "������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n" );
//	vTaskList ( ( char* ) &pcWriteBuffer );
//	printf ( "%s\r\n", pcWriteBuffer );

//	printf ( "\r\n������       ���м���         ʹ����\r\n" );
//	vTaskGetRunTimeStats ( ( char* ) &pcWriteBuffer );
//	printf ( "%s\r\n", pcWriteBuffer );
//	log_d ( "��ǰ��̬�ڴ�ʣ���С = %d�ֽ�\r\n", xPortGetFreeHeapSize() );
//}

void mqtt_thread ( void )
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	MQTTString receivedTopic;
	MQTTString topicString = MQTTString_initializer;

	int32_t rc = 0;
	unsigned char buf[MQTT_MAX_LEN];
	int buflen = sizeof ( buf );

	
	unsigned char heartBeat[128] = {0};
	unsigned char heartBeatLen = 0;

	uint8_t upack_flag = 1;
	int payloadlen_in;
	unsigned char* payload_in;
	unsigned short msgid = 1;
	int subcount;
	int granted_qos =0;
	unsigned char sessionPresent, connack_rc;
	unsigned short submsgid;
	uint32_t len = 0;
	int req_qos = 0;
	unsigned char dup;
	int qos;
	unsigned char retained = 0;
	char pingEQTimes = 0; 
	
	
    //��ȡ��ǰ�δ���Ϊ��������ʼʱ��
    uint32_t curtick  = 0;
    uint32_t sendtick = 0; 

	uint8_t msgtypes = CONNECT;		//��Ϣ״̬��ʼ��
	uint8_t t=0;    

	log_d ( "socket connect to server\r\n" );
	gMySock = transport_open ( ( char* ) HOST_NAME,HOST_PORT );
	log_d ( "1.Sending to hostname %s port %d,gMySock = %d\r\n", HOST_NAME, HOST_PORT,gMySock );
    
MQTT_START:    
	len = MQTTSerialize_disconnect ( ( unsigned char* ) buf,buflen );
	rc = transport_sendPacketBuffer ( gMySock, ( uint8_t* ) buf, len );
	if ( rc == len )															
	{
		log_d ( "send DISCONNECT Successfully,gMySock = %d\r\n", gMySock );
	}
	else
	{     
		log_d ( "send DISCONNECT failed,gMySock = %d\r\n", gMySock);
	}

	vTaskDelay (3000);

	log_d ( "socket connect to server\r\n" );
	gMySock = transport_open ( ( char* ) HOST_NAME,HOST_PORT );
	log_d ( "2.Sending to hostname %s port %d,gMySock = %d\r\n", HOST_NAME, HOST_PORT,gMySock );

    if(gMySock < 0)
    {
        log_d ( "MQTT>>connect server error...\r\n" );  
        gConnectStatus = 0;
        goto MQTT_reconnect;
    }


log_d("2 gDevBaseParam.deviceCode.deviceSn = %s\r\n",gDevBaseParam.deviceCode.deviceSn);
log_d("2 gDevBaseParam.mqttTopic.publish = %s\r\n",gDevBaseParam.mqttTopic.publish);
log_d("2 gDevBaseParam.mqttTopic.subscribe = %s\r\n",gDevBaseParam.mqttTopic.subscribe);       
log_d("2 gDevBaseParam.deviceCode.qrSn = %s,gDevBaseParam.deviceCode.qrSnLen = %d\r\n",gDevBaseParam.deviceCode.qrSn,gDevBaseParam.deviceCode.qrSnLen);

    
	data.clientID.cstring = gDevBaseParam.deviceCode.deviceSn;       
	data.keepAliveInterval = KEEPLIVE_TIME;         //���ֻ�Ծ
	data.username.cstring = USER_NAME;//gDevBaseParam.deviceCode.deviceSn;              //�û���
	data.password.cstring = PASSWORD;               //��Կ
	data.MQTTVersion = MQTT_VERSION;                //3��ʾ3.1�汾��4��ʾ3.11�汾
	data.cleansession = 1;
    
    curtick = xTaskGetTickCount();
    sendtick = xTaskGetTickCount();
    
	while ( 1 )
	{
		if ( ( xTaskGetTickCount() - curtick ) >= ( data.keepAliveInterval*200 ) )		//ÿ��200��tick
		{
			if ( msgtypes == 0 )
			{
				curtick =  xTaskGetTickCount();
				msgtypes = PINGREQ;
			}
		}

        if(gUpdateDevSn == 1)
        {
            gUpdateDevSn = 0;
            msgtypes = CONNECT; 
			gConnectStatus = 0;
            goto MQTT_reconnect;            
        }   

        //ÿ4���Ӹ���������һ������
        if(gConnectStatus == 1)
        {
            if ( ( xTaskGetTickCount() - sendtick ) >= (send_duration*1000 ) )		//ÿ��200��tick
            {
                sendtick = xTaskGetTickCount();
                sprintf((char*)heartBeat,"{\"commandCode\":\"99999\",\"deviceCode\":\"%s\"}",gDevBaseParam.deviceCode.deviceSn);
                heartBeatLen = strlen((const char*)heartBeat);
                log_d("heartBeatLen = %d,heartbeat = %s\r\n",heartBeatLen,heartBeat);                
                topicString.cstring = gDevBaseParam.mqttTopic.publish;       //�����ϱ� ����

                len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, heartBeat, heartBeatLen);//������Ϣ
                rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
                if(rc == len) 
                 {
                    log_d("send PUBLISH Successfully,rc = %d,len = %d\r\n",rc,len);
                }
                else
                {
                    log_d("send PUBLISH failed,rc = %d,len = %d\r\n",rc,len);     
                } 
            } 
        }


		switch ( msgtypes )
		{
			//���ӷ���� �ͻ����������ӷ����
			case CONNECT://1
				len = MQTTSerialize_connect ( ( unsigned char* ) buf, buflen, &data ); 			//��ȡ�����鳤��		����������Ϣ
				rc = transport_sendPacketBuffer ( gMySock, ( unsigned char* ) buf, len );		//���� ���ط������鳤��
				if ( rc == len )															
				{
					log_d ( "send CONNECT Successfully\r\n" );
				}
				else
				{
					log_d ( "send CONNECT failed\r\n" );
					msgtypes = CONNECT; 
                    goto MQTT_reconnect;
				}
				log_d ( "step = %d,MQTT concet to server!\r\n",CONNECT );
				msgtypes = 0;
				break;
			//ȷ����������
			case CONNACK://2
				if ( MQTTDeserialize_connack ( &sessionPresent, &connack_rc, ( unsigned char* ) buf, buflen ) != 1 || connack_rc != 0 )	//�յ���ִ
				{
					log_d ( "Unable to connect, return code %d\r\n", connack_rc );		//��ִ��һ�£�����ʧ��
					msgtypes = CONNECT; 
					gConnectStatus = 0;
		            goto MQTT_reconnect;
				}
				else
				{
					log_d ( "step = %d,MQTT is concet OK!\r\n",CONNACK );	
					gConnectStatus = 1;
				}
				msgtypes = SUBSCRIBE;													//���ӳɹ� ִ�� ���� ����
				break;
			//�������� �ͻ��˶�������
			case SUBSCRIBE://8
				topicString.cstring = gDevBaseParam.mqttTopic.subscribe;

				len = MQTTSerialize_subscribe ( ( unsigned char* ) buf, buflen, 0, msgid, 1, &topicString, &req_qos );
				rc = transport_sendPacketBuffer ( gMySock, ( unsigned char* ) buf, len );
				if ( rc == len )
				{
					log_d ( "send SUBSCRIBE Successfully\r\n" );
				}
				else
				{
					log_d ( "send SUBSCRIBE failed\r\n" );
					t++;
					if ( t >= 10 )
					{
						t = 0;
						msgtypes = CONNECT;
					}
					else
					{
						msgtypes = SUBSCRIBE;
					}
					break;
				}
//				log_d ( "step = %d,client subscribe:[%s]\r\n",SUBSCRIBE,topicString.cstring );
				msgtypes = 0;

				if ( upack_flag )
				{
					upack_flag = 0;
					ackUp();
				}

				break;
			//����ȷ�� ����������ȷ��
			case SUBACK://9
				rc = MQTTDeserialize_suback ( &submsgid, 1, &subcount, &granted_qos, ( unsigned char* ) buf, buflen );	//�л�ִ  QoS
//				log_d ( "step = %d,granted qos is %d\r\n",SUBACK, granted_qos );         								//��ӡ Qos
				msgtypes = 0;
				break;
			//������Ϣ
			case PUBLISH://3
				rc = MQTTDeserialize_publish ( &dup, &qos, &retained, &msgid, &receivedTopic,&payload_in, &payloadlen_in, ( unsigned char* ) buf, buflen );	//��ȡ������������Ϣ
//				log_d ( "step = %d,message arrived : %s,len= %d\r\n",PUBLISH,payload_in,strlen ( ( const char* ) payload_in ) );

                //��Ϣ������ͬ������ͬ
                if(qos == 0)
                {
                    //QOS0-����ҪACK
                    //ֱ�Ӵ�������
//                    exec_proc ( ( char* ) GetJsonItem ( ( const uint8_t* ) payload_in, ( const uint8_t* ) CMD_ID,0 ),payload_in );                    
                    Proscess(payload_in);
                }

                //����PUBACK��Ϣ
                if(qos == 1)
                {
                    printf("publish qos is 1,send PUBACK \r\n");							//QosΪ1�����л�ִ ��Ӧ
                    memset(buf,0,buflen);
                    len = MQTTSerialize_ack((unsigned char*)buf,buflen,PUBACK,dup,msgid);   
    				if ( len == 0 )
    				{
                        msgtypes = CONNECT; 
                        gConnectStatus = 0;
                        goto MQTT_reconnect;
    				}                    
                    //publish ack                       
                    rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);			//
                    if(rc == len)
                        log_d("send PUBACK Successfully\r\n");
                    else
                        log_d("send PUBACK failed\r\n");   

                    //����������ִ�У����Ƿ��͵���Ϣ�����У��ڶ���Ϣ������ִ�У�
                    //���˸о�����Ϣ�����л�õ�
//                    exec_proc ( ( char* ) GetJsonItem ( ( const uint8_t* ) payload_in, ( const uint8_t* ) CMD_ID,0 ),payload_in );                    
                    Proscess(payload_in);
                }   

                //��������2,ֻ��Ҫ����PUBREC�Ϳ�����
                if(qos == 2)
                {
                    printf("publish qos is 2,send PUBREC \r\n");
                    len = MQTTSerialize_ack ((unsigned char*)buf,buflen, PUBREC, dup, msgid );
    				if ( len == 0 )
    				{
                        msgtypes = CONNECT; 
                        gConnectStatus = 0;
                        goto MQTT_reconnect;
    				}                     
                    rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);	
                    if(rc == len)
                        log_d("send PUBREC Successfully\r\n");
                    else
                        log_d("send PUBREC failed\r\n");  
                }               

				msgtypes = 0;
				break;
			//����ȷ�� QoS	1��Ϣ�����յ�ȷ��
			case PUBACK://4
				log_d ( "step = %d,PUBACK!\r\n",PUBACK );					//�����ɹ�
				msgtypes = 0;
				break;
			//�����յ� QoS2 ��һ��
			case PUBREC://5
				log_d ( "step = %d,PUBREC!\r\n",PUBREC );     				//just for qos2
				msgtypes = 0;
				break;
			//�����ͷ� QoS2 �ڶ���
			case PUBREL://6
				log_d ( "step = %d,PUBREL!\r\n",PUBREL );        			//just for qos2
				msgtypes = 0;
				break;
			//������� QoS2 ������
			case PUBCOMP://7
				log_d ( "step = %d,PUBCOMP!\r\n",PUBCOMP );        			//just for qos2
				msgtypes = 0;
				break;

			case PINGREQ:   
//				log_d ( "step = %d,mqtt server ping ,pingEQTimes = %d\r\n",PINGREQ,pingEQTimes);  			//����			
			    len = MQTTSerialize_pingreq((unsigned char*)buf, buflen);							//����
				rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
				if(rc == len)
				{
            		if(pingEQTimes++ >= 5)
                	{
                	    pingEQTimes = 0;
                	    msgtypes = 0; 
                        gConnectStatus = 0;
                        goto MQTT_reconnect;     
                	}				    
					log_d("send PINGREQ Successfully,,pingEQTimes = %d\r\n",pingEQTimes);
			    }
				else
				{
                    log_d("time to ping mqtt server to take alive!,%d,%d\r\n",rc,len);
                    NVIC_SystemReset(); 
                }	
                msgtypes = 0;
				break;
#if 0		//��Ƶ������MQTT�����߻���	
			//��������
			case PINGREQ://12
				len = MQTTSerialize_pingreq ( ( unsigned char* ) buf, buflen );		
				//����
				rc = transport_sendPacketBuffer ( gMySock, ( unsigned char* ) buf, len );
				if ( rc == len )
				{
//					log_d ( "send PINGREQ Successfully\r\n" );                                      

                    rc = MQTTPacket_read ( ( unsigned char* ) buf, buflen, transport_getdata ); //��ѯ����MQTT�������ݣ�
                    if ( rc != PINGRESP ) //����������Ӧ
                    {
                        log_d ( "lost mqtt server connect!\r\n");
                        msgtypes = CONNECT; 
                        gConnectStatus = 0;
                        goto MQTT_reconnect;
                    }
                    log_d ( "step = %d,mqtt server Pong\r\n",PINGRESP );            //������ִ����������Ӧ

                    msgtypes = 0;
				}
				else
				{
					log_d ( "send PINGREQ failed,\r\n");
                    msgtypes = CONNECT; 
                    gConnectStatus = 0;
                    goto MQTT_reconnect;                    
				}				            
				break;
#endif				
			//������Ӧ
			case PINGRESP://13
			    pingEQTimes--;
				log_d ( "step = %d,mqtt server Pong\r\n",PINGRESP );  			//������ִ����������Ӧ
				msgtypes = 0;
				break;
            case UNSUBSCRIBE:
				log_d ( "step = %d,UNSUBSCRIBE!\r\n",UNSUBSCRIBE );					//�����ɹ�
				msgtypes = 0;                
                break;
            case UNSUBACK:                
                log_d ( "step = %d,UNSUBACK!\r\n",UNSUBACK );                   //�����ɹ�
                msgtypes = 0;
                break;
            case DISCONNECT:                
                log_d ( "step = %d,DISCONNECT!\r\n",DISCONNECT );                   //�����ɹ�
                msgtypes = 0;
                break;
            
			default:
				break;

		}
		memset ( buf,0,buflen );
		rc=MQTTPacket_read ( ( unsigned char* ) buf, buflen, transport_getdata ); //��ѯ����MQTT�������ݣ�
//		log_d("MQTTPacket_read = %d\r\n",rc);
		if ( rc >0 ) //��������ݣ�������Ӧ״̬��
		{
			msgtypes = rc;
//			log_d ( "MQTT is get recv: msgtypes = %d\r\n",msgtypes );
		}	

//		/* �����¼���־����ʾ������������ */
		xEventGroupSetBits ( xCreatedEventGroup, TASK_BIT_5 );
        vTaskDelay ( 200 );
	}

MQTT_reconnect:    
	transport_close ( gMySock );    
	log_d ( "mqtt thread exit.try again 3 sec\r\n" );  

    vTaskDelay (200);
    goto MQTT_START;        
}




static void ackUp ( void )
{
	char up_status[12] = {0};

	uint8_t up_status_len = 0;

	up_status_len = ef_get_env_blob ( "up_status", up_status, sizeof ( up_status ), NULL );

	log_d ( "up_status = %s, up_status_len = %d\r\n",up_status,up_status_len );

	if ( memcmp ( up_status,"101711",6 ) == 0 )
	{
		ef_set_env_blob ( "up_status", "101722",6 );
        exec_proc("1017"," ");
	}
    exec_proc("30131","");	
}



//{\"data\":{\"currentLayer\":2,\"identification\":\"20-6-1582360859332\",\"purposeLayer\":9,\"status\":\"1\"},\"commandCode\":\"3010\",\"deviceCode\":\"5056E1CB67136EA3E1B0\"}



