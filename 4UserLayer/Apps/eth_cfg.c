/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : eth_cfg.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2020年1月6日
  最近修改   :
  功能描述   : 网络参数管理
  函数列表   :
  修改历史   :
  1.日    期   : 2020年1月6日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "eth_cfg.h"

#define LOG_TAG    "ETH_CFG"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
MQTT_DEVICE_SN_STRU gMqttDevSn;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
void ReadLocalDevSn ( void )
{
	char sn_flag[5] = {0};
	char mac[6+1] = {0};
	char id[8] = {0};
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
		    strcpy ( gMqttDevSn.deviceSn,id);
		    
			log_d ( "sn = %s,len = %d\r\n",remote_sn,read_len );
			strcpy ( gMqttDevSn.sn,remote_sn );
			log_d("1 deviceCode = %s\r\n",gMqttDevSn.sn);
			strcpy ( gMqttDevSn.publish,DEVICE_PUBLISH );
			strcpy ( gMqttDevSn.subscribe,DEVICE_SUBSCRIBE );
			strcat ( gMqttDevSn.subscribe,remote_sn );			
		}

		 log_d("2 deviceCode = %s\r\n",gMqttDevSn.sn);
		 log_d("gMqttDevSn.deviceSn = %s\r\n",gMqttDevSn.deviceSn);
	}
	else
	{
		//使用MAC做为SN
		calcMac ( (unsigned char*)mac );
		bcd2asc ( (unsigned char*)asc, (unsigned char*)mac, 12, 0 );
		Insertchar ( asc,temp,':' );
		memcpy ( gMqttDevSn.sn,temp,strlen ( temp )-1 );

		log_d ( "strToUpper asc = %s\r\n",gMqttDevSn.sn );
		ef_set_env_blob ( "remote_sn",gMqttDevSn.sn,strlen ( gMqttDevSn.sn ) );

		strcpy ( gMqttDevSn.publish,DEV_FACTORY_PUBLISH );
		strcpy ( gMqttDevSn.subscribe,DEV_FACTORY_SUBSCRIBE );
		strcat ( gMqttDevSn.subscribe,gMqttDevSn.sn );
		memcpy ( gMqttDevSn.deviceSn,gMqttDevSn.sn,8);
	}


	log_d ( "publish = %s,subscribe = %s\r\n",gMqttDevSn.publish,gMqttDevSn.subscribe );
}






