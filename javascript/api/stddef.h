//
// Created by cout970 on 2016-10-28.
//

#ifndef COMPUTER_STDDEF_H
#define COMPUTER_STDDEF_H

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef ptrdiff_t
typedef int ptrdiff_t;
#endif

#ifndef size_t
typedef unsigned int size_t;
#endif

#ifndef int32_t
typedef int int32_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef int16_t
typedef short int16_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef int8_t
typedef char int8_t;
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#define UNSUPORTED_TYPE int

// placeholders
#define uint64_t UNSUPORTED_TYPE
#define int64_t UNSUPORTED_TYPE

#define uint_least64_t UNSUPORTED_TYPE
#define int_least64_t UNSUPORTED_TYPE

#define uint_fast64_t UNSUPORTED_TYPE
#define int_fast64_t UNSUPORTED_TYPE

typedef uint8_t uint_least8_t;
typedef int8_t int_least8_t;
typedef uint16_t uint_least16_t;
typedef int16_t int_least16_t;
typedef uint32_t uint_least32_t;
typedef int32_t int_least32_t;

typedef uint8_t uint_fast8_t;
typedef int8_t int_fast8_t;
typedef uint16_t uint_fast16_t;
typedef int16_t int_fast16_t;
typedef uint32_t uint_fast32_t;
typedef int32_t int_fast32_t;

typedef uint32_t uintptr_t;
typedef int32_t intptr_t;
typedef uint32_t uintmax_t;
typedef int32_t intmax_t;

#endif //COMPUTER_STDDEF_H
