#include "zed.h"
#include "sens.h"
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
static const uint16_t ZED_REJOIN_EVENT = 0x0001;
// Идентификатор события сброса быстрого опроса
static const uint16_t ZED_POOL_SLOW_EVENT = 0x0002;

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

// Период переподключения [сек]
static uint8_t zed_rejoin_period = 0;

// Производит перезапуск подключения
static void zed_rejoin(void)
{
    zed_rejoin_period += 10;
    if (zed_rejoin_period > 240)
        zed_rejoin_period = 240;
    
    osal_start_timerEx(zed_task_id, ZED_REJOIN_EVENT, zed_rejoin_period * 5000ul);
    HalLedSet(ZED_LED_CONNECT, HAL_LED_MODE_OFF);
    HalLedSet(ZED_LED_PRIMARY, HAL_LED_MODE_OFF);
}

// Обработчик установки медленного опроса координатора
static void zed_pool_rate_slow(void)
{
    NLME_SetPollRate(POLL_RATE);
}

// Задает состояние подключения
static void zed_join_state_set(zed_join_state_t state)
{
    if (zed_join_state == state)
        return;
    zed_join_state = state;

    // В любом случае сброс таймера замедления 
    zed_pool_rate_slow();
    osal_stop_timerEx(zed_task_id, ZED_POOL_SLOW_EVENT);
    
    // Обработка нового состояния
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
            zed_rejoin_period = 0;
            HalLedSet(ZED_LED_CONNECT, HAL_LED_MODE_ON);
            HalLedSet(ZED_LED_PRIMARY, HAL_LED_MODE_OFF);

            // Первые 10 секунд быстрый пул координатора для форсирования интервью
            NLME_SetPollRate(250);
            osal_start_timerEx(zed_task_id, ZED_POOL_SLOW_EVENT, 10000);
            
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
            bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING);
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
            
        case BDB_COMMISSIONING_FINDING_BINDING:
            switch (bdbCommissioningModeMsg->bdbCommissioningStatus)
            {
                case BDB_COMMISSIONING_FB_TARGET_IN_PROGRESS:
                    printf("FB find initiator in process...\r\n");
                    break;

                case BDB_COMMISSIONING_FB_INITITATOR_IN_PROGRESS:
                    printf("FB find target in process...\r\n");
                    break;
                    
                case BDB_COMMISSIONING_FB_NO_IDENTIFY_QUERY_RESPONSE:
                    printf("FB find target finish.\r\n");
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
    
    // Конечная точка для FB идентификации
    bdb_SetIdentifyActiveEndpoint(ZED_ENDPOINT);
    
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
            { 
                case ZCL_INCOMING_MSG:
                    // Incoming ZCL Foundation command/response messages
                    {
                        zclIncomingMsg_t * const zcl_msg = (zclIncomingMsg_t *)msg;
                        if (zcl_msg->attrCmd)
                            osal_mem_free(zcl_msg->attrCmd);
                    }
                  break;
            }
            
            osal_msg_deallocate((uint8_t *)msg);
        }

        return events ^ SYS_EVENT_MSG;
    }

    // Таймер переподключения
    if ((events & ZED_REJOIN_EVENT) != 0)
    {
        zed_join();
        return events ^ ZED_REJOIN_EVENT;
    }
    
    // Таймер переподключения
    if ((events & ZED_POOL_SLOW_EVENT) != 0)
    {
        zed_pool_rate_slow();
        return events ^ ZED_POOL_SLOW_EVENT;
    }
    
    // Отброс не известных событий
    return 0;
}

// Параметры адреса назначения
static afAddrType_t zed_dest_address =
{
    .addr =
    {
        .shortAddr = 0,
    },
    .addrMode = afAddrNotPresent,
    .endPoint = ZED_ENDPOINT,
};

void zed_attr_temp_changed(void)
{
    if (zed_join_state != ZED_JOIN_STATE_SUCCESS)
        return;
    
    // Параметры отчета
    static struct
    {
        zclReportCmd_t cmd;
        zclReport_t attr;
    } report =
    {
        .cmd = 
        {
            .numAttr = 1,
        },
        .attr = 
        {
            .attrID = ATTRID_MS_TEMPERATURE_MEASURED_VALUE,
            .dataType = ZCL_DATATYPE_INT16,
            .attrData = (uint8_t *)&sens_temp_current,
        },
    };
        
    // Передача запроса
    static uint8_t seq = 0;
    if (zcl_SendReportCmd(ZED_ENDPOINT, &zed_dest_address, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, &report.cmd, ZCL_FRAME_SERVER_CLIENT_DIR, BDB_REPORTING_DISABLE_DEFAULT_RSP, seq++) == ZSuccess)
        HalLedSet(ZED_LED_PRIMARY, HAL_LED_MODE_BLINK);
}

void zed_attr_bat_changed(void)
{
    if (zed_join_state != ZED_JOIN_STATE_SUCCESS)
        return;
    
    // Параметры отчета
    static struct
    {
        zclReportCmd_t cmd;
        zclReport_t attr;
    } report =
    {
        .cmd = 
        {
            .numAttr = 1,
        },
        .attr = 
        {
            .attrID = ATTRID_POWER_CFG_BATTERY_PERCENTAGE_REMAINING,
            .dataType = ZCL_DATATYPE_UINT8,
            .attrData = &sens_bat_precentage,
        },
    };

    // Передача запроса
    static uint8_t seq = 0;
    zcl_SendReportCmd(ZED_ENDPOINT, &zed_dest_address, ZCL_CLUSTER_ID_GEN_POWER_CFG, &report.cmd, ZCL_FRAME_SERVER_CLIENT_DIR, BDB_REPORTING_DISABLE_DEFAULT_RSP, seq++);
}
