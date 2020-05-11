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
#include "templateprocess.h"
#include "tool.h"
#include "LocalData.h"
#include "eth_cfg.h"
#include "cJSON.h"
#include "errorcode.h"
#include "calcDevNO.h"

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


void readTemplateData(void);

void readCardAndUserIdIndex(void);

void RestoreDefaultSetting(void);

void SystemUpdate(void);

void readDevState(void);

void SaveDevState(char state);

void ReadLocalDevSn(void);


uint8_t packetPayload(USERDATA_STRU *localUserData,uint8_t *descJson);
//����QRCODE����
uint8_t parseQrCode(uint8_t *jsonBuff,USERDATA_STRU *qrCodeInfo);


#endif
