/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : LocalData.h
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年3月21日
  最近修改   :
  功能描述   : LocalData.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年3月21日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
#ifndef __LOCALDATA_H__
#define __LOCALDATA_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 


#define HEAD_lEN 4               //每条记录占4个字节
#define MAX_HEAD_RECORD     2048 //最大2048条记录
#define SECTOR_SIZE         4096    //每个扇区大小
#define CARD_NO_HEAD_ADDR   0x600000
#define CARD_NO_HEAD_SIZE   (HEAD_lEN*MAX_HEAD_RECORD)
#define USER_ID_HEAD_ADDR   (CARD_NO_HEAD_ADDR+CARD_NO_HEAD_SIZE)
#define USER_ID_HEAD_SIZE   (CARD_NO_HEAD_SIZE)

#define CARD_SECTOR_NUM     (CARD_NO_HEAD_SIZE/SECTOR_SIZE)
#define USER_SECTOR_NUM     CARD_SECTOR_NUM


#define CARD_NO_DATA_ADDR   0X700000
#define USER_ID_DATA_ADDR   0X800000

#define DATA_SECTOR_NUM     ((USER_ID_DATA_ADDR-CARD_NO_DATA_ADDR)/SECTOR_SIZE)

#define CARD_USER_LEN              8
#define FLOOR_ARRAY_LENGTH         16 //每个普通用户最多10个层权限
#define TIME_LENGTH                16
#define RESERVE_LENGTH             32 //预留空间

#define CARD_MODE                   0 //卡模式
#define USER_MODE                   1 //U用户ID模式

#define TABLE_HEAD                 0xAA



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
extern uint16_t gCurCardHeaderIndex;    //卡号索引
extern uint16_t gCurUserHeaderIndex;    //用户ID索引
extern uint16_t gCurRecordIndex;


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
typedef struct HEADER
{    
    uint8_t value[HEAD_lEN];     //表头的值
}HEADER_STRU;


//HEADER_STRU cardNoHeader,userIdHeader;

#pragma pack(1)
typedef struct USERDATA
{
    uint8_t head;                                   //数据头
    uint8_t authMode;                               //鉴权模式,刷卡=2；QR=7
    uint8_t defaultFloor;                           //默认楼层  
    uint8_t state;                                  //用户状态 有效/无效/黑名单/临时卡    
    uint8_t userId[CARD_USER_LEN];                  //用户ID
    uint8_t cardNo[CARD_USER_LEN];                  //卡号
    uint8_t accessFloor[FLOOR_ARRAY_LENGTH];        //楼层权限
    uint8_t startTime[TIME_LENGTH];                 //账户有效时间
    uint8_t endTime[TIME_LENGTH];                   //账户结束时间    
    uint8_t timeStamp[TIME_LENGTH];                 //二维码时间戳
    uint8_t reserve[RESERVE_LENGTH];                //预留空间
    uint8_t crc;                                    //校验值 head~reseve
}USERDATA_STRU;
#pragma pack()



/*
typedef struct USERSTATE
{
    uint8_t isInvalid;
    uint8_t isValid;
    uint8_t isTemporary;
    uint8_t isBlackList;
}USERSTATE_STRU;
*/

void eraseHeadSector(void);
void eraseDataSector(void);
void eraseUserDataAll(void);
uint8_t writeHeader(uint8_t* header,uint8_t mode);
uint8_t searchHeaderIndex(uint8_t* header,uint8_t mode,uint16_t *index);
uint8_t writeUserData(USERDATA_STRU userData,uint8_t mode);
uint8_t readUserData(uint8_t* header,uint8_t mode,USERDATA_STRU *userData);






#endif /* __LOCALDATA_H__ */
