#include "../ch32fun/ch32fun/ch32fun.h"
#include <stdio.h>

#define HPRE_PRESCALED_PERIPH_CLOCK (FUNCONF_SYSTEM_CORE_CLOCK / 3)

#define LED_PWM_MAX_FREQ_HZ 1000 // Change me at your leisure!
#define COLOR_BIT_DEPTH 8

// colorTable[i][0:3] represent B, G, R, W channels of one color setting
uint8_t colorTable[][4] = {
	{0x0,	0x0,	0xff,	0x0},   // Color red
    {0x0,	0xff,	0xff,	0x0},   // Color yellow
	{0x0,	0xff,	0x0,	0x0},   // Color green
    {0xff,	0xff,	0x0,	0x0},   // Color cyan
	{0xff,	0x0,	0x0,	0x0},   // Color blue
    {0xff,	0x0,	0xff,	0x0},   // Color magenta
	{0x0,	0x0,	0x0,	0xff}, // Color white
};

// The color the penlight should display if it was on (index into colorTable)
static volatile int currColor;

void adcInit(void) {
    RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

    // Reset ADC1 to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // TODO...
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
    TIM1->PSC = (HPRE_PRESCALED_PERIPH_CLOCK / (LED_PWM_MAX_FREQ_HZ * (autoreload + 1)));
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
}

void updateColor(void) {
    TIM1->CH1CVR = (uint32_t) colorTable[currColor][0];
    TIM1->CH2CVR = (uint32_t) colorTable[currColor][1];
    TIM1->CH3CVR = (uint32_t) colorTable[currColor][2];
    TIM1->CH4CVR = (uint32_t) colorTable[currColor][3];
}

int main(void) {
    SystemInit();
    Delay_Ms(100);

    funGpioInitAll(); // Enables AFIO + ports A C D

    tim1PwmInit();
    adcInit();
    
    // Assign directions to general-purpose pins
    funPinMode(PA1, GPIO_CFGLR_IN_PUPD); // batt CHG_STAT
    funPinMode(PA2, GPIO_CFGLR_IN_ANALOG); // vbatt adc, on AIN0
    funPinMode(PC3, GPIO_CFGLR_OUT_2Mhz_PP); // VREG_EN
    funPinMode(PD6, GPIO_CFGLR_IN_PUPD); // SW1
    funPinMode(PD5, GPIO_CFGLR_IN_PUPD); // SW2
    // Explicitly enable input-pullup resistors for SWy by writing to GPIOx_OUTDR
    funDigitalWrite(PD6, FUN_HIGH);
    funDigitalWrite(PD5, FUN_HIGH);

    // Write timer duty cycles to display first color
    currColor = 0;
    updateColor();

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
