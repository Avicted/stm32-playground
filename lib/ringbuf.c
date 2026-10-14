#include "ringbuf.h"

void
ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t size)
{
	rb->buf = storage;
	rb->size = size;
	rb->head = 0;
	rb->tail = 0;
}

bool
ringbuf_put(ringbuf_t *rb, uint8_t byte)
{
	size_t next = (rb->head + 1) % rb->size;
	if (next == rb->tail)
	{
		return false;
	}
	rb->buf[rb->head] = byte;
	rb->head = next;
	return true;
}

bool
ringbuf_get(ringbuf_t *rb, uint8_t *byte)
{
	if (rb->tail == rb->head)
	{
		return false;
	}
	*byte = rb->buf[rb->tail];
	rb->tail = (rb->tail + 1) % rb->size;
	return true;
}

size_t
ringbuf_count(const ringbuf_t *rb)
{
	return (rb->head + rb->size - rb->tail) % rb->size;
}
