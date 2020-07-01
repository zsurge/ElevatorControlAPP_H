/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : test.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��28��
  ����޸�   :
  ��������   : ����ģ�����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��28��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malloc.h"
#include "tool.h"
//������ͷ�ļ�
#include "bsp.h"


#define LOG_TAG    "TEST"
#include "elog.h"

#define HEAD_MAX_NUM 30000

static char *createRandom(void);
static char *createRandom(void)
{ 
    char ret[8] = {0};  
    uint32_t num = 0;
//    srand(gRandomNum);
    num = rand()%30207068;      //����һ��0-32768�������

    sprintf(ret,"%08d",num);
    return ret;    
}




void wirteHeadTest(void)
{
    int i = 0;
    uint8_t ret = 1;
    uint32_t  headIndex = 0;
    USERDATA_STRU tempUserData = {0};
    memset(&tempUserData,0x00,sizeof(USERDATA_STRU));

    memcpy(tempUserData.userId,"12345678",8)     ;
    tempUserData.accessFloor[0] = 15;
    tempUserData.accessFloor[1] = 16;
    tempUserData.defaultFloor = 15;
    memcpy(tempUserData.startTime,"2020-01-01",10);
    memcpy(tempUserData.startTime,"2029-12-31",10);
    tempUserData.cardState = 1;
    
    for(i=0;i<HEAD_MAX_NUM;i++)
    {
        memcpy(tempUserData.cardNo,createRandom(),8);
//        ret = writeUserData(tempUserData,CARD_MODE);

        ret = writeHeader(tempUserData.cardNo,1,&headIndex);

        if(ret != 0)
        {
            printf("i=%d",i);
            break;
        }  
    }

    printf("wirteHeadTest end \r\n");
}



void searchHeadTest(uint8_t* header)
{
    uint16_t index = 0;
    if(searchFlashIndex(header,1,&index,fIndex) == 1)
    {
        printf("find it! index = %d\r\n",index);
    }
}





