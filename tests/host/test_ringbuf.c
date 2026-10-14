#include "ringbuf.h"
#include "unity.h"

static uint8_t storage[4];
static ringbuf_t rb;

void
setUp(void)
{
	ringbuf_init(&rb, storage, sizeof storage);
}

void
tearDown(void)
{
}

static void
test_starts_empty(void)
{
	uint8_t b;
	TEST_ASSERT_EQUAL_size_t(0, ringbuf_count(&rb));
	TEST_ASSERT_FALSE(ringbuf_get(&rb, &b));
}

static void
test_fifo_order(void)
{
	uint8_t b;
	TEST_ASSERT_TRUE(ringbuf_put(&rb, 1));
	TEST_ASSERT_TRUE(ringbuf_put(&rb, 2));
	TEST_ASSERT_EQUAL_size_t(2, ringbuf_count(&rb));
	TEST_ASSERT_TRUE(ringbuf_get(&rb, &b));
	TEST_ASSERT_EQUAL_UINT8(1, b);
	TEST_ASSERT_TRUE(ringbuf_get(&rb, &b));
	TEST_ASSERT_EQUAL_UINT8(2, b);
	TEST_ASSERT_FALSE(ringbuf_get(&rb, &b));
}

static void
test_full_rejects_put(void)
{
	TEST_ASSERT_TRUE(ringbuf_put(&rb, 1));
	TEST_ASSERT_TRUE(ringbuf_put(&rb, 2));
	TEST_ASSERT_TRUE(ringbuf_put(&rb, 3));
	TEST_ASSERT_FALSE(ringbuf_put(&rb, 4)); /* capacity is size - 1 */
	TEST_ASSERT_EQUAL_size_t(3, ringbuf_count(&rb));
}

static void
test_wraps_around(void)
{
	uint8_t b;
	for (uint8_t i = 0; i < 10; i++)
	{
		TEST_ASSERT_TRUE(ringbuf_put(&rb, i));
		TEST_ASSERT_TRUE(ringbuf_put(&rb, (uint8_t)(i + 100)));
		TEST_ASSERT_TRUE(ringbuf_get(&rb, &b));
		TEST_ASSERT_EQUAL_UINT8(i, b);
		TEST_ASSERT_TRUE(ringbuf_get(&rb, &b));
		TEST_ASSERT_EQUAL_UINT8(i + 100, b);
	}
	TEST_ASSERT_EQUAL_size_t(0, ringbuf_count(&rb));
}

int
main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_starts_empty);
	RUN_TEST(test_fifo_order);
	RUN_TEST(test_full_rejects_put);
	RUN_TEST(test_wraps_around);
	return UNITY_END();
}
