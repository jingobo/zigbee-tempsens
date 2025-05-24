#ifndef __UART_H
#define __UART_H

#include "typedefs.h"

// ������ �������
#define UART_BUFFER_SIZE    48

// �������� ����������� ����� ������
typedef void (* uart_recv_cb_t)(uint8_t *data, uint8_t size);

// ������������� ������
void uart_init(void);
// ���������� �������� � �������� �����
void uart_request(const void *source, uint8_t size, uart_recv_cb_t cb);

#endif // __UART_H
