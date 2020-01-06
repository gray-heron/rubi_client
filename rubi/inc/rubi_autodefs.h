#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "rubi_auxiliary.h"

#define RUBI_READONLY 1
#define RUBI_WRITEONLY 2
#define RUBI_READWRITE 3

#define _RUBI_TYPECODES_void 1
#define _RUBI_TYPECODES_int32_t 2
#define _RUBI_TYPECODES_int16_t 3
#define _RUBI_TYPECODES_int8_t 4
#define _RUBI_TYPECODES_uint32_t 5
#define _RUBI_TYPECODES_uint16_t 6
#define _RUBI_TYPECODES_uint8_t 7
#define _RUBI_TYPECODES_float 8
#define _RUBI_TYPECODES_shortstring 9
#define _RUBI_TYPECODES_longstring 10
#define _RUBI_TYPECODES_bool_t 11
#define _RUBI_TYPECODES_RUBI_ENUM1 12

#define _RUBI_IS_VOID_void(is) is
#define _RUBI_IS_VOID_int32_t(is)
#define _RUBI_IS_VOID_int18_t(is)
#define _RUBI_IS_VOID_int8_t(is)
#define _RUBI_IS_VOID_uint32_t(is)
#define _RUBI_IS_VOID_uint16_t(is)
#define _RUBI_IS_VOID_uint8_t(is)
#define _RUBI_IS_VOID_float(is)
#define _RUBI_IS_VOID_shortstring(is)
#define _RUBI_IS_VOID_longstring(is)
#define _RUBI_IS_VOID_RUBI_ENUM1(is)

#define __RUBI_REPEAT0(X)
#define __RUBI_REPEAT1(X) X
#define __RUBI_REPEAT2(X) __RUBI_REPEAT1(X), X
#define __RUBI_REPEAT3(X) __RUBI_REPEAT2(X), X
#define __RUBI_REPEAT4(X) __RUBI_REPEAT3(X), X
#define __RUBI_REPEAT5(X) __RUBI_REPEAT4(X), X
#define __RUBI_REPEAT6(X) __RUBI_REPEAT5(X), X
#define __RUBI_REPEAT7(X) __RUBI_REPEAT6(X), X
#define __RUBI_REPEAT8(X) __RUBI_REPEAT7(X), X
#define __RUBI_REPEAT9(X) __RUBI_REPEAT8(X), X
#define __RUBI_REPEAT10(X) __RUBI_REPEAT9(X), X

#define RUBI_STATE_IDLE (1 << 0)
#define RUBI_STATE_INITING (1 << 1)
#define RUBI_STATE_OPERATIONAL (1 << 2)
#define RUBI_STATE_HOLD (1 << 3)
#define RUBI_STATE_ERROR (1 << 4)

#define RUBI_SHORTSTRING_MAX_LEN 32
#define RUBI_LONGSTRING_MAX_LEN 256

typedef uint8_t bool_t;

typedef char shortstring[RUBI_SHORTSTRING_MAX_LEN];
typedef char longstring[RUBI_LONGSTRING_MAX_LEN];

// typedef uint8_t shortstring;

struct rubi_ffdescriptor_t;

struct rubi_ffdescriptor_t* __rubi_get_ffdesc0();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc1();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc2();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc3();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc4();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc5();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc6();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc7();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc8();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc9();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc10();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc11();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc12();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc13();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc14();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc15();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc16();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc17();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc18();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc19();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc20();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc21();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc22();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc23();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc24();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc25();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc26();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc27();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc28();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc29();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc30();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc31();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc32();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc33();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc34();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc35();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc36();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc37();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc38();
struct rubi_ffdescriptor_t* __rubi_get_ffdesc39();

extern struct rubi_ffdescriptor_t* (*__rubi_fieldescfunction_getters[])();

uint8_t rubi_type_size(uint8_t typecode);
