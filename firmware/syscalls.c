/* Minimal newlib system calls: stdout/stderr go to the console UART. */

#include <errno.h>
#include <reent.h>
#include <stdint.h>
#include <sys/stat.h>

#include "FreeRTOS.h"
#include "task.h"

#include "uart.h"

int
_write(int fd, const char *buf, int len)
{
	(void)fd;
	int start = 0;

	/* Terminals expect CRLF line endings. */
	for (int i = 0; i < len; i++)
	{
		if (buf[i] == '\n')
		{
			uart_write(buf + start, (size_t)(i - start));
			uart_write("\r\n", 2);
			start = i + 1;
		}
	}
	uart_write(buf + start, (size_t)(len - start));
	return len;
}

int
_read(int fd, char *buf, int len)
{
	(void)fd;
	(void)buf;
	(void)len;
	return 0; /* stdin is unsupported; use uart_getc() */
}

void *
_sbrk(ptrdiff_t incr)
{
	extern char end; /* end of .bss, from the linker script */
	extern char _estack;
	extern char _Min_Stack_Size;
	/* Integers, not pointers: the heap spans linker symbols that are separate objects in C. */
	static uintptr_t brk = (uintptr_t)&end;
	uintptr_t limit = (uintptr_t)&_estack - (uintptr_t)&_Min_Stack_Size;

	if (incr > 0 && (uintptr_t)incr > limit - brk)
	{
		errno = ENOMEM;
		return (void *)-1;
	}
	uintptr_t prev = brk;
	brk += (uintptr_t)incr;
	return (void *)prev;
}

/* newlib's malloc is shared by all tasks. */
void
__malloc_lock(struct _reent *r)
{
	(void)r;
	vTaskSuspendAll();
}

void
__malloc_unlock(struct _reent *r)
{
	(void)r;
	xTaskResumeAll();
}

int
_close(int fd)
{
	(void)fd;
	return -1;
}

int
_fstat(int fd, struct stat *st)
{
	(void)fd;
	st->st_mode = S_IFCHR;
	return 0;
}

int
_isatty(int fd)
{
	(void)fd;
	return 1;
}

int
_lseek(int fd, int off, int whence)
{
	(void)fd;
	(void)off;
	(void)whence;
	return 0;
}

int
_getpid(void)
{
	return 1;
}

int
_kill(int pid, int sig)
{
	(void)pid;
	(void)sig;
	errno = EINVAL;
	return -1;
}

void
_exit(int status)
{
	(void)status;
	for (;;)
		;
}
