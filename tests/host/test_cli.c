#include "cli.h"
#include "unity.h"

#include <string.h>

static struct cli_line line;
static int last_argc;
static char last_args[CLI_MAX_ARGS][CLI_LINE_MAX];

static int
record(int argc, char *argv[])
{
	last_argc = argc;
	for (int i = 0; i < argc; i++)
	{
		strcpy(last_args[i], argv[i]);
	}
	return 42;
}

static const struct cli_command cmds[] = {
	{"led", "", record},
	{"uptime", "", record},
};
#define NCMDS (sizeof cmds / sizeof cmds[0])

void
setUp(void)
{
	cli_line_init(&line);
	last_argc = -1;
}
void
tearDown(void)
{
}

static void
feed_str(const char *s)
{
	while (*s)
	{
		cli_line_feed(&line, *s++);
	}
}

static void
test_line_collects_until_newline(void)
{
	TEST_ASSERT_EQUAL(CLI_FEED_ADDED, cli_line_feed(&line, 'h'));
	TEST_ASSERT_EQUAL(CLI_FEED_ADDED, cli_line_feed(&line, 'i'));
	TEST_ASSERT_EQUAL(CLI_FEED_DONE, cli_line_feed(&line, '\r'));
	TEST_ASSERT_EQUAL_STRING("hi", line.buf);
}

static void
test_line_backspace(void)
{
	feed_str("ab");
	TEST_ASSERT_EQUAL(CLI_FEED_ERASED, cli_line_feed(&line, 0x7f));
	TEST_ASSERT_EQUAL(CLI_FEED_ADDED, cli_line_feed(&line, 'c'));
	cli_line_feed(&line, '\n');
	TEST_ASSERT_EQUAL_STRING("ac", line.buf);
}

static void
test_line_backspace_on_empty_ignored(void)
{
	TEST_ASSERT_EQUAL(CLI_FEED_IGNORED, cli_line_feed(&line, '\b'));
}

static void
test_line_crlf_is_one_line(void)
{
	feed_str("x");
	TEST_ASSERT_EQUAL(CLI_FEED_DONE, cli_line_feed(&line, '\r'));
	TEST_ASSERT_EQUAL(CLI_FEED_IGNORED, cli_line_feed(&line, '\n'));
	TEST_ASSERT_EQUAL(CLI_FEED_DONE, cli_line_feed(&line, '\n'));
	TEST_ASSERT_EQUAL_STRING("", line.buf);
}

static void
test_line_restarts_after_done(void)
{
	feed_str("one\n");
	feed_str("two\n");
	TEST_ASSERT_EQUAL_STRING("two", line.buf);
}

static void
test_line_overflow_truncates(void)
{
	for (int i = 0; i < CLI_LINE_MAX + 10; i++)
	{
		cli_line_feed(&line, 'a');
	}
	TEST_ASSERT_EQUAL(CLI_FEED_DONE, cli_line_feed(&line, '\n'));
	TEST_ASSERT_EQUAL_size_t(CLI_LINE_MAX - 1, strlen(line.buf));
}

static void
test_split_words(void)
{
	char s[] = "  led   on\t2 ";
	char *argv[CLI_MAX_ARGS];
	TEST_ASSERT_EQUAL(3, cli_split(s, argv, CLI_MAX_ARGS));
	TEST_ASSERT_EQUAL_STRING("led", argv[0]);
	TEST_ASSERT_EQUAL_STRING("on", argv[1]);
	TEST_ASSERT_EQUAL_STRING("2", argv[2]);
}

static void
test_split_too_many(void)
{
	char s[] = "a b c";
	char *argv[2];
	TEST_ASSERT_EQUAL(CLI_TOO_MANY_ARGS, cli_split(s, argv, 2));
}

static void
test_execute_dispatches(void)
{
	char s[] = "led on";
	TEST_ASSERT_EQUAL(42, cli_execute(cmds, NCMDS, s));
	TEST_ASSERT_EQUAL(2, last_argc);
	TEST_ASSERT_EQUAL_STRING("led", last_args[0]);
	TEST_ASSERT_EQUAL_STRING("on", last_args[1]);
}

static void
test_execute_errors(void)
{
	char blank[] = "   ";
	char unknown[] = "reboot";
	TEST_ASSERT_EQUAL(CLI_EMPTY, cli_execute(cmds, NCMDS, blank));
	TEST_ASSERT_EQUAL(CLI_NOT_FOUND, cli_execute(cmds, NCMDS, unknown));
	TEST_ASSERT_EQUAL(-1, last_argc);
}

int
main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_line_collects_until_newline);
	RUN_TEST(test_line_backspace);
	RUN_TEST(test_line_backspace_on_empty_ignored);
	RUN_TEST(test_line_crlf_is_one_line);
	RUN_TEST(test_line_restarts_after_done);
	RUN_TEST(test_line_overflow_truncates);
	RUN_TEST(test_split_words);
	RUN_TEST(test_split_too_many);
	RUN_TEST(test_execute_dispatches);
	RUN_TEST(test_execute_errors);
	return UNITY_END();
}
