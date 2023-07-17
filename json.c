#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

#include "json.h"

#define advance(context, token_type) \
    do{\
    if(context->curtok.type != token_type) { \
        fprintf(stderr, "Expected token %s got %s at %c\n",\
                tok2str[token_type],\
                tok2str[context->curtok.type],\
                context->text[context->pos]);\
        exit(1);\
    }\
    context->curtok=next_token(context);\
    }while(0)

const char *tok2str[] = {
    "TOKEN_STRING",
    "TOKEN_NUMBER",
    "TOKEN_NULL",
    "TOKEN_FALSE",
    "TOKEN_TRUE",
    "TOKEN_ARRAY_START",
    "TOKEN_ARRAY_END",
    "TOKEN_OBJECT_START",
    "TOKEN_OBJECT_END",
    "TOKEN_COMMA",
    "TOKEN_COLUMN",
};

typedef struct {
    u32 pos;
    u32 len;
    char *text;
    __json_token_t curtok;
} json_context_t;

u32 parse_array(json_context_t *context, json_value_t *value);
u32 parse_object(json_context_t *context, json_value_t *value);
u32 parse_string(const json_context_t *context, __json_token_t *token);
u32 parse_value(json_context_t *context, json_value_t *value);
u32 parse_property(json_context_t *context, json_property_t *prop);

static inline
u32 skip_spaces(const json_context_t *context) {
    u32 pos = context->pos;
    const char *text = context->text;
    while (text[pos] == ' '
            || text[pos] == '\n'
            || text[pos] == '\t')
        pos++;
    return pos;
}

static inline
u8 is_number(const char token) {
    return (token <= '9' && token >= '0') || token == '-';
}

static inline
u32 parse_number(const json_context_t *context, __json_token_t *token) {
    u32 pos = context->pos;
    const char *text = context->text;
    double value = 0.0;

    u8 neg = text[pos] == '-';
    if (neg) {
        pos++;
    }

    while (is_number(text[pos])) {
        value *= 10;
        value += text[pos] - '0';
        pos++;
    }

    double decimal = 0.0;
    if (text[pos] == '.') {
        pos++;
        double decimal_factor = 1.0;

        while (is_number(text[pos])) {
            decimal *= 10.0;
            decimal += text[pos] - '0';
            decimal_factor *= 10.0;
            pos++;
        }

        decimal /= decimal_factor;
    }

    token->type = TOKEN_NUMBER;
    token->number = value + decimal;
    if (neg) {
        token->number *= -1.0;
    }
    return pos;
}

static inline
u32 parse_null(const json_context_t *context, __json_token_t *token) {
    u32 pos = context->pos;
    const char *text = context->text;
    assert(text[pos++] == 'n');
    assert(text[pos++] == 'u');
    assert(text[pos++] == 'l');
    assert(text[pos++] == 'l');
    token->type = TOKEN_NULL;
    return pos;
}

static inline
u32 parse_false(const json_context_t *context, __json_token_t *token) {
    u32 pos = context->pos;
    const char *text = context->text;
    assert(text[pos++] == 'f');
    assert(text[pos++] == 'a');
    assert(text[pos++] == 'l');
    assert(text[pos++] == 's');
    assert(text[pos++] == 'e');
    token->boolean = FALSE;
    token->type = TOKEN_FALSE;
    return pos;
}

static inline
u32 parse_true(const json_context_t *context, __json_token_t *token) {
    u32 pos = context->pos;
    const char *text = context->text;
    assert(text[pos++] == 't');
    assert(text[pos++] == 'r');
    assert(text[pos++] == 'u');
    assert(text[pos++] == 'e');
    token->boolean = TRUE;
    token->type = TOKEN_TRUE;
    return pos;
}

u32 parse_string(const json_context_t *context, __json_token_t *token) {
    u32 pos = context->pos;
    const char *text = context->text;
    assert(text[pos] == '"');
    pos++;

    u32 str_len = 0;
    while (text[pos] != '"') {
        if (text[pos] == '\\') {
            pos++;
        }
        pos++;
    }

    const u32 starting_pos = context->pos + 1;
    const u32 count = pos - starting_pos;
    token->str = (char *)malloc(count + 1);
    memset(token->str, 0, count + 1);
    memcpy(token->str, &(text[starting_pos]), count);
    token->type = TOKEN_STRING;

    pos++;

    return pos;
}

static
__json_token_t next_token(json_context_t *context) {
    const char *text = context->text;

    context->pos = skip_spaces(context);
    __json_token_t token = {0};
    if (text[context->pos] == JSON_OBJECT_START) { 
        token.type = TOKEN_OBJECT_START; context->pos++;
    } else if (text[context->pos] == JSON_OBJECT_END) {
        token.type = TOKEN_OBJECT_END; context->pos++;
    } else if (text[context->pos] == JSON_ARRAY_START) {
        token.type = TOKEN_ARRAY_START; context->pos++;
    } else if (text[context->pos] == JSON_ARRAY_END) {
        token.type = TOKEN_ARRAY_END; context->pos++;
    } else if (is_number(text[context->pos])) {
        context->pos = parse_number(context, &token);
    } else if (text[context->pos] == 'n') {
        context->pos = parse_null(context, &token);
    } else if (text[context->pos] == 't') {
        context->pos = parse_true(context, &token);
    } else if (text[context->pos] == 'f') {
        context->pos = parse_false(context, &token);
    } else if (text[context->pos] == '"') {
        context->pos = parse_string(context, &token);
    } else if (text[context->pos] == ',') {
        token.type = TOKEN_COMMA; context->pos++;
    } else if (text[context->pos] == ':') {
        token.type = TOKEN_COLUMN; context->pos++;
    }

    return token;
}

u32 parse_array(json_context_t *context, json_value_t *value) {
    advance(context, TOKEN_ARRAY_START);
    u32 values_size = sizeof(json_value_t) * 32;
    json_value_t *values = malloc(values_size);

    u32 i;
    for (i = 0; context->curtok.type != TOKEN_ARRAY_END; i++) {
        json_value_t parsed_value;

        u32 parse_err = parse_value(context, &parsed_value);
        if (parse_err) {
            return parse_err;
        }

        if (i == 32) {
            values_size += values_size / 2;
            values = realloc(values, values_size);
            if (values == NULL) {
                return JSON_ALLOC_FAILED_ERR;
            }
        }

        values[i] = parsed_value;

        if (context->curtok.type != TOKEN_COMMA) {
            break;
        }

        advance(context, TOKEN_COMMA);
    }

    value->array.__cap = values_size;
    value->array.len = i;
    value->array.values = values;

    advance(context, TOKEN_ARRAY_END);
    return NONE;
}

u32 parse_object(json_context_t *context, json_value_t *value) {
    advance(context, TOKEN_OBJECT_START);

    u32 props_size = sizeof(json_property_t) * 32;
    u32 keys_size = sizeof(char *) * 32;
    char **keys = (char **)malloc(keys_size);
    json_property_t *props = (json_property_t *)malloc(props_size);
    memset(props, 0, props_size);

    u32 i;
    for (i = 0;; i++) {
        json_property_t prop;

        u32 parse_err = parse_property(context, &prop);
        if (parse_err) {
            return parse_err;
        }

        if (i == 32) {
            props_size += props_size / 2;
            props = realloc(props, props_size);

            keys_size += keys_size / 2;
            keys = realloc(keys, keys_size);

            if (props == NULL || keys == NULL) {
                return JSON_ALLOC_FAILED_ERR;
            }
        }

        props[i] = prop;
        keys[i] = prop.key;

        if (context->curtok.type != TOKEN_COMMA) {
            break;
        }

        advance(context, TOKEN_COMMA);
    }

    value->object.__keys_cap = keys_size;
    value->object.__props_cap = props_size;
    value->object.len = i;
    value->object.keys = keys;
    value->object.props = props;

    advance(context, TOKEN_OBJECT_END);
    return NONE;
}

u32 parse_value(json_context_t *context, json_value_t *value) {
    __json_token_t token = context->curtok;

    switch (token.type) {
    case TOKEN_OBJECT_START:
        value->type = JSON_TYPE_OBJECT;
        return parse_object(context, value);
    case TOKEN_ARRAY_START:
        value->type = JSON_TYPE_ARRAY;
        return parse_array(context, value);
    case TOKEN_NUMBER:
        value->type = JSON_TYPE_NUMBER;
        value->number = token.number;
        advance(context, TOKEN_NUMBER);
        break;
    case TOKEN_STRING:
        value->type = JSON_TYPE_STRING;
        value->str = token.str;
        advance(context, TOKEN_STRING);
        break;
    case TOKEN_FALSE:
        value->type = JSON_TYPE_BOOL;
        value->boolean = token.boolean;
        advance(context, TOKEN_FALSE);
        break;
    case TOKEN_TRUE:
        value->type = JSON_TYPE_BOOL;
        value->boolean = token.boolean;
        advance(context, TOKEN_TRUE);
        break;
    case TOKEN_NULL:
        value->type = JSON_TYPE_NULL;
        advance(context, TOKEN_NULL);
        break;
    default:
        assert(FALSE && "unreachale!");
    }

    return NONE;
}

u32 parse_property(json_context_t *context, json_property_t *prop) {
    __json_token_t prop_name = context->curtok;
    advance(context, TOKEN_STRING);
    advance(context, TOKEN_COLUMN);
    json_value_t prop_value;

    u32 value_result = parse_value(context, &prop_value);

    prop->key = prop_name.str;
    prop->value = prop_value;

    return value_result;
}

u32 json_parse(json_value_t *value, const char *text, const u32 len) {
    assert(value != NULL);

    u32 pos = 0;
    __json_token_t token;

    json_context_t context = {0};
    context.len = len;
    context.text = (char *)text;
    context.curtok = next_token(&context);

    if (context.curtok.type == TOKEN_ARRAY_START) {
        u32 parse_err = parse_array(&context, value);
        value->type = JSON_TYPE_ARRAY;
        return parse_err;
    }

    u32 parse_err = parse_object(&context, value);
    value->type = JSON_TYPE_OBJECT;
    return parse_err;
}

json_value_type_t __json_value_type(const json_object_t object,
        const char *key) {
    for (u32 i=0;
            i<object.len;
            i++) {
        if (!strcmp(object.props[i].key, key)) {
            return object.props[i].value.type;
        }
    }

    return JSON_TYPE_NONE;
}

u32 json_parse_file(json_value_t *value, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL)
        return JSON_FOPEN_ERR;

    fseek(file, 0, SEEK_END);
    u32 file_size = ftell(file);
    rewind(file);

    const char *json_data = (const char *)malloc(sizeof(char) * file_size);

    u32 pos = 0;
    __json_token_t token;

    json_context_t context = {0};
    context.len = file_size;
    context.text = (char *)json_data;
    context.curtok = next_token(&context);

    u32 parse_err = parse_object(&context, value);
    value->type = JSON_TYPE_OBJECT;
    return parse_err;
}

void * __json_object_get_raw(json_object_t object,
        const char **keys, const u32 len) {

    json_value_t *sub_value = NULL;

    for (u32 k=0;
            k<len;
            k++
        ) {
        const char *key = keys[k];
        if (sub_value != NULL) {
            assert(sub_value->type == JSON_TYPE_OBJECT);
            object = sub_value->object;
        }

        for (u32 i=0;
                i<object.len;
                i++) {
            if (!strcmp(object.props[i].key, key)) {
                json_value_t *v = &(object.props[i].value);
                switch (v->type) {
                    case JSON_TYPE_BOOL:
                        return &(v->boolean);
                    case JSON_TYPE_STRING:
                        return &(v->str);
                    case JSON_TYPE_NUMBER:
                        return &(v->number);
                    default:
                        sub_value = v;
                        break;
                }
            }
        }
    }

    return sub_value;
}

#define __JSON_VALUE_ON_TYPE(__VALUE, __VALUE_PTR, __TYPE)\
    do {switch (__TYPE) {\
        case JSON_TYPE_NUMBER:\
            *(__VALUE_PTR) = &(__VALUE.number);\
            break;\
        case JSON_TYPE_STRING:\
            *(__VALUE_PTR) = &(__VALUE.str);\
            break;\
        case JSON_TYPE_OBJECT:\
            *(__VALUE_PTR) = &(__VALUE.object);\
            break;\
        case JSON_TYPE_ARRAY:\
            *(__VALUE_PTR) = &(__VALUE.array);\
            break;\
        case JSON_TYPE_BOOL:\
            *(__VALUE_PTR) = &(__VALUE.boolean);\
            break;\
        default:\
            break;\
    }}while(0)


void * __json_object_set_in_place(const json_object_t object, const char *key,
        const json_value_type_t type) {
    for (u32 i=0;
            i<object.len;
            i++) {
        if (!strcmp(object.props[i].key, key)) {
            object.props[i].value.type = type;
            void *value_ptr = NULL;
            __JSON_VALUE_ON_TYPE(object.props[i].value, &value_ptr, type);
            return value_ptr;
        }
    }
    return NULL;
}

void * __json_object_add(json_object_t *object, const char *key,
        const json_value_type_t type) {
    // Check for any room left in mem
    if ((object->len + 1) * sizeof(json_property_t) >= object->__props_cap) {
        object->__props_cap += object->__props_cap / 2;
        object->props = realloc(object->props, object->__props_cap);
        assert(object->props != NULL); // FIXME: return error to the caller
    }
    if ((object->len + 1) * sizeof(char) >= object->__keys_cap) {
        object->__keys_cap += object->__keys_cap / 2;
        object->keys = realloc(object->keys, object->__keys_cap);
        assert(object->keys != NULL); // FIXME: return error to the caller
    }

    const u32 len = object->len;
    object->len++;
    const u32 keylen = strlen(key);
    object->keys[len] = (char *)malloc(keylen + 1);
    strcpy(object->keys[len], key);
    object->props[len].key = (char *)malloc(keylen + 1);
    strcpy(object->props[len].key, key);

    object->props[len].value.type = type;
    void *value_ptr = NULL;
    __JSON_VALUE_ON_TYPE(object->props[len].value, &value_ptr, type);
    return value_ptr;
}

void * __json_set(json_value_t *value, const char *key,
        const json_value_type_t type) {
    if (JSON_EXISTS((*value), (char *)key)) {
        // FIXME: mem leak! overridden objects are not getitng free'd
        return __json_object_set_in_place(value->object, key, type);
    }

    return __json_object_add(&(value->object), key, type);
}

json_value_t __json_wrap_object_value(const json_value_t value) {
    assert(value.type == JSON_TYPE_OBJECT);
    json_value_t new_value = value;

    const u32 props_size
        = sizeof(json_property_t) * new_value.object.len;
    const u32 keys_size
        = sizeof(char *) * new_value.object.len;
    new_value.object.keys =
        (char **)malloc(keys_size);

    for (u32 i=0;
            i<new_value.object.len;
            i++) {
        const u32 keylen = strlen(new_value.object.props[i].key);
        new_value.object.keys[i] = (char *)malloc(keylen);
        strcpy(new_value.object.keys[i], new_value.object.props[i].key);
    }

    new_value.object.__keys_cap = keys_size;
    new_value.object.__props_cap = props_size;

    return new_value;
}
