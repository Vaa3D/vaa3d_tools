#ifndef ZSTRINGPARAMETER_H
#define ZSTRINGPARAMETER_H

#include "zparameter.h"

class QLineEdit;

class ZStringParameter : public ZSingleValueParameter<QString>
{
  Q_OBJECT
public:
  explicit ZStringParameter(const QString &name, const QString &str, QWidget *parent = 0);

signals:
  void strChanged(QString str);
  
public slots:
  void setContent(QString str);

protected:
  virtual QWidget* actualCreateWidget(QWidget *parent);
  virtual void beforeChange(QString &value);
};

#endif // ZSTRINGPARAMETER_H
