#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Byte FIFO over caller-provided storage. Safe without locking for exactly one
 * producer and one consumer (e.g. an ISR writing, a task reading) on a single core.
 * Holds at most size - 1 bytes.
 */
typedef struct
{
	volatile uint8_t *buf;
	size_t size;
	volatile size_t head; /* next write; only the producer modifies it */
	volatile size_t tail; /* next read; only the consumer modifies it */
} ringbuf_t;

void
ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t size);

bool
ringbuf_put(ringbuf_t *rb, uint8_t byte);

bool
ringbuf_get(ringbuf_t *rb, uint8_t *byte);

size_t
ringbuf_count(const ringbuf_t *rb);

#endif // RINGBUF_H
