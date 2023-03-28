/**
 * @file    test_ring_buffer.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <string.h>
#include <unity.h>

#include "logger/logger.h"
#include "utils/circ_lifo.h"

// clang-format off
#define TEST_CIRC_LIFO_TEST_BUFFER_SIZE 6u
#define TEST_CIRC_LIFO_PROPER_CAPACITY  (1u << 8u)
#define TEST_CIRC_LIFO_BAD_CAPACITY     0xFFu
#define TEST_CIRC_LIFO_BUFFER           {5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f}
// clang-format on

static float test_proper_static_buffer[TEST_CIRC_LIFO_BAD_CAPACITY] = {0u};
static float test_bad_static_buffer[TEST_CIRC_LIFO_BAD_CAPACITY]    = {0u};
static struct circ_lifo test_cl;

void setUp(void)
{
}

void test_circ_lifo_dynamic_should_create_and_destruct(void)
{
	int error = circ_lifo_construct(
		&test_cl, NULL, TEST_CIRC_LIFO_PROPER_CAPACITY);
	circ_lifo_destruct(&test_cl);

	TEST_ASSERT_EQUAL(0, error);
}

void test_circ_lifo_dynamic_should_not_create(void)
{
	int error =
		circ_lifo_construct(&test_cl, NULL, TEST_CIRC_LIFO_BAD_CAPACITY);

	TEST_ASSERT_EQUAL(-EPERM, error);
}

void test_circ_lifo_static_should_create_and_destruct(void)
{
	int error = circ_lifo_construct(
		&test_cl,
		test_proper_static_buffer,
		TEST_CIRC_LIFO_PROPER_CAPACITY);
	circ_lifo_destruct(&test_cl);

	TEST_ASSERT_EQUAL(0, error);
}

void test_circ_lifo_static_should_not_create(void)
{
	int error = circ_lifo_construct(
		&test_cl, test_bad_static_buffer, TEST_CIRC_LIFO_BAD_CAPACITY);
	circ_lifo_destruct(&test_cl);

	TEST_ASSERT_EQUAL(-EPERM, error);
}

void test_circ_lifo_should_write_and_read(void)
{
	int error = circ_lifo_construct(
		&test_cl, NULL, TEST_CIRC_LIFO_PROPER_CAPACITY);

	for (uint8_t i = 0u; i < TEST_CIRC_LIFO_TEST_BUFFER_SIZE; i++)
		circ_lifo_write(&test_cl, i);

	float test_buffer[TEST_CIRC_LIFO_TEST_BUFFER_SIZE];
	for (uint8_t i = 0u; i < TEST_CIRC_LIFO_TEST_BUFFER_SIZE; i++)
		test_buffer[i] = circ_lifo_read(&test_cl);

	float expected_buffer[] = TEST_CIRC_LIFO_BUFFER;

	TEST_ASSERT_EQUAL_FLOAT_ARRAY(
		expected_buffer, test_buffer, TEST_CIRC_LIFO_TEST_BUFFER_SIZE);
}
