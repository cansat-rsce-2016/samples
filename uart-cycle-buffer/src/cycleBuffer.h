/*
 * cycleBuffer.h
 *
 *  Created on: 26 дек. 2015 г.
 *      Author: snork
 */

#ifndef CYCLEBUFFER_H_
#define CYCLEBUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Структура циклического буффера
typedef struct {
	uint8_t * memoryPtr;	//!< Указатель на используемый линейный буффер
	size_t memorySize;		//!< Размер линейного буффера
	volatile size_t tail;	//!< Положение хвоста буффера
	volatile size_t head;	//!< Положение головы буффера
} CycleBuffer;


// Добавление байта к голове буффера
/* Если добавление не удается (буффер полон) возвращает false */
bool cycleBuffer_pushBack(CycleBuffer * buffer, uint8_t value);

// Получение байта с хвоста буффера
/* Если получение байта не удается (буффер пуст) возвращает false */
bool cycleBuffer_popFront(CycleBuffer * buffer, uint8_t * valuePtr);


#endif /* CYCLEBUFFER_H_ */

