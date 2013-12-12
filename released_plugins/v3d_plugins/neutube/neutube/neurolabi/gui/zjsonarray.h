#ifndef ZJSONARRAY_H
#define ZJSONARRAY_H

#include <stddef.h>
#include "zjsonvalue.h"
#include "zuncopyable.h"

class ZJsonArray : public ZJsonValue, ZUncopyable
{
public:
  ZJsonArray();
  ZJsonArray(json_t *data, bool asNew);
  virtual ~ZJsonArray();

public:
  ::size_t size();
  json_t* at(::size_t index);

  /*!
   * \brief Append an element.
   * \param obj The element to be appended. Nothing is done if it is NULL.
   */
  void append(json_t *obj);
};

#endif // ZJSONARRAY_H
