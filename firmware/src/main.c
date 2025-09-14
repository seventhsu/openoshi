#include "ch32fun.h"
#include <stdio.h>

#define LED_PWM_FREQ_HZ 1000

// colorTable[i][0:3] represent B, G, R, W channels of one color setting
uint16_t colorTable[][4] = {
	{0x0,	0x0,	0x1ff,	0x0},   // Color red
    {0x0,	0x1ff,	0x1ff,	0x0},   // Color yellow
	{0x0,	0x1ff,	0x0,	0x0},   // Color green
    {0x1ff,	0x1ff,	0x0,	0x0},   // Color cyan
	{0x1ff,	0x0,	0x0,	0x0},   // Color blue
    {0x1ff,	0x0,	0x1ff,	0x0},   // Color magenta
	{0x0,	0x0,	0x0,	0x1ff}, // Color white

};

// The color the penlight should display if it was on (index into colorTable)
static volatile int currColor = 0;

int main(void) {
    SystemInit();

    // RCC clock enable to all necessary units
    funGpioInitAll(); // inits AFIO + ports A C D
    RCC->APB2PCENR |= (RCC_APB2Periph_TIM1 |
                        RCC_APB2Periph_ADC1); // TIM1, ADC1 enable
    
    // Assign directions to general-purpose pins
    funPinMode(PA1, GPIO_CFGLR_IN_PUPD); // batt CHG_STAT
    funPinMode(PA2, GPIO_CFGLR_IN_ANALOG); // vbatt adc, on AIN0
    funPinMode(PC3, GPIO_CFGLR_OUT_2Mhz_PP); // VREG_EN
    funPinMode(PD6, GPIO_CFGLR_IN_PUPD); // SW1
    funPinMode(PD5, GPIO_CFGLR_IN_PUPD); // SW2
    // Explicitly enable input pullup resistors for SWy by writing to GPIOx_OUTDR
    funDigitalWrite(PD6, FUN_HIGH);
    funDigitalWrite(PD5, FUN_HIGH);
    
    // Map and AF-configure pins to be used by TIM1
    // TIM1 map: CH1=PC4(blue), CH2=PC7(green), CH3=PC5(red), CH4=PD4(white)
    AFIO->PCFR1 |= AFIO_PCFR1_TIM1_REMAP_FULLREMAP;
    funPinMode(PC4, GPIO_CFGLR_OUT_2Mhz_AF_PP); // blue LED
    funPinMode(PC7, GPIO_CFGLR_OUT_2Mhz_AF_PP); // green LED
    funPinMode(PC5, GPIO_CFGLR_OUT_2Mhz_AF_PP); // red LED
    funPinMode(PD4, GPIO_CFGLR_OUT_2Mhz_AF_PP); // white LED

    // Reset ADC1 to init all regs
    RCC->APB2PRSTR |= RCC_ADC1RST;
	RCC->APB2PRSTR &= ~RCC_ADC1RST;

    // Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_TIM1RST;
	RCC->APB2PRSTR &= ~RCC_TIM1RST;

    // Set up timer duty cycles to display current color
    for (int i = 0; i < 4; ++i) {
        // write colorTable[currColor][i] to TIM1_CHi duty cycle reg
    }

    // Enable power supply by toggling VREG_EN, thus turning on the LED
    funDigitalWrite(PC3, FUN_HIGH);

    // Enable button interrupts

    // ADC reads battery periodically using system periodic wake timer
    // int val = funAnalogRead(0);

    // Enter sleep
    __WFI();

    /*
     * At this point the light is active and the mcu can sleep. The button interrupts
     * wake the mcu from WFI, and their ISRs simply inc/dec currColor, send the
     * new color to TIM1, and reenter sleep. If the button is long-pressed (user
     * wants to turn off the device), it should disable the light and the boost conv,
     * then WFE (standby).
    */

    while (1) {
        // stall
    }
    return 0;
}
