#include "bsp.h"

#include "gpio.h"
#include "uart.h"

#define LED_PORT	GPIOD
#define LED_PIN0	12 /* green; orange, red, blue follow */
#define BUTTON_PORT GPIOA
#define BUTTON_PIN	0

static const char *const led_names[LED_COUNT] = {"green", "orange", "red", "blue"};

void
bsp_init(void)
{
	for (unsigned i = 0; i < LED_COUNT; i++)
	{
		gpio_config(LED_PORT, LED_PIN0 + i, GPIO_OUTPUT, 0);
	}
	gpio_config(BUTTON_PORT, BUTTON_PIN, GPIO_INPUT, 0);
	uart_init(115200);
}

void
bsp_led_set(enum bsp_led led, bool on)
{
	gpio_write(LED_PORT, LED_PIN0 + led, on);
}

void
bsp_led_toggle(enum bsp_led led)
{
	bsp_led_set(led, !bsp_led_get(led));
}

bool
bsp_led_get(enum bsp_led led)
{
	return gpio_read_output(LED_PORT, LED_PIN0 + led);
}

const char *
bsp_led_name(enum bsp_led led)
{
	return led < LED_COUNT ? led_names[led] : "?";
}

bool
bsp_button_pressed(void)
{
	return gpio_read(BUTTON_PORT, BUTTON_PIN); /* active high, external pull-down */
}
