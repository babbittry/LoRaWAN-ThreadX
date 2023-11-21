#ifndef __BSP_MODBUS_H
#define __BSP_MODBUS_H

/********  ThreadX  *********/
#include "tx_api.h"
#include "tx_timer.h"
#include "tx_port.h"
#include "tx_execution_profile.h"
/********  C standard  *********/
#include  <stdio.h>
#include  <stdlib.h>
#include  <stdint.h>
#include <stdbool.h>
/********  STM32  *********/
#include "stm32wlxx.h"
/********  project  *********/
#include "main.h"
#include "bsp_uart_fifo.h"
#include "tim.h"
#include "app_azure_rtos.h"


#define ULONG_MAX     0xffffffffUL

ULONG bsp_GetRunTime(void);
ULONG bsp_CheckRunTime(ULONG _LastTime);
uint16_t BEBufToUint16(uint8_t *_pBuf);
uint16_t LEBufToUint16(uint8_t *_pBuf);
uint32_t BEBufToUint32(uint8_t *_pBuf);
uint32_t LEBufToUint32(uint8_t *_pBuf);

uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen) ;

void vMBPortTimersEnable(void);
void vMBPortTimersDisable(void);


#endif

