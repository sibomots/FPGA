/*
 * leds.c
 *
 *  Created on: Oct 25, 2017
 *      Author: Taylor
 */

/* Functions involving the use of the LEDS on the Maker Board *
 * Remember that the LEDs are ACTIVE LOW! */


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include "mss_gpio.h"
#include "system_m2sxxx.h"

/* Application includes. */
#include "leds.h"

#define boardNUM_LEDS	8

/* Remember the state of the outputs for easy toggling. */
static unsigned char ucPortState = 0;

/* MSS GPIOs */
static const mss_gpio_id_t ucLEDs[ boardNUM_LEDS ] = { MSS_GPIO_0, MSS_GPIO_1, MSS_GPIO_2, MSS_GPIO_3,
													MSS_GPIO_4, MSS_GPIO_5, MSS_GPIO_6, MSS_GPIO_7 };

void vInitializeLEDs(void)
{
	int x;

	/* Ensure the LEDs are outputs and HIGH (OFF) to start. */
	for( x = 0; x < boardNUM_LEDS; x++ )
	{
		MSS_GPIO_config( ucLEDs[ x ], MSS_GPIO_OUTPUT_MODE );
		vSetLED( x, pdTRUE );
	}
}

void vSetLED(unsigned portBASE_TYPE uxLED, signed portBASE_TYPE xValue)
{
	if( uxLED < boardNUM_LEDS )
	{
		taskENTER_CRITICAL();
		{
			MSS_GPIO_set_output( ucLEDs[ uxLED ], xValue );

			/* Remember the new output state. */
			if( xValue == 0 )
			{
				ucPortState &= ~( 1 << uxLED );
			}
			else
			{
				ucPortState |= ( 1 << uxLED );
			}
		}
		taskEXIT_CRITICAL();
	}
}

void vToggleLED(unsigned portBASE_TYPE uxLED)
{
	if( uxLED < boardNUM_LEDS )
	{
		taskENTER_CRITICAL();
		{
			vSetLED( uxLED, !( ucPortState & ( 1 << uxLED ) ) );
		}
		taskEXIT_CRITICAL();
	}
}

void vTaskSpinLEDs(void *pvParameters)
{
	/* Create a spinning effect with the LEDs */
	static unsigned char ucLEDPos = 1;
	static signed char direction = 1;
	const TickType_t xDelay85ms = pdMS_TO_TICKS(85);

	for( ;; )
	{
		if( ucLEDPos == boardNUM_LEDS - 1 || ucLEDPos == 0)
		{
			// Change direction
			direction *= -1;

			taskENTER_CRITICAL();
			{
				// Turn on current LED
				vSetLED(ucLEDPos, pdFALSE);
				// Turn off last LED
				vSetLED(ucLEDPos+direction, pdTRUE);
			}
			taskEXIT_CRITICAL();

			// Increment position
			ucLEDPos+= direction;
		}
		else
		{
			taskENTER_CRITICAL();
			{
				// Turn off last LED
				vSetLED(ucLEDPos-direction, pdTRUE);
				// Turn on current LED
				vSetLED(ucLEDPos, pdFALSE);
			}
			taskEXIT_CRITICAL();

			// Increment position
			ucLEDPos+= direction;
		}

		vTaskDelay(xDelay85ms);
	}
}
