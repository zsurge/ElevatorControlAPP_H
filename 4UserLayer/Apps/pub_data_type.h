/******************************************************************************

                  Copyright @1998 - 2021 BSG

 ******************************************************************************
文 件 名   : pub_data_type.h
 					
@author  
@date 2021年2月20日
  最近修改   :
@file pub_data_type.h
@brief 存放一些公共变量及数据定义
  函数列表   :
修改历史   :
@date 2021年2月20日
@author  
修改内容   : 创建文件

******************************************************************************/
#ifndef __PUB_DATA_TYPE_H__
#define __PUB_DATA_TYPE_H__

#include"stdint.h"

#define CARD_NO_LEN_ASC     8       //卡号ASC码长度
#define CARD_NO_LEN_BCD     (CARD_NO_LEN_ASC/2) //卡号BCD码长度
#define HEAD_lEN 8                  //每条记录占8个字节,4字节卡号，4字节flash中索引
#define MAX_HEAD_RECORD     7680   //最大32768条记录
#define SECTOR_SIZE         4096    //每个扇区大小

#define MAX_HEAD_DEL_CARDNO     128   //最大可以在删除128张卡
#define MAX_HEAD_DEL_USERID     128   //最大可以在删除128个用户ID



#define CARD_NO_HEAD_SIZE   (HEAD_lEN*MAX_HEAD_RECORD)  //60K
#define USER_ID_HEAD_SIZE   (CARD_NO_HEAD_SIZE)
#define CARD_DEL_HEAD_SIZE  (HEAD_lEN*MAX_HEAD_DEL_CARDNO)   //1K
#define USER_DEL_HEAD_SIZE  (HEAD_lEN*MAX_HEAD_DEL_USERID)


#define CARD_HEAD_SECTOR_NUM     (CARD_NO_HEAD_SIZE/SECTOR_SIZE) //15个扇区
#define USER_HEAD_SECTOR_NUM     (CARD_HEAD_SECTOR_NUM)

#define HEAD_NUM_SECTOR     (SECTOR_SIZE/HEAD_lEN) //每个扇区存储512个卡号/用户ID

//改为存储在铁电
#define CARD_NO_HEAD_ADDR   0x0000
#define USER_ID_HEAD_ADDR   (CARD_NO_HEAD_ADDR+CARD_NO_HEAD_SIZE)
#define CARD_DEL_HEAD_ADDR  (USER_ID_HEAD_ADDR+USER_ID_HEAD_SIZE)
#define USER_DEL_HEAD_ADDR  (CARD_DEL_HEAD_ADDR+CARD_DEL_HEAD_SIZE)


//基本参数存储长度及地址
#define DEVICE_BASE_PARAM_SIZE (896)
#define DEVICE_BASE_PARAM_ADDR (USER_DEL_HEAD_ADDR+USER_DEL_HEAD_SIZE)


//表头的索引存储长度及地址
#define RECORD_INDEX_SIZE   (128)
#define RECORD_INDEX_ADDR   (DEVICE_BASE_PARAM_ADDR+DEVICE_BASE_PARAM_SIZE)

//参数的存储地址
#define DEVICE_TEMPLATE_PARAM_SIZE   (1024*2)
#define DEVICE_TEMPLATE_PARAM_ADDR  (RECORD_INDEX_ADDR+RECORD_INDEX_SIZE) //参数存储分配4K空间



#define CARD_NO_DATA_ADDR   0X500000
#define USER_ID_DATA_ADDR   0X900000

#define DATA_SECTOR_NUM     ((USER_ID_DATA_ADDR-CARD_NO_DATA_ADDR)/SECTOR_SIZE)


#define CARD_USER_LEN              (8)
#define FLOOR_ARRAY_LENGTH         (64) //每个普通用户最多64层权限
#define TIME_LENGTH                (10)
#define TIMESTAMP_LENGTH           (10)
#define RESERVE_LENGTH             (4) //预留空间 为了对齐，补足一个扇区可以整除的字节数


#pragma pack(1)
typedef struct USERDATA
{
    uint8_t head;                                   //数据头
    uint8_t authMode;                               //鉴权模式,刷卡=2；QR=7
    uint8_t defaultFloor;                           //默认楼层  
    uint8_t cardState;                              //卡状态 有效/已删除/黑名单/临时卡    
    uint8_t userState;                              //用户状态 有效/已删除
    uint8_t platformType;                           //平台类型，第三方平台、博思高平台等
    uint8_t userId[CARD_USER_LEN+1];                  //用户ID
    uint8_t cardNo[CARD_USER_LEN+1];                  //卡号
    char accessFloor[FLOOR_ARRAY_LENGTH+1];        //楼层权限
    uint8_t startTime[TIME_LENGTH+1];                 //账户有效时间
    uint8_t endTime[TIME_LENGTH+1];                   //账户结束时间    
    uint8_t timeStamp[TIME_LENGTH+1];                 //二维码时间戳
    uint8_t reserve[RESERVE_LENGTH+1];                //预留空间 
    uint8_t crc;                                    //校验值 head~reseve
}USERDATA_STRU;
#pragma pack()


#endif



