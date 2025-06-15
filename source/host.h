#ifndef __HOST_H
#define __HOST_H

#include "typedefs.h"

// ������������� ������
void host_init_task(uint8_t task);
// ���������� �������� �����
uint16_t host_event_loop(uint8_t task_id, uint16_t events);

// ��������� ���������� � �������
typedef struct
{
    uint16_t index;
    void (* done)(const uint8_t *payload);
} host_req_t;

// ��������� ���������� � ��������
void host_req_list_set(const host_req_t *list, size_t count);

#endif // __HOST_H
