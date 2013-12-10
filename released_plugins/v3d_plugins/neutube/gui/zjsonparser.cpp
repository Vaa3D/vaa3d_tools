#include "zjsonparser.h"

#include <iostream>

using namespace std;

const char ZJsonParser::m_emptyString[] = {'\0' };

ZJsonParser::ZJsonParser()
{
}

bool ZJsonParser::isObject(const json_t *value)
{
  return json_is_object(value);
}

bool ZJsonParser::isArray(const json_t *value)
{
  return json_is_array(value);
}

bool ZJsonParser::isInteger(const json_t *value)
{
  return json_is_integer(value);
}

bool ZJsonParser::isReal(const json_t *value)
{
  return json_is_real(value);
}

bool ZJsonParser::isNumber(const json_t *value)
{
  return json_is_number(value);
}

bool ZJsonParser::isBoolean(const json_t *value)
{
  return json_is_boolean(value);
}

size_t ZJsonParser::arraySize(const json_t *array)
{
  if (!isArray(array)) {
    return 0;
  }

  return json_array_size(array);
}

json_t* ZJsonParser::arrayValue(const json_t *array, size_t index)
{
  return json_array_get(array, index);
}

json_type ZJsonParser::type(const json_t *value)
{
  return json_typeof(value);
}

void ZJsonParser::incref(json_t *value)
{
  incref(value);
}

void ZJsonParser::decref(json_t *value)
{
  decref(value);
}

const char* ZJsonParser::stringValue(const json_t *value)
{
  if (value == NULL) {
    return m_emptyString;
  }

  return json_string_value(value);
}

double ZJsonParser::numberValue(const json_t *value)
{
  return json_number_value(value);
}

int ZJsonParser::integerValue(const json_t *value)
{
  return json_integer_value(value);
}

bool ZJsonParser::booleanValue(const json_t *value)
{
  if (json_is_true(value)) {
    return true;
  }

  return false;
}

const char* ZJsonParser::stringValue(const json_t *value, size_t index)
{
  return stringValue(arrayValue(value, index));
}

double ZJsonParser::numberValue(const json_t *value, size_t index)
{
  return numberValue(arrayValue(value, index));
}

int ZJsonParser::integerValue(const json_t *value, size_t index)
{
  return integerValue(arrayValue(value, index));
}

void ZJsonParser::print(const char *key, json_t *object, int indent)
{
  for (int i = 0; i < indent; ++i) {
    cout << " ";
  }

  if (key != NULL) {
    cout << key << ": ";
  }

  switch (type(object)) {
  case JSON_NULL:
    cout << "NULL" << endl;
    break;
  case JSON_OBJECT:
  {
    cout << "{" << endl;
    json_t *value;
    const char *key;
    json_object_foreach(object, key, value) {
      print(key, value, indent + 2);
    }
    for (int i = 0; i < indent; ++i) {
      cout << " ";
    }
    cout << "}" << endl;
  }
    break;
  case JSON_ARRAY:
  {
    int n = arraySize(object);
    cout << "[" << endl;
    for (int i = 0; i < n; ++i) {
      print(NULL, arrayValue(object, i), indent + 2);
    }
    for (int i = 0; i < indent; ++i) {
      cout << " ";
    }
    cout << "]" << endl;
  }
    break;
  case JSON_STRING:
    cout << stringValue(object) << endl;
    break;
  case JSON_INTEGER:
  case JSON_REAL:
    cout << numberValue(object) << endl;
    break;
  case JSON_TRUE:
    cout << "true" << endl;
    break;
  case JSON_FALSE:
    cout << "false" << endl;
    break;
  }
}
