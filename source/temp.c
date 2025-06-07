#include "zed.h"
#include "temp.h"
#include "uart.h"

// �������� �������� ��� ��������
#define TEMP_SCALE      100

// �������� ��������� ����������� [C]
const int16_t TEMP_RANGE_MIN = -40 * TEMP_SCALE,
              TEMP_RANGE_MAX = +80 * TEMP_SCALE;

// �������� ��������� ����������� [C] (0.01)
const uint16_t TEMP_TOLERANCE = 1;

// ������� �������� �����������
int16_t temp_current = 25 * TEMP_SCALE;

// ����� ������� ������� �����������
static const uint16_t TEMP_QUERY_REQUEST = 0x0001;
// ����� ������� �������� �������
static const uint16_t TEMP_QUERY_TIMEOUT = 0x0002;

// ������������� ������ ������
static __no_init uint8_t temp_task_id;

// ���������� ����� ������ �� ����������
static void temp_response_cb(uint8_t *data, uint8_t size)
{
    // ����� ��������
    osal_stop_timerEx(temp_task_id, TEMP_QUERY_TIMEOUT);
    osal_pwrmgr_task_state(temp_task_id, PWRMGR_CONSERVE);

    // �������� ������� ������
    if (size != 18)
        return;
    
    // �������� ���������� ������
    static const uint8_t HEADER[] = { 0x00, 0x00, 0x00, 0x01, 0x0A, 0x12 };
    if (memcmp(HEADER, data, sizeof(HEADER)) != 0)
        return;
    
    // ���������� ��������
    memcpy(&temp_current, data + 6, sizeof(temp_current));
    
    // ���������� � ���������
    zed_attr_temp_changed();
}

void temp_init_task(uint8_t task)
{
    temp_task_id = task;
    osal_start_reload_timer(temp_task_id, TEMP_QUERY_REQUEST, 60000);
}

uint16_t temp_event_loop(uint8_t task_id, uint16_t events)
{
    // ������� ������� �����������
    if ((events & TEMP_QUERY_REQUEST) != 0)
    {
        // �������� ������� � ����������
        static const uint8_t REQUEST[] = 
        { 
            0x00, 0x00, 0x00, 0x01, 0x0A, 0x0C, 
            0x08, 0x00, 0x16, 0x61, 0xBD, 0x84
        };
        
        uart_request(REQUEST, sizeof(REQUEST), temp_response_cb);
        
        // ������ �������� ������
        osal_pwrmgr_task_state(temp_task_id, PWRMGR_HOLD);
        osal_start_timerEx(temp_task_id, TEMP_QUERY_TIMEOUT, 100);
        
        // ����������
        return events ^ TEMP_QUERY_REQUEST;
    }

    // ������� �������� �������
    if ((events & TEMP_QUERY_TIMEOUT) != 0)
    {
        // ���� � ���
        osal_pwrmgr_task_state(temp_task_id, PWRMGR_CONSERVE);
        
        // ����������
        return events ^ TEMP_QUERY_TIMEOUT;
    }
    
    // �� ��������� �������
    return 0;
}
