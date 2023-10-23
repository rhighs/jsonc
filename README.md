# JSONC - A Lightweight JSON Parser for C99

...designed to parse JSON data efficiently and to provide an easy-to-use API for working with JSON objects and arrays.

## Usage

```shell
$ git clone https://github.com/rhighs/jsonc.git
$ gcc -Wall -c json.c -std=c99
```

## Docs

### `JSON_GET(value, type, keys...)`

Retrieves a value from a JSON object.

- `value`: JSON object.
- `type`: Target C type.
- `keys...`: Keys to access the value.

Returns: Value cast to the specified C type.

### `JSON_IGET(__VALUE, __TYPE, IDX)`

Retrieves a value from a JSON array at a given index.

- `__VALUE`: JSON array.
- `__TYPE`: Target C type.
- `IDX`: Index of the value.

Returns: Value cast to the specified C type.

### `JSON_ARRAY_LEN(__VALUE)`

Gets the length of a JSON array.

- `__VALUE`: JSON array.

Returns: Length of the array.

### `JSON_EXISTS(__VALUE, ...)`

Checks if specific keys exist in a JSON object.

- `__VALUE`: JSON object.
- `...`: Key(s) to check.

Returns: `1` if the key(s) exist, `0` otherwise.

### `JSON_TYPE(__VALUE, __KEY)`

Determines the data type of a value associated with a key in a JSON object.

- `__VALUE`: JSON object.
- `__KEY`: Key to inspect.

Returns: Data type of the value.

### `JSON_SET(__VALUE, __KEY, __TYPE, TYPE, VALUE)`

Sets a value for a key in a JSON object.

- `__VALUE`: JSON object.
- `__KEY`: Key for the value.
- `__TYPE`: Data type of the value.
- `TYPE`: C type of the value.
- `VALUE`: The value to store.

### `JSON_NUMBER(VALUE)`

Creates a JSON number value from a numeric value.

- `VALUE`: Numeric value (double).

Returns: JSON value representing the number.

### `JSON_STRING(VALUE)`

Creates a JSON string value from a string.

- `VALUE`: String (char*).

Returns: JSON value representing the string.

### `JSON_BOOL(VALUE)`

Creates a JSON boolean value from a boolean value.

- `VALUE`: Boolean value (0 for false, 1 for true, u8).

Returns: JSON value representing the boolean.

### `JSON_NULL`

Creates a JSON null value.

Returns: JSON null value.

### `JSON_PROP(KEY, VALUE)`

Creates a JSON property with a key-value association.

- `KEY`: Key for the property (char*).
- `VALUE`: JSON value associated with the key.

Returns: JSON property with the specified key and value.

### `JSON_OBJECT(...)`

Creates a JSON object with a list of properties.

Returns: JSON object containing the specified properties.

## [LICENSE](https://github.com/rhighs/jsonc/blob/master/LICENSE)
