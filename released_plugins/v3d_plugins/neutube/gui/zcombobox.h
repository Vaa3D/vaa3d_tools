#ifndef ZCOMBOBOX_H
#define ZCOMBOBOX_H

#include <QComboBox>

// prevent combobox steal mouse wheel event in a scroll area

class ZComboBoxEventFilter : public QObject
{
public:
  ZComboBoxEventFilter(QObject *parent = 0);
protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);
};

class ZComboBox : public QComboBox
{
  Q_OBJECT
public:
  explicit ZComboBox(QWidget *parent = 0);

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;

public slots:
  void addItemSlot(const QString &text);
  void removeItemSlot(const QString &text);

protected:
  virtual void focusInEvent(QFocusEvent *event);
  virtual void focusOutEvent(QFocusEvent *event);
};

#endif // ZCOMBOBOX_H
