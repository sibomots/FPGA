/*
 * uart.h
 *
 *  Created on: Oct 26, 2017
 *      Author: Taylor
 */

#ifndef RTOSDEMO_INCLUDE_UART_H_
#define RTOSDEMO_INCLUDE_UART_H_

void vInitializeUARTs(uint32_t baud0, uint32_t baud1);
void vTaskUARTBridge(void *pvParameters);
void vTaskUARTUploadData(void *pvParameters);
static void prvUARTRxNotificationHandler( mss_uart_instance_t *pxUART );
static void prvUARTTxHandler( mss_uart_instance_t *pxUART );
static void prvProcessUART0(uint8_t *pcBuffer, uint32_t ulNumBytes);
static void prvProcessUART1(uint8_t *pcBuffer, uint32_t ulNumBytes);
static uint32_t prvGetReplyStatus( void );
static void prvUARTSend(mss_uart_instance_t *pxUART, const uint8_t *pcBuffer, size_t xBufferLength);

#endif /* RTOSDEMO_INCLUDE_UART_H_ */
