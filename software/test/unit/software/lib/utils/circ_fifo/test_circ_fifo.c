/**
 * @file    test_ring_buffer.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <string.h>
#include <unity.h>

#include "logger/logger.h"
#include "utils/circ_fifo.h"

// clang-format off
#define TEST_CIRC_FIFO_TEST_BUFFER_SIZE 	6u
#define TEST_CIRC_FIFO_PROPER_CAPACITY  	(1u << 4u)
#define TEST_CIRC_FIFO_OVERFLOW_CAPACITY 	(1u << 2u)
#define TEST_CIRC_FIFO_BAD_CAPACITY     	0xFFu
#define TEST_CIRC_FIFO_BUFFER           	{5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f}
// clang-format on

static float test_proper_static_buffer[TEST_CIRC_FIFO_BAD_CAPACITY] = {0u};
static float test_bad_static_buffer[TEST_CIRC_FIFO_BAD_CAPACITY]    = {0u};
static struct circ_fifo test_cf;

void setUp(void)
{
}

void test_circ_fifo_dynamic_should_create_and_destruct(void)
{
	int error = circ_fifo_construct(
		&test_cf, NULL, TEST_CIRC_FIFO_PROPER_CAPACITY);
	circ_fifo_destruct(&test_cf);

	TEST_ASSERT_EQUAL(0, error);
}

void test_circ_fifo_dynamic_should_not_create(void)
{
	int error =
		circ_fifo_construct(&test_cf, NULL, TEST_CIRC_FIFO_BAD_CAPACITY);

	TEST_ASSERT_EQUAL(-EPERM, error);
}

void test_circ_fifo_static_should_create_and_destruct(void)
{
	int error = circ_fifo_construct(
		&test_cf,
		test_proper_static_buffer,
		TEST_CIRC_FIFO_PROPER_CAPACITY);
	circ_fifo_destruct(&test_cf);

	TEST_ASSERT_EQUAL(0, error);
}

void test_circ_fifo_static_should_not_create(void)
{
	int error = circ_fifo_construct(
		&test_cf, test_bad_static_buffer, TEST_CIRC_FIFO_BAD_CAPACITY);
	circ_fifo_destruct(&test_cf);

	TEST_ASSERT_EQUAL(-EPERM, error);
}

void test_circ_fifo_should_write_and_read(void)
{
	int error = circ_fifo_construct(
		&test_cf, NULL, TEST_CIRC_FIFO_PROPER_CAPACITY);

	TEST_ASSERT_EQUAL(0, error);

	for (uint8_t i = 0u; i < TEST_CIRC_FIFO_TEST_BUFFER_SIZE; i++)
		circ_fifo_write(&test_cf, i);

	float test_buffer[TEST_CIRC_FIFO_TEST_BUFFER_SIZE];
	for (uint8_t i = 0u; i < TEST_CIRC_FIFO_TEST_BUFFER_SIZE; i++)
		test_buffer[i] = circ_fifo_read(&test_cf);

	float expected_buffer[] = TEST_CIRC_FIFO_BUFFER;
	circ_fifo_destruct(&test_cf);

	TEST_ASSERT_EQUAL_FLOAT_ARRAY(
		expected_buffer, test_buffer, TEST_CIRC_FIFO_TEST_BUFFER_SIZE);
}

void test_circ_fifo_should_override_last_two(void)
{
	int error = circ_fifo_construct(
		&test_cf, NULL, TEST_CIRC_FIFO_OVERFLOW_CAPACITY);
	TEST_ASSERT_EQUAL(0, error);

	for (uint8_t i = 0; i < TEST_CIRC_FIFO_TEST_BUFFER_SIZE; i++)
		circ_fifo_write(&test_cf, i);

	float test_buffer[TEST_CIRC_FIFO_TEST_BUFFER_SIZE];
	for (uint8_t i = 0; i < TEST_CIRC_FIFO_TEST_BUFFER_SIZE; i++)
		test_buffer[i] = circ_fifo_read(&test_cf);

	float expected_buffer[] = {5, 4, 3, 2, 5, 4};
	circ_fifo_destruct(&test_cf);

	TEST_ASSERT_EQUAL_FLOAT_ARRAY(
		expected_buffer, test_buffer, TEST_CIRC_FIFO_TEST_BUFFER_SIZE);
}

void test_circ_fifo_should_add_to_buffer(void)
{
	int error = circ_fifo_construct(
		&test_cf, NULL, TEST_CIRC_FIFO_PROPER_CAPACITY);
	TEST_ASSERT_EQUAL(0, error);

	for (uint8_t i = 0u; i < TEST_CIRC_FIFO_TEST_BUFFER_SIZE; i++)
		circ_fifo_write(&test_cf, i);

	circ_fifo_write(&test_cf, 6u);
	circ_fifo_write(&test_cf, 7u);

	float test_buffer[TEST_CIRC_FIFO_TEST_BUFFER_SIZE + 2u];
	for (uint8_t i = 0u; i < TEST_CIRC_FIFO_TEST_BUFFER_SIZE + 2u; i++)
		test_buffer[i] = circ_fifo_read(&test_cf);

	float expected_buffer[] = {7, 6, 5, 4, 3, 2, 1, 0};

	circ_fifo_destruct(&test_cf);

	TEST_ASSERT_EQUAL_FLOAT_ARRAY(
		expected_buffer, test_buffer, TEST_CIRC_FIFO_TEST_BUFFER_SIZE + 2u);
}
