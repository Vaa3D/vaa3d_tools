/**@file zstackdrawable.h
 * @brief Drawable interface
 * @author Ting Zhao
 */

#ifndef _ZSTACKDRAWABLE_H_
#define _ZSTACKDRAWABLE_H_

#include "zqtheader.h"

#include "tz_cdefs.h"
#include "zinterface.h"

#if defined(_QT_GUI_USED_)
#include <QColor>
#include <QPainter>
#endif

// abstract class for those can be drawn in an image
class ZStackDrawable : public virtual ZInterface {
public:
  enum Palette_Color {
    BLUE = 0, GREEN, RED, ALPHA
  };

  enum Display_Style {
    NORMAL, SOLID, BOUNDARY, SKELETON
  };

  enum ETarget {
    STACK, OBJECT_CANVAS, WIDGET
  };

  ZStackDrawable()
    : m_isVisible(true)
    , m_style(SOLID)
    , m_target(WIDGET)
  {}

  ZStackDrawable(bool isVisible, Display_Style style) : m_target(WIDGET)
  {
    m_isVisible = isVisible;
    m_style = style;
  }
  ZStackDrawable(const ZStackDrawable &obj)
    : ZInterface(obj.isSelected())
  {
    m_isVisible = obj.m_isVisible;
    m_style = obj.m_style;
    m_target = obj.m_target;
    m_color = obj.m_color;
  }

  virtual ~ZStackDrawable() {} 


  // Display an object to widget, xoffset and yoffset is top left corner of widget
  // zoomration is ratio of widget pixel to object pixel
  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL) const = 0;

  inline bool isVisible() const { return m_isVisible; }
  inline void setVisible(bool visible) { m_isVisible = visible; }
  inline void setDisplayStyle(Display_Style style) { m_style = style; }
  inline Display_Style displayStyle() const { return m_style; }

  inline ETarget getTarget() const { return m_target; }
  inline void setTarget(ETarget target) { m_target = target; }

  inline QColor getColor() const { return m_color; }
  inline void setColor(int red, int green, int blue) {
#if defined(_QT_GUI_USED_)
    m_color.setRed(red);
    m_color.setGreen(green);
    m_color.setBlue(blue);
#else
    UNUSED_PARAMETER(red);
    UNUSED_PARAMETER(green);
    UNUSED_PARAMETER(blue);
#endif
  }
  inline void setColor(int red, int green, int blue, int alpha) {
#if defined(_QT_GUI_USED_)
    m_color.setRed(red);
    m_color.setGreen(green);
    m_color.setBlue(blue);
    m_color.setAlpha(alpha);
#else
    UNUSED_PARAMETER(red);
    UNUSED_PARAMETER(green);
    UNUSED_PARAMETER(blue);
    UNUSED_PARAMETER(alpha);
#endif
  }

  inline void setColor(const QColor &n) { m_color = n;}

  inline void setAlpha(int alpha) {
#if defined(_QT_GUI_USED_)
    m_color.setAlpha(alpha);
#else
    UNUSED_PARAMETER(alpha);
#endif
  }

  inline int getAlpha() {
#if defined(_QT_GUI_USED_)
    return m_color.alpha();
#else
    return 0;
#endif
  }

  inline double getAlphaF() {
#if defined(_QT_GUI_USED_)
    return m_color.alphaF();
#else
    return 0.0;
#endif
  }

  inline double getRedF() {
#if defined(_QT_GUI_USED_)
    return m_color.redF();
#else
    return 0.0;
#endif
  }

  inline double getGreenF() {
#if defined(_QT_GUI_USED_)
    return m_color.greenF();
#else
    return 0.0;
#endif
  }

  inline double getBlueF() {
#if defined(_QT_GUI_USED_)
    return m_color.blueF();
#else
    return 0.0;
#endif
  }

  inline bool isOpaque() {
    return getAlpha() == 255;
  }

  inline static void setDefaultPenWidth(double width) {
      m_defaultPenWidth = width;
  }

  inline static double getDefaultPenWidth() {
      return m_defaultPenWidth;
  }

protected:
  bool m_isVisible;
  Display_Style m_style;
  QColor m_color;
  ETarget m_target;
  static double m_defaultPenWidth;
};

#endif
