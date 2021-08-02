/*
 * leds.h
 *
 *  Created on: Oct 25, 2017
 *      Author: Taylor
 */

#ifndef RTOSDEMO_LEDS_H_
#define RTOSDEMO_LEDS_H_

void vInitializeLEDs( void );
void vSetLED( UBaseType_t uxLED, BaseType_t xValue );
void vToggleLED( UBaseType_t uxLED );
void vTaskSpinLEDs(void *pvParameters);

#endif /* RTOSDEMO_LEDS_H_ */
