#ifndef RUBI_INTERNALS_H
#define RUBI_INTERNALS_H

#include "rubi.h"
#include "rubi_autodefs.h"

#include <inttypes.h>
#include <string.h>

typedef struct rubi_field_desc_t
{
    const char*    name;
    const uint8_t  size;
    const uint8_t  access;
    const uint8_t  type;
    const char*    subnames;
    const uint8_t* relayfield;
} rubi_field_desc_t;

typedef struct rubi_func_desc_t
{
    const char*    name;
    const uint8_t  args_c;
    const uint8_t  in_type, out_type;
    const char*    argnames;
    const uint8_t* relayfield;
} rubi_func_desc_t;

typedef struct rubi_ffdescriptor_t
{
    const uint32_t fftype;
    union {
        const struct rubi_field_desc_t field_descriptor;
        const struct rubi_func_desc_t  func_descriptor;
    } descriptor;
} rubi_ffdescriptor_t;

/*
    Exorcizamus te, omnis immundus spiritus,
    omnis satanica potestas, omnis incursio infernalis adversarii,
    omnis legio, omnis congregatio et secta diabolica.
*/

#define __RUBI_ENUM_MAGIC 1 * 6
#define __RUBI_FUNC_MAGIC 3 * 6
#define __RUBI_FIELD_MAGIC 4 * 6
#define __RUBI_END_MAGIC ((666 << 16) + 666)

#define __ARGS_CHAIN(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, N, ...) N
#define __ARGS_COUNT(...)                                                      \
    __ARGS_CHAIN(__SHIFT, ##__VA_ARGS__, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define __ARGS_COUNT_NOSHIFT(...)                                              \
    __ARGS_CHAIN(__VA_ARGS__, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define __ARGS_COUNT_ZERO(...)                                                 \
    __ARGS_CHAIN(__SHIFT, ##__VA_ARGS__, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1)

#define __SUPER_CONCAT_(X, Y) X##Y
#define __SUPER_CONCAT(X, Y) __SUPER_CONCAT_(X, Y)
#define __HYPER_CONCAT(X, Y, Z) __SUPER_CONCAT(__SUPER_CONCAT(X, Y), Z)

// no-const will make the compiler place this outside the config block

#define __RUBI_MAGIC_FDECL_1(name, type, ...) type name;

#define __RUBI_MAGIC_FDECL_0(name, type, ...)                                  \
    __HYPER_CONCAT(struct _rubi_, __LINE__, _field_struct)                     \
    {                                                                          \
        type __VA_ARGS__;                                                      \
    }                                                                          \
    name;                                                                      \
    type* __SUPER_CONCAT(name, _fields_ptr) = (type*)&__UNWRAP(name);

#define __RUBI_SIZE(type, ...) (sizeof(type) * __ARGS_COUNT(__VA_ARGS__))

#define __UNWRAP_(x) x
#define __UNWRAP(x) __UNWRAP_(x)

#define RUBI_REGISTER_FIELD(NAME, ACCESS, TYPE, ...)                           \
    __SUPER_CONCAT(__RUBI_MAGIC_FDECL_, __ARGS_COUNT_ZERO(__VA_ARGS__))        \
    (NAME, TYPE, __VA_ARGS__);                                                 \
                                                                               \
    rubi_ffdescriptor_t __SUPER_CONCAT(__ffdescriptor, __LINE__) = {           \
        .fftype     = __RUBI_FIELD_MAGIC,                                      \
        .descriptor = {                                                        \
            .field_descriptor = {.name = __UNWRAP_(#NAME),                     \
                                 .size = __ARGS_COUNT_NOSHIFT(__VA_ARGS__) *   \
                                         sizeof(TYPE),                         \
                                 .access     = __UNWRAP(ACCESS),               \
                                 .type       = _RUBI_TYPECODES_##TYPE,         \
                                 .subnames   = __UNWRAP_(#__VA_ARGS__),        \
                                 .relayfield = (uint8_t*)&NAME}}};             \
                                                                               \
    struct rubi_ffdescriptor_t* __SUPER_CONCAT(                                \
        __rubi_get_ffdesc, __COUNTER__)()                                      \
    {                                                                          \
        return &__SUPER_CONCAT(__ffdescriptor, __LINE__);                      \
    }

#define RUBI_REGISTER_FUNCTION(outtype, name, intype, ...)                     \
    const uint8_t __SUPER_CONCAT(__rubi_func_magic, __LINE__) =                \
        __RUBI_FUNC_MAGIC;                                                     \
    __UNWRAP(outtype)                                                          \
    name(__SUPER_CONCAT(__RUBI_REPEAT, __ARGS_COUNT(__VA_ARGS__))(             \
        __UNWRAP(intype)));                                                    \
    struct __SUPER_CONCAT(                                                     \
        rubi_func_desc_t _rubi_functiondesc_, __COUNTER__) = {                 \
        #name, __ARGS_COUNT(__VA_ARGS__),                                      \
        __SUPER_CONCAT(_RUBI_TYPECODES_, intype),                              \
        __SUPER_CONCAT(_RUBI_TYPECODES_, outtype), #__VA_ARGS__};              \
                                                                               \
    __HYPER_CONCAT(                                                            \
        __RUBI_FUNC, __SUPER_CONCAT(_RUBI_IS_VOID_, outtype)(_NORETURN),       \
        __SUPER_CONCAT(_RUBI_IS_VOID_, intype)(_NOARG))                        \
    (outtype, name, intype, __VA_ARGS__)                                       \
                                                                               \
        void (*__SUPER_CONCAT(_rubi_funcptr_, __LINE__))() =                   \
            __SUPER_CONCAT(_rubi_relayfunc_, __LINE__);

#define __RUBI_FUNC(outtype, name, intype, ...)                                \
    uint8_t __SUPER_CONCAT(                                                    \
        _rubi_relayfield_,                                                     \
        __LINE__)[sizeof(outtype) + __RUBI_SIZE(intype, __VA_ARGS__)];         \
                                                                               \
    void __SUPER_CONCAT(_rubi_relayfunc_, __LINE__)()                          \
    {                                                                          \
        outtype ret;                                                           \
        intype  __VA_ARGS__;                                                   \
        memcpy(                                                                \
            __SUPER_CONCAT(_rubi_relayfield_, __LINE__),                       \
            ((uint8_t*)&ret) + sizeof(outtype),                                \
            __RUBI_SIZE(intype, __VA_ARGS__));                                 \
        ret = name(__VA_ARGS__);                                               \
        memcpy(                                                                \
            &ret,                                                              \
            __SUPER_CONCAT(_rubi_relayfield_, __LINE__) +                      \
                __RUBI_SIZE(intype, __VA_ARGS__),                              \
            sizeof(outtype));                                                  \
    }

#define __RUBI_FUNC_NOARG(outtype, name, intype, ...)                          \
    uint8_t __SUPER_CONCAT(_rubi_relayfield_, __LINE__)[sizeof(outtype)];      \
                                                                               \
    void __SUPER_CONCAT(_rubi_relayfunc_, __LINE__)()                          \
    {                                                                          \
        outtype ret = name();                                                  \
        memcpy(                                                                \
            &ret, __SUPER_CONCAT(_rubi_relayfield_, __LINE__),                 \
            sizeof(outtype));                                                  \
    }

#define __RUBI_FUNC_NORETURN(outtype, name, intype, ...)                       \
    uint8_t __SUPER_CONCAT(                                                    \
        _rubi_relayfield_, __LINE__)[__RUBI_SIZE(intype, __VA_ARGS__)];        \
                                                                               \
    void __SUPER_CONCAT(_rubi_relayfunc_, __LINE__)()                          \
    {                                                                          \
        uint8_t ugly_marker;                                                   \
        intype  __VA_ARGS__;                                                   \
        memcpy(                                                                \
            __SUPER_CONCAT(_rubi_relayfield_, __LINE__),                       \
            ((uint8_t*)&ugly_marker) + 1, __RUBI_SIZE(intype, __VA_ARGS__));   \
        name(__VA_ARGS__);                                                     \
    }

#define __RUBI_FUNC_NORETURN_NOARG(outtype, name, intype, ...)                 \
    void __SUPER_CONCAT(_rubi_relayfunc_, __LINE__)()                          \
    {                                                                          \
        name();                                                                \
    }

#define __RUBI_ENUM(typename, ...)                                             \
    uint8_t __SUPER_CONCAT(__rubi_enum_magic, __LINE__) = __RUBI_ENUM_MAGIC;   \
    enum __SUPER_CONCAT(__, typename)                                          \
    {                                                                          \
        __VA_ARGS__                                                            \
    };                                                                         \
    uint8_t __HYPER_CONCAT(__, typename, __LINE__) =                           \
        __SUPER_CONCAT(_RUBI_TYPECODES_, typename);                            \
    const char __SUPER_CONCAT(_rubi_enum_, __LINE__)[] = #__VA_ARGS__;

#define RUBI_UNIVERSAL_BOARD(board_name, version, driver, desc, update_hz)     \
    const char     __rubi_board_name[]    = board_name;                        \
    const char     __rubi_board_version[] = version;                           \
    const char     __rubi_board_driver[]  = driver;                            \
    const char     __rubi_board_desc[]    = desc;                              \
    const uint16_t __rubi_update_hz       = update_hz;

#define RUBI_END()                                                             \
    const uint32_t __rubi_magic_end  = __RUBI_END_MAGIC;                       \
    const uint32_t __rubi_decl_count = __COUNTER__;                            \
    uint32_t       __rubi_crctable[__COUNTER__ - 1];                           \
    uint32_t*      __rubi_desctable[__COUNTER__ - 2];                          \
    int8_t         __rubi_typetable[__COUNTER__ - 3];

void rubi_go_online();
void rubi_fire_assert(int32_t line, const char* file);

#endif
