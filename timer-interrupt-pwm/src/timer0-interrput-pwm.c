/*
 * timer-interrput-pwm.c
 *
 *  Created on: 26 дек. 2015 г.
 *      Author: snork
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define PWMA_DDR_REG  DDRC
#define PWMA_PORT_REG PORTC
#define PWMA_PIN_MASK (1 << 0)

#define PWMB_DDR_REG  DDRC
#define PWMB_PORT_REG PORTC
#define PWMB_PIN_MASK (1 << 1)


ISR(TIMER0_OVF_vect) {
	// включаем в 1 оба PWM сигнала
	PWMA_PORT_REG |= PWMA_PIN_MASK;
	PWMB_PORT_REG |= PWMB_PIN_MASK;
}


ISR(TIMER0_COMPA_vect) {
	// выключаем сигнал А
	PWMA_PORT_REG &= ~PWMA_PIN_MASK;

}


ISR(TIMER0_COMPB_vect) {
	// выключаем сигнал B
	PWMB_PORT_REG &= ~PWMB_PIN_MASK;
}



int main() {
	PWMA_DDR_REG |= PWMA_PIN_MASK;
	PWMB_DDR_REG |= PWMB_PIN_MASK;

	TCCR0A = (0 << WGM01) | (0 << WGM00); // нормальный режим работы
	TCCR0B = (0 << WGM02); // нормальный режим работы

	OCR0A = 0x00;
	OCR0B = 0x00;

	TIMSK0 = (1 << OCIE0A) | (1 << OCIE0B) | (1 << TOIE0); // включаем все прервывания таймера


	// включаем прерывания глобально
	sei();

	// запускаем таймер
	TCCR0B |= (0 << CS02) | (0 << CS01) | (1 << CS00); // запуск таймера без предделителя

	while(1){
		// делаем что-нибудь полезное, например меняем значения OCR
		OCR0A += 1;
		OCR0B -= 1;
		_delay_ms(2);
	}

	return 0;
}
