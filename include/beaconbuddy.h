#ifndef BEACON_BUDDY_H
#define BEACON_BUDDY_H

// for general BSP compatibility reasons this is set to 2, but only 1 physical LED is on the PCB
#define LEDS_NUMBER     2

#define LED_START       17
#define LED_1           17
#define LED_2           18
#define LED_STOP        18

#define LEDS_LIST       { LED_1, LED_2 }

#define BSP_LED_0       LED_1
#define BSP_LED_1       LED_2

#define BSP_LED_0_MASK  (1<<BSP_LED_0)
#define BSP_LED_1_MASK  (1<<BSP_LED_1)

#define LEDS_MASK       (BSP_LED_0_MASK | BSP_LED_1_MASK)
#define LEDS_INV_MASK   LEDS_MASK

#define BUTTONS_NUMBER  0

#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#endif