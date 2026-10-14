#include "gpio.h"

void
gpio_config(GPIO_TypeDef *port, unsigned pin, enum gpio_mode mode, unsigned af)
{
	unsigned port_index = ((uintptr_t)port - GPIOA_BASE) / (GPIOB_BASE - GPIOA_BASE);

	RCC->AHB1ENR |= 1u << port_index;
	(void)RCC->AHB1ENR; /* the clock needs a couple of cycles before the port responds */

	if (mode == GPIO_ALTFN)
	{
		volatile uint32_t *afr = &port->AFR[pin / 8];
		unsigned shift = (pin % 8) * 4;
		*afr = (*afr & ~(0xFu << shift)) | ((af & 0xFu) << shift);
	}
	port->MODER = (port->MODER & ~(3u << (pin * 2))) | ((uint32_t)mode << (pin * 2));
}
