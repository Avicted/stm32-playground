#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "apps.h"
#include "bsp.h"
#include "uart.h"

static void (*const apps[])(void) = {
	blinky_start,
	shell_start,
};

int
main(void)
{
	bsp_init();
	printf("\nstm32-playground: FreeRTOS %s, core clock %lu Hz\n", tskKERNEL_VERSION_NUMBER, (unsigned long)SystemCoreClock);

	for (size_t i = 0; i < sizeof apps / sizeof apps[0]; i++)
	{
		apps[i]();
	}

	vTaskStartScheduler();
	for (;;) /* only reached if the idle task couldn't be created */
		;
}

static void
halt(const char *msg, const char *detail)
{
	taskDISABLE_INTERRUPTS();
	uart_panic_write("\r\n*** ");
	uart_panic_write(msg);
	uart_panic_write(detail);
	uart_panic_write("\r\n");
	for (;;)
		;
}

void
vAssertCalled(const char *file, int line)
{
	char buf[16];
	snprintf(buf, sizeof buf, ":%d", line);
	uart_panic_write("\r\n*** assert failed: ");
	halt(file, buf);
}

void
vApplicationMallocFailedHook(void)
{
	halt("FreeRTOS heap exhausted", "");
}

void
vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
	(void)task;
	halt("stack overflow in task ", name);
}

void
HardFault_Handler(void)
{
	halt("hard fault", "");
}
