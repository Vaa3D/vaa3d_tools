#include "zsynapseannotationmetadata.h"

#include <string.h>
#include <sstream>
#include "zjsonparser.h"

using namespace FlyEm;
using namespace std;

ZSynapseAnnotationMetadata::ZSynapseAnnotationMetadata() : m_version(0)
{
  for (int i = 0; i < 3; ++i) {
    m_resolution[i] = 1.0;
    m_sourceDimension[i] = 0;
    m_sourceOffset[i] = 0;
  }
}

void
ZSynapseAnnotationMetadata::set(std::string description, std::string software,
                                std::string softwareVersion, std::string date,
                                std::string sessionPath, int version)
{
  m_description = description;
  m_software = software;
  m_softwareVersion = softwareVersion;
  m_date = date;
  m_sessionPath = sessionPath;
  m_version = version;
}

string ZSynapseAnnotationMetadata::toString(int indent)
{
  ostringstream stream;

  string indentStr(indent, ' ');

  if (indent == 0) {
    indentStr.clear();
  }

  bool first = true;

  if (!m_description.empty()) {
    stream << indentStr << "\"description\": " << "\"" << m_description << "\"";
    first  = false;
  }
  if (!m_software.empty()) {
    if (!first) {
      stream << ',' << endl;
    } else {
      first = false;
    }
    stream << indentStr << "\"software\": " << "\"" << m_software << "\"";
  }
  if (!m_softwareVersion.empty()) {
    if (!first) {
      stream << ',' << endl;
    } else {
      first = false;
    }
    stream << indentStr << "\"software version\": " << "\""
           << m_softwareVersion << "\"";
  }
  if (!m_sessionPath.empty()) {
    if (!first) {
      stream << ',' << endl;
    } else {
      first = false;
    }
    stream << indentStr << "\"session path\": " << "\"" << m_sessionPath
           << "\"";
  }
  if (!m_date.empty()) {
    if (!first) {
      stream << ',' << endl;
    } else {
      first = false;
    }
    stream << indentStr << "\"date\": " << "\"" << m_date << "\"";
  }
  if (m_version > 0) {
    if (!first) {
      stream << ',' << endl;
    } else {
      first = false;
    }
    stream << indentStr << "\"file version\": " << m_version;
  }

  return stream.str();
}

void ZSynapseAnnotationMetadata::loadJsonObject(json_t *object)
{
  const char *key;
  json_t *value;

  json_object_foreach(object, key, value) {
    if (strcmp(key, "description") == 0) {
      m_description = json_string_value(value);
    } else if (strcmp(key, "software") == 0) {
      m_software = json_string_value(value);
    } else if (strcmp(key, "software version") == 0) {
      m_softwareVersion = json_string_value(value);
    } else if (strcmp(key, "session path") == 0) {
      m_sessionPath = json_string_value(value);
    } else if (strcmp(key, "date") == 0) {
      m_date = json_string_value(value);
    } else if (strcmp(key, "version") == 0 ||
               strcmp(key, "file version") == 0) {
      m_version = json_integer_value(value);
    } else if (strcmp(key, "source_offset")) {
      if (json_is_array(value))  {
        if (json_array_size(value) == 3) {
          for (int i = 0; i < 3; ++i) {
            m_sourceOffset[i] = ZJsonParser::integerValue(value, i);
          }
        }
      }
    } else if (strcmp(key, "source_dimension")) {
      if (json_is_array(value))  {
        if (json_array_size(value) == 3) {
          for (int i = 0; i < 3; ++i) {
            m_sourceDimension[i] = ZJsonParser::integerValue(value, i);
          }
        }
      }
    }
  }
}
