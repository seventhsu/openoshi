#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

// configurations from ch32fun.h in here
#define CH32V003 1
//#define FUNCONF_ENABLE_HPE 0
//^ try this if interrupts are acting up, is recommended for '003 in ch32fun.h

/***********************************************************
 * Personal notes about the clock setup:
 * PLL & HSI are enabled in ch32fun.h's funconf default defines, making SYSCLK = 48MHz
 * This number in itself is default-defined in FUNCONF_SYSTEM_CORE_CLOCK.
 * The RM recommends adding a flash wait state in FLASH->ACTLR when SYSCLK >= 24MHz...
 * which ch32fun.c does automatically based on FUNCONF_SYSTEM_CORE_CLOCK's value.
 * By POR defaults, RCC_HPRE = b0010 and PLL off results in default HB periph clk of 8MHz,
 * but RCC_HPRE is overridden to DIV1 by SystemInit(), so periph clk = SYSCLK = 48MHz.
*/

#endif//_FUNCONFIG_H
