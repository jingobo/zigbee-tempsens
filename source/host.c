#include "host.h"
#include "uart.h"

// Количество запросов
static size_t host_req_count = 0;
// Индекс текущего запроса
static size_t host_req_index = 0;
// Общий список запросов
static const host_req_t *host_req_list = NULL;

// Маска таймера опроса
static const uint16_t HOST_QUERY_POOL = 0x0001;
// Маска таймера запроса 
static const uint16_t HOST_QUERY_REQUEST = 0x0002;
// Маска таймера таймаута запроса
static const uint16_t HOST_QUERY_TIMEOUT = 0x0004;

// Идентификатор задачи модуля
static __no_init uint8_t host_task_id;

// Производит расчет контрольной суммы от хоста
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

// Производит запуск запроса через таймаут
static void host_req_delay(void)
{
    osal_start_timerEx(host_task_id, HOST_QUERY_REQUEST, 100);
}

// Обработчик приёма ответа от хоста
static void host_response_cb(uint8_t *data, uint8_t size)
{
    // Сброс таймаута
    osal_stop_timerEx(host_task_id, HOST_QUERY_TIMEOUT);
    osal_pwrmgr_task_state(host_task_id, PWRMGR_CONSERVE);

    // Проверка размера пакета
    if (size != 18)
        return;
    
    // Проверка заголовока пакета
    static const uint8_t HEADER[] = { 0x00, 0x00, 0x00, 0x01, 0x0A, 0x12 };
    if (memcmp(HEADER, data, sizeof(HEADER)) != 0)
        return;
    
    // Проверка контрольной суммы
    uint16_t crc;
    memcpy(&crc, data + 16, 2);
    if (crc != host_crc16(data, 16))
        return;
    
    // Извлечение значения
    host_req_list[host_req_index++].done(data + 6);
    
    // Переход к следующему запросу
    host_req_delay();
}

// Производит передачу текущего запроса
static void host_req_send(void)
{
    if (host_req_index >= host_req_count)
        return;
    
    // Подготовка запроса
    uint8_t request[12] = 
    {
        // Адрес
        0x00, 0x00, 0x00, 0x01,
        // Функция, длинна
        0x0A, 0x0C,
        0x00, 0x00,
        0x16, 0x61,
    };
    
    // Параметр
    memcpy(request + 6, &host_req_list[host_req_index].index, 2);
    
    // Контрольная сумма
    const uint16_t crc = host_crc16(request, 10);
    memcpy(request + 10, &crc, 2);

    // Передача запроса
    uart_request(request, sizeof(request), host_response_cb);
    
    // Запуск таймаута ответа
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
    
    // Событие опроса
    if ((events & HOST_QUERY_POOL) != 0)
    {
        // Запуск цепочки запросов
        host_req_index = 0;
        host_req_delay();
        
        // Обработано
        return events ^ HOST_QUERY_POOL;
    }

    // Событие запроса
    if ((events & HOST_QUERY_REQUEST) != 0)
    {
        host_req_send();
        
        // Обработано
        return events ^ HOST_QUERY_REQUEST;
    }
    
    // Событие таймаута запроса
    if ((events & HOST_QUERY_TIMEOUT) != 0)
    {
        // Уход в сон
        osal_pwrmgr_task_state(host_task_id, PWRMGR_CONSERVE);
        
        // Обработано
        return events ^ HOST_QUERY_TIMEOUT;
    }

    // Не известные события
    return 0;
}
