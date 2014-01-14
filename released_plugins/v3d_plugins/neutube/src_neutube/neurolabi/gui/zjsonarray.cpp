#include "zjsonarray.h"
#include "c_json.h"

using namespace std;

ZJsonArray::ZJsonArray() : ZJsonValue()
{
}

ZJsonArray::ZJsonArray(json_t *data, bool asNew)
{
  if (json_is_array(data)) {
    ZJsonValue(data, asNew);
  } else {
    m_data = NULL;
  }
}

ZJsonArray::~ZJsonArray()
{

}

size_t ZJsonArray::size()
{
  return json_array_size(m_data);
}

json_t* ZJsonArray::at(size_t index)
{
  return json_array_get(m_data, index);
}

void ZJsonArray::append(json_t *obj)
{
  if (m_data != NULL && obj != NULL) {
    C_Json::appendArray(m_data, obj);
  }
}
