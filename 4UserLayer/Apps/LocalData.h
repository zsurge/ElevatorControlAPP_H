/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : LocalData.h
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��3��21��
  ����޸�   :
  ��������   : LocalData.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��3��21��
    ��    ��   :  
    �޸�����   : �����ļ�

  FLASH����
  0-0x200000����������
  0x300000    ���bin�ļ�
  0x400000    ����Ѵ洢���ţ���ɾ�����ţ��Ѵ洢�û�ID����ɾ���û�ID��
  0x500000    ��ſ�������
  0x900000    ����û�����
  0x1300000   Ԥ��

FLASH����˼·��
����0.�ȶ�ȡ��ɾ������������Ϊ�㣬�����ɾ�����һ������ֵ��ֵ������������д���ţ�
      ��ɾ������ֵ�Լ�������ִ�в���2��
    1.��ɾ������Ϊ�㣬�Կ���Ϊ�������洢��FLASH�У���Ϊ������ÿ����һ�����ţ�����������
    2.���Ź�������Ϊ�̶����ݳ��ȣ��洢ʱ��д�������ݵ�ַ+ƫ����(����*�̶�����)
ɾ��1.���ҿ��ţ����У����¼�ÿ�����ֵ���洢����ɾ���ռ��ڣ�����ɾ������ֵ����,
      ���ޣ��򷵻�δ�ҵ���
�ģ�1.���ҿ��ţ�ȷ��λ�ã��޸���Ӧ��ֵ����д�뵽FLASH
�飺1.���ҿ��ţ�ȷ��λ�ã���������Ӧ��ֵ  

******************************************************************************/
#ifndef __LOCALDATA_H__
#define __LOCALDATA_H__

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stdint.h" 
#include "pub_data_type.h"

#define CARD_MODE                   1 //��ģʽ
#define USER_MODE                   2 //�û�IDģʽ
#define CARD_DEL_MODE               3 //ɾ����ģʽ
#define USER_DEL_MODE               4 //ɾ���û�IDģʽ


////���ÿ�״̬Ϊ0��ɾ����
#define CARD_DEL                    0
#define CARD_VALID                  1
#define USER_DEL                    CARD_DEL
#define USER_VALID                  CARD_VALID
#define TABLE_HEAD                  0xAA

#define NO_FIND_HEAD                (-1)


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
//extern volatile uint16_t gCurCardHeaderIndex;    //��������
//extern volatile uint16_t gCurUserHeaderIndex;    //�û�ID����
//extern volatile uint16_t gDelCardHeaderIndex;    //��ɾ����������
//extern volatile uint16_t gDelUserHeaderIndex;    //��ɾ���û�ID����
//extern volatile uint16_t gCurRecordIndex;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

typedef enum 
{
  ISFIND_NO = 0,
  ISFIND_YES 
}ISFIND_ENUM;

//��ͷ����
typedef union
{
	uint32_t id;        //����
	uint8_t sn[4];    //���Ű��ַ�
}HEADTPYE;

typedef struct CARDHEADINFO
{
    HEADTPYE headData;  //����
    uint32_t flashAddr; //��FLASH�е�����,���ַ=����*�̶��������ݳ���+����ַ 
}HEADINFO_STRU;

extern HEADINFO_STRU gSectorBuff[512];






uint8_t writeUserData(USERDATA_STRU *userData,uint8_t mode);

char readUserData(uint8_t* header,uint8_t mode,USERDATA_STRU *userData);


uint8_t delUserData(uint8_t *header,uint8_t mode);

uint8_t writeDelHeader(uint8_t* header,uint8_t mode);

void TestFlash(uint8_t mode);


//add 2020.07.14
int readHead(HEADINFO_STRU *head,uint8_t mode);

void sortHead(HEADINFO_STRU *head,int length);

//���������������
void manualSortCard(void);


void addHead(uint8_t *head,uint8_t mode);


int delHead(uint8_t *headBuff,uint8_t mode);


#endif /* __LOCALDATA_H__ */
