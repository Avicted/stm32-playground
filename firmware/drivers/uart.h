#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

/* Console UART: USART2, TX on PA2, RX on PA3, 8N1. */

#define UART_WAIT_FOREVER UINT32_MAX

void
uart_init(uint32_t baud);

/* Blocking write. Task context only; concurrent writers are serialized. */
void
uart_write(const char *data, size_t len);

/* Write with no locking or RTOS calls, for fault handlers and asserts. */
void
uart_panic_write(const char *s);

/*
 * Returns the next received byte, or -1 after timeout_ms with no input.
 * Blocks the calling task; only one task may read at a time.
 */
int
uart_getc(uint32_t timeout_ms);

#endif // UART_H
