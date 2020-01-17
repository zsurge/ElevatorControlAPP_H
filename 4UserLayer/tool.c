/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : tool.c
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

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "tool.h"
#include "stdarg.h"
#include "stdio.h"
#include <string.h>


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
// CRC ��λ�ֽ�ֵ��
const uint8_t CrcHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC ��λ�ֽ�ֵ��
const uint8_t CrcLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/


/*************************************************************************************
*   �� �� ��: CRC16_Modbus
*   ����˵��: ����CRC�� ����ModbusЭ�顣
*   ��    ��: pBuf : ����У�������
*             usLen : ���ݳ���
*   �� �� ֵ: 16λ����ֵ�� ����Modbus ���˽�����ֽ��ȴ��ͣ����ֽں��͡�
*
*   ���п��ܵ�CRCֵ����Ԥװ���������鵱�У������㱨������ʱ���Լ򵥵��������ɣ�
*   һ�����������16λCRC�������256�����ܵĸ�λ�ֽڣ���һ�����麬�е�λ�ֽڵ�ֵ��
*   ������������CRC�ķ�ʽ�ṩ�˱ȶԱ��Ļ�������ÿһ�����ַ��������µ�CRC����ķ�����
*
*  ע�⣺�˳����ڲ�ִ�и�/��CRC�ֽڵĽ������˺������ص����Ѿ�����������CRCֵ��Ҳ����˵��
*        �ú����ķ���ֵ����ֱ�ӷ���
*        �ڱ������ڷ��ͣ�
************************************************************************************/
uint16_t CRC16_Modbus(uint8_t *pBuf, uint16_t usLen)
{
    uint8_t ucCRCHi = 0xFF; /* ��CRC�ֽڳ�ʼ�� */
    uint8_t ucCRCLo = 0xFF; /* ��CRC �ֽڳ�ʼ�� */
    uint16_t  usIndex;  /* CRCѭ���е����� */

    while (usLen--)
    {
        usIndex = ucCRCLo ^ *pBuf++; /* ����CRC */
        ucCRCLo = ucCRCHi ^ CrcHi[usIndex];
        ucCRCHi = CrcLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8 | ucCRCLo);
}


/*****************************************************************************
 �� �� ��  : asc2bcd
 ��������  :     ascii��תbcd�� "123456"=====>0x12,0x34,0x56
 �������  :     unsigned char *bcd_buf     ���BCD���λ��   
             unsigned char *ascii_buf   Ҫת��ascii�ַ�����λ��
             int conv_len               ascii�ַ����ĳ���
             unsigned char type         0:��0,"12345"==>0x12,0x34,0x50
                                        1:ǰ��0,"12345"==>0x01,0x23,0x45
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��7��5��
    ��    ��   : �Ŷ�
void asc2bcd(unsigned char *bcd_buf, unsigned char *ascii_buf, int conv_len, unsigned char type)
    �޸�����   : �����ɺ���

*****************************************************************************/
void asc2bcd(unsigned char *bcd_buf, unsigned char *ascii_buf, int conv_len, unsigned char type)
{
    int    cnt;
    char   ch, ch1;

    if (conv_len&0x01 && type ) ch1=0;
    else ch1=0x55;
    for (cnt=0; cnt<conv_len; ascii_buf++, cnt++)
    {
        if (*ascii_buf >= 'a' ) ch = *ascii_buf-'a' + 10;
        else if ( *ascii_buf >= 'A' ) ch =*ascii_buf- 'A' + 10;
        else if ( *ascii_buf >= '0' ) ch =*ascii_buf-'0';
        else ch = 0;
        if (ch1==0x55) ch1=ch;
        else {
            *bcd_buf++=( ch1<<4 )| ch;
            ch1=0x55;
        }
    }
    if (ch1!=0x55) *bcd_buf=ch1<<4;
}


/*****************************************************************************
 �� �� ��  : bcd2asc
 ��������  : hex ת ascii ��  \x12\x34\x56 ===> "123456"
 �������  :   unsigned char *ascii_buf  ���ascii��ֵλ��
           unsigned char * bcd_buf   Ҫת����hex�ַ���
           int conv_len              Ҫת����hex�ַ���*2
           unsigned char type        0:��ɾ1��0x12,0x34,0x56(����=5) ===>"12345"
                                     1:ǰɾ1��0x12,0x34,0x56(����=5) ===>"23456"
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��7��5��
    ��    ��   : �Ŷ�
void bcd2asc(unsigned char *ascii_buf,unsigned char * bcd_buf, int conv_len, unsigned char type)
    �޸�����   : �����ɺ���

*****************************************************************************/
void bcd2asc(unsigned char *ascii_buf,unsigned char * bcd_buf, int conv_len, unsigned char type)
{
    int cnt=0;

    if (conv_len&0x01 && type) {cnt=1; conv_len++;}
    else cnt=0;
    for (; cnt<conv_len; cnt++, ascii_buf++){
        *ascii_buf = ((cnt&0x01) ? (*bcd_buf++&0x0f) : (*bcd_buf>>4));
        *ascii_buf += ((*ascii_buf>9) ? ('A'-10) : '0');
    }
}


/*****************************************************************************
 �� �� ��  : xorCRC
 ��������  : �������У��ֵ
 �������  : uint8_t *buf  Ҫ����Ļ�����ͷ
           uint8_t len Ҫ����ĳ���  
 �������  : ��
 �� �� ֵ  : CRCֵ
 
 �޸���ʷ      :
  1.��    ��   : 2019��7��5��
    ��    ��   : �Ŷ�
uint8_t xorCRC(uint8_t *buf,uint8_t len)
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t xorCRC(uint8_t *buf,uint8_t len)
{
    uint8_t i = 0;
    uint8_t bcc = 0;
    bcc = buf[0];

    for(i=1;i<len;i++)
    {
        bcc ^= buf[i];
    }

    return bcc;
}
    
/*****************************************************************************
 �� �� ��  : is_bit_set
 ��������  : �ж�value�ĵ�bitλ�Ƿ�Ϊ1
 �������  : uint16_t      value  
             uint16_t bit         
 �������  : ��
 �� �� ֵ  : 1 ��bitΪ1
             0 ��bitΪ0
 
 �޸���ʷ      :
  1.��    ��   : 2019��7��23��
    ��    ��   : �Ŷ�
uint8_t is_bit_set(uint16_t      value, uint16_t bit)
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t is_bit_set(uint16_t      value, uint16_t bit)
{
    if (((value >> bit) & 1) == 1)
    {
        return 1;
    }

    return 0; 
}

void dbg(const char *file, const long line, const char *format, ...)
{
    #ifdef DEBUG_PRINT
    va_list args;
	char debug_buf[255] = {0};	
    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[DEBUG](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(debug_buf, sizeof(debug_buf), format, args);
    printf("%s\n", debug_buf);
    va_end(args);
    #endif

}

void dbh(char *title,char *buf,int len)
{
    #ifdef DEBUG_PRINT
    int i = 0;
    printf("<%s,len=%d>:<",title,len);
    for(i=0;i<len;i++)
    {
        printf("%02x ",buf[i]);
    }
    printf(" >\r\n");   
    #endif
}


/*****************************************************************************
 �� �� ��  : Int2Str
 ��������  : ����ת��Ϊ�ַ���  ��int a = 3008 ===>  "3008"
 �������  : int32_t intnum                
 �������  : uint8_t* str
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��7��5��
    ��    ��   : �Ŷ�
    void Int2Str(uint8_t* str, int32_t intnum)
    �޸�����   : �����ɺ���

*****************************************************************************/
void int2Str(uint8_t* str, int32_t intnum)
{
	uint32_t i, Div = 1000000000, j = 0, Status = 0;

	for (i = 0; i < 10; i++)
	{
		str[j++] = (intnum / Div) + 48;

		intnum = intnum % Div;
		Div /= 10;
		if ((str[j - 1] == '0') & (Status == 0))
		{
			j = 0;
		}
		else
		{
			Status++;
		}
	}
}

/*****************************************************************************
 �� �� ��  : str2int
 ��������  : ascii���ʽ��hexstringתΪ���� "1234" ===> 1234
 �������  : const char* str  
 �������  : ��
 �� �� ֵ  : �����ַ���������ֵ
 
 �޸���ʷ      :
  1.��    ��   : 2019��7��5��
    ��    ��   : �Ŷ�
    int32_t str2int(const char* str)
    �޸�����   : �����ɺ���

*****************************************************************************/
int32_t str2int(const char* str)
{
	int32_t temp = 0;
	const char* ptr = str;			//ptr����str�ַ�����ͷ  
	if (*str == '-' || *str == '+')	//�����һ���ַ��������ţ�
	{								//���Ƶ���һ���ַ�
		str++;
	}
	while (*str != 0)
	{
		if ((*str < '0') || (*str > '9'))  //�����ǰ�ַ���������
		{					                //���˳�ѭ��
			break;
		}
		temp = temp * 10 + (*str - '0');    //�����ǰ�ַ��������������ֵ
		str++;                              //�Ƶ���һ���ַ�
	}
	if (*ptr == '-')                        //����ַ������ԡ�-����ͷ����ת�������෴��
	{
		temp = -temp;
	}

	return temp;
}

/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - ���Ŀ���ַ���
// [IN] pbSrc - ����16����������ʼ��ַ
// [IN] nLen - 16���������ֽ���
// return value: 
// remarks : ��16������ת��Ϊ�ַ���
*/
void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen)
{
	char ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = '\0';
}



/*****************************************************************************
 �� �� ��  : split
 ��������  : �ַ����ָ�
 �������  : char *src       Դ�ַ������׵�ַ(buf�ĵ�ַ)        
             const char *separator  ָ���ķָ��ַ�
             char **dest   �������ַ���������         
             int *num    �ָ�����ַ����ĸ���           
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��12��30��
    ��    ��   : �Ŷ�
void split(char *src,const char *separator,char **dest,int *num) 
    �޸�����   : �����ɺ���

*****************************************************************************/
void split(char *src,const char *separator,char **dest,int *num) 
{

     char *pNext;
     int count = 0;
     if (src == NULL || strlen(src) == 0) //�������ĵ�ַΪ�ջ򳤶�Ϊ0��ֱ����ֹ 
        return;
     if (separator == NULL || strlen(separator) == 0) //��δָ���ָ���ַ�����ֱ����ֹ 
        return;
     pNext = (char *)strtok(src,separator); //����ʹ��(char *)����ǿ������ת��(��Ȼ��д�еı������в������ָ�����)
     while(pNext != NULL) {
          *dest++ = pNext;
          ++count;
         pNext = (char *)strtok(NULL,separator);  //����ʹ��(char *)����ǿ������ת��
    }  
    *num = count;
} 	



char *strstr_t(const char *str, const char *sub_str)
{   
    const char *str_local = NULL;
    const char *sub_str_local = NULL;
 
    if(!str || !sub_str)
    {   
        printf("fun:%s param is error\n", __FUNCTION__);
        return NULL;
    }
 
    while(*str)
    {
        str_local = str;
        sub_str_local = sub_str;
 
        do
        {
            if(*sub_str_local == '\0')
            {   
                return (char *)str;
            }
        }while(*str_local++ == *sub_str_local++);
        str += 1;
    }
    
    return NULL;
}

int setbit(int num, int bit) 
{
	return num |= (1 << bit);
}

int clearbit(int num, int bit) 
{
	return num &= ~(1 << bit);
}

void calcMac(unsigned char *mac)
{
    uint16_t crc_value = 0;
    unsigned int chipid[3] = { 0 };   
    unsigned char buf[6] = {0x02,0x00,0x00,0x00,0x00,0x00};  
    char id[30] = {0};    
    
    char id1[10] = { 0 };
    char id2[10] = { 0 };
    char id3[10] = { 0 };  
    
    chipid[0] = *(volatile uint32_t *)(0x1FFF7A10 + 0x00);
    chipid[1] = *(volatile uint32_t *)(0x1FFF7A10 + 0x04);
    chipid[2] = *(volatile uint32_t *)(0x1FFF7A10 + 0x08); 

    sprintf(id1,"%d",chipid[0]);
    sprintf(id2,"%d",chipid[1]);
    sprintf(id3,"%d",chipid[2]);

    strcpy(id,(char *)id1);
    strcat(id,(char *)id2);
    strcat(id,(char *)id3);    
    
    crc_value = CRC16_Modbus((uint8_t *)id, strlen((const char*)id));

    buf[3] =  crc_value & 0xff;
    buf[4] =  crc_value>>8;
    buf[5] =  chipid[0]&0xff;

    memcpy(mac,buf,6);

}


void Insertchar(char *src,char*desc,char c)
{
    while (*src)
    {
        (*desc++) = (*src++);
        (*desc++) = (*src++);
        (*desc++) = c;
    }
    *desc = '\0';
}



