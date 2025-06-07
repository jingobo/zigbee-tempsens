#include <APS.h>
#include <ZDApp.h>
#include <aps_frag.h>
#include <hal_drivers.h>
#include <bdb_interface.h>

#include "zed.h"
#include "temp.h"

// Глобальный список обработчиков событий
const pTaskEventHandlerFn tasksArr[] = 
{
    macEventLoop,
    nwk_event_loop,
    Hal_ProcessEvent,
    APS_event_loop,
    APSF_ProcessEvent,
    ZDApp_event_loop,
    zcl_event_loop,
    bdb_event_loop,
    
    zed_event_loop,
    temp_event_loop,
};

// Маска событий
uint16_t *tasksEvents;
// Количество задач
const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );

void osalInitTasks(void)
{
    tasksEvents = osal_mem_alloc(sizeof(uint16_t) * tasksCnt);
    osal_memset(tasksEvents, 0, (sizeof(uint16_t) * tasksCnt));

    uint8_t task = 0;
    
    macTaskInit(task++);
    nwk_init(task++);
    Hal_Init(task++);
    APS_Init(task++);
    APSF_Init(task++);
    ZDApp_Init(task++);
    zcl_Init(task++);
    bdb_Init(task++);
    
    zed_init_task(task++);
    temp_init_task(task++);
}
