/*
 * uart.c
 *
 *  Created on: Oct 26, 2017
 *      Author: Taylor
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Library includes. */
#include "mss_uart.h"
#include "system_m2sxxx.h"
#include <string.h>
#include <stdlib.h>

/* Application includes. */
#include "uart.h"

#define UART_BUFFER_SIZE	128

#define REPLY_QUEUE_SIZE	5

#define WIFI_SUCCESS		0
#define WIFI_ERROR			1
#define WIFI_DISCONNECT		2
#define WIFI_GOT_IP			3

static const IRQn_Type xUART0_IRQ = UART0_IRQn;
static const IRQn_Type xUART1_IRQ = UART1_IRQn;

static uint8_t uart0buffer[UART_BUFFER_SIZE];
static uint8_t uart1buffer[UART_BUFFER_SIZE];

static uint8_t copied_buffer[UART_BUFFER_SIZE];

static size_t uxUART0UnreadBytes;
static size_t uxUART1UnreadBytes;

TaskHandle_t xUART0RxTaskToNotify;
TaskHandle_t xUART1RxTaskToNotify;
TaskHandle_t xUART1UploadTaskToNotify;

SemaphoreHandle_t xUART1Mutex;

QueueHandle_t xReplyQueue;


void vInitializeUARTs(uint32_t ulBaud0, uint32_t ulBaud1)
{
	/* Initialize UART 0 */
	MSS_UART_init(&g_mss_uart0,
					ulBaud0,
					MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

	/* Initialize UART 1 */
	MSS_UART_init(&g_mss_uart1,
					ulBaud1,
					MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

	/* Create a mutex that will allow the UART to be accessed by multiple tasks
	 * Most likely will not be necessary, but just in case */
	xUART1Mutex = xSemaphoreCreateMutex();

	/* Create a queue to notify the upload task of replies from the module */
	xReplyQueue = xQueueCreate(REPLY_QUEUE_SIZE, sizeof(uint32_t));

	/* The interrupt handler makes use of FreeRTOS API functions, so its
	priority must be at or below the configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
	setting (the higher the numeric priority, the lower the logical priority). */
	NVIC_SetPriority( xUART0_IRQ, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
	NVIC_SetPriority( xUART1_IRQ, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );

	/* Set the UART Rx notification function to trigger after a single byte is received. */
	MSS_UART_set_rx_handler(&g_mss_uart0, prvUARTRxNotificationHandler, MSS_UART_FIFO_SINGLE_BYTE );
	MSS_UART_set_rx_handler(&g_mss_uart1, prvUARTRxNotificationHandler, MSS_UART_FIFO_SINGLE_BYTE );
}

void vTaskUARTBridge(void *pvParameters)
{
	uint32_t ulNumRecvBytes;
	uint8_t ucUARTByte;
	BaseType_t xResult;
	const mss_uart_instance_t *my_uart = (mss_uart_instance_t *) pvParameters;
	const uint8_t *my_buffer = (my_uart == &g_mss_uart0) ? uart0buffer : uart1buffer;
	size_t *uxUnreadBytes = (my_uart == &g_mss_uart0) ? &uxUART0UnreadBytes : &uxUART1UnreadBytes;
	size_t uxBytesRead;

	for( ;; )
	{
		/* Check if data has been received
		 * Block indefinitely until it has */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		/* Copy data before it can be overwritten */
		if(*uxUnreadBytes > 0)
		{
			taskENTER_CRITICAL();
			{
				strlcpy(copied_buffer, my_buffer, *uxUnreadBytes + 1);

				/* Subtract the number of bytes processed from the available bytes */
				*uxUnreadBytes -= *uxUnreadBytes;
			}
			taskEXIT_CRITICAL();
		}

		uxBytesRead = strlen(copied_buffer);

		/* Echo back all data to the terminal */
		prvUARTSend(&g_mss_uart0, (const uint8_t *) copied_buffer, strlen(copied_buffer));

		/* Do any special processing based on the origin on the data */
		if(my_uart == &g_mss_uart0)
		{
			prvProcessUART0(copied_buffer, strlen(copied_buffer));
		}
		else if(my_uart == &g_mss_uart1)
		{
			prvProcessUART1(copied_buffer, strlen(copied_buffer));
		}
	}
}

void vTaskUARTUploadData(void *pvParameters)
{
	const uint8_t ucConnectCmd[] = "AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n";
	const uint8_t ucSendCmd[] = "AT+CIPSEND=";
	const uint8_t ucGetReq[] = "GET /update?api_key=XXXXXXXXXXXXXXXX&field1=";
	const uint8_t ucField2[] = "&field2=";
	const uint8_t ucCloseCmd[] = "AT+CIPCLOSE\r\n";
	const uint8_t ucCRLF[] = "\r\n";

	uint32_t ulSensorData;
	uint32_t ulUploadStatus = 0;

	char cReqSize[5];
	char cCh0Val[5];
	char cCh1Val[5];
	char cSendCmdBuffer[30];
	char cGetReqBuffer[100];
	size_t xSendCmdLength;
	size_t xGetReqLength;

	BaseType_t xResult;
	TickType_t xDelay = pdMS_TO_TICKS(1000);

	for( ;; )
	{
		/* Once data from the sensor is ready, prepare the strings to be sent
		 * No fprintf available, so strings have to be done the hard way */
		xResult = xTaskNotifyWait(0, 0, &ulSensorData, portMAX_DELAY);

		/* Lower 16 bits of sensor data are Ch0 */
		itoa( (int) (ulSensorData & 0xFFFF), cCh0Val, 10);
		strcpy(cGetReqBuffer, (const char *) ucGetReq);
		strcat(cGetReqBuffer, cCh0Val);

		/* Upper 16 bits of sensor data are Ch1 */
		itoa( (int) ((ulSensorData >> 16) & 0xFFFF), cCh1Val, 10);
		strcat(strcat(strcat(cGetReqBuffer, (const char *) ucField2), cCh1Val), (const char *) ucCRLF);

		/* Find the size of the GET command */
		xGetReqLength = strlen(cGetReqBuffer);
		itoa( (int) xGetReqLength, cReqSize, 10);
		strcpy(cSendCmdBuffer, (const char *) ucSendCmd);
		strcat(strcat(cSendCmdBuffer, cReqSize), (const char *) ucCRLF);

		/* Find the size of the send cmd */
		xSendCmdLength = strlen(cSendCmdBuffer);

		/* The strings will look like:
		 *  AT+CIPSEND=xx\r\n
		 *  GET /update?api_key=XXXXXXXXXXXXXXXX&field1=xx&field2=xx\r\n
		 */

		do
		{
			/* Connect to the ThingSpeak server */
			/* Take the mutex for UART 1 so it cannot be interrupted */
			do
			{
				xSemaphoreTake(xUART1Mutex, portMAX_DELAY);
				/* Subtract 1 from the length since the null terminator is not transmitted */
				prvUARTSend(&g_mss_uart1, ucConnectCmd, sizeof(ucConnectCmd) - 1);
				/* Return the mutex */
				xSemaphoreGive(xUART1Mutex);

				/* Wait for the module to reply to the command */
				ulUploadStatus = prvGetReplyStatus();
				if(ulUploadStatus != WIFI_SUCCESS)
				{
					/* If an error is received, wait and try again */
					vTaskDelay( pdMS_TO_TICKS(500) );
				}
			} while(ulUploadStatus != WIFI_SUCCESS);

			/* Once a connection has been made, send the data */

			xSemaphoreTake(xUART1Mutex, portMAX_DELAY);
			/* Send the command with the number of bytes */
			prvUARTSend(&g_mss_uart1, (uint8_t *) cSendCmdBuffer, xSendCmdLength);
			/* Return the mutex */
			xSemaphoreGive(xUART1Mutex);

			/* Wait for the module to reply to the command */
			ulUploadStatus = prvGetReplyStatus();
			if( ulUploadStatus == WIFI_SUCCESS )
			{
				/* HTTP GET request */
				xSemaphoreTake(xUART1Mutex, portMAX_DELAY);
				/* Send the HTTP GET request to upload the data */
				prvUARTSend(&g_mss_uart1, cGetReqBuffer, xGetReqLength);
				/* Return the mutex */
				xSemaphoreGive(xUART1Mutex);

				/* Wait for the module to reply to the command */
				ulUploadStatus = prvGetReplyStatus();
			}

			/* Close the connection
			 * This part isn't necessary since the server will close the connection
			 */
		} while(xResult != pdPASS && ulUploadStatus != WIFI_SUCCESS);

		/* ThingSpeak only allows uploads every 15 seconds so delay until data can be uploaded again */
		vTaskDelay(pdMS_TO_TICKS(20*1000));
	}
}

static void prvUARTRxNotificationHandler( mss_uart_instance_t *pxUART )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	TaskHandle_t xTaskToNotify = (pxUART == &g_mss_uart0) ? xUART0RxTaskToNotify : xUART1RxTaskToNotify;
	uint8_t *target_buffer = (pxUART == &g_mss_uart0) ? uart0buffer : uart1buffer;

	size_t uxNumBytesRecvd;
	size_t *uxUnreadBytes = (pxUART == &g_mss_uart0) ? &uxUART0UnreadBytes : &uxUART1UnreadBytes;

	/* Read the UART's FIFO */
	uxNumBytesRecvd = MSS_UART_get_rx(pxUART, target_buffer + *uxUnreadBytes, sizeof(target_buffer));
	if( uxNumBytesRecvd )
	{
		/* Increment the number of unread bytes available */
		*uxUnreadBytes += uxNumBytesRecvd;

		/* Notify the receiving task */
		vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);

		/* portEND_SWITCHING_ISR() or portYIELD_FROM_ISR() can be used here. */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

static void prvProcessUART0(uint8_t *pcBuffer, uint32_t ulNumBytes)
{
	/* Actions to be taken with data received from UART 0
	 * Keep track of the characters added through the terminal
	 * Then once a \n has been received, forward the entire command to the ESP8266 */

	/* String to contain the entire command to be sent to the ESP8266 module */
	static uint8_t ucCommandString[UART_BUFFER_SIZE];
	static uint8_t pos = 0;
	uint8_t i;
    static uint8_t saved_pos = 0;
    static uint8_t crlf[] = { '\r', '\n' };

	/* Add characters onto the command string */
	memcpy(&ucCommandString[pos], pcBuffer, ulNumBytes);
	pos += ulNumBytes;

	if(ucCommandString[pos-1] == 13)
	{
		/* End of line has been received. Send to module */
		xSemaphoreTake(xUART1Mutex, portMAX_DELAY);

		prvUARTSend(&g_mss_uart1, (const uint8_t *) &ucCommandString, pos);

		prvUARTSend(&g_mss_uart0, (const uint8_t *) &crlf, 2);
		prvUARTSend(&g_mss_uart0, (const uint8_t *) &ucCommandString, pos);
		prvUARTSend(&g_mss_uart0, (const uint8_t *) &crlf, 2);

		xSemaphoreGive(xUART1Mutex);

		pos = 0;
	}
}

static void prvProcessUART1(uint8_t *pcBuffer, uint32_t ulNumBytes )
{
	/* Actions to be taken with data received from UART 1 */
	uint32_t ulUploadStatus;
	BaseType_t xResult;
	TickType_t x50msTimeout = pdMS_TO_TICKS(50);

	/* Keep track of the replies from the ESP8266 */
	static uint8_t ucReplyString[UART_BUFFER_SIZE];

	/* Various strings to check for in the replies */
	const uint8_t ucOK[] = "OK";
	const uint8_t ucConn[] = "ALREADY CONNECTED\r\n\r\nERROR";
	const uint8_t ucError[] = "ERROR";
	const uint8_t ucDiscon[] = "WIFI DISCONNECT";
	const uint8_t ucNoIP[] = "no ip";
	const uint8_t ucGotIP[] = "WIFI GOT IP";


	/* Append new characters to the reply string */
	strcat((char *) ucReplyString, (char *) pcBuffer);

	/* Check if an OK is contained in the buffer */
	if( (strstr(ucReplyString, ucOK) != NULL) || (strstr(ucReplyString, ucConn) != NULL) )
	{
		/* Once an "OK" or "ALREADY CONNECTED" has been detected, let the upload process continue */
		strcpy(ucReplyString, "");		// Erase the previous string
		ulUploadStatus = WIFI_SUCCESS;
		xResult = xQueueSendToBack(xReplyQueue, &ulUploadStatus, x50msTimeout);
	}
	else if( strstr(ucReplyString, ucDiscon) != NULL || strstr(ucReplyString, ucNoIP) != NULL )
	{
		/* If the connection to the network has been lost, tell the upload process to wait a while */
		strcpy(ucReplyString, "");
		ulUploadStatus = WIFI_DISCONNECT;
		xResult = xQueueSendToBack(xReplyQueue, &ulUploadStatus, x50msTimeout);
	}
	else if( strstr(ucReplyString, ucGotIP) != NULL)
	{
		strcpy(ucReplyString, "");
		ulUploadStatus = WIFI_GOT_IP;
		xResult = xQueueSendToBack(xReplyQueue, &ulUploadStatus, x50msTimeout);
	}
	else if( strstr(ucReplyString, ucError) != NULL )
	{
		/* Handle error */
		strcpy(ucReplyString, "");		// Erase the previous string
		ulUploadStatus = WIFI_ERROR;
		xResult = xQueueSendToBack(xReplyQueue, &ulUploadStatus, x50msTimeout);
	}
	else if( strlen(ucReplyString) > 100 )
	{
		/* Erase the string if the buffer starts filling up
		 * Prevents strcat from ending up out of bounds of the array */
		strcpy(ucReplyString, "");
	}
}

static uint32_t prvGetReplyStatus( void )
{
	BaseType_t xResult;
	TickType_t xTimeoutDelay = pdMS_TO_TICKS(1000);
	TickType_t xDisconnDelay = pdMS_TO_TICKS(10*1000);
	uint32_t ulUploadStatus = 0;


	xResult = xQueueReceive(xReplyQueue, &ulUploadStatus, xTimeoutDelay);
	if(xResult == pdPASS)
	{
		/* Data was successfully read from the queue */
		if( ulUploadStatus == WIFI_SUCCESS )
		{
			return WIFI_SUCCESS;
		}
		else if( ulUploadStatus == WIFI_ERROR)
		{
			return WIFI_ERROR;
		}
		else if( ulUploadStatus == WIFI_DISCONNECT )
		{
			/* If the network connection is lost, wait for a while to see if it reconnects */
			while( ulUploadStatus == WIFI_DISCONNECT)
			{
				vTaskDelay(xDisconnDelay);
				xResult = xQueueReceive(xReplyQueue, &ulUploadStatus, xTimeoutDelay);
			}
			/* Return an error to reset the communication */
			return WIFI_ERROR;
		}
	}

	/* If the function reaches here, no data was available in the queue */
	return WIFI_ERROR;
}

static void prvUARTSend(mss_uart_instance_t *pxUART, const uint8_t *pcBuffer, size_t xBufferLength)
{
	const TickType_t xVeryShortDelay = 2UL;

	if(xBufferLength > 0)
	{
		MSS_UART_irq_tx( (mss_uart_instance_t *) pxUART, pcBuffer, xBufferLength );

		/* Ensure any previous transmissions have completed.  The default UART
		interrupt does not provide an event based method of	signally the end of a Tx
		- this is therefore a crude poll of the Tx end status.  Replacing the
		default UART handler with one that 'gives' a semaphore when the Tx is
		complete would allow this poll loop to be replaced by a simple semaphore
		block. */
		while( MSS_UART_tx_complete( ( mss_uart_instance_t * ) pxUART ) == pdFALSE )
		{
			vTaskDelay( xVeryShortDelay );
		}
	}
}

