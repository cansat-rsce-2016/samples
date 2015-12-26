/*
 * timer-interrput-pwm.c
 *
 *  Created on: 26 дек. 2015 г.
 *      Author: snork
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main() {
	TCCR0A = (1 << WGM01) | (1 << WGM00) // режим fastpwm
		| (1 << COM0A1) | (1 << COM0A0) // Clear OC0A on Compare Match, set OC0A at BOTTOM (non-inverting mode)
		| (1 << COM0B1) | (1 << COM0B0) // Clear OC0B on Compare Match, set OC0B at BOTTOM (non-inverting mode)
	;

	TCCR0B = (0 << WGM02); // режим fastpwm

	OCR0A = 0x00;
	OCR0B = 0x00;

	TIMSK0 = 0; // не используем прерывания

	// запускаем таймер
	TCCR0B |= (0 << CS02) | (0 << CS01) | (1 << CS00); // запуск таймера без предделителя

	// output идет на PD0 и PD1 (у меня они сожжены)

	while(1){
		// делаем что-нибудь полезное, например меняем значения OCR
		OCR0A += 1;
		OCR0B -= 1;
		_delay_ms(2);
	}

	return 0;
}
