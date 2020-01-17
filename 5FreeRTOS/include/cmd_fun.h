/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : cmd_fun.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��31��
  ����޸�   :
  ��������   : CLI ָ��ĳ�ʼ����ʵ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��31��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __CMD_FUN_H
#define __CMD_FUN_H

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#ifndef  configINCLUDE_TRACE_RELATED_CLI_COMMANDS
	#define configINCLUDE_TRACE_RELATED_CLI_COMMANDS 0
#endif
    
#ifndef configINCLUDE_QUERY_HEAP_COMMAND
	#define configINCLUDE_QUERY_HEAP_COMMAND 0
#endif


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/



/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

extern void CMD_Init(void);

/*
 * The function that registers the commands that are defined within this file.
 */
//extern void vRegisterSampleCLICommands( void );

/*
 * Implements the task-stats command.
 */
static BaseType_t prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the run-time-stats command.
 */
#if( configGENERATE_RUN_TIME_STATS == 1 )
    static BaseType_t prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif /* configGENERATE_RUN_TIME_STATS */

/*
 * Implements the echo-three-parameters command.
 */
static BaseType_t prvThreeParameterEchoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the echo-parameters command.
 */
static BaseType_t prvParameterEchoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the "query heap" command.
 */
#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
    static BaseType_t prvQueryHeapCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif

/*
 * Implements the "trace start" and "trace stop" commands;
 */
#if( configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1 )
    static BaseType_t prvStartStopTraceCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif

//��ӡ�����б�
static BaseType_t prvPrintEnv(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

//���ò���
static BaseType_t prvSetEnv(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

//��keyֵ��ȡֵ
static BaseType_t prvGetEnv(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

//ɾ��key��ֵ
static BaseType_t prvDelEnv(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

//��ʼ��������
static BaseType_t prvResetEnv(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);









#endif

