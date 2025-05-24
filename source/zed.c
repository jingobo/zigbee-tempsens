#include "zed.h"
#include "zed_info.h"

#include <hal_led.h>

#include <ZDApp.h>
#include <ZDObject.h>

#include <zcl_ms.h>
#include <zcl_general.h>
#include <bdb_interface.h>

#define ZED_LED_IDENTIFY    HAL_LED_1
#define ZED_LED_PRIMARY     HAL_LED_2
#define ZED_LED_CONNECT     HAL_LED_3

// --- Работа с адрексами --- //

// Структура короткого адреса
typedef uint16_t zed_short_addr_t;
// Структура длинного адреса
typedef uint8_t zed_long_addr_t[SADDR_EXT_LEN];

// Длинный адрес устройства (имя используется стэком)
zed_long_addr_t aExtendedAddress;

// Производит вывод короткого адреса
static void zed_addr_print_short(const char *preffix, zed_short_addr_t addr)
{
    printf("%s Nwk: 0x%04x\r\n", preffix, addr);
}

// Производит вывод длинного адреса
static void zed_addr_print_long(const char *preffix, zed_long_addr_t addr)
{
    printf("%s IEEE: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
        preffix,
        addr[7], addr[6], addr[5], addr[4], 
        addr[3], addr[2], addr[1], addr[0]);
}

// Устанавливает фиксированный длинный адрес
#define ZED_FIXED_ADDRESS

// Восстанавилвает длинный адрес из NV
static void zed_addr_long_load(void)
{
#ifdef ZED_FIXED_ADDRESS
    // Статичный адрес
    static const uint8_t ADDRESS[] = { 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0xFA, 0x30, 0xF8 };
    memcpy(aExtendedAddress, ADDRESS, Z_EXTADDR_LEN);
#else
    // Пустое значение адреса
    static const uint8_t NULL_ADDRESS[Z_EXTADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Попытка восстановить из хранилища
    if ((osal_nv_item_init(ZCD_NV_EXTADDR, Z_EXTADDR_LEN, NULL) == SUCCESS) &&
        (osal_nv_read(ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, aExtendedAddress) == SUCCESS) &&
        (osal_memcmp(aExtendedAddress, NULL_ADDRESS, Z_EXTADDR_LEN) == 0))
        return;

    // Формирование нового адреса
    const uint8_t ADDRESS_TAIL = Z_EXTADDR_LEN - 2;
    for (uint8_t i = 0; i < ADDRESS_TAIL; i += 2)
    {
        const uint16 randy = osal_rand();
        osal_memcpy(aExtendedAddress + i, &randy, 2);
    }

    // Тип ZigBee устройства
    aExtendedAddress[ADDRESS_TAIL + 0] = 0x30;
    // Фиксированное историческое значение
    aExtendedAddress[ADDRESS_TAIL + 1] = 0xF8;

    osal_nv_write(ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, aExtendedAddress);
#endif
}

// Применение расширенного адреса в стэк
static void zed_addr_long_apply(void)
{
    // Set the MAC PIB extended address according to results from above.
    ZMacSetReq(MAC_EXTENDED_ADDRESS, aExtendedAddress);
    zed_addr_print_long("End device", aExtendedAddress);
}

// --- Подключение к сети --- //

// Идентификатор основной задачи
static __no_init uint8_t zed_task_id;

// Идентификатор события переподключения
static const uint16_t ZED_REJOIN_EVENT = 0x0004;

// Перечисление состояния подключения
typedef enum
{
    // Простой
    ZED_JOIN_STATE_IDLE,
    // Потеряно
    ZED_JOIN_STATE_LOST,
    // Подключение
    ZED_JOIN_STATE_PROCESS,
    // Успешно
    ZED_JOIN_STATE_SUCCESS,
} zed_join_state_t;

// Состояние подключения
static zed_join_state_t zed_join_state = ZED_JOIN_STATE_IDLE;   

// Производит перезапуск подключения
static void zed_rejoin(void)
{
    osal_start_timerEx(zed_task_id, ZED_REJOIN_EVENT, 5000);
    HalLedSet(ZED_LED_CONNECT, HAL_LED_MODE_OFF);
    HalLedSet(ZED_LED_PRIMARY, HAL_LED_MODE_OFF);
}

// Задает состояние подключения
static void zed_join_state_set(zed_join_state_t state)
{
    if (zed_join_state == state)
        return;
    zed_join_state = state;
    
    switch (state)
    {
        case ZED_JOIN_STATE_LOST:
            printf("Join lost!\r\n");
            zed_rejoin();
            break;
            
        case ZED_JOIN_STATE_IDLE:
            printf("Join failed!\r\n");
            zed_rejoin();
            break;
        
        case ZED_JOIN_STATE_PROCESS:
            printf("Joining...\r\n");
            HalLedBlink(ZED_LED_PRIMARY, 0, 50, 200);
            break;
            
        case ZED_JOIN_STATE_SUCCESS:
            printf("Joined!\r\n");
            HalLedSet(ZED_LED_CONNECT, HAL_LED_MODE_ON);
            HalLedSet(ZED_LED_PRIMARY, HAL_LED_MODE_OFF);

            // Вывод короткого адреса устройства
            zed_addr_print_short("End device", NLME_GetShortAddr());
            // Вывод длинного адреса координатора
            {
                zed_long_addr_t laddr;
                NLME_GetCoordExtAddr(laddr);
                zed_addr_print_long("Coordinator", laddr);
            }
            break;
            
        default:
            assert(FALSE);
            return;
    }
}

// Производит запуск подключения
static void zed_join(void)
{
    switch (zed_join_state)
    {
        case ZED_JOIN_STATE_IDLE:
            bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING);
            break;
          
        case ZED_JOIN_STATE_LOST:
            bdb_ZedAttemptRecoverNwk();
            break;
            
        default:
            return;
    }
    
    zed_join_state_set(ZED_JOIN_STATE_PROCESS);
}

// Обработчик изменения статуса входа в сеть
static void zed_process_commissioning(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
    switch (bdbCommissioningModeMsg->bdbCommissioningMode)
    {
        case BDB_COMMISSIONING_INITIALIZATION:
            zed_join_state_set(ZED_JOIN_STATE_SUCCESS);
            break;
            
        case BDB_COMMISSIONING_NWK_STEERING:
            switch (bdbCommissioningModeMsg->bdbCommissioningStatus)
            {
                case BDB_COMMISSIONING_IN_PROGRESS:
                    zed_join_state_set(ZED_JOIN_STATE_PROCESS);
                    break;
                
                case BDB_COMMISSIONING_SUCCESS:
                    zed_join_state_set(ZED_JOIN_STATE_SUCCESS);
                    break;
                    
                default:
                    zed_join_state_set(ZED_JOIN_STATE_IDLE);
                    break;
            }
            break;
            
        case BDB_COMMISSIONING_PARENT_LOST:
            switch (bdbCommissioningModeMsg->bdbCommissioningStatus)
            {
                case BDB_COMMISSIONING_NETWORK_RESTORED:
                    zed_join_state_set(ZED_JOIN_STATE_SUCCESS);
                    break;
                    
                default:
                    zed_join_state_set(ZED_JOIN_STATE_LOST);
                    break;
            }
            break;
    }
}

// --- Кластер индикации --- //

// Время до завершения идентификации [сек]
__no_init uint16_t zed_identify_time;

static void zed_identify_time_changed(uint8 endpoint)
{
    if (zed_identify_time > 0)
        HalLedBlink(ZED_LED_IDENTIFY, 0xFF, 50, HAL_LED_DEFAULT_FLASH_TIME);
    else
        HalLedSet(ZED_LED_IDENTIFY, HAL_LED_MODE_OFF);
}

// --- Базовый кластер --- //

// Обработчик команды сброса устройства
static void zed_reset_device_cb(void)
{
    zed_identify_time = 0;
}

// --- Внешние функции --- //

void zed_init(void)
{
    // MAC
    ZMacInit();
    
    // Длинный адрес
    zed_addr_long_load();
    zed_addr_long_apply();
    
    // Базовая инициализация стэка
    zgInit();
    // Базовая инициализация App Framework
    afInit();
}

void zed_init_task(uint8_t task_id)
{
    zed_task_id = task_id;

    // Регистрация дескриптора приложения
    bdb_RegisterSimpleDescriptor(&ZED_INFO_APP_DESC);

    // Регистрация колбеков ZCL General Cluster Library
    {
        static zclGeneral_AppCallbacks_t callbacks =
        {
            .pfnBasicReset = zed_reset_device_cb,
        };

        zclGeneral_RegisterCmdCallbacks(ZED_ENDPOINT, &callbacks);
    }

    // Регистрация основной задачи в стеке
    zdpExternalStateTaskID = task_id;
    zcl_registerForMsg(task_id);

    // Регистрация атрибутов
    zed_reset_device_cb();
    zcl_registerAttrList(ZED_ENDPOINT, ZED_INFO_ATTRIBUTES_COUNT, ZED_INFO_ATTRIBUTES);
        
    // Регистрация команд
    zcl_registerCmdList(ZED_ENDPOINT, ZED_INFO_COMMANDS_COUNT, ZED_INFO_COMMANDS);

    // Регистрация колбеков BDB
    bdb_RegisterCommissioningStatusCB(zed_process_commissioning);
    bdb_RegisterIdentifyTimeChangeCB(zed_identify_time_changed);
    
    // Запуск подключения
    zed_join();
}

uint16_t zed_event_loop(uint8_t task_id, uint16_t events)
{
    // Системные события
    if ((events & SYS_EVENT_MSG) != 0)
    {
        for (;;)
        {
            osal_event_hdr_t * const msg = (osal_event_hdr_t *)osal_msg_receive(zed_task_id);
            if (msg == NULL)
                break;
            
            switch (msg->event)
            { }
            
            osal_msg_deallocate((uint8_t *)msg);
        }

        return (events ^ SYS_EVENT_MSG);
    }

    // Таймер переподключения
    if ((events & ZED_REJOIN_EVENT) != 0)
    {
        zed_join();
        return events ^ ZED_REJOIN_EVENT;
    }
    
    // Отброс не известных событий
    return 0;
}

void zed_attr_temp_changed(void)
{
    if (zed_join_state != ZED_JOIN_STATE_SUCCESS)
        return;
    
    HalLedSet(ZED_LED_PRIMARY, HAL_LED_MODE_BLINK);
    bdb_RepChangedAttrValueInstant(ZED_ENDPOINT, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, ATTRID_MS_TEMPERATURE_MEASURED_VALUE);
}
