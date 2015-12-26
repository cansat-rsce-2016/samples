/*
 * timer0_servo.c
 *
 *  Created on: 26 дек. 2015 г.
 *      Author: snork
 */


#include <avr/io.h>
#include <util/delay.h>

#define MAX_COUNTER_VALUE 0x2000
//0x9FFF
#define MIN_COUNTER_VALUE 0x200
//0x1000

int main() {
	// настраиваем таймер
	TCCR1A = (1 << WGM11) | (0 << WGM10) // режим fastpwm до ICRA
		| (1 << COM1B1) | (1 << COM1B0) // Clear OC1B on Compare Match, set OC1B at BOTTOM (non-inverting mode)
		| (1 << COM1A1) | (1 << COM1A0)
	;
	TCCR1B = (1 << WGM13) | (1 << WGM12); // режим fastpwm до OC1A

	ICR1 = MAX_COUNTER_VALUE;
	OCR1A = MAX_COUNTER_VALUE; // пототолок таймера
	OCR1B = MIN_COUNTER_VALUE; // начальное положение сервы

	// запускаем таймер
	TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10); // запуск таймера без предделителя

	while(1) {
		OCR1B += 1;
		if (OCR1B >= MAX_COUNTER_VALUE)
			OCR1B = MIN_COUNTER_VALUE;
		_delay_ms(100);
	}

	return 0;
}
