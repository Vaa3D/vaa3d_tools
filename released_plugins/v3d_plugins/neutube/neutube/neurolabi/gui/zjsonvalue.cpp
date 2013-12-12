#include "zjsonvalue.h"

#include <sstream>

#include "zjsonparser.h"

using namespace std;

ZJsonValue::ZJsonValue()
{
  m_data = NULL;
}

ZJsonValue::ZJsonValue(json_t *data, bool asNew) : m_data(NULL)
{
  set(data, asNew);
}

ZJsonValue::ZJsonValue(const ZJsonValue &value)
{
  m_data = NULL;
  set(value.m_data, false);
}

ZJsonValue::ZJsonValue(int data)
{
  m_data = json_integer(data);
}

ZJsonValue::ZJsonValue(const char *data)
{
  m_data = json_string(data);
}

ZJsonValue::~ZJsonValue()
{
  if (m_data != NULL) {
    json_decref(m_data);
  }
}

bool ZJsonValue::isObject()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_object(m_data) > 0;
}

bool ZJsonValue::isArray()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_array(m_data) > 0;
}

bool ZJsonValue::isString()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_string(m_data) > 0;
}

bool ZJsonValue::isInteger()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_integer(m_data) > 0;
}

bool ZJsonValue::isReal()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_real(m_data) > 0;
}

bool ZJsonValue::isNumber()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_number(m_data) > 0;
}

bool ZJsonValue::isBoolean()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_boolean(m_data) > 0;
}

bool ZJsonValue::isEmpty()
{
  return m_data == NULL;
}

void ZJsonValue::set(json_t *data, bool asNew)
{
  if (m_data != NULL) {
    json_decref(m_data);
  }

  m_data = data;

  if ((!asNew) && (data != NULL)) {
    json_incref(data);
  }
}

void ZJsonValue::decodeString(const char *str)
{
  if (m_data != NULL) {
    json_decref(m_data);
  }

#if 0
  m_data = json_loads(str, JSON_DECODE_ANY, &m_error);
#endif
}

void ZJsonValue::print()
{
  ZJsonParser::print(NULL, m_data, 0);
}

ZJsonValue& ZJsonValue::operator = (const ZJsonValue &value)
{
  set(value.m_data, false);

  return (*this);
}

std::vector<ZJsonValue> ZJsonValue::toArray()
{
  std::vector<ZJsonValue> array;

  if (isArray()) {
    json_t *value = getValue();
    size_t n = ZJsonParser::arraySize(value);
    array.resize(n);
    for (size_t i = 0; i < n; ++i) {
      array[i].set(ZJsonParser::arrayValue(value, i), false);
    }
  }
  return array;
}

std::string ZJsonValue::getErrorString() const
{
  ostringstream stream;
#if 0
  stream << "Line " << m_error.line << " Column " << m_error.column
         << ": " << m_error.text;
#endif

  return stream.str();
}
