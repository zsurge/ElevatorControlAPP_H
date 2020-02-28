/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : comm.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��18��
  ����޸�   :
  ��������   : ����ͨѶЭ�����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __COMM_H
#define __COMM_H

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "errorcode.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


extern QueueHandle_t xTransQueue;

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MQTT_MAX_LEN 1024*2


#define AUTH_MODE_CARD      2
#define AUTH_MODE_REMOTE    3
#define AUTH_MODE_UNBIND    4
#define AUTH_MODE_BIND      5

#define AUTH_MODE_QR        7



#define QUEUE_BUF_LEN   64



#pragma pack(1)
typedef struct
{
    uint8_t data[QUEUE_BUF_LEN];         //��Ҫ���͸�������������
    uint8_t authMode;                     //��Ȩģʽ,ˢ��=2��QR=7
    uint8_t dataLen;                     //���ݳ���    
}READER_BUFF_T;
#pragma pack()

extern READER_BUFF_T gReaderMsg;



extern int gConnectStatus;
extern int  gMySock;

//������Ϣ��������
int PublishData(uint8_t *payload_out,uint16_t payload_out_len); 

SYSERRORCODE_E exec_proc ( char* cmd_id, uint8_t *msg_buf );


#endif

