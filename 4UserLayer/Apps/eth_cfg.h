/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : eth_cfg.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2020年1月6日
  最近修改   :
  功能描述   : 网络相关参数
  函数列表   :
  修改历史   :
  1.日    期   : 2020年1月6日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#ifndef __ETH_CFG__H
#define __ETH_CFG__H


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "tool.h"
#include "easyflash.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define   HOST_NAME       "192.168.110.109"     //服务器IP地址 线下 
#define   HOST_PORT     1883    //由于是TCP连接，端口必须是1883

#define DEVICE_PUBLISH		"/smartCloud/server/msg/device"	
#define DEVICE_SUBSCRIBE	"/smartCloud/terminal/msg/"   
#define DEVICE_SN           "3E51E8848A4C00863617"
//#define DEVICE_SN           "0BF49D025715AFB3B0A1"
//#define DEVICE_SN           "33F85538F0EB8D1796F6"
                              //5056E1CB67136EA3E1B0



#define DEV_FACTORY_PUBLISH		"/smartCloud/production/msg/device"	
#define DEV_FACTORY_SUBSCRIBE	"/smartCloud/production/msg/"    


typedef struct
{
    char deviceSn[4];
    char sn[32];    
    char publish[128];
    char subscribe[128];
}MQTT_DEVICE_SN_STRU;


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
extern MQTT_DEVICE_SN_STRU gMqttDevSn;
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

void ReadLocalDevSn(void);

#endif


