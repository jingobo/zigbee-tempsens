#include "zed.h"
#include "sens.h"
#include "host.h"

// Значение масштаба для градусов
#define SENS_TEMP_SCALE     100

// Диапазон измерений температуры [C]
const int16_t SENS_TEMP_RANGE_MIN = -40 * SENS_TEMP_SCALE,
              SENS_TEMP_RANGE_MAX = +80 * SENS_TEMP_SCALE;

// Точность измерения температуры [C] (0.01)
const uint16_t SENS_TEMP_TOLERANCE = 1;

// Текущее значение температуры
int16_t sens_temp_current = 25 * SENS_TEMP_SCALE;

// Обработчик готовности температуры
static void sens_temp_ready(const uint8_t *data)
{
    // Перенос
    memcpy(&sens_temp_current, data, sizeof(sens_temp_current));
    
    // Оповещение
    zed_attr_temp_changed();
}

// Текщее значение напряжения (только чтение)
uint8_t sens_bat_volatage = 30;
// Текщее значение процента использования (только чтение)
uint8_t sens_bat_precentage = 160;

// Обработчик готовности напряжения батареи
static void sens_bat_ready(const uint8_t *data)
{
    // Сырое значение [мВ]
    uint16_t mv;
    memcpy(&mv, data, sizeof(mv));
    
    // Пороги напряжения [мВ]
    const uint16_t TH_LOW = 2800;
    const uint16_t TH_HIGH = 3600;
    const uint16_t TH_DELTA = TH_HIGH - TH_LOW;
    
    // Конечное напряжение
    sens_bat_volatage = (uint8_t)(mv / 100);
    
    // Нормализация для процента
    if (mv < TH_LOW)
        mv = TH_LOW;
    if (mv > TH_HIGH)
        mv = TH_HIGH;
    mv -= TH_LOW;
    
    // Расчет процента
    const uint32_t MAX_PRECENT = 200;
    sens_bat_precentage = (MAX_PRECENT * mv / TH_DELTA);
    
    // Прескалер передач
    static uint8_t prescaler = 0;
    if (prescaler > 0)
    {
        prescaler--;
        return;
    }
    prescaler = 10;
    
    // Оповещение
    zed_attr_bat_changed();
}

void sens_init(void)
{
    static const host_req_t REQ_LIST[] =
    {
        { 0x0008, sens_temp_ready },
        { 0x000A, sens_bat_ready },
    };
    
    host_req_list_set(REQ_LIST, ARRAY_SIZE(REQ_LIST));
}