#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "json.h"

i32 main(void) {
    const char *json_string = \
        "{ \"ciao\": 1234.1234,\
           \"name\": \"roberto\",\
           \"truthy\": true,\
           \"falsey\": false,\
           \"nully\": null,\
           \"stuff_here\": [1, 2, 3, 4, 5, { \"more\": 12 }]\
        }";
    json_value_t value;
    json_parse(&value, json_string, strlen(json_string));
    assert(strcmp(value.object.keys[1], "stuff_here"));
 
    if (JSON_EXISTS(value, "ciao")
            && JSON_TYPE(value, "ciao") == JSON_TYPE_NUMBER) {
        double number = JSON_GET(value, "ciao", double);
        printf("Found number: %f\n", number);
    }

    if (JSON_EXISTS(value, "name")
            && JSON_TYPE(value, "name") == JSON_TYPE_STRING) {
        const char *name = JSON_GET(value, "name", const char *);
        printf("Found name: %s\n", name);
    }

    if (JSON_EXISTS(value, "truthy")
            && JSON_TYPE(value, "truthy") == JSON_TYPE_BOOL) {
        u8 b = JSON_GET(value, "truthy", u8);
        printf("Found truthy: %d\n", b);
    }

    if (JSON_EXISTS(value, "falsey")
            && JSON_TYPE(value, "falsey") == JSON_TYPE_BOOL) {
        u8 b = JSON_GET(value, "falsey", u8);
        printf("Found falsey: %d\n", b);
    }

    if (JSON_EXISTS(value, "nully")
            && JSON_TYPE(value, "nully") == JSON_TYPE_NULL) {
        printf("Found nully: null\n");
    }

    return 0;
}
