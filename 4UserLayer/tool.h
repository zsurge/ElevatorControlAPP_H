/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : tool.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��20��
  ����޸�   :
  ��������   : ������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��20��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __TOOL_H
#define __TOOL_H
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stdio.h"
#include "stdlib.h" 
#include <stdint.h>



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 
#define DEBUG_PRINT


#define SETBIT(x,y) x|=(1<<y)         //��X�ĵ�Yλ��1    
#define CLRBIT(x,y) x&=~(1<<y)        //��X�ĵ�Yλ��0

#define DBG(...) dbg(__FILE__, __LINE__, __VA_ARGS__)

typedef struct {
    char **str;     //the PChar of string array
    size_t num;     //the number of string
}IString;



/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
uint16_t CRC16_Modbus(uint8_t *pBuf, uint16_t usLen);

uint8_t xorCRC(uint8_t *buf,uint8_t len);

void asc2bcd(unsigned char *bcd_buf, unsigned char *ascii_buf, int conv_len, unsigned char type);
void bcd2asc(unsigned char *ascii_buf,unsigned char * bcd_buf, int conv_len, unsigned char type);

void dbg(const char *file, const long line, const char *format, ...);


void dbh(char *title,char *buf,int len);

uint8_t is_bit_set(uint16_t      value, uint16_t bit);

void int2Str(uint8_t* str, int32_t intnum);
int32_t str2int(const char* str);
uint8_t bcd2byte(uint8_t ucBcd);
void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen);


void split(char *src,const char *separator,char **dest,int *num);

char *strstr_t(const char *str, const char *sub_str);


int setbit(int num, int bit);
int clearbit(int num, int bit);

void calcMac(unsigned char *mac);



void Insertchar(char *src,char*desc,char c);




#endif
