#include "/home/dkop/anc/software/software/lib/utils/inc/utils/ring_buffer.h"
#include "/home/dkop/anc/software/software/lib/logger/inc/logger/logger.h"
#include "/var/lib/gems/3.0.0/gems/ceedling-0.31.1/vendor/unity/src/unity.h"












static ring_buffer_t* rb;



void setUp(void)

{

        rb = 

            ((void *)0)

                ;

}



void test_ring_buffer_should_construct(void)

{

        rb = ring_buffer_construct(16u);

        do {if (((rb) != (

       ((void *)0)

       ))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(29)));}} while(0);

}



void test_ring_buffer_should_destruct(void)

{

        test_ring_buffer_should_construct();

        ring_buffer_destruct(rb);

}



void test_ring_buffer_should_write_one_sample(void)

{

        test_ring_buffer_should_construct();



        uint16_t const test_sample = 0xABCDu;

        ring_buffer_write(rb, test_sample);

}



void test_ring_buffer_should_read_written_sample(void)

{

        test_ring_buffer_should_write_one_sample();



        uint16_t test_sample = 0u;

        int error = ring_buffer_read(rb, &test_sample);



        UnityAssertEqualNumber((UNITY_INT)((error)), (UNITY_INT)((0)), (

       ((void *)0)

       ), (UNITY_UINT)(53), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((test_sample)), (UNITY_INT)((0xABCDu)), (

       ((void *)0)

       ), (UNITY_UINT)(54), UNITY_DISPLAY_STYLE_INT);

}



void test_ring_buffer_should_write_burst(void)

{

        test_ring_buffer_should_construct();



        uint16_t test_samples[] = {0x0001u, 0x0002u, 0x0003u, 0x0004u, 0x0005u, 0x0006u, 0x0007u, 0x0008u};;

        ring_buffer_write_burst(rb, test_samples, 8u);

}



void test_ring_buffer_should_read_written_burst(void)

{

        test_ring_buffer_should_write_burst();



        uint16_t expected_samples[] = {0x0001u, 0x0002u, 0x0003u, 0x0004u, 0x0005u, 0x0006u, 0x0007u, 0x0008u};;

        uint16_t test_samples[8u] = {0u};

        int error = ring_buffer_read_burst(

                rb, test_samples, 8u);



        UnityAssertEqualIntArray(( const void*)((expected_samples)), ( const void*)((test_samples)), (UNITY_UINT32)((8u)), (

       ((void *)0)

       ), (UNITY_UINT)(74), UNITY_DISPLAY_STYLE_UINT16, UNITY_ARRAY_TO_ARRAY)

                                                                            ;

}



void test_ring_buffer_should_read_written_burst_directly(void)

{

        test_ring_buffer_should_write_burst();



        uint16_t expected_samples[] = {0x0001u, 0x0002u, 0x0003u, 0x0004u, 0x0005u, 0x0006u, 0x0007u, 0x0008u};;

        uint16_t test_samples[8u] = {0u};



        uint16_t* direct_data_ptr =

                ring_buffer_read_burst_directly(rb, 8u);



        do {if (((

       ((void *)0)

       ) != (direct_data_ptr))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(88)));}} while(0);

        UnityAssertEqualIntArray(( const void*)((expected_samples)), ( const void*)((direct_data_ptr)), (UNITY_UINT32)((8u)), (

       ((void *)0)

       ), (UNITY_UINT)(89), UNITY_DISPLAY_STYLE_UINT16, UNITY_ARRAY_TO_ARRAY)

                                                                               ;

}



void test_ring_buffer_should_override_old_data(void)

{

        test_ring_buffer_should_construct();



        uint16_t written_samples[16u + 4u] = {0u};

        for (uint8_t i = 0u; i < 16u + 4u; i++)

                written_samples[i] = i;



        ring_buffer_write_burst(

                rb, written_samples, 16u + 4u);



        uint16_t* const expected_samples = &written_samples[5u];



        uint16_t test_samples[16u - 1] = {0u};

        int error = ring_buffer_read_burst(

                rb, test_samples, 16u - 1);



        UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((error)), (

       ((void *)0)

       ), (UNITY_UINT)(110), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualIntArray(( const void*)((expected_samples)), ( const void*)((test_samples)), (UNITY_UINT32)((16u - 1)), (

       ((void *)0)

       ), (UNITY_UINT)(111), UNITY_DISPLAY_STYLE_UINT16, UNITY_ARRAY_TO_ARRAY)

                                                                              ;

}
