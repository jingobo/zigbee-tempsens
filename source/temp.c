#include "zed.h"
#include "temp.h"
#include "uart.h"

// Значение масштаба для градусов
#define TEMP_SCALE      100

// Диапазон измерений температуры [C]
const int16_t TEMP_RANGE_MIN = -40 * TEMP_SCALE,
              TEMP_RANGE_MAX = +80 * TEMP_SCALE;

// Точность измерения температуры [C] (0.01)
const uint16_t TEMP_TOLERANCE = 1;

// Текущее значение температуры
int16_t temp_current = 25 * TEMP_SCALE;

// Маска таймера запроса температуры
static const uint16_t TEMP_QUERY_REQUEST = 0x0001;
// Маска таймера таймаута запроса
static const uint16_t TEMP_QUERY_TIMEOUT = 0x0002;

// Идентификатор задачи модуля
static __no_init uint8_t temp_task_id;

// Обработчик приёма ответа от измерителя
static void temp_response_cb(uint8_t *data, uint8_t size)
{
    // Сброс таймаута
    osal_stop_timerEx(temp_task_id, TEMP_QUERY_TIMEOUT);
    osal_pwrmgr_task_state(temp_task_id, PWRMGR_CONSERVE);

    // Проверка размера пакета
    if (size != 18)
        return;
    
    // Проверка заголовока пакета
    static const uint8_t HEADER[] = { 0x00, 0x00, 0x00, 0x01, 0x0A, 0x12 };
    if (memcmp(HEADER, data, sizeof(HEADER)) != 0)
        return;
    
    // Извлечение значения
    memcpy(&temp_current, data + 6, sizeof(temp_current));
    
    // Оповещение о изменении
    zed_attr_temp_changed();
}

void temp_init_task(uint8_t task)
{
    temp_task_id = task;
    osal_start_reload_timer(temp_task_id, TEMP_QUERY_REQUEST, 60000);
}

uint16_t temp_event_loop(uint8_t task_id, uint16_t events)
{
    // Событие запроса температуры
    if ((events & TEMP_QUERY_REQUEST) != 0)
    {
        // Передача запроса к измерителю
        static const uint8_t REQUEST[] = 
        { 
            0x00, 0x00, 0x00, 0x01, 0x0A, 0x0C, 
            0x08, 0x00, 0x16, 0x61, 0xBD, 0x84
        };
        
        uart_request(REQUEST, sizeof(REQUEST), temp_response_cb);
        
        // Запуск таймаута ответа
        osal_pwrmgr_task_state(temp_task_id, PWRMGR_HOLD);
        osal_start_timerEx(temp_task_id, TEMP_QUERY_TIMEOUT, 100);
        
        // Обработано
        return events ^ TEMP_QUERY_REQUEST;
    }

    // Событие таймаута запроса
    if ((events & TEMP_QUERY_TIMEOUT) != 0)
    {
        // Уход в сон
        osal_pwrmgr_task_state(temp_task_id, PWRMGR_CONSERVE);
        
        // Обработано
        return events ^ TEMP_QUERY_TIMEOUT;
    }
    
    // Не известные события
    return 0;
}
