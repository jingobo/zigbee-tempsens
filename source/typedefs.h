#ifndef __TYPEDEFS_H
#define __TYPEDEFS_H

#include <osal.h>
#include <osal_nv.h>
#include <osal_pwrmgr.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <intrinsics.h>

// --- Модуль stdint не досутпен --- //

typedef int8 int8_t;
typedef uint8 uint8_t;

typedef int16 int16_t;
typedef uint16 uint16_t;

typedef int32 int32_t;
typedef uint32 uint32_t;

typedef bool bool_t;

typedef float float32_t;

// --- Прерывания --- //

// Включение/Отключение прерываний
#define IRQ_ENABLE()        HAL_ENABLE_INTERRUPTS()
#define IRQ_DISABLE()       HAL_DISABLE_INTERRUPTS()

// Вход в блок с отключенными прерываниями
#define IRQ_SUPPRESS_ENTER()                \
    halIntState_t __irq_ctx;                \
    HAL_ENTER_CRITICAL_SECTION(__irq_ctx)

// Выход из блока с отключенными прерываниями
#define IRQ_SUPPRESS_LEAVE()                \
    HAL_EXIT_CRITICAL_SECTION(__irq_ctx)

// --- Разное --- //

// Расчет размера массива
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
        
#endif // __TYPEDEFS_H
