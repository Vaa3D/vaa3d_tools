#ifndef ZCOLORMAPEDITOR_H
#define ZCOLORMAPEDITOR_H

#include <QWidget>
class ZColorMapParameter;
class QLabel;
class QDoubleSpinBox;
class QPushButton;
class QCheckBox;

class ZColorMapWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZColorMapWidget(ZColorMapParameter *colorMap, QWidget *parent = 0);

  void editLColor(size_t index);
  void editRColor(size_t index);
  
public slots:
  void updateIntensityScreenWidth();

protected:
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void mouseMoveEvent(QMouseEvent *e);
  virtual void mouseReleaseEvent(QMouseEvent *e);
  virtual void mouseDoubleClickEvent(QMouseEvent *e);
  virtual void contextMenuEvent(QContextMenuEvent *e);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual bool event(QEvent *e);
  virtual void paintEvent(QPaintEvent *);
  virtual void resizeEvent(QResizeEvent *);
  virtual QSize sizeHint() const;

  enum FindKeyResult
  {
    NONE,
    LEFT,
    RIGHT
  };

  FindKeyResult findkey(const QPoint &pos, size_t &index, bool includeBoundKey = false) const;
  double intensityToScreenXPosition(double intensity) const;
  double screenXPositionToIntensity(double x) const;
  QRect sliderBounds(size_t index) const;

  ZColorMapParameter *m_colorMap;

  int m_sliderWidth;
  int m_sliderHeight;
  double m_intensityScreenWidth;
  bool m_isDragging;
  int m_dragStartX;
  size_t m_pressedIndex;
  int m_pressedPosX;
};

class ZColorMapEditor : public QWidget
{
  Q_OBJECT
public:
  explicit ZColorMapEditor(ZColorMapParameter *colorMap, QWidget *parent = 0);

public slots:
  void updateFromColorMap();
  void setDomainMin(double min);
  void setDomainMax(double max);
  void fitDomainToDataRange();
private:

  void createWidget();

  ZColorMapParameter *m_colorMap;
  ZColorMapWidget *m_colorMapWidget;
  QLabel *m_dataMinNameLabel;
  QLabel *m_dataMinValueLabel;
  QLabel *m_dataMaxNameLabel;
  QLabel *m_dataMaxValueLabel;
  QLabel *m_domainMinNameLabel;
  QLabel *m_domainMaxNameLabel;
  QDoubleSpinBox *m_domainMinSpinBox;
  QDoubleSpinBox *m_domainMaxSpinBox;
  QPushButton *m_fitDomainToDataButton;
  QCheckBox *m_rescaleKeys;
};

#endif // ZCOLORMAPEDITOR_H
