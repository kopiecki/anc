/**
 * @file    common.h
 * @brief   <Provide some short description here>
 */

#ifndef COMMON_H_
#define COMMON_H_

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef CONCAT
#define CONCAT(x, y) x ## y
#endif

#endif /* COMMON_H_ */
