/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : ymodem.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��3��
  ����޸�   :
  ��������   : YmodemЭ��
            IAP_PORT��ymodem.h�ж���            
            IAP_PORT����:USART1, 115200bps, 8, n, 1
            IAP_PORT��Ҫ������ʼ��
  �����б�   :
              Receive_Byte
              Receive_Packet
              Send_Byte
              SerialKeyPressed
              Ymodem_Receive
  �޸���ʷ   :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "ymodem.h"
#include "easyflash.h"

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

uint8_t FileName[FILE_NAME_LENGTH] = {0};
static uint32_t update_file_cur_size;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
//check if one byte rx successfully
//key: variabl to store rx data
//return: 1=success, 0=fail

static uint32_t SerialKeyPressed ( uint8_t* key )
{
	if ( USART_GetFlagStatus ( IAP_PORT, USART_FLAG_RXNE ) != RESET )
	{
		*key = ( uint8_t ) IAP_PORT->DR;
		return 1;
	}
	else
	{
		return 0;
	}
}


//Rx a byte from sender
//c: variable to store data
//timeout: read time out
//return: 0=success, -1=fail
static  int32_t Receive_Byte ( uint8_t* c, uint32_t timeout )
{
	while ( timeout-- > 0 )
	{
		if ( SerialKeyPressed ( c ) == 1 )
		{
			return 0;
		}
	}
	return -1;
}

//send a byte via IAP_PORT
//c: byte to send
//return: 0
static uint32_t Send_Byte ( uint8_t c )
{
	USART_SendData ( IAP_PORT, c );
	while ( USART_GetFlagStatus ( IAP_PORT, USART_FLAG_TXE ) == RESET );
    //USART_ClearFlag (USART1, USART_FLAG_TXE);    
	return 0;
}


//Rx a packet from sender
//data: pointer to store rx data
//length: packet length
//timeout: rx time out
//return:0=normally return
//      -1=timeout or packet error
//       1=abort by user
static int32_t Receive_Packet ( uint8_t* data, int32_t* length, uint32_t timeout )
{
	uint16_t i, packet_size;
	uint8_t c;

	*length = 0;
	if ( Receive_Byte ( &c, timeout ) != 0 )
	{
		return -1;  //��ʱ����-1
	}
    
	switch ( c )        //c��ʾ���յ������ݵĵ�һ���ֽ�
	{
		case SOH:       //���ݰ���ʼ
			packet_size = PACKET_SIZE;
			break;
		case STX:       //���Ŀ�ʼ
			packet_size = PACKET_1K_SIZE;
			break;
		case EOT:       //���ݰ�����
			return 0;
		case CA:        //���ͷ���ֹ����
			if ( ( Receive_Byte ( &c, timeout ) == 0 ) && ( c == CA ) )
			{
				*length = -1;
				return 0;
			}
			else
			{
				return -1;
			}
		case ABORT1:    //A
		case ABORT2:    //a
			return 1;
		default:
			return -1;
	}
    
	*data = c;
	for ( i = 1; i < ( packet_size + PACKET_OVERHEAD ); i ++ )  //��ȡʣ�µ����ݣ����ֽ�Ϊ��λ��
	{
		if ( Receive_Byte ( data + i, timeout ) != 0 )  
		{
			return -1;  //�������ݳ�ʱ
		}
	}
//  if ((uint8_t)data[PACKET_SEQNO_INDEX] != ((uint8_t)(data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
//  {
//    return -1;
//  }
	*length = packet_size;

    
    
	return 0;
}


/*****************************************************************************
 �� �� ��  : Ymodem_Receive
 ��������  : ʹ��ymodemЭ������ļ�
 �������  : uint8_t* buf ���ݴ洢ָ��     
           uint32_t appaddr  Ҫд��ĵ�ַ
 �������  : ��
 �� �� ֵ  : size �����ļ���С
 
 �޸���ʷ      :
  1.��    ��   : 2019��6��3��
    ��    ��   : �Ŷ�

    �޸�����   : �����ɺ���

*****************************************************************************/
int32_t Ymodem_Receive ( uint8_t* buf, uint32_t appaddr )
{
	uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], *file_ptr, *buf_ptr,flag_EOT;
	int32_t i, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;
	char file_size[FILE_SIZE_LENGTH];
	//uint32_t flashdestination, ramsource;   

	//Initialize flashdestination variable
	//flashdestination = appaddr;
	
	flag_EOT = 0;
	for ( session_done = 0, errors = 0, session_begin = 0; ; )
	{
		for ( packets_received = 0, file_done = 0, buf_ptr = buf; ; )
		{
			switch ( Receive_Packet ( packet_data, &packet_length, NAK_TIMEOUT ) )//�������ݰ�
			{
				case 0:
					errors = 0;
					switch ( packet_length )
					{
						case - 1:   //���Ͷ���ֹ����
							Send_Byte ( ACK );
							return 0;

						case 0: //���ս�������մ���
							if ( flag_EOT==0 ) //first EOT
							{
								Send_Byte ( NACK );
								flag_EOT = 1;
							}
							else if ( flag_EOT==1 ) //second EOT
							{
								Send_Byte ( ACK );
								Send_Byte ( 'C' );
								file_done = 1;
							}
							break;
						/* Normal packet */
						default:     //����������
							if ( ( packet_data[PACKET_SEQNO_INDEX] & 0xff ) != ( packets_received & 0xff ) )
							{
                                //���մ�������ݣ��ظ�NAK
								Send_Byte ( NACK ); //local data sequence number is different to rx data packet.
							}
							else
							{
								if ( packets_received == 0 ) //���յ�һ֡����
								{
									/* Filename packet */
									if ( packet_data[PACKET_HEADER] != 0 ) //�����ļ���Ϣ���ļ������ļ����ȵ�
									{
										/* Filename packet has valid data */
										for ( i = 0, file_ptr = packet_data + PACKET_HEADER; ( *file_ptr != 0 ) && ( i < FILE_NAME_LENGTH ); )
										{
											FileName[i++] = *file_ptr++;    //�����ļ���
										}
										FileName[i++] = '\0';   //�ļ�����'\0'����
										for ( i = 0, file_ptr ++; ( *file_ptr != ' ' ) && ( i < FILE_SIZE_LENGTH ); )
										{
											file_size[i++] = *file_ptr++;   //�����ļ���С
										}
										file_size[i++] = '\0';  //�ļ���С��'\0'����
										size = atoi ( file_size );  //���ļ���С�ַ���ת�������� 

										/* Test the size of the image to be sent */
										/* Image size is greater than Flash size */
										if ( size > ( int32_t ) ( USER_FLASH_SIZE + 1 ) )   //�����̼�����
										{
											/* End session */
											Send_Byte ( CA );
											Send_Byte ( CA );   //��������2����ֹ��CA
											return -1;
										}
										/* erase user application area */
//										STM_FLASH_Erase ( appaddr ); //����Ƭ��FLASH
                                        ef_erase_bak_app( size );   //������Ӧ��flash�ռ�

                                        //���ļ���Сд��FLASH������
                                        ef_set_env((const char *)"FileName",(const char *)FileName);
                                        ef_set_env((const char *)"FileSize",(const char *)file_size);
                                        
										Send_Byte ( ACK );      //�ظ�ACk
										Send_Byte ( CRC16 );    //����'C',ѯ������
									}
									/* Filename packet is empty, end session */
									else    //�ļ������ݰ�Ϊ�գ���������
									{
										Send_Byte ( ACK );
										file_done = 1;      //ֹͣ����
										session_done = 1;   //�����Ի�
										break;
									}
								}
								/* Data packet */
								else        //�յ����ݰ�
								{
									memcpy ( buf_ptr, packet_data + PACKET_HEADER, packet_length );
                                        
//                                    //ramsource = ( uint32_t ) buf_ptr; //ֱ��д�ڲ�FLASH��Ҫ�������ⲿFLASH����Ҫ

									/* Write received data in Flash */
                                    if(ef_write_data_to_bak(buf_ptr, packet_length,&update_file_cur_size,size) == EF_NO_ERR)                           
//    								if ( STM_FLASH_Write ( &flashdestination, ( uint32_t* ) ramsource, ( uint16_t ) packet_length/4 )  == 0 ) //ֱ��дƬ��FLASH
									{
										Send_Byte ( ACK );
									}
									else /* An error occurred while writing to Flash memory */
									{
										/* End session */
										Send_Byte ( CA );
										Send_Byte ( CA );
										return -2;
									}
								}
								packets_received ++;    //�յ����ݰ��ĸ���
								session_begin = 1;      //���ý����б�־
							}
					}
					break;
				case 1:     //�û���ֹ
					Send_Byte ( CA );
					Send_Byte ( CA );
					return -3;
				default:
					if ( session_begin > 0 )    //��������з�������
					{
						errors ++;
					}
					if ( errors > MAX_ERRORS )  //���󳬹�����
					{
						Send_Byte ( CA );
						Send_Byte ( CA );
						return 0;   //������̷����������
					}
					Send_Byte ( CRC16 );
					break;
			}

            
			if ( file_done != 0 )   //�ļ�������ϣ��˳�ѭ��
			{
				break;
			}
		}
        
		if ( session_done != 0 )    //�Ի�����������ѭ��
		{
			break;
		}
	}
	return ( int32_t ) size;    //���ؽ��յ��ļ��Ĵ�С
}
