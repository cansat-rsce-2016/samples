/*
 * i2c-imu.c
 *
 *  Created on: 24 дек. 2015 г.
 *      Author: snork
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <float.h>

#include <avr/io.h>

#include <util/delay.h>
#include <util/twi.h>

#include "i2c.h"

#define BMP085_SLAW 0xEE
#define BMP085_SLAR 0xEF


void initStdios();


typedef struct {
	int16_t ac1, ac2, ac3;
	uint16_t ac4, ac5, ac6;
	int16_t b1, b2;
	int16_t mb, mc, md;

} BMP085Calibration;


void bmp085LoadCalibration(BMP085Calibration * cal) {

	const uint8_t calRegAddr = 0xAA;
	uint8_t status;

	status = i2cStart();
	if (status != 0) {
		printf("i2cStart error 0x%X\r\n", status);
		return;
	}
	status = i2cSendSLAW(BMP085_SLAW);
	if (status != 0) {
		printf("i2cSendSLAW error 0x%X\r\n", status);
		return;
	}
	status = i2cWrite(&calRegAddr, 1);
	if (status != 0) {
		printf("i2cWrite error 0x%X\r\n", status);
		return;
	}


	status = i2cStart();
	if (status != 0) {
		printf("i2cWrite start error 0x%X\r\n", status);
		return;
	}
	status = i2cSendSLAW(BMP085_SLAR);
	if (status != 0) {
		printf("i2cSendSLAW error 0x%X\r\n", status);
		return;
	}
	status = i2cRead(cal, sizeof(*cal));
	if (status != 0) {
		printf("i2cRead error 0x%X\r\n", status);
		return;
	}
	status = i2cStop();
	if (status != 0) {
		printf("i2cStop error 0x%X\r\n", status);
		return;
	}
}



int main()
{
	DDRB |= (1 << 5);

	initStdios();

	TWBR = 12;
	TWSR = (0 << TWPS0) | (0 << TWPS1);


	BMP085Calibration cal;
	bmp085LoadCalibration(&cal);

	printf("success!\r\n");
	printf("%d %d %d %d %d %d\r\n", cal.ac1, cal.ac2, cal.ac3, cal.ac4, cal.ac5, cal.ac6);
	printf("%d %d", cal.b1, cal.b2);
	printf("%d %d %d", cal.mb, cal.mc, cal.md);

	while(1)
	{
		//printf("test\r\n");
	}

	return 0;
}


int customPutChar(char value, FILE * stream) {
	(void)stream;

	while (!(UCSR0A & (1 << UDRE0))) // ждем, пока предыущий байт не покинет буфер
	{}

	UDR0 = value; // передаем текущий байт в буффер
	return 0; // нет ошибки
}

FILE custom_stdout = FDEV_SETUP_STREAM(customPutChar, NULL, _FDEV_SETUP_WRITE);

// инициализация STDOUT в UART0
// baud rate 9600; parity: none; stop bits: one
// подразумевается, что контроллер запущен на 16 МГЦ
void initStdios() {
	// инициализация UART
	UCSR0B = (1 << TXEN0)  // включаем только TX
	;
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01) // Размер символа - 8 бит
		| (0 << UPM00) | (0 << UPM01)      // Бит чертности отключен
		| (0 << USBS0) // 1 стоп бит
	;

	// baud на 9600 по таблице на частоте в 16мгц
	UBRR0H = 103 / 0xFF;
	UBRR0L = 103 % 0xFF;

	// инициализция STDOUT
	stdout = &custom_stdout;
}
