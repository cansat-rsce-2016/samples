/*
 * i2c.h
 *
 *  Created on: 25 дек. 2015 г.
 *      Author: snork
 */

#ifndef I2C_H_
#define I2C_H_



uint8_t i2cGetStatus();
uint8_t i2cStart();
uint8_t i2cStop();
uint8_t i2cSendSLAW(uint8_t sla);
uint8_t i2cWrite(const void * bytes, uint8_t bytesCount);
uint8_t i2cRead(void * bytes, uint8_t bytesCount);



#endif /* I2C_H_ */
