/**********************************************************************************************************
** �ļ���		:mqtt_app.h
** ����			:maxlicheng<licheng.chn@outlook.com>
** ����github	:https://github.com/maxlicheng
** ���߲���		:https://www.maxlicheng.com/	
** ��������		:2018-08-08
** ����			:mqtt�������ͷ�ļ�
************************************************************************************************************/
#ifndef _MQTT_APP_H_
#define _MQTT_APP_H_
#include "sys.h"

//�û���Ҫ�����豸��Ϣ�������º궨���е���Ԫ������
#define PRODUCT_KEY    	"a1Yjxb6GjGk"															//�����ư䷢�Ĳ�ƷΨһ��ʶ��11λ���ȵ�Ӣ������������
#define DEVICE_NAME    	"elevator"																//�û�ע���豸ʱ���ɵ��豸Ψһ��ţ�֧��ϵͳ�Զ����ɣ�Ҳ��֧���û�����Զ����ţ���Ʒά����Ψһ
#define DEVICE_SECRET  	"vfW2KtmvfGy9AcBwNY9h4wksJifwt2Lf"				//�豸��Կ����DeviceName�ɶԳ��֣�������һ��һ�ܵ���֤����


#define CMD_ID   "commandCode"


#define   KEEPLIVE_TIME   50
#define   MQTT_VERSION    4

//���º궨��̶�������Ҫ�޸�


//#define CONTENT				"7A13DCC67054F72CC07F"//"clientId"DEVICE_NAME"deviceName"DEVICE_NAME"productKey"PRODUCT_KEY"timestamp789"	//�����¼������
//#define CLIENT_ID			DEVICE_NAME"|1233444444434234234234234234"											//�ͻ���ID
#define USER_NAME			"adm"																													//�ͻ����û���
#define PASSWORD			"1234567812345678"																			//�ͻ��˵�¼passwordͨ��hmac_sha1�㷨�õ�����Сд������


//#define   HOST_NAME       "192.168.110.78"     //������IP��ַ ���� 
//#define   HOST_PORT     1883    //������TCP���ӣ��˿ڱ�����1883
//#define DEVICE_PUBLISH		"/smartCloud/server/msg/device"			
//#define DEVICE_SUBSCRIBE	"/smartCloud/terminal/msg/"DEVICE_SN
//#define DEVICE_SN           "3E51E8848A4C00863617"

//#define DEVICE_SN           //"C7A052661C5D0CBE1E5F"
//#define DEVICE_SN         "0BF49D025715AFB3B0A1"


//�����豸����

//��������TOPIC�ĺ궨�岻��Ҫ�û��޸ģ�����ֱ��ʹ��
//IOT HUBΪ�豸��������TOPIC��update�����豸������Ϣ��error�����豸��������get���ڶ�����Ϣ
//#define TOPIC_UPDATE         "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
//#define TOPIC_ERROR          "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
//#define TOPIC_GET            "/"PRODUCT_KEY"/"DEVICE_NAME"/get"

void mqtt_thread(void);
//u32 PublishData(float temp, float humid, unsigned char *payloadbuf);






#endif



