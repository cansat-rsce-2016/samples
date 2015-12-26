#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <avr/io.h>
#include <util/delay.h>


void initUartStdio();

int main()
{
	initUartStdio();
	uint8_t i = 0;
	while(1){
		int a, b;
		scanf("%d %d", &a, &b);
		printf("Hello world %d %d %d\r\n", a, b, a+b);
		i++;
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
