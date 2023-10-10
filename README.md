# JSONC - A Lightweight JSON Parser for C99

...designed to parse JSON data efficiently and to provide an easy-to-use API for working with JSON objects and arrays.

## Usage

```shell
$ git clone https://github.com/rhighs/jsonc.git
$ gcc -Wall -c json.c -std=c99
```

## Examples

```c
#include <stdio.h>
#include <string.h>
#include "json.h"

int main() {
    const char* json_data = "{\"name\": \"John\", \"age\": 30}";
    json_value_t root;

    if (json_parse(&root, json_data, strlen(json_data)) == NONE) {
        // Access JSON data here
        const char* name = json_get_string(&root.object, "name");
        int age = json_get_number(&root.object, "age");

        printf("Name: %s\n", name);
        printf("Age: %d\n", age);
    } else {
        printf("Failed to parse JSON data.\n");
    }

    return 0;
}
```

## [LICENSE](https://github.com/rhighs/jsonc/blob/master/LICENSE)
