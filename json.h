#ifndef JSON
#define JSON

#include <stdint.h>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define NONE 0
#define TRUE 1
#define FALSE 0
#define BOOL u8

#define JSON_ARRAY_START    '['
#define JSON_ARRAY_END      ']'
#define JSON_OBJECT_START   '{'
#define JSON_OBJECT_END     '}'
#define JSON_COMMA          ','
#define JSON_COLUMN         ':'

#define JSON_LEN_MISMATCH_ERR 0x1
#define JSON_ALLOC_FAILED_ERR 0x2
#define JSON_FOPEN_ERR        0x3

#define JSON_GET(__VALUE, __TYPE, ...) \
    (assert(__VALUE.type == JSON_TYPE_OBJECT), \
     *(__TYPE *)__json_object_get_raw(__VALUE.object,\
         (const char **)((char *[]){ __VA_ARGS__ }),\
         sizeof((char *[]){ __VA_ARGS__ })/sizeof(char *)))

#define JSON_IGET(__VALUE, __TYPE, IDX) \
    (assert(__VALUE.type == JSON_TYPE_ARRAY), \
     *(__TYPE *)__json_array_get_raw(__VALUE.array, IDX) \
     )

#define JSON_ARRAY_LEN(__VALUE) \
    (assert(__VALUE.type == JSON_TYPE_ARRAY), \
        __VALUE.array.len)

#define JSON_EXISTS(__VALUE, ...) \
    (assert(__VALUE.type == JSON_TYPE_OBJECT), \
     __json_object_get_raw(__VALUE.object,\
         (const char **)((char *[]){ __VA_ARGS__ }),\
         sizeof((char *[]){ __VA_ARGS__ })/sizeof(char *))) != NULL

#define JSON_TYPE(__VALUE, __KEY) \
    (assert(__VALUE.type == JSON_TYPE_OBJECT), \
     __json_value_type(__VALUE.object, __KEY))

#define JSON_SET(__VALUE, __KEY, __TYPE, TYPE, VALUE) \
    do{\
        assert(__VALUE.type == JSON_TYPE_OBJECT);\
        TYPE *value_ptr = \
            (TYPE *)__json_set(&(__VALUE), __KEY, __TYPE);\
        *value_ptr = VALUE;\
    }while(0)

#define JSON_NUMBER(VALUE)\
    ((json_value_t) {\
     JSON_TYPE_NUMBER,\
     .number = (double)(VALUE),\
    })

#define JSON_STRING(VALUE)\
    ((json_value_t) {\
     JSON_TYPE_STRING,\
     .str = (char *)(VALUE),\
    })

#define JSON_BOOL(VALUE)\
    ((json_value_t) {\
     JSON_TYPE_BOOL,\
     .boolean = (BOOL)(VALUE),\
    })

#define JSON_NULL\
    ((json_value_t) {\
     JSON_TYPE_NULL,\
     0,\
    })

#define JSON_PROP(KEY, VALUE)\
    ((json_property_t) {\
     (char *)KEY,\
     (VALUE),\
     }\
    )

#define JSON_OBJECT(...)\
    __json_wrap_object_value((json_value_t) {\
     JSON_TYPE_OBJECT,\
     .object = (json_object_t){\
        .__props_cap = (u32)0,\
        .__keys_cap = (u32)0,\
        .keys = (char **)NULL,\
        .len = (u32)sizeof((json_property_t[]){ __VA_ARGS__ })/sizeof(json_property_t),\
        .props = (json_property_t *)((json_property_t[]){ __VA_ARGS__ }),\
     },\
    })

typedef enum {
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_NULL,
    TOKEN_FALSE,
    TOKEN_TRUE,
    TOKEN_ARRAY_START,
    TOKEN_ARRAY_END,
    TOKEN_OBJECT_START,
    TOKEN_OBJECT_END,
    TOKEN_COMMA,
    TOKEN_COLUMN,
} __json_token_type_t;

typedef enum {
    JSON_TYPE_NUMBER,
    JSON_TYPE_STRING,
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_NULL,
    JSON_TYPE_BOOL,
    JSON_TYPE_NONE,
} json_value_type_t;

typedef struct {
    __json_token_type_t type;
    union {
    char* str;
    double number;
    u8 boolean;
    };
} __json_token_t;

struct __json_value_t;
struct __json_array_t;
struct __json_object_t;
struct __json_property_t;

typedef struct __json_array_t {
    u32 __cap;
    struct __json_value_t *values;
    u32 len;
} json_array_t;

typedef struct __json_object_t {
    u32 len;
    char **keys;
    u32 __keys_cap;
    u32 __props_cap;
    struct __json_property_t *props;
} json_object_t;

typedef struct __json_value_t { 
    json_value_type_t type;
    union {
        double number;
        u8 boolean;
        char *str;
        struct __json_array_t array;
        struct __json_object_t object;
    };
} json_value_t;

typedef struct __json_property_t {
    char *key;
    struct __json_value_t value;
} json_property_t;

u32 json_parse(json_value_t *value, const char *text, const u32 len);

void * __json_object_get_raw(const json_object_t object,
        const char **keys, const u32 len);

json_value_type_t __json_value_type(const json_object_t value,
        const char *key);

void * __json_set(json_value_t *value, const char *key,
        const json_value_type_t type);

json_value_t __json_wrap_object_value(const json_value_t value);

void * __json_array_get_raw(const json_array_t array, const u32 idx);

#endif
