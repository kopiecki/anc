/**
 * @file    common.h
 * @brief   Common macros.
 */

#ifndef COMMON_H_
#define COMMON_H_

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef CONCAT
#define CONCAT(x, y) x##y
#endif

#ifndef IS_POW_2
#define IS_POW_2(v) (v && !(v & (v - 1)))
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(p) (void)(p)
#endif

#endif /* COMMON_H_ */
