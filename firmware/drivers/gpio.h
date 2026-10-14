#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include "stm32f4xx.h"

enum gpio_mode
{
	GPIO_INPUT = 0,
	GPIO_OUTPUT = 1,
	GPIO_ALTFN = 2,
	GPIO_ANALOG = 3,
};

/* Enables the port's clock and sets the pin's mode; af is used only for GPIO_ALTFN. */
void
gpio_config(GPIO_TypeDef *port, unsigned pin, enum gpio_mode mode, unsigned af);

static inline void
gpio_write(GPIO_TypeDef *port, unsigned pin, bool on)
{
	port->BSRR = on ? (1u << pin) : (1u << (pin + 16));
}

static inline bool
gpio_read(GPIO_TypeDef *port, unsigned pin)
{
	return (port->IDR >> pin) & 1u;
}

static inline bool
gpio_read_output(GPIO_TypeDef *port, unsigned pin)
{
	return (port->ODR >> pin) & 1u;
}

#endif // GPIO_H
