// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Seventhsu

#include "../ch32fun/ch32fun/ch32fun.h"
#include <stdio.h>

#define PERIPH_CLOCK FUNCONF_SYSTEM_CORE_CLOCK  // HPRE=DIV1 after SystemInit

#define LED_PWM_MAX_FREQ_HZ 1000 // Change me at your leisure!
#define COLOR_BIT_DEPTH 8

#define NUM_COLORS     (sizeof(colorTable) / sizeof(colorTable[0]))
#define DEBOUNCE_MS    30
#define LONG_PRESS_MS  800
#define POLL_MS        10

// colorTable[i][0:3] represent B, G, R, W channels of one color setting
uint8_t colorTable[][4] = {
	{0x0,	0x0,	0xff,	0x0},   // Color red
    {0x0,	0xff,	0xff,	0x0},   // Color yellow
	{0x0,	0xff,	0x0,	0x0},   // Color green
    {0xff,	0xff,	0x0,	0x0},   // Color cyan
	{0xff,	0x0,	0x0,	0x0},   // Color blue
    {0xff,	0x0,	0xff,	0x0},   // Color magenta
	{0x0,	0x0,	0x0,	0xff},  // Color white
};

// The color the penlight should display if it was on (index into colorTable)
static volatile int currColor;

// Bitmask set by ISR: bit 0 = SW1 (PD6), bit 1 = SW2 (PD5)
static volatile uint8_t btnEvent;

void EXTI7_0_IRQHandler(void) __attribute__((interrupt));
void EXTI7_0_IRQHandler(void) {
    if (EXTI->INTFR & EXTI_Line6)  btnEvent |= 0x01;  // SW1
    if (EXTI->INTFR & EXTI_Line5)  btnEvent |= 0x02;  // SW2
    EXTI->INTFR = EXTI_Line6 | EXTI_Line5;          // Acknowledge both
}

void tim1PwmInit(void) {
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;

    // Map and AF-configure pins to be used by TIM1
    // TIM1 map: CH1=PC4(blue), CH2=PC7(green), CH3=PC5(red), CH4=PD4(white)
    AFIO->PCFR1 |= AFIO_PCFR1_TIM1_REMAP_FULLREMAP;
    funPinMode(PC4, GPIO_CFGLR_OUT_2Mhz_AF_PP); // blue LED
    funPinMode(PC7, GPIO_CFGLR_OUT_2Mhz_AF_PP); // green LED
    funPinMode(PC5, GPIO_CFGLR_OUT_2Mhz_AF_PP); // red LED
    funPinMode(PD4, GPIO_CFGLR_OUT_2Mhz_AF_PP); // white LED

    // Reset TIM1 to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

    /* Yes, the colors could have better than 8-bit depth
     * if the prescaler was set to 0 and autoreload was larger.
     * But 8-bit color is well-documented and easy to work with.
     */
    // Autoreload (PWM period) is set so that 8-bit duty cycle values match its range
    // PSC is calculated such that the actual PWM freq is <= LED_PWM_MAX_FREQ_HZ
    const uint32_t autoreload = (1<<COLOR_BIT_DEPTH) - 1; // 255
    TIM1->PSC = (PERIPH_CLOCK / (LED_PWM_MAX_FREQ_HZ * (autoreload + 1)));
    TIM1->ATRLR = autoreload;

    // Set mode = b110 (PWM mode 1, left aligned) and enable shadow regs ("preload") for all
    TIM1->CHCTLR1 |= TIM_OC2M_2 | TIM_OC2M_1 | TIM_OC2PE | TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1PE;
    TIM1->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1 | TIM_OC4PE | TIM_OC3M_2 | TIM_OC3M_1 | TIM_OC3PE;

    // CTLR1 defaults to up-count, UEVs generated (auto-copy from shadow reg), edge align
	TIM1->CTLR1 |= TIM_ARPE; // enable auto-reload of preload

    TIM1->CCER |= TIM_CC4E | TIM_CC3E | TIM_CC2E | TIM_CC1E; // All channels output enable

    // Kickstart the counter with a UG bit set
    TIM1->SWEVGR |= TIM_UG;
    TIM1->CTLR1 |= TIM_CEN;
    TIM1->BDTR |= TIM_MOE; // Main Output Enable required for advanced timer TIM1
}

void updateColor(void) {
    TIM1->CH1CVR = (uint32_t) colorTable[currColor][0];
    TIM1->CH2CVR = (uint32_t) colorTable[currColor][1];
    TIM1->CH3CVR = (uint32_t) colorTable[currColor][2];
    TIM1->CH4CVR = (uint32_t) colorTable[currColor][3];
}

void extiInit(void) {
    // Map PD5 -> EXTI5, PD6 -> EXTI6
    AFIO->EXTICR |= AFIO_EXTICR_EXTI5_PD | AFIO_EXTICR_EXTI6_PD;

    // Enable interrupt (for WFI wake) and event (for WFE wake) on lines 5 and 6
    EXTI->INTENR |= EXTI_INTENR_MR5 | EXTI_INTENR_MR6;
    EXTI->EVENR  |= EXTI_EVENR_MR5  | EXTI_EVENR_MR6;

    // Falling edge trigger (buttons are active-low with pull-ups)
    EXTI->FTENR  |= EXTI_FTENR_TR5  | EXTI_FTENR_TR6;

    NVIC_EnableIRQ(EXTI7_0_IRQn);
}

void enterStandby(void) {
    // Turn off LED outputs
    TIM1->CH1CVR = 0; TIM1->CH2CVR = 0;
    TIM1->CH3CVR = 0; TIM1->CH4CVR = 0;

    // Disable boost converter
    funDigitalWrite(PC3, FUN_LOW);

    // Configure for standby wake
    RCC->APB1PCENR |= RCC_APB1Periph_PWR;
    PWR->CTLR |= PWR_CTLR_PDDS;
    PFIC->SCTLR |= (1 << 2); // SLEEPDEEP

    // Wait for button release before sleeping (avoid instant wake)
    while (funDigitalRead(PD6) == 0 || funDigitalRead(PD5) == 0)
        Delay_Ms(10);
    Delay_Ms(DEBOUNCE_MS);

    // Clear any pending EXTI events
    EXTI->INTFR = EXTI_Line6 | EXTI_Line5;

    // Enter deep standby (WFE)
    __WFE();
}

void exitStandby(void) {
    SystemInit();                       // Restore PLL and clocks
    PFIC->SCTLR &= ~(1 << 2);           // Clear SLEEPDEEP for WFI mode
    funDigitalWrite(PC3, FUN_HIGH);     // Re-enable boost converter
    Delay_Ms(5);                        // Let boost converter stabilize
    updateColor();                      // Restore saved color to PWM

    // Debounce the wake button press
    while (funDigitalRead(PD6) == 0 || funDigitalRead(PD5) == 0)
        Delay_Ms(10);
    Delay_Ms(DEBOUNCE_MS);
    btnEvent = 0;                       // Clear any events from the wake press
}

uint16_t readBatteryMv(void) {
    // Enable internal Vrefint on ADC channel 8
    ADC1->CTLR2 |= ADC_TSVREFE;

    // Read internal 1.2V reference (channel 8)
    uint16_t vref_raw = funAnalogRead(ADC_Channel_Vrefint);

    // Disable Vrefint to save power
    ADC1->CTLR2 &= ~ADC_TSVREFE;

    // VDD = Vrefint_known * ADC_max / vref_raw
    // In mV: (1200 * 1023) / vref_raw
    if (vref_raw == 0) return 0; // Guard against division by zero
    return (uint16_t)((1200UL * 1023) / vref_raw);
}

int main(void) {
    SystemInit();
    Delay_Ms(5000); // Reprogramming window: allow reflashing before sleep

    funGpioInitAll(); // Enables AFIO + ports A C D

    tim1PwmInit();
    funAnalogInit(); // Initialize ADC via ch32fun helper

    // Assign directions to general-purpose pins
    funPinMode(PA1, GPIO_CFGLR_IN_PUPD);        // batt CHG_STAT
    //funPinMode(PA2, GPIO_CFGLR_IN_ANALOG);      // vbatt adc, on AIN0
    funPinMode(PC3, GPIO_CFGLR_OUT_2Mhz_PP);    // VREG_EN
    funPinMode(PD6, GPIO_CFGLR_IN_PUPD);        // SW1
    funPinMode(PD5, GPIO_CFGLR_IN_PUPD);        // SW2
    // Explicitly enable input-pullup resistors for SWy by writing to GPIOx_OUTDR
    funDigitalWrite(PD6, FUN_HIGH);
    funDigitalWrite(PD5, FUN_HIGH);

    // Configure EXTI for button interrupts and events
    extiInit();

    // Write timer duty cycles to display first color
    currColor = 0;
    updateColor();

    // Enable power supply by asserting VREG_EN, thus turning on the LED
    funDigitalWrite(PC3, FUN_HIGH);

    while (1) {
        // WFI in loop ensures mcu always goes to sleep when not doing anything
        __WFI(); // Wait For (button) Interrupt

        /*
         * At this point the light is active and the mcu can sleep. The button interrupts
         * wake the mcu from WFI, and their ISRs simply set a flag and return execution to
         * the loop code below. This code reads the flag, debounces the button, and if
         * short-pressed, sends a new color to TIM1 and reenters sleep. If long-pressed (user
         * wants to turn off the device), it should disable the light and the boost conv,
         * then WFE (standby).
         */
        
        if (!btnEvent) continue; // Spurious wake
        uint8_t btn = btnEvent;
        btnEvent = 0;

        // Software debounce
        Delay_Ms(DEBOUNCE_MS);

        // Determine which pin to poll (pick first if both pressed)
        uint8_t pin = (btn & 0x01) ? PD6 : PD5;

        if (funDigitalRead(pin) != 0) continue; // Bounced away, ignore

        // Long press detection: poll until release or timeout
        uint16_t held = DEBOUNCE_MS;
        while (funDigitalRead(pin) == 0 && held < LONG_PRESS_MS) {
            Delay_Ms(POLL_MS);
            held += POLL_MS;
        }

        if (held >= LONG_PRESS_MS) {
            // LONG PRESS: transition to LED_OFF (standby)
            enterStandby();
            // Execution resumes here after WFE wake
            exitStandby();
        } else {
            // SHORT PRESS: cycle color
            if (btn & 0x01) {
                currColor = (currColor + 1) % NUM_COLORS; // SW1: forward
            } else {
                currColor = (currColor + NUM_COLORS - 1) % NUM_COLORS; // SW2: backward
            }
            updateColor();
        }
    }
    return 0;
}
