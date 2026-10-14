/* Blinks the green LED at 1 Hz. */

#include "FreeRTOS.h"
#include "task.h"

#include "apps.h"
#include "bsp.h"

#define BLINK_HALF_PERIOD_MS 500

static void
blinky_task(void *arg)
{
	(void)arg;
	TickType_t wake = xTaskGetTickCount();

	for (;;)
	{
		bsp_led_toggle(LED_GREEN);
		vTaskDelayUntil(&wake, pdMS_TO_TICKS(BLINK_HALF_PERIOD_MS));
	}
}

void
blinky_start(void)
{
	xTaskCreate(blinky_task, "blinky", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
}
