/* Interactive command shell on the console UART. */

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "apps.h"
#include "bsp.h"
#include "cli.h"
#include "uart.h"

static int
cmd_help(int argc, char *argv[]);

static int
cmd_led(int argc, char *argv[])
{
	if (argc != 3)
	{
		goto usage;
	}
	for (enum bsp_led led = 0; led < LED_COUNT; led++)
	{
		if (strcmp(argv[1], bsp_led_name(led)) != 0)
		{
			continue;
		}
		if (strcmp(argv[2], "on") == 0)
		{
			bsp_led_set(led, true);
		}
		else if (strcmp(argv[2], "off") == 0)
		{
			bsp_led_set(led, false);
		}
		else if (strcmp(argv[2], "toggle") == 0)
		{
			bsp_led_toggle(led);
		}
		else
		{
			goto usage;
		}
		printf("%s led %s\n", argv[1], bsp_led_get(led) ? "on" : "off");
		return 0;
	}
usage:
	printf("usage: led <green|orange|red|blue> <on|off|toggle>\n");
	return 1;
}

static int
cmd_button(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	printf("button %s\n", bsp_button_pressed() ? "pressed" : "released");
	return 0;
}

static int
cmd_uptime(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	printf("%lu ms\n", (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS));
	return 0;
}

static int
cmd_tasks(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	static char buf[512];
	vTaskListTasks(buf, sizeof buf);
	printf("%-*s\tstate\tprio\tstack\tnum\n%s", configMAX_TASK_NAME_LEN - 1, "name", buf);
	return 0;
}

static const struct cli_command commands[] = {
	{"help", "list commands", cmd_help},		 {"led", "set an LED: led <name> <on|off|toggle>", cmd_led}, {"button", "read the user button", cmd_button}, {"uptime", "time since boot", cmd_uptime},
	{"tasks", "list FreeRTOS tasks", cmd_tasks},
};
#define NUM_COMMANDS (sizeof commands / sizeof commands[0])

static int
cmd_help(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	for (size_t i = 0; i < NUM_COMMANDS; i++)
	{
		printf("  %-8s %s\n", commands[i].name, commands[i].help);
	}
	return 0;
}

static void
prompt(void)
{
	printf("> ");
	fflush(stdout);
}

static void
shell_task(void *arg)
{
	(void)arg;
	struct cli_line line;

	cli_line_init(&line);
	printf("type 'help' for commands\n");
	prompt();

	for (;;)
	{
		char c = (char)uart_getc(UART_WAIT_FOREVER);

		switch (cli_line_feed(&line, c))
		{
		case CLI_FEED_ADDED:
			uart_write(&c, 1);
			break;
		case CLI_FEED_ERASED:
			uart_write("\b \b", 3);
			break;
		case CLI_FEED_DONE:
			uart_write("\r\n", 2);
			int rc = cli_execute(commands, NUM_COMMANDS, line.buf);
			if (rc == CLI_NOT_FOUND)
			{
				printf("unknown command, try 'help'\n");
			}
			else if (rc == CLI_TOO_MANY_ARGS)
			{
				printf("too many arguments\n");
			}
			prompt();
			break;
		case CLI_FEED_IGNORED:
			break;
		}
	}
}

void
shell_start(void)
{
	xTaskCreate(shell_task, "shell", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
}
