#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../json.h"

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
        double number = JSON_GET(value, double, "ciao");
        printf("Found number: %f\n", number);
    }

    if (JSON_EXISTS(value, "name")
            && JSON_TYPE(value, "name") == JSON_TYPE_STRING) {
        const char *name = JSON_GET(value, const char *, "name");
        printf("Found name: %s\n", name);
    }

    if (JSON_EXISTS(value, "truthy")
            && JSON_TYPE(value, "truthy") == JSON_TYPE_BOOL) {
        u8 b = JSON_GET(value, u8, "truthy");
        printf("Found truthy: %d\n", b);
    }

    if (JSON_EXISTS(value, "falsey")
            && JSON_TYPE(value, "falsey") == JSON_TYPE_BOOL) {
        u8 b = JSON_GET(value, u8, "falsey");
        printf("Found falsey: %d\n", b);
    }

    if (JSON_EXISTS(value, "nully")
            && JSON_TYPE(value, "nully") == JSON_TYPE_NULL) {
        printf("Found nully: null\n");
    }

    JSON_SET(value, "ciaociao", JSON_TYPE_NUMBER, double, 100.0);
    if (JSON_EXISTS(value, "ciaociao")
            && JSON_TYPE(value, "ciaociao") == JSON_TYPE_NUMBER) {
        double number = JSON_GET(value, double, "ciaociao");
        printf("Found number: %f\n", number);
    }

    json_value_t o = JSON_OBJECT(
            JSON_PROP("test", JSON_NUMBER(9999.0)),
            JSON_PROP("value", JSON_OBJECT(
                    JSON_PROP("test", JSON_NUMBER(100.0)),
                    JSON_PROP("test_object", JSON_OBJECT(
                            JSON_PROP("nested", JSON_STRING("Some deeply nested string"))
                            ))
                    ))
    );

    const char *nested_str =
        JSON_GET(o, const char*, "value", "test_object", "nested");

    printf("Value str: %s\n", nested_str);

    if (JSON_EXISTS(o, "test")
            && JSON_TYPE(o, "test") == JSON_TYPE_NUMBER) {
        double number = JSON_GET(o, double, "test");
        printf("Found number: %f\n", number);
    }
 
    return 0;
}
