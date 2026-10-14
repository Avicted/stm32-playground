#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stddef.h>

#define CLI_LINE_MAX 64
#define CLI_MAX_ARGS 8

/* Line editor: feed it characters one at a time as they arrive. */
struct cli_line
{
	char buf[CLI_LINE_MAX];
	size_t len;
	bool done;	  /* buf holds a finished line; cleared by the next feed */
	bool last_cr; /* so a CRLF line ending counts as one */
};

enum cli_feed_result
{
	CLI_FEED_IGNORED, /* nothing changed (control char, line full, ...) */
	CLI_FEED_ADDED,	  /* char appended; caller should echo it */
	CLI_FEED_ERASED,  /* last char removed; caller should echo "\b \b" */
	CLI_FEED_DONE,	  /* line complete, NUL-terminated in buf */
};

void
cli_line_init(struct cli_line *line);

enum cli_feed_result
cli_line_feed(struct cli_line *line, char c);

/* Command dispatch */
struct cli_command
{
	const char *name;
	const char *help;
	int (*handler)(int argc, char *argv[]);
};

enum
{
	CLI_EMPTY = -1000, /* blank line */
	CLI_NOT_FOUND,	   /* no command with that name */
	CLI_TOO_MANY_ARGS, /* more than CLI_MAX_ARGS words */
};

/* Splits line in place on spaces/tabs. Returns argc, or CLI_TOO_MANY_ARGS. */
int
cli_split(char *line, char *argv[], int max_args);

/* Splits line and runs the matching command. Returns the handler's result or a CLI_* error. */
int
cli_execute(const struct cli_command *cmds, size_t count, char *line);

#endif // CLI_H
