/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : eth_cfg.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2020��1��6��
  ����޸�   :
  ��������   : �����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��1��6��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "eth_cfg.h"

#define LOG_TAG    "ETH_CFG"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
MQTT_DEVICE_SN_STRU gMqttDevSn;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void ReadLocalDevSn ( void )
{
	char sn_flag[5] = {0};
	char mac[6+1] = {0};
	char id[4+1] = {0};
	char temp[32] = {0};
	char asc[12+1] = {0};
	char remote_sn[20+1] = {0};
	uint8_t read_len = 0;

	memset ( &gMqttDevSn,0x00,sizeof ( gMqttDevSn ) );

	read_len = ef_get_env_blob ( "sn_flag", sn_flag, sizeof ( sn_flag ), NULL );

	log_d ( "sn_flag = %s, sn_flag_len = %d\r\n",sn_flag,read_len );

	if ( ( memcmp ( sn_flag,"1111",4 ) == 0 ) && ( read_len == 4 ) )
	{
		read_len = ef_get_env_blob ( "remote_sn", remote_sn, sizeof ( remote_sn ), NULL );
		ef_get_env_blob("device_sn",id,sizeof ( id ), NULL ); 
		if ( read_len == 20 )
		{
			log_d ( "sn = %s,len = %d\r\n",remote_sn,read_len );
			strcpy ( gMqttDevSn.sn,remote_sn );
			strcpy ( gMqttDevSn.publish,DEVICE_PUBLISH );
			strcpy ( gMqttDevSn.subscribe,DEVICE_SUBSCRIBE );
			strcat ( gMqttDevSn.subscribe,remote_sn );
			strcpy ( gMqttDevSn.deviceSn,id);
		}
	}
	else
	{
		//ʹ��MAC��ΪSN
		calcMac ( (unsigned char*)mac );
		bcd2asc ( (unsigned char*)asc, (unsigned char*)mac, 12, 0 );
		Insertchar ( asc,temp,':' );
		memcpy ( gMqttDevSn.sn,temp,strlen ( temp )-1 );

		log_d ( "strToUpper asc = %s\r\n",gMqttDevSn.sn );
		ef_set_env_blob ( "remote_sn",gMqttDevSn.sn,strlen ( gMqttDevSn.sn ) );

		strcpy ( gMqttDevSn.publish,DEV_FACTORY_PUBLISH );
		strcpy ( gMqttDevSn.subscribe,DEV_FACTORY_SUBSCRIBE );
		strcat ( gMqttDevSn.subscribe,gMqttDevSn.sn );
	}


	log_d ( "publish = %s,subscribe = %s\r\n",gMqttDevSn.publish,gMqttDevSn.subscribe );
}






