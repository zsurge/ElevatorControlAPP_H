
#ifndef __VERSION_H
#define __VERSION_H

#include "stm32f4xx_conf.h"

typedef uint8_t*(*CallBackFun)(void); //��ȡSN�Ļص�


typedef struct
{
	const uint8_t *SoftwareVersion;       //����汾��
    const uint8_t *HardwareVersion;  //Ӳ���汾    	
	const uint8_t *BulidDate;         //����ʱ��
    const uint8_t *Model;        //�豸�ͺ�
    const uint8_t *ProductBatch; //��������
    CallBackFun GetSn; //��ȡ��ǰ�豸SN    
    CallBackFun GetIP; //��ȡ��ǰ�豸IP  
}DEVICEINFO_T;

extern const DEVICEINFO_T gDevinfo;

#endif

