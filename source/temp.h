#ifndef __TEMP_H
#define __TEMP_H

#include "typedefs.h"

// Текущее значение температуры (только чтение)
extern int16_t temp_current;

// Диапазон измерений температуры [C]
extern const int16_t TEMP_RANGE_MIN,
                     TEMP_RANGE_MAX;

// Точность измерения температуры [C]
extern const uint16_t TEMP_TOLERANCE;

// Инициалзиация модулдя
void temp_init_task(uint8_t task);
// Обработчик главного цикла
uint16_t temp_event_loop(uint8_t task_id, uint16_t events);

#endif // __TEMP_H
