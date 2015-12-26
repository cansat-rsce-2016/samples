/*
 * adc-basic.c
 *
 * Пример работы с АЦП, основанный на прерываниях
 * Программа запускает АЦП на непрерывное преобразование.
 * Результаты преобразования складываются в линейный буфер, откуда, по его заполнению, забираются основным потоком.
 * Основной поток вычисляет среднее значение накопленных измерений АЦП и передает по UART в виде текста
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// размер буфера со значениями АЦП
#define ADC_VALUES_BUFFER_SIZE 300

// Контейнер для измерений АЦП
volatile uint16_t adcValuesBuffer[ADC_VALUES_BUFFER_SIZE];

// Каретка буфера измерений АЦП
volatile uint16_t adcBufferCarret = 0;

// обработчик прерывания АЦП. Забирает из него значение и складывает в буфер adcValuesBuffer
ISR(ADC_vect) {
	// если буффер измерений полон - выбрасываем текущее измерение, его некуда деть. Заканчиваем обработку прерывания
	if (adcBufferCarret >= ADC_VALUES_BUFFER_SIZE) {
		//volatile uint16_t adcw = ADCW;
		return;
	}

	// закладываем прочитанное в буфер по положению каретки
	adcValuesBuffer[adcBufferCarret] = ADCW;

	// перемещаем каретку на следующую позицию
	adcBufferCarret++;

	// флаг прерывания снимается автоматически, нам не нужно это делать вручную
}



// Инициалазция UART0 для передачи данных с АЦП в терминал
void initUartStdio();

int main() {
	// инициализируем UART для передачи значений, полученных с АЦП на компьютер
	// ==================================================
	initUartStdio();

	// Конфигурируем АЦП
	// =============================
	ADMUX = (0 << REFS1) | (1 << REFS0) // опорное напряжение - AVcc (напряжение питания контроллера = 5 Вольт
			| (0 << ADLAR) // выравнивание значения в буфере ADC по правой границе
			| (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0) // одиночный канал ADC0
	;

	ADCSRA = (1 << ADEN)   // Включаем модуль АЦП (просто подача питания)
			| (0 << ADSC)  // пока не начинаем преобразование
			| (1 << ADATE) // используем режим непрерывных измерений
			| (1 << ADIF)  // флаг прерывания изначально сброшен, а сбрасывается он записью в него единцы (можно вообще ничего не писать)
			| (1 << ADIE)  // используем прерывания
			| (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) // Используем максимальный делитель (128) частоты для модуля АЦП
														 // чтобы обеспечить высокую точность даже на 16МГЦ процессора
	;

	sei(); 	// включаем прерывания глобально
	ADCSRA |= (1 << ADSC); // запускаем первое преобразование АЦП, за которым пойдут остальные

	while(1) {
		// проверяем насколько заполнен буфер
		// для этого нам нужно посмотреть положение каретки буфера.
		// каретка буфера может быть записана из прерывания, поэтому у нас есть все шансы поймать гоночную ситуацию
		// чтобы избежать этого - снимаем копию значения каретки атомарной операцией и работаем с ней.
		// Мы знаем, что положение каретки не уменьшится, как только она достигнет буфера, поэтому можем так делать
		uint16_t adcBufferCarretSnapshot;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			adcBufferCarretSnapshot = adcBufferCarret;
		}

		if (adcBufferCarretSnapshot < ADC_VALUES_BUFFER_SIZE) {
			// буффер еще не заполнен, тут можно усыпить контроллер или заняться другими полезными вещами, пока он заполняется
			char buffer[50] = { 0x00 };
			sprintf(buffer, "buffer underflow: %d\r\n", adcBufferCarretSnapshot); // пишем сколько значений в буфере есть
			uartWrite(buffer);
			continue;
		}

		uint32_t adcAverageValue = 0;
		for (uint16_t i = 0; i < ADC_VALUES_BUFFER_SIZE; i++) {
			adcAverageValue += adcValuesBuffer[i];
		}
		adcAverageValue /= ADC_VALUES_BUFFER_SIZE;

		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			adcBufferCarret = 0; // сбрасываем каретку, показывая что мы прочитали буфер и пора набирать новый
		}

		printf("average adc value: 0x%08lx\r\n", adcAverageValue);
	}

	return 0;
}



static int myPutChar(char value, FILE * stream) {
	(void)stream; // не используем переменную. Таким образом глушим варнинг о неиспользуемой переменной

	while ( !(UCSR0A & (1 << UDRE0)) )
	{}

	UDR0 = value;
	return 0;
}

static int myGetChar(FILE * stream) {
	(void)stream; // не используем переменную. Таким образом глушим варнинг о неиспользуемой переменной

	while ( !(UCSR0A & (1 << RXC0)) )
	{}

	return UDR0;
}


// глобальная переменная stdout
FILE mystdout = FDEV_SETUP_STREAM(myPutChar, NULL, _FDEV_SETUP_WRITE);
FILE mystdin = FDEV_SETUP_STREAM(NULL, myGetChar, _FDEV_SETUP_READ);

void initUartStdio() {
	UCSR0B = (1 << TXEN0) | (1 << RXEN0); // включаем TX RX
	;
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01) // Размер символа - 8 бит
		| (0 << UPM00) | (0 << UPM01)      // Бит чертности отключен
		| (0 << USBS0) // 1 стоп бит
	;

	// baud на 9600 по таблице на частоте в 16мгц
	UBRR0H = 103 / 0xFF;
	UBRR0L = 103 % 0xFF;

	stdout = &mystdout;
	stdin = &mystdin;
}

