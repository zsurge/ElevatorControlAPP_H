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
  * @note    这个文件是数据的缓冲区处理文件
  ***********************************************************/


/************************************************************************
  * 如果使用了互斥量进行资源保护，则需要按照os的互斥量控制块进行修改
  * MUTEX_T 否则无法正常使用，并且用户需要自定义实现：
  * err_t create_mutex(MUTEX_T *mutex) 
  * err_t deleta_mutex(MUTEX_T *mutex)
  * request_mutex(MUTEX_T *mutex)
  * release_mutex(MUTEX_T *mutex)
  * 这四个函数，该函数在ringbuff.c中已经定义，
  * 目前支持的os有rtt、win32、ucos、FreeRTOS
  ************************************************************************/
  
  
#define  USE_MUTEX   0

#define SUCCESS        ( 0 )
#define ERR_NOK       ( -1 )
#define ERR_NULL      ( -2 )
#define ERR_UNUSE     ( -3 )

#if USE_MUTEX
#define  MUTEX_TIMEOUT   1000     /* 超时时间 */
#define  MUTEX_T         mutex_t  /* 互斥量控制块 */
#endif




#define spin_lock_irqsave(lock_ptr, flags) pthread_mutex_lock(lock_ptr)
#define spin_unlock_irqrestore(lock_ptr, flags) pthread_mutex_unlock(lock_ptr)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ringbuff 
{
	uint8_t *buffer;  	/* 数据区域 */
	uint32_t size;      /* 环形缓冲区大小 */
	uint32_t in;        /* 数据入队指针 (in % size) */
	uint32_t out;       /* 数据出队指针 (out % size) */
#if USE_MUTEX
	MUTEX_T *mutex;       /* 支持rtos的互斥 */
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













