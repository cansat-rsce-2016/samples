/*
 * Пример работы по протоколу OneWire
 * В примере ведется работа с утройством iButton - популярный электронный ключ.
 * Программа читает серийный номер ключа и пишет его в UART (baud: 9600, stop_bits: 1, parity: none)
 * Для правильной работы UART контроллер должен быть запущен на 16 мгц
 *
 * Для работы программы необходимо подключить iButton аналогично схеме из папки schematic проекта,
 * (или сконфигурировать свой пин в блоке макросов ниже)
 *
 * Код основан на примере http://aterlux.ru/index.php?page=article&art=1wire авторства Погребняка Дмитрия
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>

#define ONEWIRE_PORT PORTD  // регистр PORT используемого вывода
#define ONEWIRE_DDR DDRD    // регистр DDR используемого вывода
#define ONEWIRE_PIN PIND    // регистр PIN используемого вывода
#define ONEWIRE_PIN_NUM PD4 // номер вывода в порту

// Устанавливает низкий уровень на шине 1-wire
inline void onewire_setBusLow() {
	ONEWIRE_DDR |= (1 << ONEWIRE_PIN_NUM);
}

// "Отпускает" шину 1-wire
inline void onewire_setBusHigh() {
	ONEWIRE_DDR &= ~(1 << ONEWIRE_PIN_NUM);
}

// Читает значение уровня на шине 1-wire
inline uint8_t onewire_readBus() {
	return ONEWIRE_PIN & (1 << ONEWIRE_PIN_NUM);
}


// Выдаёт импульс сброса (reset), ожидает ответный импульс присутствия.
// Если импульс присутствия получен, дожидается его завершения и возвращает 1, иначе возвращает 0
uint8_t onewire_reset()
{
	onewire_setBusLow();
	_delay_us(640); // Пауза 480..960 мкс
	onewire_setBusHigh();
	_delay_us(2); // Время необходимое подтягивающему резистору, чтобы вернуть высокий уровень на шине
	_delay_us(60); // Ждём не менее 60 мс до появления импульса присутствия;

	if (!onewire_readBus()) {
		// Если обнаружен импульс присутствия, ждём его окончания
		while (!onewire_readBus()) {
			// Ждём конца сигнала присутствия
		}
		return 1;
	}
	_delay_us(1);
	return 0;
}


// Отправляет один бит
// bit отправляемое значение, 0 - ноль, любое другое значение - единица
void onewire_send_bit(uint8_t bit) {
	if (bit) {
		onewire_setBusLow();
		_delay_us(5); // Низкий импульс, от 1 до 15 мкс (с учётом времени восстановления уровня)

		onewire_setBusHigh();
		_delay_us(90); // Ожидание до завершения таймслота (не менее 60 мкс)
	} else {
		onewire_setBusLow();
		_delay_us(90); // Высокий уровень на весь таймслот (не менее 60 мкс, не более 120 мкс)

		onewire_setBusHigh();
		_delay_us(5); // Время восстановления высокого уровеня на шине + 1 мс (минимум)
	}
}


// отправляет dataSize байт на шину из буффера data
void onewire_send(const void * data, uint8_t dataSize) {

	for (uint8_t i = 0; i < dataSize; i++)
	{
		uint8_t b = ((uint8_t*)data)[i];

		for (uint8_t p = 8; p > 0; p--) {
			onewire_send_bit(b & 0x01);
			b >>= 1;
		}
	}
}


// читает значение бита, передаваемое уйстройством.
// Возвращает 0 - если передан 0, отличное от нуля значение - если передана единица
bool onewire_readBit() {
	onewire_setBusLow();
	_delay_us(2); // Длительность низкого уровня, минимум 1 мкс

	onewire_setBusHigh();
	_delay_us(8); // Пауза до момента сэмплирования, всего не более 15 мкс

	bool retval = onewire_readBus();
	_delay_us(80); // Ожидание до следующего тайм-слота, минимум 60 мкс с начала низкого уровня
	return retval;
}


// Читает dataSize байт, переданный устройством (младший бит вперёд) в буффер data
void onewire_read(void * data, uint8_t dataSize) {
	for (uint8_t i = 0; i < dataSize; i++) {

		uint8_t r = 0;
		for (uint8_t p = 8; p > 0; p--) {
			r >>= 1;

			if (onewire_readBit())
				r |= 0x80;

			((uint8_t*)data)[i] = r;
		}
	}
}



void initStdios();


int main(void)
{
	// Инициализация printf
	initStdios();

	// Инициализация шины oneWire
	ONEWIRE_PORT &= (1 << ONEWIRE_PIN_NUM);
	onewire_setBusHigh();


	for ( ; ; _delay_ms(1000))
	{
		if (!onewire_reset()) {
			printf("No one at the bus\r\n");
			continue; // если никого на линии не оказалось - пропускаем эту итерацию
		}

		uint8_t command = 0x33; // команда на чтение ROM устроства
		onewire_send(&command, 1);

		// буфер для хранения ROM устройства
		uint8_t slaveDeviceData[8] = {0x00};
		onewire_read(slaveDeviceData, sizeof(slaveDeviceData));

		printf("deviceData: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
				slaveDeviceData[0], slaveDeviceData[1], slaveDeviceData[2], slaveDeviceData[3],
				slaveDeviceData[4], slaveDeviceData[5], slaveDeviceData[6], slaveDeviceData[7]
		);
	}
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

