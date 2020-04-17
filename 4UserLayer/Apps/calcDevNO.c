/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : calcDevNO.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��4��8��
  ����޸�   :
  ��������   : �����豸������Ϊ�ж϶�ά��Ȩ�޵�����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��4��8��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/
/******************************************************************************
90����,ʹ�������ַ�
"!", "#", "$", "%", "&", "(", ")", "*", "+", ",", 
"-", ".", "/", "0", "1", "2", "3", "4", "5", "6", 
"7", "8", "9", ":", ";", "<", "=", ">", "?", "@", 
"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", 
"K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", 
"U", "V", "W", "X", "Y", "Z", "[", "]", "^", "_", 
"`", "a", "b", "c", "d", "e", "f", "g", "h", "i", 
"j", "k", "l", "m", "n", "o", "p", "q", "r", "s", 
"t", "u", "v", "w", "x", "y", "z", "{", "}", "~"

�����ַ�: |"'\

���ӣ�
10���� 0     = 90����  !
10���� 89    = 90����  ~
10���� 90    = 90����  #!
10���� 8099  = 90����  ~~
10���� 8100  = 90����  #!!
10���� 18883 = 90����  $@m
******************************************************************************/



/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include<stdio.h>
#include<string.h>
#include "calcDevNO.h"
#include "eth_cfg.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define DEVICE_NO_FMT       90
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void StrReversal(char *str);
static int findChar(const char acher);


static void StrReversal(char *str)
{
    char tmp;
    int len = strlen(str);
    for(int i=0,j=len-1;i<=j;i++,j--)
    {
        tmp=str[i];
        str[i]=str[j];
        str[j]=tmp;
    }
    str[len] = '\0';
}

//תΪ�ַ���
void myItoa(char *str,int n,int radix)
{
    char chars[] = {'!','#','$','%','&','(',')','*','+',',','-','.','/','0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',']','^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{','}','~'};
    int i=0;

    if(n == 0)
    {
        str[0] = chars[0];
    }
    else
    {
        while(n!=0)
        {
            str[i++]=chars[n%radix];
            n/=radix;
        }
    }
    StrReversal(str);
}

int findChar(const char acher)
{
    int i = 0;
    char chars[] = {'!','#','$','%','&','(',')','*','+',',','-','.','/','0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',']','^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{','}','~'};
    for (i = 0; i < strlen(chars);i++)
    {
        if(acher == chars[i])
            return i;
    }

    return -1;
}

//ת��Ϊ10����
int myAtoi(const char *str)
{
    int total = 0;          //����ת�������ֵ    
    int length = 0;         //��¼�ַ����ĳ���
    //char chars[] = {'!','#','$','%','&','(',')','*','+',',','-','.','/','0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',']','^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{','}','~'};
    const char *p = str;

    char temp = '!';

    if(NULL == p)           //�ж�ָ��ĺϷ���
    {
        printf("error");
        return -1;
    }
    
    while(*p++!='\0')       //�����ַ����ĳ���
    {
        length++;
    }

    p = str;            //����ָ���ַ������׵�ַ


    
    for(int i = 0;i < length;i++)
    {        
        temp = findChar(*p++);

        //if(total != 0)     //�˳��ַ�����ʼ��0�ַ�
        if(total != 0||temp != -1)     //�˳��ַ�����ʼ��0�ַ�
        {
            //temp -= '0';
            total = total*DEVICE_NO_FMT + temp;
        }
    }

    return total;               //����ת�������ֵ
}


unsigned char findDev(unsigned char *str,unsigned char dataFormat)
{
    unsigned char tmpBuff[9] = {0};
    unsigned char isFind = 0;
    int len = strlen((const char*)str);
    int i = 0;
    int devSN = 0;

    int tmpValue = 0;

    memset(tmpBuff,0x00,sizeof(tmpBuff));

    memcpy(tmpBuff,gMqttDevSn.deviceSn,8);

    devSN = atoi((const char*)tmpBuff);

    printf("devSN = %d,str len = %d\r\n",devSN,len);
    char buf[6] = {0};

    for(i=0;i<len/dataFormat;i++)
    {
        memset(buf,0x00,sizeof(buf));
        memcpy(buf,str+i*dataFormat,dataFormat);
        tmpValue = myAtoi(buf);
        printf("i = %d, tmpValue = %d\t",i,tmpValue);      
        
        if(devSN == tmpValue)
        {
            isFind = 1;
        }
    }

    printf("\r\n"); 

    return isFind;
}

//int searchHeaderIndex(unsigned char *str,unsigned char dataFormat )
//{
//    int isFind = 0;
//    
//    switch (dataFormat)
//    {
//        case 1:
//            findDev(str)
//            break;
//        case 2:
//            break;
//        case 3:
//            break;
//        case 4:
//            break;
//        case 5:
//            break;            
//       default:
//            break;
//    }

//    return isFind;
//}


unsigned char *parseAccessFloor(unsigned char *str)
{
    static unsigned char result[64] = {0};;
    char buf[1] = {0};
    int len = strlen((const char*)str);
    int i = 0;

    if(len >64)
    {
        len = 64;
    }

    for(i=0;i<len;i++)
    {
        memset(buf,0x00,sizeof(buf));
        memcpy(buf,str+i,1);
        result[i] = myAtoi(buf); 
    }
    
    return result;    
}



