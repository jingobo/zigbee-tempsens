#ifndef __SENS_H
#define __SENS_H

#include "typedefs.h"

// Текущее значение температуры (только чтение)
extern int16_t sens_temp_current;

// Диапазон измерений температуры [C]
extern const int16_t SENS_TEMP_RANGE_MIN,
                     SENS_TEMP_RANGE_MAX;

// Точность измерения температуры [C]
extern const uint16_t SENS_TEMP_TOLERANCE;

// Текщее значение напряжения (только чтение)
extern uint8_t sens_bat_volatage;
// Текщее значение процента использования (только чтение)
extern uint8_t sens_bat_precentage;

// Инициалзиация модулдя
void sens_init(void);

#endif // __SENS_H
