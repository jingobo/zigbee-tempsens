#ifndef __UART_H
#define __UART_H

#include "typedefs.h"

// Размер буферов
#define UART_BUFFER_SIZE    48

// Прототип обработчика приёма данных
typedef void (* uart_recv_cb_t)(uint8_t *data, uint8_t size);

// Инициалзиация модуля
void uart_init(void);
// Производит передачу и ожидание приёма
void uart_request(const void *source, uint8_t size, uart_recv_cb_t cb);

#endif // __UART_H
