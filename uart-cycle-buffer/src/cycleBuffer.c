/*
 * Имлементация работы с цикличечким буффером
 */

#include "cycleBuffer.h"

bool cycleBuffer_pushBack(CycleBuffer * buffer, uint8_t value) {

	// Пробуем увеличить позицию головы
	size_t newHead = buffer->head + 1;

	// если буффер ушел за верхнюю границу линейного кадра - возвращаем его в 0
	if (newHead >= buffer->memorySize)
		newHead = 0;

	// Если голова догнала хвост - буффер полон.
	if (newHead == buffer->tail)
		return false;

	// записываем в текущее положение головы передаваемое нам значение
	buffer->memoryPtr[buffer->head] = value;
	// закреплеяем новое положение головы
	buffer->head = newHead;

	return true;
}


bool cycleBuffer_popFront(CycleBuffer * buffer, uint8_t * valuePtr) {

	 // если буфер пуст
	if (buffer->tail == buffer->head)
		return false;

	// пробуем увеличить позицию хвоста
	size_t newTail = buffer->tail + 1;

	// если буффер ушел за верхнюю границу линейного кадра - возвращаем его в 0
	if (newTail >= buffer->memorySize)
		newTail = 0;

	// возвращаем значение из буффера
	*valuePtr = buffer->memoryPtr[buffer->tail];
	// закрепляем новое значение хвоста
	buffer->tail = newTail;

	return true;
}
