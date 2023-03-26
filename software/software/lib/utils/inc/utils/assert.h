/**
 * @file    assert.h
 * @brief   <Provide some short description here>
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#if (DEBUG == 1)
#define ASSERT(_x)                                                             \
        do                                                                     \
        {                                                                      \
                if (!(_x))                                                     \
                {                                                              \
                        LOG_ERROR("Assert failed!");                           \
                        assert(_x);                                            \
                }                                                              \
        } while (0)
#else
#define ASSERT(_x) (void)(_x)
#endif

#define STATIC_ASSERT(_x) static_assert(_x)

#endif /* ASSERT_H_ */
