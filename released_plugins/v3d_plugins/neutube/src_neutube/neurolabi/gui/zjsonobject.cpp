#include "zjsonobject.h"

#include <iostream>
#include <sstream>
#include <string.h>

#include "tz_error.h"
#include "zjsonparser.h"
#include "c_json.h"

using namespace std;

ZJsonObject::ZJsonObject(json_t *json, bool asNew) : ZJsonValue()
{
  if (ZJsonParser::isObject(json)) {
    set(json, asNew);
  }
}

ZJsonObject::ZJsonObject()
{
}

ZJsonObject::ZJsonObject(const ZJsonObject &obj) : ZJsonValue(obj)
{
}

ZJsonObject::~ZJsonObject()
{

}

bool ZJsonObject::hasKey(const char *key) const
{
  return (*this)[key] != NULL;
}

json_t* ZJsonObject::operator[] (const char *key)
{
  return const_cast<json_t*>(static_cast<const ZJsonObject&>(*this)[key]);
}

bool ZJsonObject::isEmpty()
{
  if (m_data == NULL) {
    return true;
  }

  return json_object_size(m_data) == 0;
}

const json_t* ZJsonObject::operator[] (const char *key) const
{
  if (m_data != NULL) {
    const char *qkey;
    json_t *value;

    json_object_foreach(m_data, qkey, value) {
      if (strcmp(qkey, key) == 0) {
        return value;
      }
    }
  }

  return NULL;
}

bool ZJsonObject::load(string filePath)
{
  if (m_data != NULL) {
    json_decref(m_data);
  }

  m_data = json_load_file(filePath.c_str(), 0, &m_error);
  if (m_data) {
    return true;
  }

#if defined(HAVE_LIBJANSSON)
  cout << m_error.source << endl << ": " << m_error.text << endl;
#endif

  return false;
}

string ZJsonObject::toString()
{
  ostringstream stream;

  if (m_data != NULL) {
    const char *key;
    json_t *value;

    json_object_foreach(m_data, key, value) {
      if (json_is_object(value)) {
        ZJsonObject obj(value, false);
        stream << obj.toString();
      } else if (json_is_array(value)) {
        stream << key << " : " << "Array " << endl;
        for (size_t i = 0; i < json_array_size(value); i++) {
          json_t *element = json_array_get(value, i);
          if (json_is_object(element)) {
            ZJsonObject obj(element, false);
            stream << obj.toString();
          } else {
            stream << "Element : " << "Type " << json_typeof(element) << endl;
          }
        }
      } else {
        stream << key << " : " << "Type " << json_typeof(value) << endl;
      }
    }
  }

  return stream.str();
}

void ZJsonObject::appendEntries(const char *key, json_t *obj,
                                map<string, json_t*> *entryMap)
{
  if (key != NULL && obj != NULL) {
    (*entryMap)[key] = obj;
  }

  if (ZJsonParser::isObject(obj)) {
    const char *subkey = NULL;
    json_t *value = NULL;
    json_object_foreach(obj, subkey, value) {
      appendEntries(subkey, value, entryMap);
    }
  }
}

map<string, json_t*> ZJsonObject::toEntryMap(bool recursive) const
{
  TZ_ASSERT(json_is_object(m_data), "Invalid json object");

  map<std::string, json_t*> entryMap;

  if (recursive) {
    appendEntries(NULL, m_data, &entryMap);
  } else {
    const char *subkey = NULL;
    json_t *value = NULL;
    json_object_foreach(m_data, subkey, value) {
      entryMap[subkey] = value;
    }
  }

  return entryMap;
}


bool ZJsonObject::isValidKey(const char *key)
{
  if (key == NULL) {
    return false;
  }

  if (key[0] == '\0') {
    return false;
  }

  return true;
}

void ZJsonObject::setEntryWithoutKeyCheck(const char *key, json_t *obj)
{
  if (obj != NULL) {
    if (m_data == NULL) {
      m_data = C_Json::makeObject();
    }
    json_object_set(m_data, key, obj);
  }
}

void ZJsonObject::setEntry(const char *key, json_t *obj)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, obj);
}

void ZJsonObject::setEntry(const char *key, const string &value)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, json_string(value.c_str()));
}

void ZJsonObject::setEntry(const char *key, const double *array, size_t n)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeArray(array, n));
}

void ZJsonObject::setEntry(const char *key, const int *array, size_t n)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeArray(array, n));
}

void ZJsonObject::setEntry(const char *key, bool v)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeBoolean(v));
}

void ZJsonObject::setEntry(const char *key, int v)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeInteger(v));
}

void ZJsonObject::setEntry(const char *key, double v)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeNumber(v));
}

bool ZJsonObject::dump(const string &path) const
{
  return C_Json::dump(m_data, path.c_str());
}
