#ifndef __HOST_H
#define __HOST_H

#include "typedefs.h"

// Инициалзиация модуля
void host_init_task(uint8_t task);
// Обработчик главного цикла
uint16_t host_event_loop(uint8_t task_id, uint16_t events);

// Структура информации о запросе
typedef struct
{
    uint16_t index;
    void (* done)(const uint8_t *payload);
} host_req_t;

// Установка информации о запросах
void host_req_list_set(const host_req_t *list, size_t count);

#endif // __HOST_H
