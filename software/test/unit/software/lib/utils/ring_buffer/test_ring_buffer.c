/**
 * @file    test_ring_buffer.c
 * @brief   <Provide some short description here>
 */

#include <string.h>
#include <unity.h>

#include "logger/logger.h"
#include "utils/ring_buffer.h"

#define TEST_RING_BUFFER_CAPACITY    16u
#define TEST_RING_BUFFER_TEST_SAMPLE 0xABCDu
#define TEST_RING_BUFFER_BURST_SIZE  8u
#define TEST_RING_BUFFER_BURST                                                 \
        {0x0001u, 0x0002u, 0x0003u, 0x0004u, 0x0005u, 0x0006u, 0x0007u, 0x0008u};


static ring_buffer_t* rb;

void setUp(void)
{
        rb = NULL;
}

void test_ring_buffer_should_construct(void)
{
        rb = ring_buffer_construct(TEST_RING_BUFFER_CAPACITY);
        TEST_ASSERT_NOT_EQUAL(rb, NULL);
}

void test_ring_buffer_should_destruct(void)
{
        test_ring_buffer_should_construct();
        ring_buffer_destruct(rb);
}

void test_ring_buffer_should_write_one_sample(void)
{
        test_ring_buffer_should_construct();

        uint16_t const test_sample = TEST_RING_BUFFER_TEST_SAMPLE;
        ring_buffer_write(rb, test_sample);
}

void test_ring_buffer_should_read_written_sample(void)
{
        test_ring_buffer_should_write_one_sample();

        uint16_t test_sample = 0u;
        int error            = ring_buffer_read(rb, &test_sample);

        TEST_ASSERT_EQUAL(error, 0);
        TEST_ASSERT_EQUAL(test_sample, TEST_RING_BUFFER_TEST_SAMPLE);
}

void test_ring_buffer_should_write_burst(void)
{
        test_ring_buffer_should_construct();

        uint16_t test_samples[] = TEST_RING_BUFFER_BURST;
        ring_buffer_write_burst(rb, test_samples, TEST_RING_BUFFER_BURST_SIZE);
}

void test_ring_buffer_should_read_written_burst(void)
{
        test_ring_buffer_should_write_burst();

        uint16_t expected_samples[] = TEST_RING_BUFFER_BURST;
        uint16_t test_samples[TEST_RING_BUFFER_BURST_SIZE] = {0u};
        int error = ring_buffer_read_burst(
                rb, test_samples, TEST_RING_BUFFER_BURST_SIZE);

        TEST_ASSERT_EQUAL_UINT16_ARRAY(
                expected_samples, test_samples, TEST_RING_BUFFER_BURST_SIZE);
}

void test_ring_buffer_should_read_written_burst_directly(void)
{
        test_ring_buffer_should_write_burst();

        uint16_t expected_samples[] = TEST_RING_BUFFER_BURST;
        uint16_t test_samples[TEST_RING_BUFFER_BURST_SIZE] = {0u};

        uint16_t* direct_data_ptr =
                ring_buffer_read_burst_directly(rb, TEST_RING_BUFFER_BURST_SIZE);

        TEST_ASSERT_NOT_EQUAL(NULL, direct_data_ptr);
        TEST_ASSERT_EQUAL_UINT16_ARRAY(
                expected_samples, direct_data_ptr, TEST_RING_BUFFER_BURST_SIZE);
}

void test_ring_buffer_should_override_old_data(void)
{
        test_ring_buffer_should_construct();

        uint16_t written_samples[TEST_RING_BUFFER_CAPACITY + 4u] = {0u};
        for (uint8_t i = 0u; i < TEST_RING_BUFFER_CAPACITY + 4u; i++)
                written_samples[i] = i;

        ring_buffer_write_burst(
                rb, written_samples, TEST_RING_BUFFER_CAPACITY + 4u);

        uint16_t* const expected_samples = &written_samples[5u];

        uint16_t test_samples[TEST_RING_BUFFER_CAPACITY - 1] = {0u};
        int error = ring_buffer_read_burst(
                rb, test_samples, TEST_RING_BUFFER_CAPACITY - 1);

        TEST_ASSERT_EQUAL(0, error);
        TEST_ASSERT_EQUAL_UINT16_ARRAY(
                expected_samples, test_samples, TEST_RING_BUFFER_CAPACITY - 1);
}
