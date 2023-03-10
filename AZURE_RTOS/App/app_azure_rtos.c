
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_azure_rtos.c
  * @author  MCD Application Team
  * @brief   azure_rtos application implementation file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "app_azure_rtos.h"
#include "stm32wlxx.h"
#include "app_lorawan.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sys_app.h"
#include "stdbool.h"
#include "tx_api.h"
#include "tx_timer.h"
#include "tx_port.h"
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include "tx_execution_profile.h"
#include "mb.h"
#include "mbport.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// ��������
#define APP_CFG_TASK_IDLE_STK_SIZE      256u
static TX_THREAD    AppTaskIdleTCB;
static uint64_t     AppTaskIdleStk[APP_CFG_TASK_IDLE_STK_SIZE / 8];
#define APP_CFG_TASK_IDLE_PRIO          31u
static void AppTaskIDLE(ULONG thread_input);

// ���ڴ�ӡ����
#define APP_CFG_TASK_PRINT_STK_SIZE     1024u
static TX_THREAD    AppTaskPrintTCB;
static uint64_t     AppTaskPrintStk[APP_CFG_TASK_PRINT_STK_SIZE / 8];
#define APP_CFG_TASK_PRINT_PRIO         17u
static void AppTaskPrint(ULONG thread_input);

// ͳ������
#define  APP_CFG_TASK_STAT_STK_SIZE     512u
static  TX_THREAD   AppTaskStatTCB;
static  uint64_t    AppTaskStatStk[APP_CFG_TASK_STAT_STK_SIZE/8];
#define  APP_CFG_TASK_STAT_PRIO         30u
static  void  AppTaskStat           (ULONG thread_input);
static  void  OSStatInit            (void);

// mosbus ����
#define APP_CFG_TASK_MODBUS_STK_SIZE     1024u
static TX_THREAD    AppTaskModbusTCB;
static uint64_t     AppTaskModbusStk[APP_CFG_TASK_MODBUS_STK_SIZE / 8];
#define APP_CFG_TASK_MODBUS_PRIO         17u
static void AppTaskModbus(ULONG thread_input);


static  void  App_Printf (const char *fmt, ...);
/*
*******************************************************************************************************
*                               ����
*******************************************************************************************************
*/
static  TX_MUTEX   AppPrintfSemp;	/* ����printf���� */
/* ͳ������ʹ�� */

/* �궨�� */
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004
#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)

__IO double     OSCPUUsage;       /* CPU�ٷֱ� */
extern EXECUTION_TIME     _tx_execution_thread_time_total;
extern EXECUTION_TIME     _tx_execution_isr_time_total;
extern EXECUTION_TIME     _tx_execution_idle_time_total;
void bsp_InitDWT(void);

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN TX_Pool_Buffer */
/* USER CODE END TX_Pool_Buffer */
#if defined ( __ICCARM__ )
#pragma data_alignment=4
#endif
__ALIGN_BEGIN static UCHAR tx_byte_pool_buffer[TX_APP_MEM_POOL_SIZE] __ALIGN_END;
static TX_BYTE_POOL tx_app_byte_pool;

/* USER CODE BEGIN LORAWAN_Pool_Buffer */

/* USER CODE END LORAWAN_Pool_Buffer */
static UCHAR lorawan_byte_pool_buffer[LORAWAN_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL lorawan_app_byte_pool;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Define the initial system.
  * @param  first_unused_memory : Pointer to the first unused memory
  * @retval None
  */
VOID tx_application_define(VOID *first_unused_memory)
{
  /* USER CODE BEGIN  tx_application_define */

  /* USER CODE END  tx_application_define */

  VOID *memory_ptr;

  if (tx_byte_pool_create(&tx_app_byte_pool, "Tx App memory pool", tx_byte_pool_buffer, TX_APP_MEM_POOL_SIZE) != TX_SUCCESS)
  {
    /* USER CODE BEGIN TX_Byte_Pool_Error */

    /* USER CODE END TX_Byte_Pool_Error */
  }
  else
  {
    /* USER CODE BEGIN TX_Byte_Pool_Success */

    /* USER CODE END TX_Byte_Pool_Success */

    memory_ptr = (VOID *)&tx_app_byte_pool;

    if (App_ThreadX_Init(memory_ptr) != TX_SUCCESS)
    {
      /* USER CODE BEGIN  App_ThreadX_Init_Error */

      /* USER CODE END  App_ThreadX_Init_Error */
    }

    /* USER CODE BEGIN  App_ThreadX_Init_Success */

    /**************����ͳ������*********************/
    tx_thread_create(&AppTaskStatTCB,                   /* ������ƿ��ַ */
                       "App Task STAT",                 /* ������ */
                       AppTaskStat,                     /* ������������ַ */
                       0,                               /* ���ݸ�����Ĳ��� */
                       &AppTaskStatStk[0],              /* ��ջ����ַ */
                       APP_CFG_TASK_STAT_STK_SIZE,      /* ��ջ�ռ��С */
                       APP_CFG_TASK_STAT_PRIO,          /* �������ȼ�*/
                       APP_CFG_TASK_STAT_PRIO,          /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,                /* ������ʱ��Ƭ */
                       TX_AUTO_START);                  /* �������������� */


    /**************������������*********************/
    tx_thread_create(&AppTaskIdleTCB,                   /* ������ƿ��ַ */
                       "App Task IDLE",                 /* ������ */
                       AppTaskIDLE,                     /* ������������ַ */
                       0,                               /* ���ݸ�����Ĳ��� */
                       &AppTaskIdleStk[0],              /* ��ջ����ַ */
                       APP_CFG_TASK_IDLE_STK_SIZE,      /* ��ջ�ռ��С */
                       APP_CFG_TASK_IDLE_PRIO,          /* �������ȼ�*/
                       APP_CFG_TASK_IDLE_PRIO,          /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,                /* ������ʱ��Ƭ */
                       TX_AUTO_START);                  /* �������������� */

    /**************���� print ����*********************/
    tx_thread_create(&AppTaskPrintTCB,                  /* ������ƿ��ַ */
                       "App Task print",                /* ������ */
                       AppTaskPrint,                    /* ������������ַ */
                       0,                               /* ���ݸ�����Ĳ��� */
                       &AppTaskPrintStk[0],             /* ��ջ����ַ */
                       APP_CFG_TASK_PRINT_STK_SIZE,     /* ��ջ�ռ��С */
                       APP_CFG_TASK_PRINT_PRIO,         /* �������ȼ�*/
                       APP_CFG_TASK_PRINT_PRIO,         /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,                /* ������ʱ��Ƭ */
                       TX_AUTO_START);                  /* �������������� */
    /* USER CODE END  App_ThreadX_Init_Success */

  }
  if (tx_byte_pool_create(&lorawan_app_byte_pool, "LORAWAN App memory pool", lorawan_byte_pool_buffer, LORAWAN_APP_MEM_POOL_SIZE) != TX_SUCCESS)
  {
    /* USER CODE BEGIN LORAWAN_Byte_Pool_Error */

    /* USER CODE END LORAWAN_Byte_Pool_Error */
  }
  else
  {
    /* USER CODE BEGIN LORAWAN_Byte_Pool_Success */

    /* USER CODE END LORAWAN_Byte_Pool_Success */

    memory_ptr = (VOID *)&lorawan_app_byte_pool;
    if (MX_LoRaWAN_Init(memory_ptr) != TX_SUCCESS)
    {
      /* USER CODE BEGIN  MX_LoRaWAN_Init_Error */

      /* USER CODE END  MX_LoRaWAN_Init_Error */
    }
    /* USER CODE BEGIN  MX_LoRaWAN_Init_Success */

    /* USER CODE END  MX_LoRaWAN_Init_Success */
  }
}

/* USER CODE BEGIN  0 */


/*
*********************************************************************************************************
*	�� �� ��: AppTaskIDLE
*	����˵��: ��������
*	��    ��: thread_input ����������ʱ���ݵ��β�
*	�� �� ֵ: ��
    �� �� ��: 31
*********************************************************************************************************
*/
static void AppTaskIDLE(ULONG thread_input)
{
  TX_INTERRUPT_SAVE_AREA

  (void)thread_input;

  while(1)
  {
       TX_DISABLE
       TX_RESTORE
       tx_thread_sleep(2000);
  }
}


static void AppTaskStat(ULONG thread_input)
{
    (void)thread_input;
    /*
     * TolTime: �ܵ�ʱ��
     * IdleTime�� �ܵĿ���ʱ��
     * deltaTolTime��200ms �ڵ���ʱ��
     * deltaIdleTime�� 200ms �ڵĿ���ʱ��
     */
    EXECUTION_TIME TolTime, IdleTime, deltaTolTime, deltaIdleTime;
    uint32_t uiCount = 0;
    /* ���ȿ��� DWT */
    bsp_InitDWT();

    /* ����CPU������ */
    IdleTime = _tx_execution_idle_time_total;
    TolTime = _tx_execution_thread_time_total + _tx_execution_isr_time_total + _tx_execution_idle_time_total;

    for (;;)
    {
        uiCount = 0;
        deltaIdleTime = _tx_execution_idle_time_total - IdleTime;
        deltaTolTime = _tx_execution_thread_time_total + _tx_execution_isr_time_total + _tx_execution_idle_time_total - TolTime;
        if(deltaTolTime != 0)
        {
            OSCPUUsage = (double)deltaIdleTime / deltaTolTime;
            OSCPUUsage = 100- OSCPUUsage * 100;
        }else
        {
            OSCPUUsage = 100;
        }

        IdleTime = _tx_execution_idle_time_total;
        TolTime = _tx_execution_thread_time_total + _tx_execution_isr_time_total + _tx_execution_idle_time_total;
        tx_thread_sleep(2000);        /* ÿ 2s ͳ��һ�� */
    }
}

/*
*********************************************************************************************************
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨѶ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
     /* ���������ź��� */
    tx_mutex_create(&AppPrintfSemp, "AppPrintfSemp", TX_NO_INHERIT);
}

/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  App_Printf(const char *fmt, ...)
{
    char  buf_str[200 + 1]; /* �ر�ע�⣬���printf�ı����϶࣬ע��˾ֲ������Ĵ�С�Ƿ��� */
    va_list   v_args;


    va_start(v_args, fmt);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) fmt,
                                  v_args);
    va_end(v_args);

    /* ������� */
    tx_mutex_get(&AppPrintfSemp, TX_WAIT_FOREVER);

    APP_LOG(TS_OFF, VLEVEL_M, "%s", buf_str);

    tx_mutex_put(&AppPrintfSemp);
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskPrint
*	����˵��: �� ThreadX ������Ϣͨ�����ڴ�ӡ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppTaskPrint(ULONG thread_input)
{
    TX_INTERRUPT_SAVE_AREA

    (void)thread_input;

    TX_THREAD *p_tcb; /* ����һ��������ƿ�ָ�� */
    while (1)
    {
        p_tcb = &AppTaskPrintTCB;

        /* ��ӡ���� */
        App_Printf("===============================================================\r\n");
        App_Printf("CPU������ = %5.2f%%\r\n", OSCPUUsage);
        App_Printf("����ִ��ʱ�� = %.9fs\r\n", (double)_tx_execution_thread_time_total/SystemCoreClock);
        App_Printf("����ִ��ʱ�� = %.9fs\r\n", (double)_tx_execution_idle_time_total/SystemCoreClock);
        App_Printf("�ж�ִ��ʱ�� = %.9fs\r\n", (double)_tx_execution_isr_time_total/SystemCoreClock);
        App_Printf("ϵͳ��ִ��ʱ�� = %.9fs\r\n", (double)(_tx_execution_thread_time_total + \
                                                       _tx_execution_idle_time_total +  \
                                                       _tx_execution_isr_time_total)/SystemCoreClock);

        App_Printf("===============================================================\r\n");
        App_Printf(" �������ȼ� ����ջ��С ��ǰʹ��ջ  ���ջʹ��   ������\r\n");
        App_Printf("   Prio     StackSize   CurStack    MaxStack   Taskname\r\n");

        /* ����������ƿ���(TCB list)����ӡ���е���������ȼ��������� */
        while (p_tcb != (TX_THREAD *)0)
        {

            APP_LOG(TS_OFF, VLEVEL_M, "   %2d        %5d      %5d       %5d      %s\r\n",
                      p_tcb->tx_thread_priority,
                      p_tcb->tx_thread_stack_size,
                      ((int)p_tcb->tx_thread_stack_end - (int)p_tcb->tx_thread_stack_ptr),
                      ((int)p_tcb->tx_thread_stack_end - (int)p_tcb->tx_thread_stack_highest_ptr),
                      p_tcb->tx_thread_name);

            p_tcb = p_tcb->tx_thread_created_next;

            if (p_tcb == &AppTaskPrintTCB)
                break;
        }
        App_Printf("===============================================================\r\n");

        tx_thread_sleep(10000); /* 10s��ӡһ�� */
    }
}


/*
*********************************************************************************************************
*	�� �� ��: AppTaskModbus
*	����˵��: Modbus RTU ����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppTaskModbus(ULONG thread_input)
{
    (void)thread_input;

    while (1)
    {
        eMBPoll();              // ����modbus����
        tx_thread_sleep(10); /* 10ms ����һ�� */
    }
}

/*
*********************************************************************************************************
*   �� �� ��: bsp_InitDWT
*   ����˵��: ��ʼ��DWT. �ú����� bsp_Init() ���á�
*   ��    ��: ��
*   �� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitDWT(void)
{
    DEM_CR         |= (unsigned int)DEM_CR_TRCENA;
    DWT_CYCCNT      = (unsigned int)0u;
    DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}
/* USER CODE END  0 */
