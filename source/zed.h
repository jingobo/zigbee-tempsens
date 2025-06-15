#ifndef __ZED_H
#define __ZED_H

// ZED - Zigbee End Device

#include "typedefs.h"

// Номер конечной точки
#define ZED_ENDPOINT        8

// Состояние активности интикатора
extern bool_t zed_light_state;
// Время до завершения идентификации [сек]
extern uint16_t zed_identify_time;

// Инициалзиация модуля
void zed_init(void);
// Инициалзиация задачи
void zed_init_task(uint8_t task_id);
// Обпботчик очереди событий
uint16 zed_event_loop(uint8_t task_id, uint16_t events);

// Обработчик события изменения атрибута батареи
void zed_attr_bat_changed(void);
// Обработчик события изменения атрибута температуры
void zed_attr_temp_changed(void);

#endif // __ZED_H
