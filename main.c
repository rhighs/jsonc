#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "json.h"

i32 main(void) {
    const char *json_string = \
        "{ \"ciao\": 1234.1234, \"stuff_here\": [1, 2, 3, 4, 5, { \"more\": 12 }]}";
    json_value_t value;
    json_parse(&value, json_string, strlen(json_string));
    assert(strcmp(value.object.keys[1], "stuff_here"));

    if (JSON_EXISTS(value, "ciao")
            && JSON_TYPE(value, "ciao") == JSON_TYPE_NUMBER) {
        double number = JSON_GET(value, "ciao", double);
        printf("Found number: %f\n", number);
    }

    return 0;
}
