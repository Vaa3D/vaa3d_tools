#include "znumericparameter.h"
#include "zspinboxwithslider.h"
#include "zspinbox.h"
#include "zclickablelabel.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include "zspanslider.h"

ZIntParameter::ZIntParameter(const QString &name, int value, int min, int max, QObject *parent)
  : ZNumericParameter<int>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZIntParameter::setValue(int v)
{
  set(v);
}

void ZIntParameter::beforeChange(int &value)
{
  emit valueChanged(value);
}

QWidget *ZIntParameter::actualCreateWidget(QWidget *parent)
{
  if (m_style == "SPINBOX") {
    ZSpinBox *sb = new ZSpinBox(parent);
    sb->setRange(m_min, m_max);
    sb->setValue(m_value);
    sb->setSingleStep(m_step);
    connect(sb, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    connect(this, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)));
    connect(this, SIGNAL(rangeChanged(int,int)), sb, SLOT(setDataRange(int,int)));
    return sb;
  } else {
    ZSpinBoxWithSlider *sbws = new ZSpinBoxWithSlider(m_value, m_min, m_max, m_step, m_tracking, parent);
    connect(sbws, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    connect(this, SIGNAL(valueChanged(int)), sbws, SLOT(setValue(int)));
    connect(this, SIGNAL(rangeChanged(int,int)), sbws, SLOT(setDataRange(int,int)));
    return sbws;
  }
}

void ZIntParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}

ZDoubleParameter::ZDoubleParameter(const QString &name, double value, double min, double max, QObject *parent)
  : ZNumericParameter<double>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZDoubleParameter::setValue(double v)
{
  set(v);
}

void ZDoubleParameter::beforeChange(double &value)
{
  emit valueChanged(value);
}

QWidget *ZDoubleParameter::actualCreateWidget(QWidget *parent)
{
  if (m_style == "SPINBOX") {
    ZDoubleSpinBox *sb = new ZDoubleSpinBox(parent);
    sb->setRange(m_min, m_max);
    sb->setValue(m_value);
    sb->setSingleStep(m_step);
    sb->setDecimals(m_decimal);
    connect(sb, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    connect(this, SIGNAL(valueChanged(double)), sb, SLOT(setValue(double)));
    connect(this, SIGNAL(rangeChanged(double,double)), sb, SLOT(setDataRange(double,double)));
    return sb;
  } else {
    ZDoubleSpinBoxWithSlider *sbws = new ZDoubleSpinBoxWithSlider(m_value, m_min, m_max, m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    connect(this, SIGNAL(valueChanged(double)), sbws, SLOT(setValue(double)));
    connect(this, SIGNAL(rangeChanged(double,double)), sbws, SLOT(setDataRange(double,double)));
    return sbws;
  }
}

void ZDoubleParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}

ZFloatParameter::ZFloatParameter(const QString &name, float value, float min, float max, QObject *parent)
  : ZNumericParameter<float>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZFloatParameter::setValue(double v)
{
  set(static_cast<float>(v));
}

void ZFloatParameter::beforeChange(float &value)
{
  emit valueChanged(static_cast<double>(value));
}

QWidget *ZFloatParameter::actualCreateWidget(QWidget *parent)
{
  if (m_style == "SPINBOX") {
    ZDoubleSpinBox *sb = new ZDoubleSpinBox(parent);
    sb->setRange(m_min, m_max);
    sb->setValue(m_value);
    sb->setSingleStep(m_step);
    sb->setDecimals(m_decimal);
    connect(sb, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    connect(this, SIGNAL(valueChanged(double)), sb, SLOT(setValue(double)));
    connect(this, SIGNAL(rangeChanged(double,double)), sb, SLOT(setDataRange(double,double)));
    return sb;
  } else {
    ZDoubleSpinBoxWithSlider *sbws = new ZDoubleSpinBoxWithSlider(m_value, m_min, m_max, m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    connect(this, SIGNAL(valueChanged(double)), sbws, SLOT(setValue(double)));
    connect(this, SIGNAL(rangeChanged(double,double)), sbws, SLOT(setDataRange(double,double)));
    return sbws;
  }
}

void ZFloatParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}

//---------------------------------------------------------------------------------------------------------------

ZVec2Parameter::ZVec2Parameter(const QString &name, glm::vec2 value, glm::vec2 min, glm::vec2 max, QObject *parent)
  : ZNumericVectorParameter<glm::vec2>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZVec2Parameter::setValue1(double v)
{
  set(glm::vec2(static_cast<float>(v), m_value[1]));
}

void ZVec2Parameter::setValue2(double v)
{
  set(glm::vec2(m_value[0], static_cast<float>(v)));
}

void ZVec2Parameter::beforeChange(glm::vec2 &value)
{
  if (value[0] != m_value[0])
    emit value1Changed(value[0]);
  if (value[1] != m_value[1])
    emit value2Changed(value[1]);
}

QWidget *ZVec2Parameter::actualCreateWidget(QWidget *parent)
{
  QWidget *w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout *lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    ZDoubleSpinBox *sb1 = new ZDoubleSpinBox();
    sb1->setRange(m_min[0], m_max[0]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    sb1->setDecimals(m_decimal);
    connect(sb1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sb1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb2 = new ZDoubleSpinBox();
    sb2->setRange(m_min[1], m_max[1]);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    sb2->setDecimals(m_decimal);
    connect(sb2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sb2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }

  } else {
    ZDoubleSpinBoxWithSlider *sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sbws1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sbws1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sbws1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sbws2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws2);
        lo->addLayout(hlo);
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//---------------------------------------------------------------------------------------------------------------

ZVec3Parameter::ZVec3Parameter(const QString &name, glm::vec3 value, glm::vec3 min, glm::vec3 max, QObject *parent)
  : ZNumericVectorParameter<glm::vec3>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

void ZVec3Parameter::setValue1(double v)
{
  set(glm::vec3(static_cast<float>(v), m_value[1], m_value[2]));
}

void ZVec3Parameter::setValue2(double v)
{
  set(glm::vec3(m_value[0], static_cast<float>(v), m_value[2]));
}

void ZVec3Parameter::setValue3(double v)
{
  set(glm::vec3(m_value[0], m_value[1], static_cast<float>(v)));
}

void ZVec3Parameter::beforeChange(glm::vec3 &value)
{
  if (value[0] != m_value[0])
    emit value1Changed(value[0]);
  if (value[1] != m_value[1])
    emit value2Changed(value[1]);
  if (value[2] != m_value[2])
    emit value3Changed(value[2]);
}

QWidget *ZVec3Parameter::actualCreateWidget(QWidget *parent)
{
  if (m_style == "COLOR") {
    return new ZClickableColorLabel(this, parent);
  }

  QWidget *w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout *lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    ZDoubleSpinBox *sb1 = new ZDoubleSpinBox();
    sb1->setRange(m_min[0], m_max[0]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    sb1->setDecimals(m_decimal);
    connect(sb1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sb1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb2 = new ZDoubleSpinBox();
    sb2->setRange(m_min[1], m_max[1]);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    sb2->setDecimals(m_decimal);
    connect(sb2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sb2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb3 = new ZDoubleSpinBox();
    sb3->setRange(m_min[2], m_max[2]);
    sb3->setValue(m_value[2]);
    sb3->setSingleStep(m_step);
    sb3->setDecimals(m_decimal);
    connect(sb3, SIGNAL(valueChanged(double)), this, SLOT(setValue3(double)));
    connect(this, SIGNAL(value3Changed(double)), sb3, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sb3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb3);
        lo->addLayout(hlo);
      }
    }

  } else {
    ZDoubleSpinBoxWithSlider *sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sbws1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sbws1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sbws2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sbws2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws2);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws3 = new ZDoubleSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws3, SIGNAL(valueChanged(double)), this, SLOT(setValue3(double)));
    connect(this, SIGNAL(value3Changed(double)), sbws3, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sbws3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws3);
        lo->addLayout(hlo);
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//---------------------------------------------------------------------------------------------------------------

ZVec4Parameter::ZVec4Parameter(const QString &name, glm::vec4 value, glm::vec4 min, glm::vec4 max, QObject *parent)
  : ZNumericVectorParameter<glm::vec4>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

void ZVec4Parameter::setValue1(double v)
{
  set(glm::vec4(static_cast<float>(v), m_value[1], m_value[2], m_value[3]));
}

void ZVec4Parameter::setValue2(double v)
{
  set(glm::vec4(m_value[0], static_cast<float>(v), m_value[2], m_value[3]));
}

void ZVec4Parameter::setValue3(double v)
{
  set(glm::vec4(m_value[0], m_value[1], static_cast<float>(v), m_value[3]));
}

void ZVec4Parameter::setValue4(double v)
{
  set(glm::vec4(m_value[0], m_value[1], m_value[2], static_cast<float>(v)));
}

void ZVec4Parameter::beforeChange(glm::vec4 &value)
{
  if (value[0] != m_value[0])
    emit value1Changed(value[0]);
  if (value[1] != m_value[1])
    emit value2Changed(value[1]);
  if (value[2] != m_value[2])
    emit value3Changed(value[2]);
  if (value[3] != m_value[3])
    emit value4Changed(value[3]);
}

QWidget *ZVec4Parameter::actualCreateWidget(QWidget *parent)
{
  if (m_style == "COLOR") {
    return new ZClickableColorLabel(this, parent);
  }

  QWidget *w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout *lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    ZDoubleSpinBox *sb1 = new ZDoubleSpinBox();
    sb1->setRange(m_min[0], m_max[0]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    sb1->setDecimals(m_decimal);
    connect(sb1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sb1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb2 = new ZDoubleSpinBox();
    sb2->setRange(m_min[1], m_max[1]);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    sb2->setDecimals(m_decimal);
    connect(sb2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sb2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb3 = new ZDoubleSpinBox();
    sb3->setRange(m_min[2], m_max[2]);
    sb3->setValue(m_value[2]);
    sb3->setSingleStep(m_step);
    sb3->setDecimals(m_decimal);
    connect(sb3, SIGNAL(valueChanged(double)), this, SLOT(setValue3(double)));
    connect(this, SIGNAL(value3Changed(double)), sb3, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sb3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb3);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb4 = new ZDoubleSpinBox();
    sb4->setRange(m_min[3], m_max[3]);
    sb4->setValue(m_value[3]);
    sb4->setSingleStep(m_step);
    sb4->setDecimals(m_decimal);
    connect(sb4, SIGNAL(valueChanged(double)), this, SLOT(setValue4(double)));
    connect(this, SIGNAL(value4Changed(double)), sb4, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(3).isEmpty()) {
      lo->addWidget(sb4);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[3]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb4);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb4);
        lo->addLayout(hlo);
      }
    }
  } else {
    ZDoubleSpinBoxWithSlider *sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sbws1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sbws1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sbws2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sbws2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws2);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws3 = new ZDoubleSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws3, SIGNAL(valueChanged(double)), this, SLOT(setValue3(double)));
    connect(this, SIGNAL(value3Changed(double)), sbws3, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sbws3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws3);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws4 = new ZDoubleSpinBoxWithSlider(m_value[3], m_min[3], m_max[3], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws4, SIGNAL(valueChanged(double)), this, SLOT(setValue4(double)));
    connect(this, SIGNAL(value4Changed(double)), sbws4, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(3).isEmpty()) {
      lo->addWidget(sbws4);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[3]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws4);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws4);
        lo->addLayout(hlo);
      }
    }
  }

  w->setLayout(lo);
  return w;
}


//---------------------------------------------------------------------------------------------------------------

ZDVec2Parameter::ZDVec2Parameter(const QString &name, glm::dvec2 value, glm::dvec2 min, glm::dvec2 max, QObject *parent)
  : ZNumericVectorParameter<glm::dvec2>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZDVec2Parameter::setValue1(double v)
{
  set(glm::dvec2(v, m_value[1]));
}

void ZDVec2Parameter::setValue2(double v)
{
  set(glm::dvec2(m_value[0], v));
}

void ZDVec2Parameter::beforeChange(glm::dvec2 &value)
{
  if (value[0] != m_value[0])
    emit value1Changed(value[0]);
  if (value[1] != m_value[1])
    emit value2Changed(value[1]);
}

QWidget *ZDVec2Parameter::actualCreateWidget(QWidget *parent)
{
  QWidget *w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout *lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    ZDoubleSpinBox *sb1 = new ZDoubleSpinBox();
    sb1->setRange(m_min[0], m_max[0]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    sb1->setDecimals(m_decimal);
    connect(sb1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sb1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb2 = new ZDoubleSpinBox();
    sb2->setRange(m_min[1], m_max[1]);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    sb2->setDecimals(m_decimal);
    connect(sb2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sb2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }

  } else {
    ZDoubleSpinBoxWithSlider *sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sbws1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sbws1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sbws1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sbws2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws2);
        lo->addLayout(hlo);
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//---------------------------------------------------------------------------------------------------------------

ZDVec3Parameter::ZDVec3Parameter(const QString &name, glm::dvec3 value, glm::dvec3 min, glm::dvec3 max, QObject *parent)
  : ZNumericVectorParameter<glm::dvec3>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

void ZDVec3Parameter::setValue1(double v)
{
  set(glm::dvec3(v, m_value[1], m_value[2]));
}

void ZDVec3Parameter::setValue2(double v)
{
  set(glm::dvec3(m_value[0], v, m_value[2]));
}

void ZDVec3Parameter::setValue3(double v)
{
  set(glm::dvec3(m_value[0], m_value[1], v));
}

void ZDVec3Parameter::beforeChange(glm::dvec3 &value)
{
  if (value[0] != m_value[0])
    emit value1Changed(value[0]);
  if (value[1] != m_value[1])
    emit value2Changed(value[1]);
  if (value[2] != m_value[2])
    emit value3Changed(value[2]);
}

QWidget *ZDVec3Parameter::actualCreateWidget(QWidget *parent)
{
  if (m_style == "COLOR") {
    return new ZClickableColorLabel(this, parent);
  }

  QWidget *w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout *lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    ZDoubleSpinBox *sb1 = new ZDoubleSpinBox();
    sb1->setRange(m_min[0], m_max[0]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    sb1->setDecimals(m_decimal);
    connect(sb1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sb1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb2 = new ZDoubleSpinBox();
    sb2->setRange(m_min[1], m_max[1]);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    sb2->setDecimals(m_decimal);
    connect(sb2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sb2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb3 = new ZDoubleSpinBox();
    sb3->setRange(m_min[2], m_max[2]);
    sb3->setValue(m_value[2]);
    sb3->setSingleStep(m_step);
    sb3->setDecimals(m_decimal);
    connect(sb3, SIGNAL(valueChanged(double)), this, SLOT(setValue3(double)));
    connect(this, SIGNAL(value3Changed(double)), sb3, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sb3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb3);
        lo->addLayout(hlo);
      }
    }

  } else {
    ZDoubleSpinBoxWithSlider *sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sbws1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sbws1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sbws2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sbws2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws2);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws3 = new ZDoubleSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws3, SIGNAL(valueChanged(double)), this, SLOT(setValue3(double)));
    connect(this, SIGNAL(value3Changed(double)), sbws3, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sbws3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws3);
        lo->addLayout(hlo);
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//---------------------------------------------------------------------------------------------------------------

ZDVec4Parameter::ZDVec4Parameter(const QString &name, glm::dvec4 value, glm::dvec4 min, glm::dvec4 max, QObject *parent)
  : ZNumericVectorParameter<glm::dvec4>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

void ZDVec4Parameter::setValue1(double v)
{
  set(glm::dvec4(v, m_value[1], m_value[2], m_value[3]));
}

void ZDVec4Parameter::setValue2(double v)
{
  set(glm::dvec4(m_value[0], v, m_value[2], m_value[3]));
}

void ZDVec4Parameter::setValue3(double v)
{
  set(glm::dvec4(m_value[0], m_value[1], v, m_value[3]));
}

void ZDVec4Parameter::setValue4(double v)
{
  set(glm::dvec4(m_value[0], m_value[1], m_value[2], v));
}

void ZDVec4Parameter::beforeChange(glm::dvec4 &value)
{
  if (value[0] != m_value[0])
    emit value1Changed(value[0]);
  if (value[1] != m_value[1])
    emit value2Changed(value[1]);
  if (value[2] != m_value[2])
    emit value3Changed(value[2]);
  if (value[3] != m_value[3])
    emit value4Changed(value[3]);
}

QWidget *ZDVec4Parameter::actualCreateWidget(QWidget *parent)
{
  if (m_style == "COLOR") {
    return new ZClickableColorLabel(this, parent);
  }

  QWidget *w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout *lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    ZDoubleSpinBox *sb1 = new ZDoubleSpinBox();
    sb1->setRange(m_min[0], m_max[0]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    sb1->setDecimals(m_decimal);
    connect(sb1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sb1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb2 = new ZDoubleSpinBox();
    sb2->setRange(m_min[1], m_max[1]);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    sb2->setDecimals(m_decimal);
    connect(sb2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sb2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb3 = new ZDoubleSpinBox();
    sb3->setRange(m_min[2], m_max[2]);
    sb3->setValue(m_value[2]);
    sb3->setSingleStep(m_step);
    sb3->setDecimals(m_decimal);
    connect(sb3, SIGNAL(valueChanged(double)), this, SLOT(setValue3(double)));
    connect(this, SIGNAL(value3Changed(double)), sb3, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sb3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb3);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBox *sb4 = new ZDoubleSpinBox();
    sb4->setRange(m_min[3], m_max[3]);
    sb4->setValue(m_value[3]);
    sb4->setSingleStep(m_step);
    sb4->setDecimals(m_decimal);
    connect(sb4, SIGNAL(valueChanged(double)), this, SLOT(setValue4(double)));
    connect(this, SIGNAL(value4Changed(double)), sb4, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(3).isEmpty()) {
      lo->addWidget(sb4);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[3]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb4);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb4);
        lo->addLayout(hlo);
      }
    }
  } else {
    ZDoubleSpinBoxWithSlider *sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(double)), this, SLOT(setValue1(double)));
    connect(this, SIGNAL(value1Changed(double)), sbws1, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sbws1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws1);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws2, SIGNAL(valueChanged(double)), this, SLOT(setValue2(double)));
    connect(this, SIGNAL(value2Changed(double)), sbws2, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sbws2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws2);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws3 = new ZDoubleSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws3, SIGNAL(valueChanged(double)), this, SLOT(setValue3(double)));
    connect(this, SIGNAL(value3Changed(double)), sbws3, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sbws3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws3);
        lo->addLayout(hlo);
      }
    }
    ZDoubleSpinBoxWithSlider *sbws4 = new ZDoubleSpinBoxWithSlider(m_value[3], m_min[3], m_max[3], m_step,
                                                                  m_decimal, m_tracking, parent);
    connect(sbws4, SIGNAL(valueChanged(double)), this, SLOT(setValue4(double)));
    connect(this, SIGNAL(value4Changed(double)), sbws4, SLOT(setValue(double)));
    if (m_nameOfEachValue.at(3).isEmpty()) {
      lo->addWidget(sbws4);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[3]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws4);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws4);
        lo->addLayout(hlo);
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//-------------------------------------------------------------------------------------------------

ZIVec2Parameter::ZIVec2Parameter(const QString &name, glm::ivec2 value, glm::ivec2 min, glm::ivec2 max, QObject *parent)
  : ZNumericVectorParameter<glm::ivec2>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZIVec2Parameter::setValue1(int v)
{
  set(glm::ivec2(v, m_value[1]));
}

void ZIVec2Parameter::setValue2(int v)
{
  set(glm::ivec2(m_value[0], v));
}

void ZIVec2Parameter::beforeChange(glm::ivec2 &value)
{
  if (value[0] != m_value[0])
    emit value1Changed(value[0]);
  if (value[1] != m_value[1])
    emit value2Changed(value[1]);
}

QWidget *ZIVec2Parameter::actualCreateWidget(QWidget *parent)
{
  QWidget *w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout *lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    ZSpinBox *sb1 = new ZSpinBox();
    sb1->setRange(m_min[0], m_max[0]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    connect(sb1, SIGNAL(valueChanged(int)), this, SLOT(setValue1(int)));
    connect(this, SIGNAL(value1Changed(int)), sb1, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    ZSpinBox *sb2 = new ZSpinBox();
    sb2->setRange(m_min[1], m_max[1]);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    connect(sb2, SIGNAL(valueChanged(int)), this, SLOT(setValue2(int)));
    connect(this, SIGNAL(value2Changed(int)), sb2, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }

  } else {
    ZSpinBoxWithSlider *sbws1 = new ZSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                       m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(int)), this, SLOT(setValue1(int)));
    connect(this, SIGNAL(value1Changed(int)), sbws1, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sbws1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws1);
        lo->addLayout(hlo);
      }
    }
    ZSpinBoxWithSlider *sbws2 = new ZSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                       m_tracking, parent);
    connect(sbws2, SIGNAL(valueChanged(int)), this, SLOT(setValue2(int)));
    connect(this, SIGNAL(value2Changed(int)), sbws2, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sbws2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws2);
        lo->addLayout(hlo);
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//-------------------------------------------------------------------------------------------------

ZIVec3Parameter::ZIVec3Parameter(const QString &name, glm::ivec3 value, glm::ivec3 min, glm::ivec3 max, QObject *parent)
  : ZNumericVectorParameter<glm::ivec3>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZIVec3Parameter::setValue1(int v)
{
  set(glm::ivec3(v, m_value[1], m_value[2]));
}

void ZIVec3Parameter::setValue2(int v)
{
  set(glm::ivec3(m_value[0], v, m_value[2]));
}

void ZIVec3Parameter::setValue3(int v)
{
  set(glm::ivec3(m_value[0], m_value[1], v));
}

void ZIVec3Parameter::beforeChange(glm::ivec3 &value)
{
  if (value[0] != m_value[0])
    emit value1Changed(value[0]);
  if (value[1] != m_value[1])
    emit value2Changed(value[1]);
  if (value[2] != m_value[2])
    emit value3Changed(value[2]);
}

QWidget *ZIVec3Parameter::actualCreateWidget(QWidget *parent)
{
  QWidget *w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout *lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    ZSpinBox *sb1 = new ZSpinBox();
    sb1->setRange(m_min[0], m_max[0]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    connect(sb1, SIGNAL(valueChanged(int)), this, SLOT(setValue1(int)));
    connect(this, SIGNAL(value1Changed(int)), sb1, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    ZSpinBox *sb2 = new ZSpinBox();
    sb2->setRange(m_min[1], m_max[1]);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    connect(sb2, SIGNAL(valueChanged(int)), this, SLOT(setValue2(int)));
    connect(this, SIGNAL(value2Changed(int)), sb2, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }
    ZSpinBox *sb3 = new ZSpinBox();
    sb3->setRange(m_min[2], m_max[2]);
    sb3->setValue(m_value[2]);
    sb3->setSingleStep(m_step);
    connect(sb3, SIGNAL(valueChanged(int)), this, SLOT(setValue3(int)));
    connect(this, SIGNAL(value3Changed(int)), sb3, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sb3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb3);
        lo->addLayout(hlo);
      }
    }

  } else {
    ZSpinBoxWithSlider *sbws1 = new ZSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                       m_tracking, parent);
    connect(sbws1, SIGNAL(valueChanged(int)), this, SLOT(setValue1(int)));
    connect(this, SIGNAL(value1Changed(int)), sbws1, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sbws1);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[0]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws1);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws1);
        lo->addLayout(hlo);
      }
    }
    ZSpinBoxWithSlider *sbws2 = new ZSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                       m_tracking, parent);
    connect(sbws2, SIGNAL(valueChanged(int)), this, SLOT(setValue2(int)));
    connect(this, SIGNAL(value2Changed(int)), sbws2, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sbws2);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[1]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws2);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws2);
        lo->addLayout(hlo);
      }
    }
    ZSpinBoxWithSlider *sbws3 = new ZSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                       m_tracking, parent);
    connect(sbws3, SIGNAL(valueChanged(int)), this, SLOT(setValue3(int)));
    connect(this, SIGNAL(value3Changed(int)), sbws3, SLOT(setValue(int)));
    if (m_nameOfEachValue.at(2).isEmpty()) {
      lo->addWidget(sbws3);
    } else {
      QLabel *lb = new QLabel(m_nameOfEachValue[2]);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sbws3);
      } else {
        QHBoxLayout *hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sbws3);
        lo->addLayout(hlo);
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//-------------------------------------------------------------------------------------------------

ZIntSpanParameter::ZIntSpanParameter(const QString &name, glm::ivec2 value, int min, int max, QObject *parent)
  : ZNumericSpanParameter<glm::ivec2>(name, value, min, max, parent)
{
}


void ZIntSpanParameter::setLowerValue(int v)
{
  if (v <= m_value[1])
    set(glm::ivec2(v, m_value[1]));
}

void ZIntSpanParameter::setUpperValue(int v)
{
  if (v >= m_value[0])
    set(glm::ivec2(m_value[0], v));
}

void ZIntSpanParameter::beforeChange(glm::ivec2 &value)
{
  if (value[0] != m_value[0])
    emit lowerValueChanged(value[0]);
  if (value[1] != m_value[1])
    emit upperValueChanged(value[1]);
}

QWidget *ZIntSpanParameter::actualCreateWidget(QWidget *parent)
{
  ZSpanSliderWithSpinBox *spanSlider = new ZSpanSliderWithSpinBox(m_value[0], m_value[1], m_min, m_max, m_step, m_tracking, parent);
  connect(spanSlider, SIGNAL(lowerValueChanged(int)), this, SLOT(setLowerValue(int)));
  connect(spanSlider, SIGNAL(upperValueChanged(int)), this, SLOT(setUpperValue(int)));
  connect(this, SIGNAL(lowerValueChanged(int)), spanSlider, SLOT(setLowerValue(int)));
  connect(this, SIGNAL(upperValueChanged(int)), spanSlider, SLOT(setUpperValue(int)));
  connect(this, SIGNAL(rangeChanged(int,int)), spanSlider, SLOT(setDataRange(int,int)));
  return spanSlider;
}

void ZIntSpanParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}

ZFloatSpanParameter::ZFloatSpanParameter(const QString &name, glm::vec2 value, float min, float max, QObject *parent)
  : ZNumericSpanParameter<glm::vec2>(name, value, min, max, parent)
{
}

void ZFloatSpanParameter::setLowerValue(double v)
{
  if (static_cast<float>(v) <= m_value[1])
    set(glm::vec2(static_cast<float>(v), m_value[1]));
}

void ZFloatSpanParameter::setUpperValue(double v)
{
  if (static_cast<float>(v) >= m_value[0])
    set(glm::vec2(m_value[0], static_cast<float>(v)));
}

void ZFloatSpanParameter::beforeChange(glm::vec2 &value)
{
  if (value[0] != m_value[0])
    emit lowerValueChanged(value[0]);
  if (value[1] != m_value[1])
    emit upperValueChanged(value[1]);
}

QWidget *ZFloatSpanParameter::actualCreateWidget(QWidget *parent)
{
  ZDoubleSpanSliderWithSpinBox *spanSlider = new ZDoubleSpanSliderWithSpinBox(m_value[0], m_value[1], m_min, m_max, m_step, m_tracking, parent);
  connect(spanSlider, SIGNAL(lowerValueChanged(double)), this, SLOT(setLowerValue(double)));
  connect(spanSlider, SIGNAL(upperValueChanged(double)), this, SLOT(setUpperValue(double)));
  connect(this, SIGNAL(lowerValueChanged(double)), spanSlider, SLOT(setLowerValue(double)));
  connect(this, SIGNAL(upperValueChanged(double)), spanSlider, SLOT(setUpperValue(double)));
  connect(this, SIGNAL(rangeChanged(double,double)), spanSlider, SLOT(setDataRange(double,double)));
  return spanSlider;
}

void ZFloatSpanParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}


ZDoubleSpanParameter::ZDoubleSpanParameter(const QString &name, glm::dvec2 value, double min, double max, QObject *parent)
  : ZNumericSpanParameter<glm::dvec2>(name, value, min, max, parent)
{
}

void ZDoubleSpanParameter::setLowerValue(double v)
{
  if (v <= m_value[1])
    set(glm::dvec2(v, m_value[1]));
}

void ZDoubleSpanParameter::setUpperValue(double v)
{
  if (v >= m_value[0])
    set(glm::dvec2(m_value[0], v));
}

void ZDoubleSpanParameter::beforeChange(glm::dvec2 &value)
{
  if (value[0] != m_value[0])
    emit lowerValueChanged(value[0]);
  if (value[1] != m_value[1])
    emit upperValueChanged(value[1]);
}

QWidget *ZDoubleSpanParameter::actualCreateWidget(QWidget *parent)
{
  ZDoubleSpanSliderWithSpinBox *spanSlider = new ZDoubleSpanSliderWithSpinBox(m_value[0], m_value[1], m_min, m_max, m_step, m_tracking, parent);
  connect(spanSlider, SIGNAL(lowerValueChanged(double)), this, SLOT(setLowerValue(double)));
  connect(spanSlider, SIGNAL(upperValueChanged(double)), this, SLOT(setUpperValue(double)));
  connect(this, SIGNAL(lowerValueChanged(double)), spanSlider, SLOT(setLowerValue(double)));
  connect(this, SIGNAL(upperValueChanged(double)), spanSlider, SLOT(setUpperValue(double)));
  connect(this, SIGNAL(rangeChanged(double,double)), spanSlider, SLOT(setDataRange(double,double)));
  return spanSlider;
}

void ZDoubleSpanParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}
