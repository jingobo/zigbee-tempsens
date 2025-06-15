#include "host.h"
#include "uart.h"

// ���������� ��������
static size_t host_req_count = 0;
// ������ �������� �������
static size_t host_req_index = 0;
// ����� ������ ��������
static const host_req_t *host_req_list = NULL;

// ����� ������� ������
static const uint16_t HOST_QUERY_POOL = 0x0001;
// ����� ������� ������� 
static const uint16_t HOST_QUERY_REQUEST = 0x0002;
// ����� ������� �������� �������
static const uint16_t HOST_QUERY_TIMEOUT = 0x0004;

// ������������� ������ ������
static __no_init uint8_t host_task_id;

// ���������� ������ ����������� ����� �� �����
static uint16_t host_crc16(const void *buffer, size_t size)
{
    assert(buffer != NULL);
    
    uint16_t value = 0xFFFF;
    const uint16_t polynom = 0xA001;
    
    for (uint8_t *buf = (uint8_t *)buffer; size > 0; size--)
    {
        value ^= *buf++;
        for (uint8_t i = 0; i < 8; i++)
            value = (value & 1) ? (value >> 1) ^ polynom : value >> 1;
    }
    
    return value;
}

// ���������� ������ ������� ����� �������
static void host_req_delay(void)
{
    osal_start_timerEx(host_task_id, HOST_QUERY_REQUEST, 100);
}

// ���������� ����� ������ �� �����
static void host_response_cb(uint8_t *data, uint8_t size)
{
    // ����� ��������
    osal_stop_timerEx(host_task_id, HOST_QUERY_TIMEOUT);
    osal_pwrmgr_task_state(host_task_id, PWRMGR_CONSERVE);

    // �������� ������� ������
    if (size != 18)
        return;
    
    // �������� ���������� ������
    static const uint8_t HEADER[] = { 0x00, 0x00, 0x00, 0x01, 0x0A, 0x12 };
    if (memcmp(HEADER, data, sizeof(HEADER)) != 0)
        return;
    
    // �������� ����������� �����
    uint16_t crc;
    memcpy(&crc, data + 16, 2);
    if (crc != host_crc16(data, 16))
        return;
    
    // ���������� ��������
    host_req_list[host_req_index++].done(data + 6);
    
    // ������� � ���������� �������
    host_req_delay();
}

// ���������� �������� �������� �������
static void host_req_send(void)
{
    if (host_req_index >= host_req_count)
        return;
    
    // ���������� �������
    uint8_t request[12] = 
    {
        // �����
        0x00, 0x00, 0x00, 0x01,
        // �������, ������
        0x0A, 0x0C,
        0x00, 0x00,
        0x16, 0x61,
    };
    
    // ��������
    memcpy(request + 6, &host_req_list[host_req_index].index, 2);
    
    // ����������� �����
    const uint16_t crc = host_crc16(request, 10);
    memcpy(request + 10, &crc, 2);

    // �������� �������
    uart_request(request, sizeof(request), host_response_cb);
    
    // ������ �������� ������
    osal_pwrmgr_task_state(host_task_id, PWRMGR_HOLD);
    osal_start_timerEx(host_task_id, HOST_QUERY_TIMEOUT, 100);
}

void host_init_task(uint8_t task)
{
    host_task_id = task;
    osal_start_reload_timer(host_task_id, HOST_QUERY_POOL, 60000);    
}

void host_req_list_set(const host_req_t *list, size_t count)
{
    host_req_list = list;
    host_req_count = count;
}

uint16_t host_event_loop(uint8_t task_id, uint16_t events)
{
    assert(host_task_id == task_id);
    
    // ������� ������
    if ((events & HOST_QUERY_POOL) != 0)
    {
        // ������ ������� ��������
        host_req_index = 0;
        host_req_delay();
        
        // ����������
        return events ^ HOST_QUERY_POOL;
    }

    // ������� �������
    if ((events & HOST_QUERY_REQUEST) != 0)
    {
        host_req_send();
        
        // ����������
        return events ^ HOST_QUERY_REQUEST;
    }
    
    // ������� �������� �������
    if ((events & HOST_QUERY_TIMEOUT) != 0)
    {
        // ���� � ���
        osal_pwrmgr_task_state(host_task_id, PWRMGR_CONSERVE);
        
        // ����������
        return events ^ HOST_QUERY_TIMEOUT;
    }

    // �� ��������� �������
    return 0;
}
