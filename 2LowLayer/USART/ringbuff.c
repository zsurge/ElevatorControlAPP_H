/*
	***********************************************************
  * @brief   ringbuff.c
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ���ļ����ڴ����λ���������
  ***********************************************************
	*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ringbuff.h"

//#define kmalloc(size, mask) 		        malloc(size)
//#define kfree(ptr) 							free(ptr)
//#define BUG_ON(cond) 						ASSERT(!(cond))


#define min(a,b)  ((a)>(b) ? (b) : (a)) 

/* ��ͬ������ѡ�� */
#if defined(__GNUC__) || defined(__x86_64__)
#define TPOOL_COMPILER_BARRIER() __asm__ __volatile("" : : : "memory")

static inline void FullMemoryBarrier()
{
    __asm__ __volatile__("mfence": : : "memory");
}
#define smp_mb() FullMemoryBarrier()
#define smp_rmb() TPOOL_COMPILER_BARRIER()
#define smp_wmb() TPOOL_COMPILER_BARRIER()	
#endif



/*********************** �ڲ����ú��� ******************************/

/*********************************** mutex **************************************************/
#if USE_MUTEX
/************************************************************
  * @brief   create_mutex
  * @param   mutex:�����ź������
  * @return  �����ɹ�Ϊ1��0Ϊ���ɹ���
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ����һ��������,�û���os�л���ʹ��ringbuff��
  *          ֧�ֵ�os��rtt��win32��ucos��FreeRTOS��LiteOS
  ***********************************************************/
static err_t create_mutex(MUTEX_T *mutex)
{
  err_t ret = 0;

//	*mutex = rt_mutex_create("test_mux",RT_IPC_FLAG_PRIO); /* rtt */
//	ret = (err_t)(*mutex != RT_NULL);
	
//	*mutex = CreateMutex(NULL, FALSE, NULL);		/* Win32 */
//	ret = (err_t)(*mutex != INVALID_HANDLE_VALUE);

//	*mutex = OSMutexCreate(0, &err);		/* uC/OS-II */
//	ret = (err_t)(err == OS_NO_ERR);

//	*mutex = xSemaphoreCreateMutex();	/* FreeRTOS */
//	ret = (err_t)(*mutex != NULL);

//  ret = LOS_MuxCreate(&mutex);  /* LiteOS */
//	ret = (err_t)(ret != LOS_OK);
  return ret;
}
/************************************************************
  * @brief   deleta_mutex
  * @param   mutex:���������
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ɾ��һ����������֧�ֵ�os��rtt��win32��ucos��FreeRTOS��LiteOS
  ***********************************************************/
static err_t deleta_mutex(MUTEX_T *mutex)
{
	err_t ret;
	
//	ret = rt_mutex_delete(mutex);	/* rtt */
	
//	ret = CloseHandle(mutex);	/* Win32 */

//	OSMutexDel(mutex, OS_DEL_ALWAYS, &err);	/* uC/OS-II */
//	ret = (err_t)(err == OS_NO_ERR);

//  vSemaphoreDelete(mutex);		/* FreeRTOS */
//	ret = 1;

//  ret = LOS_MuxDelete(&mutex);  /* LiteOS */
//	ret = (err_t)(ret != LOS_OK);

	return ret;
}
/************************************************************
  * @brief   request_mutex
  * @param   mutex:���������
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ����һ�����������õ����������̲߳�������з��ʻ�����
  *          ֧�ֵ�os��rtt��win32��ucos��FreeRTOS��LiteOS
  ***********************************************************/
static err_t request_mutex(MUTEX_T *mutex)
{
	err_t ret;

//	ret = (err_t)(rt_mutex_take(mutex, MUTEX_TIMEOUT) == RT_EOK);/* rtt */
	
//	ret = (err_t)(WaitForSingleObject(mutex, MUTEX_TIMEOUT) == WAIT_OBJECT_0);	/* Win32 */

//	OSMutexPend(mutex, MUTEX_TIMEOUT, &err));		/* uC/OS-II */
//	ret = (err_t)(err == OS_NO_ERR);

//	ret = (err_t)(xSemaphoreTake(mutex, MUTEX_TIMEOUT) == pdTRUE);	/* FreeRTOS */

//  ret = (err_t)(LOS_MuxPend(mutex,MUTEX_TIMEOUT) == LOS_OK);  		/* LiteOS */

	return ret;
}
/************************************************************
  * @brief   release_mutex
  * @param   mutex:���������
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    �ͷŻ����������߳�ʹ������Դ�����ͷŻ�����
  *          ֧�ֵ�os��rtt��win32��ucos��FreeRTOS��LiteOS
  ***********************************************************/
static void release_mutex(MUTEX_T *mutex)
{
//	rt_mutex_release(mutex);/* rtt */
	
//	ReleaseMutex(mutex);		/* Win32 */

//	OSMutexPost(mutex);		/* uC/OS-II */

//	xSemaphoreGive(mutex);	/* FreeRTOS */

//  LOS_MuxPost(mutex);   /* LiteOS */
}
#endif
/*********************************** mutex **************************************************/

/*********************************** data **************************************************/
static int32_t fls(int32_t x)
{
  int r = 32;

  if (!x)
          return 0;
  if (!(x & 0xffff0000u)) {
          x <<= 16;
          r -= 16;
  }
  if (!(x & 0xff000000u)) {
          x <<= 8;
          r -= 8;
  }
  if (!(x & 0xf0000000u)) {
          x <<= 4;
          r -= 4;
  }
  if (!(x & 0xc0000000u)) {
          x <<= 2;
          r -= 2;
  }
  if (!(x & 0x80000000u)) {
          x <<= 1;
          r -= 1;
  }
  return r;
}

#ifdef __GNUC__
#define __attribute_const__     __attribute__((__const__))
#endif

/************************************************************
  * @brief   roundup_pow_of_two
  * @param   size�����ݽ��������ݳ���
  * @return  size�����ش���֮������ݳ���
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ���ڴ������ݣ�ʹ���ݳ��ȱ���Ϊ 2^n
	*					 ������ǣ���ת�����������ಿ�֣���
	*					 roundup_pow_of_two(66) -> ���� 64
  ***********************************************************/
static unsigned long roundup_pow_of_two(unsigned long x)
{
	return (1 << (fls(x-1)-1));				//���¶���
  //return (1UL << fls(x - 1));			//���϶��룬�ö�̬�ڴ����ʹ��
}

/*********************** �ڲ����ú��� ******************************/

/************************************************************
  * @brief   Create_RingBuff
  * @param   rb�����λ��������
  *          buffer�����λ���������������
  *          size�����λ������Ĵ�С����������СҪΪ2^n
  * @return  err_t��ERR_OK��ʾ�����ɹ���������ʾʧ��
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ���ڴ���һ�����λ�����
  ***********************************************************/
int32_t Create_RingBuff(RingBuff_t* rb, 
                      uint8_t *buffer,
                      uint32_t size
								)
{
	if((rb == NULL)||(buffer == NULL)||(size == 0))
	{
		printf("data is null!");
		return ERR_NULL;
	}
	
	printf("ringbuff size is %d!",size);
	/* ��������С����Ϊ2^n�ֽ�,ϵͳ��ǿ��ת��,
		 ������ܻᵼ��ָ����ʷǷ���ַ��
		 �ռ��СԽ��,ǿתʱ��ʧ�ڴ�Խ�� */
	if(size&(size - 1))
	{
		size = roundup_pow_of_two(size);
		printf("change ringbuff size is %d!",size);
	}

	rb->buffer = buffer;
	rb->size = size;
	rb->in = rb->out = 0;
#if USE_MUTEX	
  /* �����ź������ɹ� */
  if(!create_mutex(rb->mutex))
  {
//    printf("create mutex fail!");
    ASSERT(ASSERT_ERR);
    return ERR_NOK;
  }
#endif
	printf("create ringBuff ok!");
	return ERR_OK;
}

/************************************************************
  * @brief   Delete_RingBuff
  * @param   rb�����λ��������
  * @return  err_t��ERR_OK��ʾ�ɹ���������ʾʧ��
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ɾ��һ�����λ�����
  ***********************************************************/
int32_t Delete_RingBuff(RingBuff_t *rb)
{
	if(rb == NULL)
	{
//		printf("ringbuff is null!");
		return ERR_NULL;
	}
	
	rb->buffer = NULL;
	rb->size = 0;
	rb->in = rb->out = 0;
#if USE_MUTEX	
  if(!deleta_mutex(rb->mutex))
  {
//    printf("deleta mutex is fail!");
    return ERR_NOK;
  }
#endif
	return ERR_OK;
}

/************************************************************
  * @brief   Write_RingBuff
  * @param   rb:���λ��������
  * @param   wbuff:д���������ʼ��ַ
  * @param   len:д�����ݵĳ���(�ֽ�)
  * @return  len:ʵ��д�����ݵĳ���(�ֽ�)
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ����������buff�ռ俽��len�ֽڳ��ȵ����ݵ�
             rb���λ������еĿ��пռ䡣
  ***********************************************************/
uint32_t Write_RingBuff(RingBuff_t *rb,
                        uint8_t *wbuff, 
                        uint32_t len)
{
  uint32_t l;
#if USE_MUTEX
  /* ���󻥳������ɹ����ܽ���ringbuff�ķ��� */
  if(!request_mutex(rb->mutex))
  {
//    printf("request mutex fail!");
    return 0;
  }
  else  /* ��ȡ�������ɹ� */
  {
#endif
    len = min(len, rb->size - rb->in + rb->out);

    /* ��һ���ֵĿ���:�ӻ��λ�����д������ֱ�����������һ����ַ */
    l = min(len, rb->size - (rb->in & (rb->size - 1)));
    memcpy(rb->buffer + (rb->in & (rb->size - 1)), wbuff, l);

    /* ���������ڻ�����ͷд��ʣ��Ĳ���
       ���û����������൱����Ч */
    memcpy(rb->buffer, wbuff + l, len - l);

    rb->in += len;
    
//    printf("write ringBuff len is %d!",len);
#if USE_MUTEX
  }
  /* �ͷŻ����� */
  release_mutex(rb->mutex);
#endif
  return len;
}

/************************************************************
  * @brief   Read_RingBuff
  * @param   rb:���λ��������
  * @param   wbuff:��ȡ���ݱ������ʼ��ַ
  * @param   len:��Ҫ��ȡ���ݵĳ���(�ֽ�)
  * @return  len:ʵ�ʶ�ȡ���ݵĳ���(�ֽ�)
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ����������rb���λ������е��������򿽱�len�ֽ�
             ���ȵ����ݵ�rbuff�ռ䡣
  ***********************************************************/
uint32_t Read_RingBuff(RingBuff_t *rb,
                       uint8_t *rbuff, 
                       uint32_t len)
{
  uint32_t l;
#if USE_MUTEX
  /* ���󻥳������ɹ����ܽ���ringbuff�ķ��� */
  if(!request_mutex(rb->mutex))
  {
    printf("request mutex fail!\r\n");
    return 0;
  }
  else
  {
#endif
    len = min(len, rb->in - rb->out);

    /* ��һ���ֵĿ���:�ӻ��λ�������ȡ����ֱ�����������һ�� */
    l = min(len, rb->size - (rb->out & (rb->size - 1)));
    memcpy(rbuff, rb->buffer + (rb->out & (rb->size - 1)), l);

    /* ���������ڻ�����ͷ��ȡʣ��Ĳ���
       ���û����������൱����Ч */
    memcpy(rbuff + l, rb->buffer, len - l);

    rb->out += len;
    
//    printf("read ringBuff len is %d!\r\n",len);
#if USE_MUTEX
  }
  /* �ͷŻ����� */
  release_mutex(rb->mutex);
#endif
  return len;
}

/************************************************************
  * @brief   CanRead_RingBuff
	* @param   rb:���λ��������
	* @return  uint32:�ɶ����ݳ��� 0 / len
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    �ɶ����ݳ���
  ***********************************************************/
uint32_t CanRead_RingBuff(RingBuff_t *rb)
{
	if(NULL == rb)
	{
		printf("ringbuff is null!");
		return 0;
	}
	if(rb->in == rb->out)
		return 0;
	
	if(rb->in > rb->out)
		return (rb->in - rb->out);
	
	return (rb->size - (rb->out - rb->in));
}

/************************************************************
  * @brief   CanRead_RingBuff
	* @param   rb:���λ��������
	* @return  uint32:��д���ݳ��� 0 / len
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    ��д���ݳ���
  ***********************************************************/
uint32_t CanWrite_RingBuff(RingBuff_t *rb)
{
	if(NULL == rb)
	{
		printf("ringbuff is null!");
		return 0;
	}

	return (rb->size - CanRead_RingBuff(rb));
}


/******************************** DEMO *****************************************************/
//	RingBuff_t ringbuff_handle;
//	
//	uint8_t rb[64];
//	uint8_t res[64];
//	Create_RingBuff(&ringbuff_handle, 
//								rb,
//								sizeof(rb));
//			Write_RingBuff(&ringbuff_handle,
//                     res, 
//                     datapack.data_length);
//			
//			printf("CanRead_RingBuff = %d!",CanRead_RingBuff(&ringbuff_handle));
//			printf("CanWrite_RingBuff = %d!",CanWrite_RingBuff(&ringbuff_handle));
//			
//			Read_RingBuff(&ringbuff_handle,
//                     res, 
//                     datapack.data_length);
/******************************** DEMO *****************************************************/


