/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"
#include "stm32wlxx_hal.h"
#include "usart.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
// static void prvvUARTTxReadyISR( void );
// static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( _Bool xRxEnable, _Bool xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    if (xRxEnable)                                  //将串口收发中断和modbus联系起来，下面的串口改为自己使能的串口
    {
        __HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);	//我用的是串口2，故为&huart2
    }
    else
    {
        __HAL_UART_DISABLE_IT(&huart2,UART_IT_RXNE);
    }
    if (xTxEnable)
    {
        __HAL_UART_ENABLE_IT(&huart2,UART_IT_TXE);
    }
    else
    {
        __HAL_UART_DISABLE_IT(&huart2,UART_IT_TXE);
    }
}

_Bool
xMBPortSerialInit( uint8_t ucPORT, uint32_t ulBaudRate, uint8_t ucDataBits, eMBParity eParity )
{
    return TRUE;
}

_Bool xMBPortSerialPutByte(int8_t ucByte)
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    if (HAL_UART_Transmit(&huart2, (uint8_t *)&ucByte, 1, 0xFF) != HAL_OK) // 添加发送一位代码
        return FALSE;
    else
        return TRUE;
}

_Bool xMBPortSerialGetByte(int8_t *pucByte)
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    if (HAL_UART_Receive(&huart2, (uint8_t *)pucByte, 1, 0xFF) != HAL_OK) // 添加接收一位代码
        return FALSE;
    else
        return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}
