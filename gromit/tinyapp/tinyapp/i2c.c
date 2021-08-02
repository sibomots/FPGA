/*
 * i2c.c
 *
 *  Created on: Oct 27, 2017
 *      Author: Taylor
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include "mss_i2c.h"
#include "system_m2sxxx.h"

/* Application includes. */
#include "i2c.h"

#define I2C_BUFFER_SIZE	10

static uint8_t ucI2CTxBuffer[I2C_BUFFER_SIZE];
static uint8_t ucI2CRxBuffer[I2C_BUFFER_SIZE];

extern TaskHandle_t xUART1UploadTaskToNotify;

void vInitializeI2C(void)
{
	/* Initialize I2C0 to use with the light sensor
	 * It will only be used as a master so use 0x00 for its slave addr */
	MSS_I2C_init(&g_mss_i2c0, 0x00, MSS_I2C_PCLK_DIV_256);

	/* Initialize I2C1 if necessary */
}

void vTaskReadLightSensor(void *pvParameters)
{
	/* 7 Bit Slave addr for the light sensor */
	const uint8_t ucSensorSlaveAddr = 0x29;
	uint8_t ucData_Status;
	uint8_t ucData_Valid;
	uint16_t usCh0_Data;
	uint16_t usCh1_Data;
	uint32_t ulSensorData;

	BaseType_t xResult;

	/* Wait 200 ms for sensor initial start up */
	vTaskDelay( pdMS_TO_TICKS(100) );

	/* Start the sensor when the task is created */
	ucI2CTxBuffer[0] = 0x80;
	ucI2CTxBuffer[1] = 0x01;
	MSS_I2C_write(&g_mss_i2c0, ucSensorSlaveAddr, ucI2CTxBuffer, 2, MSS_I2C_RELEASE_BUS);

	/* Wait for the transaction to complete
	 * Instead of using the MSS_I2C_wait_complete() function which will keep this task stuck in the running state
	 * Just block the task for a few ms and come back */
	while( MSS_I2C_get_status( &g_mss_i2c0 ) != MSS_I2C_SUCCESS)
	{
		vTaskDelay( pdMS_TO_TICKS(5) );
	}

	/* Wait 10 ms for wakeup from standby */
	vTaskDelay( pdMS_TO_TICKS(10) );

	for( ;; )
	{
		/* Check status register before reading */
		ucI2CTxBuffer[0] = 0x8C;
		MSS_I2C_write_read(&g_mss_i2c0, ucSensorSlaveAddr, ucI2CTxBuffer, 1, ucI2CRxBuffer, 1, MSS_I2C_RELEASE_BUS);
		while( MSS_I2C_get_status( &g_mss_i2c0 ) != MSS_I2C_SUCCESS )
		{
			/* Wait 2 ms for the transfer to complete
			 * Block to allow other processes to run in the meantime */
			vTaskDelay( pdMS_TO_TICKS(2) );
		}

		ucData_Status = ucI2CRxBuffer[0] & 0x4;

		if( ucData_Status )
		{
			/* New data is available. Registers must be read in the following order */

			/* Read CH1 data first */
			ucI2CTxBuffer[0] = 0x88;	// Low byte
			MSS_I2C_write_read(&g_mss_i2c0, ucSensorSlaveAddr, ucI2CTxBuffer, 1, &ucI2CRxBuffer[0], 1, MSS_I2C_RELEASE_BUS);
			while( MSS_I2C_get_status( &g_mss_i2c0 ) != MSS_I2C_SUCCESS)
			{
				/* Wait 2 ms for the transfer to complete
				 * Block to allow other processes to run in the meantime */
				vTaskDelay( pdMS_TO_TICKS(2) );
			}

			ucI2CTxBuffer[0] = 0x89;	// High byte
			MSS_I2C_write_read(&g_mss_i2c0, ucSensorSlaveAddr, ucI2CTxBuffer, 1, &ucI2CRxBuffer[1], 1, MSS_I2C_RELEASE_BUS);
			MSS_I2C_wait_complete(&g_mss_i2c0, 2);	// Wait up to 2 ms for the transfer to complete
			while( MSS_I2C_get_status( &g_mss_i2c0 ) != MSS_I2C_SUCCESS)
			{
				/* Wait 2 ms for the transfer to complete
				 * Block to allow other processes to run in the meantime */
				vTaskDelay( pdMS_TO_TICKS(2) );
			}

			/* Read CH0 data next */
			ucI2CTxBuffer[0] = 0x8A;	// Low byte
			MSS_I2C_write_read(&g_mss_i2c0, ucSensorSlaveAddr, ucI2CTxBuffer, 1, &ucI2CRxBuffer[2], 1, MSS_I2C_RELEASE_BUS);
			MSS_I2C_wait_complete(&g_mss_i2c0, 2);	// Wait up to 2 ms for the transfer to complete
			while( MSS_I2C_get_status( &g_mss_i2c0 ) != MSS_I2C_SUCCESS)
			{
				/* Wait 2 ms for the transfer to complete
				 * Block to allow other processes to run in the meantime */
				vTaskDelay( pdMS_TO_TICKS(2) );
			}

			ucI2CTxBuffer[0] = 0x8B;	// High byte
			MSS_I2C_write_read(&g_mss_i2c0, ucSensorSlaveAddr, ucI2CTxBuffer, 1, &ucI2CRxBuffer[3], 1, MSS_I2C_RELEASE_BUS);
			MSS_I2C_wait_complete(&g_mss_i2c0, 2);	// Wait up to 2 ms for the transfer to complete
			while( MSS_I2C_get_status( &g_mss_i2c0 ) != MSS_I2C_SUCCESS)
			{
				/* Wait 2 ms for the transfer to complete
				 * Block to allow other processes to run in the meantime */
				vTaskDelay( pdMS_TO_TICKS(2) );
			}

			usCh0_Data = (ucI2CRxBuffer[3] << 8) | ucI2CRxBuffer[2];
			usCh1_Data = (ucI2CRxBuffer[1] << 8) | ucI2CRxBuffer[0];
			ulSensorData = (usCh1_Data << 16) | usCh0_Data;

			/* Send data to the upload task
			 * Any data already there is overwritten so only the most recent value is ever uploaded */
			xResult = xTaskNotify(xUART1UploadTaskToNotify, ulSensorData, eSetValueWithOverwrite);
			configASSERT(xResult == pdPASS);
		}

		/* Wait the specified sample period
		 * Default is 500 ms */
		vTaskDelay( pdMS_TO_TICKS(500) );
	}

}
