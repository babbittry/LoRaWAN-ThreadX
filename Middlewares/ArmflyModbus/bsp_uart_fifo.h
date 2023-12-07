/*
*********************************************************************************************************
*
*	ģ������ : �����ж�+FIFO����ģ��
*	�ļ����� : bsp_uart_fifo.h
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_USART_FIFO_H_
#define _BSP_USART_FIFO_H_

/*
    ���ڷ��䣺
    ������1�� ��ӡ���Կ�
        PB6/USART1_TX
        PB7/USART1_RX

    ������2  RS485 ͨ�� - TTL ���� �� ����
        PA2/USART2_TX
        PA3/USART2_RX
*/


#define	UART1_FIFO_EN	0
#define	UART2_FIFO_EN	1


/* PB2 ����RS485оƬ�ķ���ʹ�� */
#define RS485_TXEN_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define RS485_TXEN_GPIO_PORT              RS485_CON_GPIO_Port
#define RS485_TXEN_PIN                    RS485_CON_Pin

//#define RS485_RX_EN()	RS485_TXEN_GPIO_PORT->BSRRH = RS485_TXEN_PIN
//#define RS485_TX_EN()	RS485_TXEN_GPIO_PORT->BSRRL = RS485_TXEN_PIN

#define RS485_RX_EN()	RS485_TXEN_GPIO_PORT->BSRR = ((uint32_t)RS485_TXEN_PIN << 16U)
#define RS485_TX_EN()	RS485_TXEN_GPIO_PORT->BSRR = RS485_TXEN_PIN

/* ����˿ں� */
typedef enum
{
    COM1 = 0,	/* USART1 */
    COM2 = 1,	/* USART2 */
}COM_PORT_E;

/* ���崮�ڲ����ʺ�FIFO��������С����Ϊ���ͻ������ͽ��ջ�����, ֧��ȫ˫�� */
#if UART1_FIFO_EN == 1
    #define UART1_BAUD			921600
    #define UART1_TX_BUF_SIZE	1*1024
    #define UART1_RX_BUF_SIZE	1*1024
#endif

#if UART2_FIFO_EN == 1
    #define UART2_BAUD			115200
    #define UART2_TX_BUF_SIZE	1*1024
    #define UART2_RX_BUF_SIZE	1*1024
#endif


/* �����豸�ṹ�� */
typedef struct
{
    USART_TypeDef *uart;		/* STM32�ڲ������豸ָ�� */
    uint8_t *pTxBuf;			/* ���ͻ����� */
    uint8_t *pRxBuf;			/* ���ջ����� */
    uint16_t usTxBufSize;		/* ���ͻ�������С */
    uint16_t usRxBufSize;		/* ���ջ�������С */
    __IO uint16_t usTxWrite;	/* ���ͻ�����дָ�� */
    __IO uint16_t usTxRead;		/* ���ͻ�������ָ�� */
    __IO uint16_t usTxCount;	/* �ȴ����͵����ݸ��� */

    __IO uint16_t usRxWrite;	/* ���ջ�����дָ�� */
    __IO uint16_t usRxRead;		/* ���ջ�������ָ�� */
    __IO uint16_t usRxCount;	/* ��δ��ȡ�������ݸ��� */

    void (*SendBefor)(void); 	/* ��ʼ����֮ǰ�Ļص�����ָ�루��Ҫ����RS485�л�������ģʽ�� */
    void (*SendOver)(void); 	/* ������ϵĻص�����ָ�루��Ҫ����RS485������ģʽ�л�Ϊ����ģʽ�� */
    void (*ReciveNew)(uint8_t _byte);	/* �����յ����ݵĻص�����ָ�� */
    uint8_t Sending;			/* ���ڷ����� */
}UART_T;

void bsp_InitUart(void);
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte);
uint8_t comGetChar(COM_PORT_E _ucPort, uint8_t *_pByte);
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comClearTxFifo(COM_PORT_E _ucPort);
void comClearRxFifo(COM_PORT_E _ucPort);
void comSetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate);

void USART_SetBaudRate(USART_TypeDef* USARTx, uint32_t BaudRate);
void bsp_SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode);

void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen);
void RS485_SendStr(char *_pBuf);
void RS485_SetBaud(uint32_t _baud);
uint8_t UartTxEmpty(COM_PORT_E _ucPort);

// void USART1_IRQHandler(void);
void USART2_IRQHandler(void);


#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
