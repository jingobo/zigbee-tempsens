/**************************************************************************************************
  Filename:       OnBoard.h
  Revised:        $Date: 2012-04-18 14:06:48 -0700 (Wed, 18 Apr 2012) $
  Revision:       $Revision: 30252 $

  Description:    Defines stuff for EVALuation boards
  Notes:          This file targets the Chipcon CC2530/31


  Copyright 2005-2010 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef __ONBOARD_H
#define __ONBOARD_H

#include "hal_mcu.h"
#include "hal_uart.h"
#include "hal_sleep.h"
#include "osal.h"
#include "mac_radio_defs.h"

// Timer clock and power-saving definitions
#define TIMER_DECR_TIME    1  // 1ms - has to be matched with TC_OCC

/* OSAL timer defines */
#define TICK_TIME   1000   // Timer per tick - in micro-sec
/*
  Timer4 interrupts @ 1.0 msecs using 1/128 pre-scaler
  TICK_COUNT = (CPUMHZ / 128) / 1000
*/
#define TICK_COUNT  1  // 32 Mhz Output Compare Count

/* CC2430 DEFINITIONS */

// MEMCTR bit definitions
#define ALWAYS1    0x01  // Reserved: always=1
#define CACHDIS    0x02  // Flash cache: disable=1
#define FMAP0      0x10  // Flash bank map, bit0
#define FMAP1      0x20  // Flash bank map, bit1
#define FMAP       0x30  // Flash bank map, mask
#define MUNIF      0x40  // Memory mapping: unified=1

// PCON bit definitions
#define PMODESET   0x01  // Power mode control: 1=set PMx

// Reset bit definitions
#define LRESET     0x18  // Last reset bit mask
#define RESETPO    0x00  // Power-On reset
#define RESETEX    0x08  // External reset
#define RESETWD    0x10  // WatchDog reset

/* GPIO PORT DEFINITIONS */
// GPIO bit definitions
#define GPIO_0     0x01  // Px_0: GPIO=0, PIO=1
#define GPIO_1     0x02  // Px_1: GPIO=0, PIO=1
#define GPIO_2     0x04  // Px_2: GPIO=0, PIO=1
#define GPIO_3     0x08  // Px_3: GPIO=0, PIO=1
#define GPIO_4     0x10  // Px_4: GPIO=0, PIO=1
#define GPIO_5     0x20  // Px_5: GPIO=0, PIO=1
#define GPIO_6     0x40  // Px_6: GPIO=0, PIO=1
#define GPIO_7     0x80  // Px_7: GPIO=0, PIO=1

/* WATCHDOG TIMER DEFINITIONS */
// WDCTL bit definitions

#define WDINT0     0x01  // Interval, bit0
#define WDINT1     0x02  // Interval, bit1
#define WDINT      0x03  // Interval, mask
#define WDMODE     0x04  // Mode: watchdog=0, timer=1
#define WDEN       0x08  // Timer: disabled=0, enabled=1
#define WDCLR0     0x10  // Clear timer, bit0
#define WDCLR1     0x20  // Clear timer, bit1
#define WDCLR2     0x40  // Clear timer, bit2
#define WDCLR3     0x80  // Clear timer, bit3
#define WDCLR      0xF0  // Clear timer, mask

// WD timer intervals
#define WDTISH     0x03  // Short: clk * 64
#define WDTIMD     0x02  // Medium: clk * 512
#define WDTILG     0x01  // Long: clk * 8192
#define WDTIMX     0x00  // Maximum: clk * 32768

// WD clear timer patterns
#define WDCLP1     0xA0  // Clear pattern 1
#define WDCLP2     0x50  // Clear pattern 2

// These Key definitions are unique to this development system.
// They are used to bypass functions when starting up the device.
#define SW_BYPASS_NV    HAL_KEY_SW_5  // Bypass Network layer NV restore
#define SW_BYPASS_START HAL_KEY_SW_1  // Bypass Network initialization

// Restart system from absolute beginning
// Disables interrupts, forces WatchDog reset
#define SystemReset()       \
{                           \
  HAL_DISABLE_INTERRUPTS(); \
  HAL_SYSTEM_RESET();       \
}

#define SystemResetSoft()  Onboard_soft_reset()

#define OSAL_SET_CPU_INTO_SLEEP(timeout) halSleep(timeout); /* Called from OSAL_PwrMgr */

/* The following Heap sizes are setup for typical TI sample applications,
 * and should be adjusted to your systems requirements.
 */
#if !defined INT_HEAP_LEN
#if defined RTR_NWK
  #define INT_HEAP_LEN  3072
#else
  #define INT_HEAP_LEN  2048
#endif
#endif
#define MAXMEMHEAP INT_HEAP_LEN

/* Measure voltage and report */
static bool OnBoard_CheckVoltage(void)
{
    return TRUE;
}

/*
* Board specific random number generator
*/
static uint16 Onboard_rand(void)
{
    return MAC_RADIO_RANDOM_WORD();
}

/*
* Board specific soft reset.
*/
static __near_func void Onboard_soft_reset(void)
{
    HAL_DISABLE_INTERRUPTS();
    DMAARM = 0x80 | 0x1F;
    asm("LJMP 0x0");
}

#endif // __ONBOARD_H
