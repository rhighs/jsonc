#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <memory.h>

#include "json.h"

#define advance(context, token_type) \
    do{assert(context->curtok.type==token_type); context->curtok=next_token(context);}while(0)

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
    return token <= '9' && token >= '0';
}

static inline
u32 parse_number(const json_context_t *context, __json_token_t *token) {
    u32 pos = context->pos;
    const char *text = context->text;
    double value = 0.0;
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
            pos++; // skip escaped as it could be \"
        }
        pos++;
    }

    const u32 count = pos - context->pos;
    token->str = (char *)malloc(count + 1);
    memcpy(token->str, &(text[context->pos]), count);
    token->str[count] = '\0';

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
    for (i = 0; context->curtok.type != TOKEN_OBJECT_END; i++) {
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

    u32 parse_err = parse_object(&context, value);
    value->type = JSON_TYPE_OBJECT;
    return parse_err;
}

void * __json_object_get_raw(const json_object_t object, const char *key) {
    for (u32 i=0;
         i<object.len;
         i++) {
        if (strcmp(object.props[i].key, key)) {
            json_value_t *v = &(object.props[i].value);
            switch (v->type) {
                case JSON_TYPE_BOOL:
                    return &(v->boolean);
                case JSON_TYPE_STRING:
                    return &(v->str);
                case JSON_TYPE_NUMBER:
                    return &(v->number);
                case JSON_TYPE_ARRAY:
                    return &(v->array);
                case JSON_TYPE_OBJECT:
                    return &(v->object);
                default:
                    return v;
            }
        }
    }

    return NULL;
}

json_value_type_t __json_value_type(const json_object_t object,
        const char *key) {
    for (u32 i=0;
            i<object.len;
            i++) {
        if (strcmp(object.props[i].key, key)) {
            return object.props[i].value.type;
        }
    }

    return JSON_TYPE_NONE;
}
