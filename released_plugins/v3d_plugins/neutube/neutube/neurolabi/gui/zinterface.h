/**@file ZInterface.h
 * @brief Interface base class
 * @author Ting Zhao
 */
#ifndef _ZINTERFACE_H_
#define _ZINTERFACE_H_

#include <string>

class ZInterface {
public:
  ZInterface() : m_selected(false) {}
  ZInterface(bool selected) : m_selected(selected) {}
  ZInterface(const ZInterface &/*obj*/) {}
  virtual ~ZInterface() {}

  virtual const std::string& className() const = 0;

  virtual void setSelected(bool selected) { m_selected = selected; }
  virtual bool isSelected() const { return m_selected; }
protected:
  bool m_selected;
};

#define ZINTERFACE_DEFINE_CLASS_NAME(c) \
  const std::string& c::className() const {\
    static const std::string name = #c;\
    return name; \
  }

#endif
