#include "zed.h"
#include "temp.h"
#include "zed_info.h"

#include <zcl_ha.h>
#include <zcl_ms.h>
#include <zcl_general.h>

// --- Коменды --- //

// Список поддерживаемых команд
CONST zclCommandRec_t ZED_INFO_COMMANDS[] =
{
    { ZCL_CLUSTER_ID_GEN_BASIC,     COMMAND_BASIC_RESET_FACT_DEFAULT,   CMD_DIR_SERVER_RECEIVED },
};

// --- Атрибуты --- //

// Значение аппаратной версии устройства
static const uint8_t ZED_INFO_HW_VERSION = 1;

// Имя производителя
static const uint8_t ZED_INFO_MANUFACTURER[] = 
{ 
    3, 
    'A', 'V', 'G'
};

// Имя модели
static const uint8_t ZED_INFO_MODEL_ID[] = 
{ 
    16,
    'O', 'u', 't', 'd', 'o', 'o', 'r', ' ',
    'T', 'e', 'm', 'p', 'S', 'e', 'n', 's',
};

// Дата производитсва
static const uint8_t ZED_INFO_RELEASE_DATE[] = 
{ 
    8,
    '2','0','2','5','0','5','2','2'
};

// Источник питания
static const uint8_t ZED_INFO_POWER_SOURCE = POWER_SOURCE_BATTERY;

// Значение поддержки имен групп (не менять)
static const bool_t ZED_INFO_GROUPS_NAME_SUPPORT = FALSE;

// Значение версии спецификации ZCL (не менять)
static const uint8_t ZED_INFO_ZCL_VERSION = 1;
// Значение ревизии кластера (не менять)
static const uint16_t ZED_INFO_CLUESTER_REV = 1;

// Статическая инициалзиация аттрибута
#define ZED_INFO_ATTR(cluster_id, attr_id, data_type, access, data_ptr)     \
    { (cluster_id), { (attr_id), (data_type), (access), (void *)(data_ptr) } }

// Список поддерживаемых аттрибутов
// Идентификаторы атрибутов в кластере должны быть в порядке возрастания
CONST zclAttrRec_t ZED_INFO_ATTRIBUTES[] =
{
    //            Cluster ID                                    Attribute ID                                Data Type               Access                                          Data
    
    // General Basic Cluster Attributes (7)
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_BASIC,                     ATTRID_BASIC_ZCL_VERSION,                   ZCL_DATATYPE_UINT8,     ACCESS_CONTROL_READ,                            &ZED_INFO_ZCL_VERSION),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_BASIC,                     ATTRID_BASIC_HW_VERSION,                    ZCL_DATATYPE_UINT8,     ACCESS_CONTROL_READ,                            &ZED_INFO_HW_VERSION),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_BASIC,                     ATTRID_BASIC_MANUFACTURER_NAME,             ZCL_DATATYPE_CHAR_STR,  ACCESS_CONTROL_READ,                            &ZED_INFO_MANUFACTURER),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_BASIC,                     ATTRID_BASIC_MODEL_ID,                      ZCL_DATATYPE_CHAR_STR,  ACCESS_CONTROL_READ,                            &ZED_INFO_MODEL_ID),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_BASIC,                     ATTRID_BASIC_DATE_CODE,                     ZCL_DATATYPE_CHAR_STR,  ACCESS_CONTROL_READ,                            &ZED_INFO_RELEASE_DATE),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_BASIC,                     ATTRID_BASIC_POWER_SOURCE,                  ZCL_DATATYPE_ENUM8,     ACCESS_CONTROL_READ,                            &ZED_INFO_POWER_SOURCE),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_BASIC,                     ATTRID_CLUSTER_REVISION,                    ZCL_DATATYPE_UINT16,    ACCESS_CONTROL_READ,                            &ZED_INFO_CLUESTER_REV),
    
    // Identify Cluster Attribute (2)
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_IDENTIFY,                  ATTRID_IDENTIFY_TIME,                       ZCL_DATATYPE_UINT16,    ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,     &zed_identify_time),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_IDENTIFY,                  ATTRID_CLUSTER_REVISION,                    ZCL_DATATYPE_UINT16,    ACCESS_CONTROL_READ,                            &ZED_INFO_CLUESTER_REV),
    
    // Temperature Measurement Cluster Attributes (5)
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,    ATTRID_MS_TEMPERATURE_MEASURED_VALUE,       ZCL_DATATYPE_INT16,     ACCESS_CONTROL_READ | ACCESS_REPORTABLE,        &temp_current),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,    ATTRID_MS_TEMPERATURE_MIN_MEASURED_VALUE,   ZCL_DATATYPE_INT16,     ACCESS_CONTROL_READ,                            &TEMP_RANGE_MIN),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,    ATTRID_MS_TEMPERATURE_MAX_MEASURED_VALUE,   ZCL_DATATYPE_INT16,     ACCESS_CONTROL_READ,                            &TEMP_RANGE_MAX),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,    ATTRID_MS_TEMPERATURE_TOLERANCE,            ZCL_DATATYPE_UINT16,    ACCESS_CONTROL_READ,                            &TEMP_TOLERANCE),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,    ATTRID_CLUSTER_REVISION,                    ZCL_DATATYPE_UINT16,    ACCESS_CONTROL_READ,                            &ZED_INFO_CLUESTER_REV),

    // Groups Cluster Attributes (2)
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_GROUPS,                    ATTRID_GROUP_NAME_SUPPORT,                  ZCL_DATATYPE_BITMAP8,   ACCESS_CONTROL_READ,                            &ZED_INFO_GROUPS_NAME_SUPPORT),
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_GROUPS,                    ATTRID_CLUSTER_REVISION,                    ZCL_DATATYPE_UINT16,    ACCESS_CONTROL_READ,                            &ZED_INFO_CLUESTER_REV),
  
    // Scenes Cluster Attributes (1)
    ZED_INFO_ATTR(ZCL_CLUSTER_ID_GEN_SCENES,                    ATTRID_CLUSTER_REVISION,                    ZCL_DATATYPE_UINT16,    ACCESS_CONTROL_READ,                            &ZED_INFO_CLUESTER_REV),
};

// --- Дескриптор --- //

// Список входящих кластеров
static cId_t ZED_INFO_IN_CLUESTERS[] =
{
    ZCL_CLUSTER_ID_GEN_BASIC,
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    ZCL_CLUSTER_ID_GEN_GROUPS,
    ZCL_CLUSTER_ID_GEN_SCENES,
    ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT    
};

// Список исходящих кластеров
static cId_t ZED_INFO_OUT_CLUESTERS[] =
{
    ZCL_CLUSTER_ID_GEN_BASIC,
};

// Дескриптор приложения
SimpleDescriptionFormat_t ZED_INFO_APP_DESC =
{
    // EP
    ZED_ENDPOINT,
    
    // App Profile/Device ID
    ZCL_HA_PROFILE_ID,
    ZCL_HA_DEVICEID_TEMPERATURE_SENSOR,
    
    // App Version/Flags
    1,
    0,
    
    // In Clusters
    5,
    ZED_INFO_IN_CLUESTERS,
    
    // Out Clusters
    1,
    ZED_INFO_OUT_CLUESTERS,
};
