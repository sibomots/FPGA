/*
 * i2c.h
 *
 *  Created on: Oct 27, 2017
 *      Author: Taylor
 */

#ifndef RTOSDEMO_INCLUDE_I2C_H_
#define RTOSDEMO_INCLUDE_I2C_H_

void vInitializeI2C(void);
void vTaskReadLightSensor(void *pvParameters);

#endif /* RTOSDEMO_INCLUDE_I2C_H_ */
