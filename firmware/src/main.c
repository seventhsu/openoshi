#include "ch32fun.h"
#include <stdio.h>

#define LED_PWM_FREQ_HZ 1000

// colorTable[i][0:3] represent R, G, B, W channels of one color setting
uint16_t colorTable[][4] = {
	{0x1ff,	0x0,	0x0,	0x0},
	{0x0,	0x1ff,	0x0,	0x0},
	{0x0,	0x0,	0x1ff,	0x0},
	{0x0,	0x0,	0x0,	0x1ff},

};

int colorIndex = 0;


int main(void) {
    SystemInit();

    // RCC clock enable to all necessary units
    funGpioInitAll(); // inits AFIO + ports A C D
    RCC->APB2PCENR |= (RCC_APB2Periph_TIM1 
                    | RCC_APB2Periph_ADC1); // TIM1, ADC1 enable
    
    // Assign pin directions
    funPinMode( PA1, GPIO_CFGLR_IN_PUPD ); // batt CHG_STAT
    funPinMode( PD6, GPIO_CFGLR_IN_PUPD ); // SW1
    funPinMode( PD5, GPIO_CFGLR_IN_PUPD ); // SW2
    funPinMode( PA2, GPIO_CFGLR_IN_ANALOG ); // vbatt adc, on AIN0
    // FIXME: to use AF or not to use AF?
    funPinMode( PC3, GPIO_CFGLR_OUT_2Mhz_AF_PP ); // VREG_EN
    AFIO->PCFR1 |= AFIO_PCFR1_TIM1_REMAP_FULLREMAP;
    // TIM1 CH1=PC4(blue), CH2=PC7(green), CH3=PC5(red), CH4=PD4(white)

    // Reset ADC1 to init all regs
    RCC->APB2PRSTR |= RCC_ADC1RST;
	RCC->APB2PRSTR &= ~RCC_ADC1RST;

    // Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_TIM1RST;
	RCC->APB2PRSTR &= ~RCC_TIM1RST;

    // Set up timer regs to display current color
    //colorIndex = 0;
    for (int i = 0; i < 4; ++i) {
        // write colorTable[colorIndex][i] to TIM1 duty cycle
    }

    // Enable button interrupts

    // Enter sleep
    __WFI();

    /*
     * At this point the light is active and the mcu can sleep. The button interrupts
     * wake the mcu from WFI, and their ISRs simply inc/dec colorIndex, send the
     * new color to TIM1, and reenter sleep. If the button is long-pressed (user
     * wants to turn off the device), it should disable the light and the boost conv,
     * then WFE (standby).
    */

    while (1) {
        // stall
    }
    return 0;
}
