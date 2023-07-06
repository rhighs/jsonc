#ifndef JSON
#define JSON

#include <stdint.h>

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

#define JSON_GET(VALUE, KEY, TYPE) \
    (assert(VALUE.type == JSON_TYPE_OBJECT), \
     *(TYPE *)__json_object_get_raw(VALUE.object, KEY))

#define JSON_EXISTS(VALUE, KEY) \
    (assert(VALUE.type == JSON_TYPE_OBJECT), \
     __json_object_get_raw(VALUE.object, KEY) != NULL)

#define JSON_TYPE(VALUE, KEY) \
    (assert(VALUE.type == JSON_TYPE_OBJECT), \
     __json_value_type(VALUE.object, KEY))

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
    struct __json_value_t *values;
    u32 len;
} json_array_t;

typedef struct __json_object_t {
    u32 len;
    char **keys;
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

void * __json_object_get_raw(const json_object_t object, const char *key);

json_value_type_t __json_value_type(const json_object_t value,
        const char *key);
#endif
