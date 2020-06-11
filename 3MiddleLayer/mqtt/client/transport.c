#include "transport.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "string.h"
#include <netdb.h>


static int mysock;

/************************************************************************
** ��������: transport_sendPacketBuffer									
** ��������: ��TCP��ʽ��������
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int buflen�����ݳ���
** ���ڲ���: <0��������ʧ��							
************************************************************************/
s32 transport_sendPacketBuffer( u8* buf, s32 buflen)
{
	s32 rc;
	rc = write(mysock, buf, buflen);
	return rc;
}

/************************************************************************
** ��������: transport_getdata									
** ��������: �������ķ�ʽ����TCP����
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int count�����ݳ���
** ���ڲ���: <=0��������ʧ��									
************************************************************************/
s32 transport_getdata(u8* buf, s32 count)
{
	s32 rc;
	//������������ﲻ����
  rc = recv(mysock, buf, count, 0);
	return rc;
}


/************************************************************************
** ��������: transport_open									
** ��������: ��һ���ӿڣ����Һͷ����� ��������
** ��ڲ���: char* servip:����������
**           int   port:�˿ں�
** ���ڲ���: <0������ʧ��										
************************************************************************/
s32 transport_open(s8* servip, s32 port)
{

    int32_t* sock = &mysock;
    struct hostent *server;
    struct sockaddr_in serv_addr;

    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if(*sock < 0)
        printf("[ERROR] Create socket failed\n");

    server = gethostbyname(servip);
    if(server == NULL)
        printf("[ERROR] Get host ip failed\n");

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
    if(connect(*sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    {
        //�ر�����
        close(*sock);        
        printf("[ERROR] connect failed\n");
        return -1;
    }

    //�����׽���
    return *sock;

//    int* sock = &mysock;
//	struct hostent *server;
//	struct sockaddr_in serv_addr;
//	static struct  timeval tv;
//	int timeout = 1000;
//	fd_set readset;
//	fd_set writeset;
//	*sock = socket(AF_INET, SOCK_STREAM, 0);
//	if(*sock < 0)
//		printf("[ERROR] Create socket failed\n");
//	
//	server = gethostbyname(servip);
//	if(server == NULL)
//		printf("[ERROR] Get host ip failed\n");
//	
//	memset(&serv_addr,0,sizeof(serv_addr));
//	serv_addr.sin_family = AF_INET;
//	serv_addr.sin_port = htons(port);
//	memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
//	if(connect(*sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
//	{
//		//�ر�����
//		close(*sock);        
//		printf("[ERROR] connect failed\n");
//        return -1;
//	}
//	tv.tv_sec = 10;  /* 1 second Timeout */
//	tv.tv_usec = 0; 
//	setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,sizeof(timeout));
//    
//    return *sock;    
}


/************************************************************************
** ��������: transport_close									
** ��������: �ر��׽���
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int buflen�����ݳ���
** ���ڲ���: <0��������ʧ��							
************************************************************************/
int transport_close(void)
{
	int rc;
	rc = close(mysock);
	return rc;
}
