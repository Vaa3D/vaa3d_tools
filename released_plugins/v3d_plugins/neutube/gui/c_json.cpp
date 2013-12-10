#include "c_json.h"

json_t* C_Json::makeObject()
{
  return json_object();
}

json_t* C_Json::makeInteger(int v)
{
  return json_integer(v);
}

json_t* C_Json::makeNumber(double v)
{
  return json_real(v);
}

json_t* C_Json::makeBoolean(bool v)
{
  return v ? json_true() : json_false();
}

json_t* C_Json::makeArray()
{
  return json_array();
}

json_t* C_Json::makeArray(const double *array, size_t n)
{
  if (array == NULL || n <= 0) {
    return NULL;
  }

  json_t *obj = makeArray();
  for (size_t i = 0; i < n; ++i) {
    appendArray(obj, makeNumber(array[i]));
  }

  return obj;
}

json_t* C_Json::makeArray(const int *array, size_t n)
{
  if (array == NULL || n <= 0) {
    return NULL;
  }

  json_t *obj = makeArray();
  for (size_t i = 0; i < n; ++i) {
    appendArray(obj, makeInteger(array[i]));
  }

  return obj;
}

void C_Json::appendArray(json_t *array, json_t *v)
{
  json_array_append(array, v);
}

bool C_Json::dump(const json_t *obj, const char *filePath)
{
  if (obj == NULL || filePath == NULL) {
    return false;
  }

  if (json_dump_file(obj, filePath, JSON_INDENT(2)) != 0) {
    return false;
  }

  return true;
}
