/**@file testjson.c
 * @author Ting Zhao
 * @date 16-Sep-2012
 */

#include "tz_utilities.h"
#include "tz_json.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

#if 0
  json_t *array = json_array();
  json_t *integer = json_integer(42);
  json_array_append(array, integer);
  json_decref(array);

  printf("Type: %d\n", json_typeof(integer));
#endif

#if 1
  json_error_t error;
  json_t *obj = json_load_file("../data/benchmark/sample.json", 0, &error);
  
  printf("Type: %d\n", json_typeof(obj));

  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    /* block of code that uses key and value */
    printf("%s : type %d\n", key, json_typeof(value));
  }
#endif

  return 0;
}
