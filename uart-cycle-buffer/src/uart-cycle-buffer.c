/*
 * Продвинутый пример использования UART
 * Программа запускает UART с настройками baudRate: 9600, stopBits: 1, parity: none, charSize: 8bit
 * Затем, используя прерывания, программа собирает все данные приходящии по UART RX в кольцевой буффер
 * и возвращает в TX повторив каждый символ дважды
 *
 * Если буффер переполняется, контроллер зажигает диод на пине PB5
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "cycleBuffer.h"

// регистр DDR порта с сигнальным светодиодом
#define OVERFLOW_LED_DDR   		DDRB
// регистр PORT для сигнала о переполнении буффера
#define OVERFLOW_LED_PORT  		PORTB
// маска пина для сигнала о переполнении буффера
#define OVERFLOW_LED_PIN_MASK   (1 << 5);


// линейный буффер для данных, приходящих по RX
uint8_t rxBufferData[100];
// циклический буффер для данных, приходящих по RX
CycleBuffer rxbuffer = {rxBufferData, sizeof(rxBufferData), 0, 0};

// линейный буффер для данных, подлежащих отправки по TX
uint8_t txBufferData[100];
// циклический буффер данных для данных, подлежащих отправки по TX
CycleBuffer txbuffer = {txBufferData, sizeof(txBufferData), 0, 0};



// обработчик прерывания по новому символу в буффере UART RX
ISR(USART_RX_vect) {
	uint8_t rxByte; // буффер для полученного байта
	rxByte = UDR0; // получаем байт от UART модуля

	// пробуем вложить байт в кольцевой буффер
	if (!cycleBuffer_pushBack(&rxbuffer, rxByte)) {
		// если что-то пошло не так, что мы можем сделать?
		OVERFLOW_LED_PORT |= OVERFLOW_LED_PIN_MASK;

	}

}


// обработчик прерывания по опустошению буффера TX
ISR(USART_UDRE_vect) {
	// пробуем достать из кольцевого RX буффера
	uint8_t nextByte;
	if (cycleBuffer_popFront(&txbuffer, &nextByte)) {
		// отлично, байт получен - передаем его дальше в UART
		UDR0 = nextByte;
	} else {
		// байт не был получен - буффер пустой. Включаем прерывания UDRE
		// основная программа его включит, когда у неё будут данные
		UCSR0B &= ~(1 << UDRIE0);
	}

}


int main() {

	// Настройка переферии
	// ===============================================

	// Настраиваем пин сигнального светодиода на вывод
	OVERFLOW_LED_DDR |= OVERFLOW_LED_PIN_MASK;

	// настраиваем UART
	UCSR0B = (1 << TXEN0) | (1 << RXEN0)   // включаем TX и RX
		| (1 << RXCIE0) // включаем прерывание для RX флага
	;
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01)  // размер символа на 8 бит
		| (0 << UPM00) | (0 << UPM01)       // выключаем parity
		| (0 << USBS0) // 1 стоп бит
	;

	// бауд на 9600
	UBRR0H = 103 / 0xFF;
	UBRR0L = 103 % 0xFF;

	// глобально включаем прерывания.
	sei();

	while(1) {

	}

	return 0;
}


