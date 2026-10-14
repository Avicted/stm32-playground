#ifndef BSP_H
#define BSP_H

#include <stdbool.h>

/* STM32F407 Discovery board: user LEDs on PD12-15, user button on PA0. */

enum bsp_led
{
	LED_GREEN,
	LED_ORANGE,
	LED_RED,
	LED_BLUE,
	LED_COUNT,
};

/* Sets up LEDs, button and the console UART (115200 8N1). */
void
bsp_init(void);

void
bsp_led_set(enum bsp_led led, bool on);

void
bsp_led_toggle(enum bsp_led led);

bool
bsp_led_get(enum bsp_led led);

const char *
bsp_led_name(enum bsp_led led);

bool
bsp_button_pressed(void);

#endif // BSP_H
