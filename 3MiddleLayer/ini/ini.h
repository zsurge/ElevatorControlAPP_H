/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : ini.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��28��
  ����޸�   :
  ��������   : �Բ������в���
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��28��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __INI_H
#define __INI_H

#include "sfud.h"
#include "tool.h"
#include "easyflash.h"
#include "errorcode.h"

typedef struct
{
    uint8_t FunState;
    
    SYSERRORCODE_E (*SaveICParam)(void); //�������
}ICREADER_T;

typedef struct
{
    uint8_t FunState;
    uint8_t LightMode;
    uint8_t ScanMode; 
    uint8_t TimeInterval; 
    
    SYSERRORCODE_E (*SaveQRParam)(void); //�������   
}QRCODE_T;


extern ICREADER_T   gICReaderParam; 
extern QRCODE_T     gQRCodeParam;

void ReadIAP(void);

void RestoreDefaultSetting(void);

void SystemUpdate(void);

SYSERRORCODE_E RecordBootTimes(void);
SYSERRORCODE_E ParseDevParam(uint8_t *ParamBuff);







#endif
