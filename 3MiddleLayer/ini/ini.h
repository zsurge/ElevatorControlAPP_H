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

#include "easyflash.h"
#include "version.h"
#include "string.h"
#include "deviceInfo.h"
#include "tool.h"
#include "pub_data_type.h"
#include "eth_cfg.h"
#include "cJSON.h"
#include "errorcode.h"
#include "calcDevNO.h"
#include "malloc.h"
#include "bsp_MB85RC128.h"
#include "bsp_ds1302.h"




//�豸����״̬
#define ON_LINE                 1
#define OFF_LINE                (-1)

//����״̬��1���ɹ� 0/2ʧ�ܣ�3 QR�豸�ѽ���  
#define CALL_OK                 1
#define CALL_NG                 2
#define CALL_ERR                0
#define QR_DISABLE              3

//�������� 1���� 2����
#define DIRECTION_IN            1
#define DIRECTION_OUT           2

#define MAX_FLOOR               200 //���200�㣬����200�㣬����¥�㴦��

typedef uint8_t(*CallBackParam)(void * struParam,uint8_t mode,uint32_t len,uint32_t addr);

void readTemplateData(void);


void RestoreDefaultSetting(void);

void SystemUpdate(void);

//void readDevState(void);

void SaveDevState(uint32_t     state);

//void ReadLocalDevSn(void);


uint8_t packetPayload(USERDATA_STRU *localUserData,uint8_t *descJson);
//����QRCODE����
char parseQrCode(uint8_t *jsonBuff,USERDATA_STRU *qrCodeInfo);
//����ģ����Ϣ
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff);


void eraseHeadSector(void);
void eraseDataSector(void);
void eraseUserDataAll(void);


//add 2020.07.06
void initTemplateParam(void);
void ClearTemplateParam(void);
uint8_t optTemplateParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr);

void initDevBaseParam(void);
void ClearDevBaseParam(void);
uint8_t optDevBaseParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr);

void initRecordIndex(void);
void ClearRecordIndex(void);

uint8_t optRecordIndex(RECORDINDEX_STRU *recoIndex,uint8_t mode);
void clearTemplateFRAM(void);



#endif
