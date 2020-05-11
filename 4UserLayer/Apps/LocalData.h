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

  FLASH划分
  0-0x200000：参数区域
  0x300000    存放bin文件
  0x400000    存放已存储卡号，已删除卡号，已存储用户ID，已删除用户ID，
  0x500000    存放卡号数据
  0x900000    存放用户数据
  0x1300000   预留

FLASH操作思路：
增：0.先读取已删除索引，若不为零，则把已删除最后一个索引值赋值给卡号索引并写卡号，
      已删除索引值自减，并且执行步骤2；
    1.若删除索引为零，以卡号为索引，存储在FLASH中，做为索引表，每增加一个卡号，索引自增；
    2.卡号关连内容为固定数据长度，存储时，写卡号数据地址+偏移量(索引*固定长度)
删：1.查找卡号，若有，查记录该卡索引值，存储在已删除空间内，且已删除索引值自增,
      若无，则返回未找到；
改：1.查找卡号，确定位置，修改相应的值，并写入到FLASH
查：1.查找卡号，确定位置，并返回相应的值  

******************************************************************************/
#ifndef __LOCALDATA_H__
#define __LOCALDATA_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 

#define HEAD_lEN 4                  //每条记录占4个字节
#define MAX_HEAD_RECORD     32768   //最大32768条记录
#define SECTOR_SIZE         4096    //每个扇区大小


#define CARD_NO_HEAD_SIZE   (HEAD_lEN*MAX_HEAD_RECORD)  //128K
#define USER_ID_HEAD_SIZE   (CARD_NO_HEAD_SIZE)
#define CARD_DEL_HEAD_SIZE  (CARD_NO_HEAD_SIZE)
#define USER_DEL_HEAD_SIZE  (CARD_NO_HEAD_SIZE)


#define CARD_HEAD_SECTOR_NUM     (CARD_NO_HEAD_SIZE/SECTOR_SIZE) //32个扇区
#define USER_HEAD_SECTOR_NUM     (CARD_HEAD_SECTOR_NUM)
#define CARD_HEAD_DEL_SECTOR_NUM (CARD_HEAD_SECTOR_NUM)
#define USER_HEAD_DEL_SECTOR_NUM (CARD_HEAD_SECTOR_NUM)

#define HEAD_NUM_SECTOR     (SECTOR_SIZE/HEAD_lEN) //1024个


#define CARD_NO_HEAD_ADDR   0x400000
#define USER_ID_HEAD_ADDR   (CARD_NO_HEAD_ADDR+CARD_NO_HEAD_SIZE)
#define CARD_DEL_HEAD_ADDR  (USER_ID_HEAD_ADDR+USER_ID_HEAD_SIZE)
#define USER_DEL_HEAD_ADDR  (CARD_DEL_HEAD_ADDR+CARD_DEL_HEAD_SIZE)


#define CARD_NO_DATA_ADDR   0X500000
#define USER_ID_DATA_ADDR   0X900000

#define DATA_SECTOR_NUM     ((USER_ID_DATA_ADDR-CARD_NO_DATA_ADDR)/SECTOR_SIZE)


#define CARD_USER_LEN              (8)
#define FLOOR_ARRAY_LENGTH         (64) //每个普通用户最多64层权限
#define TIME_LENGTH                (10)
#define TIMESTAMP_LENGTH           (10)
#define RESERVE_LENGTH             (4) //预留空间 为了对齐，补足一个扇区可以整除的字节数

#define CARD_MODE                   1 //卡模式
#define USER_MODE                   2 //用户ID模式
#define CARD_DEL_MODE               3 //删除卡模式
#define USER_DEL_MODE               4 //删除用户ID模式


////设置卡状态为0，删除卡
#define CARD_DEL                    0
#define CARD_VALID                  1
#define USER_DEL                    CARD_DEL
#define USER_VALID                  CARD_VALID
#define TABLE_HEAD                  0xAA



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
extern uint16_t gDelCardHeaderIndex;    //已删除卡号索引
extern uint16_t gDelUserHeaderIndex;    //已删除用户ID索引
extern uint16_t gCurRecordIndex;


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
typedef struct HEADER
{    
    uint8_t value[HEAD_lEN];     //表头的值
}HEADER_STRU;

typedef enum 
{
  ISFIND_NO = 0,
  ISFIND_YES 
}ISFIND_ENUM;



//HEADER_STRU cardNoHeader,userIdHeader;

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

extern USERDATA_STRU gUserDataStru;

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
uint8_t writeHeader(uint8_t* header,uint8_t mode,uint32_t *headIndex);
ISFIND_ENUM searchHeaderIndex(uint8_t* header,uint8_t mode,uint16_t *index);
uint8_t writeUserData(USERDATA_STRU userData,uint8_t mode);
uint8_t readUserData(uint8_t* header,uint8_t mode,USERDATA_STRU *userData);
uint8_t modifyUserData(USERDATA_STRU userData,uint8_t mode);
uint8_t delUserData(uint8_t *header,uint8_t mode);

uint8_t writeDelHeader(uint8_t* header,uint8_t mode);



void TestFlash(uint8_t mode);






#endif /* __LOCALDATA_H__ */
