/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/******************************************************************************
 * This project provides two demo applications.  A simple blinky style project,
 * and a more comprehensive test and demo application.  The
 * configCREATE_SIMPLE_BLINKY_DEMO_ONLY setting (defined in FreeRTOSConfig.h) is
 * used to select between the two.  The simply blinky demo is implemented and
 * described in main_blinky.c.  The more comprehensive test and demo application
 * is implemented and described in main_full.c.
 *
 * This file implements the code that is not demo specific, including the
 * hardware setup and FreeRTOS hook functions.
 *
 */

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Driver includes. */
#include "mss_uart.h"	// For baud rate defines and instances

/* Application includes. */
#include "leds.h"
#include "uart.h"
#include "i2c.h"

/* Set to 1 to enable the upload tasks once the module has been set up */
#define ENABLE_UPLOAD

/* External variables */
extern TaskHandle_t xUART0RxTaskToNotify;
extern TaskHandle_t xUART1RxTaskToNotify;
extern TaskHandle_t xUART1UploadTaskToNotify;

/*
 * Set up the hardware ready to run this demo.
 */
static void prvSetupHardware( void );

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
within this file. */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );

/*-----------------------------------------------------------*/
/* See the documentation page for this demo on the FreeRTOS.org web site for
full information - including hardware setup requirements. */

int main( void )
{
	BaseType_t status;

	/* Prepare the hardware to run this demo. */
	prvSetupHardware();

	// Create LED spinning task
	status = xTaskCreate(	vTaskSpinLEDs,				// The task function that spins the LEDs
							"LED Spinner",				// Text name for debugging
							1000,						// Size of the stack allocated for this task
							NULL,						// Task parameter is not used
							1,							// Task runs at priority 1
							NULL);						// Task handle is not used

	// Create UART0 RX Task
	status = xTaskCreate(	vTaskUARTBridge,			// The task function that handles all UART RX events
							"UART0 Receiver",			// Text name for debugging
							1000,						// Size of the stack allocated for this task
							(void *) &g_mss_uart0,		// Task parameter is the UART instance used by the task
							2,							// Task runs at priority 2
							&xUART0RxTaskToNotify);		// Task handle for task notification

	// Create UART1 RX Task
	status = xTaskCreate(	vTaskUARTBridge,			// The task function that handles all UART RX events
							"UART1 Receiver",			// Text name for debugging
							1000,						// Size of the stack allocated for this task
							(void *) &g_mss_uart1,		// Task parameter is the UART instance used by the task
							2,							// Task runs at priority 2
							&xUART1RxTaskToNotify);		// Task handle for task notification

#ifdef ENABLE_UPLOAD
	// Create I2C Sensor Task
	status = xTaskCreate(	vTaskReadLightSensor,		// The task function that reads the I2C sensor
							"I2C Sensor",				// Text name for debugging
							1000,						// Size of the stack allocated for this task
							NULL,						// Task parameter is not used
							1,							// Task runs at priority 1
							NULL);						// Task handle not used at this time


	// Create UART 1 Upload Task
	status = xTaskCreate(	vTaskUARTUploadData,		// The task function that sends sensor data to the ESP8266
							"UART Upload",				// Text name for debugging
							1000,						// Size of the stack allocated for this task
							NULL,						// Task parameter is not used
							2,							// Task runs at priority 2
							&xUART1UploadTaskToNotify);	// Task handle for task notification
#endif

	vTaskStartScheduler();

	return 0;
}

/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Perform any configuration necessary to use the hardware peripherals on the board. */
	vInitializeLEDs();

	/* UARTs are set for 8 data - no parity - 1 stop bit, see the vInitializeUARTs function to modify
	 * UART 0 set to 115200 to connect to terminal, 38400 to connect to ESP8266 */
	vInitializeUARTs(MSS_UART_115200_BAUD, MSS_UART_38400_BAUD);

	vInitializeI2C();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */

}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
}
/*-----------------------------------------------------------*/
