
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


static  void  App_Printf (const char *fmt, ...);
/*
*******************************************************************************************************
*                               ����
*******************************************************************************************************
*/
static  TX_MUTEX   AppPrintfSemp;	/* ����printf���� */
/* ͳ������ʹ�� */
__IO uint8_t   OSStatRdy;        /* ͳ�����������־ */
__IO uint32_t  OSIdleCtr;        /* ����������� */
__IO float     OSCPUUsage;       /* CPU�ٷֱ� */
uint32_t       OSIdleCtrMax;     /* 1�������Ŀ��м��� */
uint32_t       OSIdleCtrRun;     /* 1���ڿ�������ǰ���� */

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
*	�� �� ��: AppTaskStatistic
*	����˵��: ͳ����������ʵ��CPU�����ʵ�ͳ�ơ�Ϊ�˲��Ը���׼ȷ�����Կ���ע�͵��õ�ȫ���жϿ���
*	��    ��: thread_input ����������ʱ���ݵ��β� 
*	�� �� ֵ: ��
*   �� �� ��: 30
*********************************************************************************************************
*/
void  OSStatInit (void)
{
    OSStatRdy = FALSE;
    
    tx_thread_sleep(2u);        /* ʱ��ͬ�� */
    
    //__disable_irq();
    OSIdleCtr    = 0uL;         /* ����м��� */
    //__enable_irq();
    
    tx_thread_sleep(1000);       /* ͳ��1s�ڣ������м��� */
    
       //__disable_irq();
    OSIdleCtrMax = OSIdleCtr;   /* ���������м��� */
    OSStatRdy    = TRUE;
    //__enable_irq();
}


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
       OSIdleCtr++;
       TX_RESTORE
  }
}


static void AppTaskStat(ULONG thread_input)
{
    (void)thread_input;
    /* ����ִ������ͳ�� */
    OSStatInit();
    while (OSStatRdy == FALSE) 
    {
        tx_thread_sleep(200);     /* �ȴ�ͳ��������� */
    }

    OSIdleCtrMax /= 10uL;
    if (OSIdleCtrMax == 0uL) 
    {
        OSCPUUsage = 0u;
    }
    
    //__disable_irq();
    OSIdleCtr = OSIdleCtrMax * 100uL;  /* ���ó�ʼCPU������ 0% */
    //__enable_irq();
    
    for (;;) 
    {
       // __disable_irq();
        OSIdleCtrRun = OSIdleCtr;    /* ���1s�ڿ��м��� */
        OSIdleCtr    = 0uL;          /* ��λ���м��� */
       //	__enable_irq();            /* ����1s�ڵ�CPU������ */
        OSCPUUsage   = (100uL - (float)OSIdleCtrRun / OSIdleCtrMax);
        tx_thread_sleep(1000);        /* ÿ1sͳ��һ�� */
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
        App_Printf("OS CPU Usage = %5.2f%%\r\n", OSCPUUsage);
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
/* USER CODE END  0 */
