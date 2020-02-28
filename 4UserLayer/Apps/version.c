/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Version.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��8��21��
  ����޸�   :
  ��������   : ʵ�ְ汾����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��8��21��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "version.h" 
#include "tool.h"
#include "string.h"
#include "stdio.h"
#include "lwip_comm.h" 

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static uint8_t * GetDevSn(void);
static uint8_t * GetDevIP(void);
    
const uint8_t VersionString[] = "1.0.1";
const uint8_t Hardversion[] = "1.0.1";
const uint8_t SoftBulidDate[] = __DATE__ " " __TIME__;
const uint8_t DevModel[] = "ElevatorControlAPP";
const uint8_t DevBatch[] = "20191202001";


static uint8_t * GetDevSn(void)
{ 
    static unsigned int chipid[3] = { 0 };
    
    static unsigned char id[30] = {0};
    
    static unsigned char id1[10] = { 0 };
    static unsigned char id2[10] = { 0 };
    static unsigned char id3[10] = { 0 };   


    chipid[0] = *(__I uint32_t *)(0x1FFF7A10 + 0x00);
    chipid[1] = *(__I uint32_t *)(0x1FFF7A10 + 0x04);
    chipid[2] = *(__I uint32_t *)(0x1FFF7A10 + 0x08); 

    int2Str(id1, chipid[0]);
	int2Str(id2, chipid[1]);
	int2Str(id3, chipid[2]);

    strcpy((char *)id,(char *)id1);
    strcat((char *)id,(char *)id2);
    strcat((char *)id,(char *)id3);
    return id;  
}


static uint8_t * GetDevIP(void)
{
    static char localIP[16] = {0};
    sprintf(localIP,"%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
    return localIP;
}

const DEVICEINFO_T gDevinfo = {VersionString, Hardversion,SoftBulidDate,DevModel,DevBatch,GetDevSn,GetDevIP};
