#ifndef __DATA_FIFO_H
#define __DATA_FIFO_H


#include "stm32f4xx_conf.h"

/************************************************************
  * @brief   __DATA_FIFO_H
  * @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ����ļ������ݵĻ����������ļ�
  ***********************************************************/


/************************************************************************
  * ���ʹ���˻�����������Դ����������Ҫ����os�Ļ��������ƿ�����޸�
  * MUTEX_T �����޷�����ʹ�ã������û���Ҫ�Զ���ʵ�֣�
  * err_t create_mutex(MUTEX_T *mutex) 
  * err_t deleta_mutex(MUTEX_T *mutex)
  * request_mutex(MUTEX_T *mutex)
  * release_mutex(MUTEX_T *mutex)
  * ���ĸ��������ú�����ringbuff.c���Ѿ����壬
  * Ŀǰ֧�ֵ�os��rtt��win32��ucos��FreeRTOS
  ************************************************************************/
  
  
#define  USE_MUTEX   0

#define ERR_OK        ( 0 )
#define ERR_NOK       ( -1 )
#define ERR_NULL      ( -2 )
#define ERR_UNUSE     ( -3 )

#if USE_MUTEX
#define  MUTEX_TIMEOUT   1000     /* ��ʱʱ�� */
#define  MUTEX_T         mutex_t  /* ���������ƿ� */
#endif




#define spin_lock_irqsave(lock_ptr, flags) pthread_mutex_lock(lock_ptr)
#define spin_unlock_irqrestore(lock_ptr, flags) pthread_mutex_unlock(lock_ptr)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ringbuff 
{
	uint8_t *buffer;  	/* �������� */
	uint32_t size;      /* ���λ�������С */
	uint32_t in;        /* �������ָ�� (in % size) */
	uint32_t out;       /* ���ݳ���ָ�� (out % size) */
#if USE_MUTEX
	MUTEX_T *mutex;       /* ֧��rtos�Ļ��� */
#endif
}RingBuff_t ;

extern int32_t Create_RingBuff(RingBuff_t *rb, 
                             uint8_t *buffer,
                             uint32_t size
                            );
																
//extern struct kfifo *kfifo_alloc(unsigned int size
//#if USE_MUTEX
//																,spinlock_t *lock
//#endif
//																);
//																
//extern void kfifo_free(struct kfifo *fifo);

extern uint32_t Write_RingBuff(RingBuff_t *rb,
                               uint8_t *buffer, 
                               uint32_t len);
																
extern uint32_t Read_RingBuff(RingBuff_t *rb,
                              uint8_t *buffer, 
                              uint32_t len);

extern uint32_t CanRead_RingBuff(RingBuff_t *rb);

extern uint32_t CanWrite_RingBuff(RingBuff_t *rb);



#endif /* __DATA_FIFO_H */













