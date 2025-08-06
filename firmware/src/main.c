#include "ch32fun.h"

#define     LED_PWM_FREQ_HZ     1000U

// colortable[i][0:3] represent R, G, B, W channels of one color setting
uint16_t colortable[][4] = {
	{0x1ff,	0x0,	0x0,	0x0},
	{0x0,	0x1ff,	0x0,	0x0},
	{0x0,	0x0,	0x1ff,	0x0},
	{0x0,	0x0,	0x0,	0x1ff},

};

int main(void) {
    uint32_t x;

    while (1) {
        // stall
    }

    return 0;
}
