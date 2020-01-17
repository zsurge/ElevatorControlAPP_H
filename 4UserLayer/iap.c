/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : iap.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年6月4日
  最近修改   :
  功能描述   : 串口升级程序
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月4日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "iap.h"
#include <stdio.h>
#include "easyflash.h"
#include "sfud.h"
#include "stmflash.h"
#include "tool.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define READ_BIN_BUFFER_SIZE 2048
#define MCU_FLASH_START_ADDR 0x10000

typedef  void (*pFunction)(void); 
pFunction Jump_To_Application;

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/




uint8_t IAP_JumpToApplication(void)
//void IAP_JumpToApplication(void)
{
    uint32_t JumpAddress = 0;//跳转地址
    
    if ( ( (* (volatile uint32_t*) APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    {
		/* Jump to user application */
		JumpAddress = *(volatile uint32_t*) (APPLICATION_ADDRESS + 4);
		Jump_To_Application = (pFunction) JumpAddress;

		/* Initialize user application's Stack Pointer */
		__set_PSP(*(volatile uint32_t*) APPLICATION_ADDRESS);

		Jump_To_Application();	
        
		while(1);
    }    
    else
    {
        return 1;
    }
}

//通过串口接收文件并写入外部FLASH
int32_t IAP_DownLoadToFlash(void)
{
    uint32_t file_total_size = 0 ;
    uint8_t buf_1k[1024] = {0};
    char *tmp;
    uint32_t tmp_total_size = 0 ;  
    
    
    file_total_size = Ymodem_Receive(&buf_1k[0],APPLICATION_ADDRESS);

    if (file_total_size > 0)
	{
        //获取文件大小
        tmp = ef_get_env((const char * )"FileSize");
        tmp_total_size = str2int((const char *)tmp);

        if(tmp_total_size != file_total_size)
        {
            ef_set_env("WSPIFLASH",W_SPI_FLASH_NEED);     

            return 0;
        }

		DBG("Name: %s, Size: %s Bytes\r\n",(char*)FileName,(char*)tmp); 


        if(ef_set_env("WSPIFLASH",W_SPI_FLASH_OK) == EF_NO_ERR)
        {
            return file_total_size;
        }
	}

    return 0;
}





int32_t IAP_DownLoadToSTMFlash(int32_t filesize)
{
	int32_t sent=0;
	int32_t rest=filesize;
    uint8_t read_buf[READ_BIN_BUFFER_SIZE];
    uint32_t flash_addr = MCU_FLASH_START_ADDR;
    uint32_t  user_app_addr,ramsource;
    size_t DATAS_LENGTH = READ_BIN_BUFFER_SIZE;
    sfud_err result = SFUD_SUCCESS;
    const sfud_flash *flash = sfud_get_device_table() + 0;

    
    user_app_addr = APPLICATION_ADDRESS;
    
    //清除用户APP区域
    STM_FLASH_Erase ( user_app_addr );    

//    //从外部FLASH读取数据
    while( sent < filesize)
    {
        memset (read_buf, 0, sizeof (read_buf) );
        result = sfud_read(flash, flash_addr, DATAS_LENGTH, read_buf);     
        
        if(result != SFUD_SUCCESS)
        {
            //出错
            return 3;        
        }

        ramsource = ( uint32_t ) read_buf;
        
        if ( STM_FLASH_Write (&user_app_addr, ( uint32_t* ) ramsource, ( uint16_t ) DATAS_LENGTH/4 )  != 0 ) //直接写片内FLASH
        {
            //出错
            return 2;
        }  

        sent += DATAS_LENGTH;
        rest -= DATAS_LENGTH;

        if (rest >= READ_BIN_BUFFER_SIZE) 
        {
            DATAS_LENGTH = READ_BIN_BUFFER_SIZE;
        }
        else 
        {
            //因为在写FLASH的时候补齐1024了，所以这里需要写1024个字节
            DATAS_LENGTH = READ_BIN_BUFFER_SIZE;
            //DATAS_LENGTH = rest; //只写剩余数据
        }

        flash_addr += DATAS_LENGTH;        
    } 
    
    return 1;    
}





