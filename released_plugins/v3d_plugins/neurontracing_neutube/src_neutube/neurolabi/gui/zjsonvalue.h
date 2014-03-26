#ifndef ZJSONVALUE_H
#define ZJSONVALUE_H

#include <vector>
#include <string>

#include "neurolabi_config.h"
#include "tz_json.h"

class ZJsonValue
{
public:
  ZJsonValue();
  ZJsonValue(const ZJsonValue &value);
  /*!
   * \brief Constructor
   *
   * \param data json value pointer
   * \param asNew Take it as a new value or just increase its reference count
   */
  ZJsonValue(json_t *data, bool asNew);

  /*!
   * \brief Constructor
   *
   * Create a json object with integer data
   *
   * \param data The input integer
   */
  ZJsonValue(int data);
  ZJsonValue(const char *data);

  ZJsonValue& operator= (const ZJsonValue &value);

  virtual ~ZJsonValue();

public:
  inline json_t *getData() { return m_data; }
  inline json_t *getValue() { return m_data; }

  bool isObject();
  bool isArray();
  bool isString();
  bool isInteger();
  bool isReal();
  bool isNumber();
  bool isBoolean();
  virtual bool isEmpty();

  int getInteger();
  double getReal();
  const char *getString();

  void set(json_t *data, bool asNew);
  void decodeString(const char *str);

  void print();

  /*!
   * \brief Get elements of a JSON array
   * \return Empty array if the object is not an array.
   */
  std::vector<ZJsonValue> toArray();

  /*!
   * \brief Get a string describing the current error
   */
  std::string getErrorString() const;

protected:
  json_error_t m_error;
  json_t *m_data;
};

#endif
