#include "uart.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "gpio.h"
#include "ringbuf.h"

#define UART_IRQ_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

static uint8_t rx_storage[128];
static ringbuf_t rx_buf;
static TaskHandle_t volatile rx_waiter;
static SemaphoreHandle_t tx_lock;

void
uart_init(uint32_t baud)
{
	gpio_config(GPIOA, 2, GPIO_ALTFN, 7);
	gpio_config(GPIOA, 3, GPIO_ALTFN, 7);

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	(void)RCC->APB1ENR;

	ringbuf_init(&rx_buf, rx_storage, sizeof rx_storage);
	tx_lock = xSemaphoreCreateMutex();
	configASSERT(tx_lock);

	/* APB1 runs at the core clock (no prescaler configured); 16x oversampling. */
	USART2->BRR = (SystemCoreClock + baud / 2) / baud;
	USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

	NVIC_SetPriority(USART2_IRQn, UART_IRQ_PRIORITY);
	NVIC_EnableIRQ(USART2_IRQn);
}

static void
put_byte(char c)
{
	while (!(USART2->SR & USART_SR_TXE))
		;
	USART2->DR = (uint8_t)c;
}

void
uart_write(const char *data, size_t len)
{
	bool locked = xTaskGetSchedulerState() == taskSCHEDULER_RUNNING;

	if (locked)
	{
		xSemaphoreTake(tx_lock, portMAX_DELAY);
	}
	for (size_t i = 0; i < len; i++)
	{
		put_byte(data[i]);
	}
	if (locked)
	{
		xSemaphoreGive(tx_lock);
	}
}

void
uart_panic_write(const char *s)
{
	while (*s)
	{
		put_byte(*s++);
	}
}

int
uart_getc(uint32_t timeout_ms)
{
	TickType_t timeout = timeout_ms == UART_WAIT_FOREVER ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
	uint8_t c;

	rx_waiter = xTaskGetCurrentTaskHandle();
	while (!ringbuf_get(&rx_buf, &c))
	{
		if (ulTaskNotifyTake(pdTRUE, timeout) == 0)
		{
			return -1;
		}
	}
	return c;
}

void
USART2_IRQHandler(void)
{
	BaseType_t woken = pdFALSE;

	/* Reading SR then DR clears RXNE and any overrun flag. */
	if (USART2->SR & (USART_SR_RXNE | USART_SR_ORE))
	{
		uint8_t c = (uint8_t)USART2->DR;
		ringbuf_put(&rx_buf, c); /* drops the byte if the buffer is full */
		if (rx_waiter)
		{
			vTaskNotifyGiveFromISR(rx_waiter, &woken);
		}
	}
	portYIELD_FROM_ISR(woken);
}
