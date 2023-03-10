
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
// 空闲任务
#define APP_CFG_TASK_IDLE_STK_SIZE      256u
static TX_THREAD    AppTaskIdleTCB;
static uint64_t     AppTaskIdleStk[APP_CFG_TASK_IDLE_STK_SIZE / 8];
#define APP_CFG_TASK_IDLE_PRIO          31u
static void AppTaskIDLE(ULONG thread_input);

// 串口打印任务
#define APP_CFG_TASK_PRINT_STK_SIZE     1024u
static TX_THREAD    AppTaskPrintTCB;
static uint64_t     AppTaskPrintStk[APP_CFG_TASK_PRINT_STK_SIZE / 8];
#define APP_CFG_TASK_PRINT_PRIO         17u
static void AppTaskPrint(ULONG thread_input);

// 统计任务
#define  APP_CFG_TASK_STAT_STK_SIZE     512u
static  TX_THREAD   AppTaskStatTCB;
static  uint64_t    AppTaskStatStk[APP_CFG_TASK_STAT_STK_SIZE/8];
#define  APP_CFG_TASK_STAT_PRIO         30u
static  void  AppTaskStat           (ULONG thread_input);
static  void  OSStatInit            (void);

// mosbus 任务
#define APP_CFG_TASK_MODBUS_STK_SIZE     1024u
static TX_THREAD    AppTaskModbusTCB;
static uint64_t     AppTaskModbusStk[APP_CFG_TASK_MODBUS_STK_SIZE / 8];
#define APP_CFG_TASK_MODBUS_PRIO         17u
static void AppTaskModbus(ULONG thread_input);


static  void  App_Printf (const char *fmt, ...);
/*
*******************************************************************************************************
*                               变量
*******************************************************************************************************
*/
static  TX_MUTEX   AppPrintfSemp;	/* 用于printf互斥 */
/* 统计任务使用 */

/* 宏定义 */
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004
#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)

__IO double     OSCPUUsage;       /* CPU百分比 */
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

    /**************创建统计任务*********************/
    tx_thread_create(&AppTaskStatTCB,                   /* 任务控制块地址 */
                       "App Task STAT",                 /* 任务名 */
                       AppTaskStat,                     /* 启动任务函数地址 */
                       0,                               /* 传递给任务的参数 */
                       &AppTaskStatStk[0],              /* 堆栈基地址 */
                       APP_CFG_TASK_STAT_STK_SIZE,      /* 堆栈空间大小 */
                       APP_CFG_TASK_STAT_PRIO,          /* 任务优先级*/
                       APP_CFG_TASK_STAT_PRIO,          /* 任务抢占阀值 */
                       TX_NO_TIME_SLICE,                /* 不开启时间片 */
                       TX_AUTO_START);                  /* 创建后立即启动 */


    /**************创建空闲任务*********************/
    tx_thread_create(&AppTaskIdleTCB,                   /* 任务控制块地址 */
                       "App Task IDLE",                 /* 任务名 */
                       AppTaskIDLE,                     /* 启动任务函数地址 */
                       0,                               /* 传递给任务的参数 */
                       &AppTaskIdleStk[0],              /* 堆栈基地址 */
                       APP_CFG_TASK_IDLE_STK_SIZE,      /* 堆栈空间大小 */
                       APP_CFG_TASK_IDLE_PRIO,          /* 任务优先级*/
                       APP_CFG_TASK_IDLE_PRIO,          /* 任务抢占阀值 */
                       TX_NO_TIME_SLICE,                /* 不开启时间片 */
                       TX_AUTO_START);                  /* 创建后立即启动 */

    /**************创建 print 任务*********************/
    tx_thread_create(&AppTaskPrintTCB,                  /* 任务控制块地址 */
                       "App Task print",                /* 任务名 */
                       AppTaskPrint,                    /* 启动任务函数地址 */
                       0,                               /* 传递给任务的参数 */
                       &AppTaskPrintStk[0],             /* 堆栈基地址 */
                       APP_CFG_TASK_PRINT_STK_SIZE,     /* 堆栈空间大小 */
                       APP_CFG_TASK_PRINT_PRIO,         /* 任务优先级*/
                       APP_CFG_TASK_PRINT_PRIO,         /* 任务抢占阀值 */
                       TX_NO_TIME_SLICE,                /* 不开启时间片 */
                       TX_AUTO_START);                  /* 创建后立即启动 */
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
*	函 数 名: AppTaskIDLE
*	功能说明: 空闲任务
*	形    参: thread_input 创建该任务时传递的形参
*	返 回 值: 无
    优 先 级: 31
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
     * TolTime: 总的时间
     * IdleTime： 总的空闲时间
     * deltaTolTime：200ms 内的总时间
     * deltaIdleTime： 200ms 内的空闲时间
     */
    EXECUTION_TIME TolTime, IdleTime, deltaTolTime, deltaIdleTime;
    uint32_t uiCount = 0;
    /* 优先开启 DWT */
    bsp_InitDWT();

    /* 计算CPU利用率 */
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
        tx_thread_sleep(2000);        /* 每 2s 统计一次 */
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通讯
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
     /* 创建互斥信号量 */
    tx_mutex_create(&AppPrintfSemp, "AppPrintfSemp", TX_NO_INHERIT);
}

/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  App_Printf(const char *fmt, ...)
{
    char  buf_str[200 + 1]; /* 特别注意，如果printf的变量较多，注意此局部变量的大小是否够用 */
    va_list   v_args;


    va_start(v_args, fmt);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) fmt,
                                  v_args);
    va_end(v_args);

    /* 互斥操作 */
    tx_mutex_get(&AppPrintfSemp, TX_WAIT_FOREVER);

    APP_LOG(TS_OFF, VLEVEL_M, "%s", buf_str);

    tx_mutex_put(&AppPrintfSemp);
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskPrint
*	功能说明: 将 ThreadX 任务信息通过串口打印出来
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskPrint(ULONG thread_input)
{
    TX_INTERRUPT_SAVE_AREA

    (void)thread_input;

    TX_THREAD *p_tcb; /* 定义一个任务控制块指针 */
    while (1)
    {
        p_tcb = &AppTaskPrintTCB;

        /* 打印标题 */
        App_Printf("===============================================================\r\n");
        App_Printf("CPU利用率 = %5.2f%%\r\n", OSCPUUsage);
        App_Printf("任务执行时间 = %.9fs\r\n", (double)_tx_execution_thread_time_total/SystemCoreClock);
        App_Printf("空闲执行时间 = %.9fs\r\n", (double)_tx_execution_idle_time_total/SystemCoreClock);
        App_Printf("中断执行时间 = %.9fs\r\n", (double)_tx_execution_isr_time_total/SystemCoreClock);
        App_Printf("系统总执行时间 = %.9fs\r\n", (double)(_tx_execution_thread_time_total + \
                                                       _tx_execution_idle_time_total +  \
                                                       _tx_execution_isr_time_total)/SystemCoreClock);

        App_Printf("===============================================================\r\n");
        App_Printf(" 任务优先级 任务栈大小 当前使用栈  最大栈使用   任务名\r\n");
        App_Printf("   Prio     StackSize   CurStack    MaxStack   Taskname\r\n");

        /* 遍历任务控制块列(TCB list)，打印所有的任务的优先级和任务名 */
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

        tx_thread_sleep(10000); /* 10s打印一次 */
    }
}


/*
*********************************************************************************************************
*	函 数 名: AppTaskModbus
*	功能说明: Modbus RTU 任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskModbus(ULONG thread_input)
{
    (void)thread_input;

    while (1)
    {
        eMBPoll();              // 启动modbus侦听
        tx_thread_sleep(10); /* 10ms 侦听一次 */
    }
}

/*
*********************************************************************************************************
*   函 数 名: bsp_InitDWT
*   功能说明: 初始化DWT. 该函数被 bsp_Init() 调用。
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDWT(void)
{
    DEM_CR         |= (unsigned int)DEM_CR_TRCENA;
    DWT_CYCCNT      = (unsigned int)0u;
    DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}
/* USER CODE END  0 */
