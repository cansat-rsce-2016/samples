/*
 * i2c.c
 *
 *  Created on: 25 дек. 2015 г.
 *      Author: snork
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <util/twi.h>

#include "i2c.h"

inline uint8_t i2cGetStatus()
{
	while (!(TWCR & (1 << TWINT))) {}

	return TWSR & 0xF8;
}


inline uint8_t i2cStart() {
	TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);

	uint8_t status = i2cGetStatus();
	printf("i2c start status 0x%X\r\n", status);
	if (0x08 == status || 0x10 == status)
		return 0;
	else
		return status;
}

inline uint8_t i2cStop() {
	TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);

	return 0;
}


inline uint8_t i2cSendSLAW(uint8_t sla) {
	TWDR = sla;
	TWCR = (1 << TWEN) | (1 << TWINT);

	printf("here\r\n");
	uint8_t status = i2cGetStatus();
	if (0x18 == status || 0x40 == status)
		return 0;
	else
		return status;
}


inline uint8_t i2cWrite(const void * bytes, uint8_t bytesCount) {
	printf("here now\r\n");
	for (uint8_t i = 0; i < bytesCount; i++) {
		TWDR = ((uint8_t*)bytes)[i];
		TWCR = (1 << TWEN) | (1 << TWINT);

		uint8_t status = i2cGetStatus();
		if (0x28 == status || 0x50 == status)
			continue;
		else
			return status;
	}

	return 0;
}


inline uint8_t i2cRead(void * bytes, uint8_t bytesCount) {
	for (uint8_t i = 0; i < bytesCount; i++) {

		TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
		((uint8_t*)bytes)[i] = TWDR;
		uint8_t status = i2cGetStatus();

		if (0x28 == status || 0x50 == status)
			continue;
		else
			return status;
	}

	return 0;
}

