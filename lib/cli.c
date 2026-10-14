#include "cli.h"

#include <string.h>

void
cli_line_init(struct cli_line *line)
{
	line->len = 0;
	line->buf[0] = '\0';
	line->done = false;
	line->last_cr = false;
}

enum cli_feed_result
cli_line_feed(struct cli_line *line, char c)
{
	bool after_cr = line->last_cr;

	if (line->done)
	{
		cli_line_init(line);
	}
	line->last_cr = (c == '\r');
	if (c == '\n' && after_cr)
	{
		return CLI_FEED_IGNORED;
	}

	switch (c)
	{
	case '\r':
	case '\n':
		line->buf[line->len] = '\0';
		line->done = true;
		return CLI_FEED_DONE;
	case '\b':
	case 0x7f: /* DEL, sent by most terminals for backspace */
		if (line->len == 0)
		{
			return CLI_FEED_IGNORED;
		}
		line->len--;
		return CLI_FEED_ERASED;
	default:
		if (c < ' ' || c > '~' || line->len >= CLI_LINE_MAX - 1)
		{
			return CLI_FEED_IGNORED;
		}
		line->buf[line->len++] = c;
		return CLI_FEED_ADDED;
	}
}

int
cli_split(char *line, char *argv[], int max_args)
{
	int argc = 0;
	char *p = line;

	for (;;)
	{
		while (*p == ' ' || *p == '\t')
		{
			p++;
		}
		if (*p == '\0')
		{
			return argc;
		}
		if (argc == max_args)
		{
			return CLI_TOO_MANY_ARGS;
		}
		argv[argc++] = p;
		while (*p != '\0' && *p != ' ' && *p != '\t')
		{
			p++;
		}
		if (*p != '\0')
		{
			*p++ = '\0';
		}
	}
}

int
cli_execute(const struct cli_command *cmds, size_t count, char *line)
{
	char *argv[CLI_MAX_ARGS];
	int argc = cli_split(line, argv, CLI_MAX_ARGS);

	if (argc < 0)
	{
		return argc;
	}
	if (argc == 0)
	{
		return CLI_EMPTY;
	}
	for (size_t i = 0; i < count; i++)
	{
		if (strcmp(cmds[i].name, argv[0]) == 0)
		{
			return cmds[i].handler(argc, argv);
		}
	}
	return CLI_NOT_FOUND;
}
