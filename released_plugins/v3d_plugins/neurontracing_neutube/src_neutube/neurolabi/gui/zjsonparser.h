#ifndef ZJSONPARSER_H
#define ZJSONPARSER_H

#include "neurolabi_config.h"
#include "tz_json.h"
#include "zjsonobject.h"
#include "zjsonarray.h"

class ZJsonParser
{
public:
  ZJsonParser();

  static bool isObject(const json_t *value);
  static bool isArray(const json_t *value);
  static bool isInteger(const json_t *value);
  static bool isReal(const json_t *value);
  static bool isNumber(const json_t *value);
  static bool isBoolean(const json_t *value);

  //It returns 0 if <array> is not an array
  static size_t arraySize(const json_t *array);

  static json_t* arrayValue(const json_t *array, size_t index);
  static json_type type(const json_t *value);
  static void incref(json_t *value);
  static void decref(json_t *value);

  static const char* stringValue(const json_t *value);
  static double numberValue(const json_t *value);
  static int integerValue(const json_t *value);
  static bool booleanValue(const json_t *value);

  static const char* stringValue(const json_t *value, size_t index);
  static double numberValue(const json_t *value, size_t index);
  static int integerValue(const json_t *value, size_t index);

  static void print(const char *key, json_t *value, int indent);

private:
  json_error_t m_error;

  const static char m_emptyString[1];
};

#endif // ZJSONPARSER_H
