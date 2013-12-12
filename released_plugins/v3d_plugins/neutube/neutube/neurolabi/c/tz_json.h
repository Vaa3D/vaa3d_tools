/**@file tz_json.h
 * @author Ting Zhao
 * @date 16-Sep-2012
 */

#ifndef _TZ_JSON_H_
#define _TZ_JSON_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#if defined(HAVE_LIBJANSSON)
#include <jansson.h>
#else
typedef void* json_t;
typedef void* json_error_t;
static json_t* json_object() { return 0; }
static json_t* json_true() { return 0; }
static json_t* json_false() { return 0; }
static json_t* json_array() { return 0; }
#define JSON_INDENT(v) 0
#define json_real(v) 0
#define json_object_foreach(data, key, value)
#define json_load_file(path, flags, error) 0
#define json_dump_file(path, flags, error) 0
#define json_is_object(value) 0
#define json_object_set(data, key, obj) 0
#define json_object_size(value) 0
#define json_is_array(value) 0
#define json_is_string(value) 0
#define json_is_integer(value) 0
#define json_is_real(value) 0
#define json_is_number(value) 0
#define json_is_boolean(value) 0
#define json_is_true(value) 0
#define json_array_size(value) 0
#define json_array_get(value, index) 0
#define json_array_append(value, index) 0
#define json_typeof(value) 0
#define json_incref(value) UNUSED_PARAMETER(value)
#define json_decref(value) UNUSED_PARAMETER(value)
#define json_integer(value) 0
#define json_string(value) 0
#define json_string_value(key) "\0"
#define json_number_value(key) 0
#define json_integer_value(key) 0
#define json_boolean_value(key) 0
#define json_array_value(key) 0
#define json_loads(input, flags, error) 0
#define JSON_REJECT_DUPLICATES 0
#define JSON_DECODE_ANY 0
#define JSON_DISABLE_EOF_CHECK 0
#define JSON_OBJECT 1
#define JSON_ARRAY 2
#define JSON_STRING 3
#define JSON_INTEGER 4
#define JSON_REAL 5
#define JSON_TRUE 6
#define JSON_FALSE 7
#define JSON_NULL 0
typedef int json_type;
#endif

void json_info();

__END_DECLS

#endif
