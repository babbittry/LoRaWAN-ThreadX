/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_lorawan.c
  * @author  MCD Application Team
  * @brief   Application of the LRWAN Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "app_lorawan.h"
#include "lora_app.h"
#include "sys_app.h"
#include "tx_api.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
TX_THREAD App_MainThread;
TX_BYTE_POOL *byte_pool;
CHAR *pointer;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/

uint32_t MX_LoRaWAN_Init(void *memory_ptr)
{
  uint32_t ret = TX_SUCCESS;

  /* USER CODE BEGIN MX_LoRaWAN_Init_1 */

  /* USER CODE END MX_LoRaWAN_Init_1 */
  byte_pool = memory_ptr;

  /* Allocate the stack for App App_MainThread.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       CFG_APP_LORA_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create App_MainThread.  */
  if (ret == TX_SUCCESS)
  {
    if (tx_thread_create(&App_MainThread,                   /* 任务控制块地址 */
                          "App LoRaWAN Main Thread",        /* 任务名 */
                          App_Main_Thread_Entry,            /* 启动任务函数地址 */
                          0,                                /* 传递给任务的参数 */
                          pointer,                          /* 堆栈基地址 */
                          CFG_APP_LORA_THREAD_STACK_SIZE,   /* 堆栈空间大小 */
                          CFG_APP_LORA_THREAD_PRIO,         /* 任务优先级*/
                          CFG_APP_LORA_THREAD_PREEMPTION_THRESHOLD, /* 任务抢占阀值 */
                          TX_NO_TIME_SLICE,                 /* 不开启时间片 */
                          TX_AUTO_START) != TX_SUCCESS)     /* 创建后立即启动 */
    {
      ret = TX_THREAD_ERROR;
    }
  }

  /* USER CODE BEGIN MX_LoRaWAN_Init_Last */

  /* USER CODE END MX_LoRaWAN_Init_Last */
  return ret;
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/
/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
