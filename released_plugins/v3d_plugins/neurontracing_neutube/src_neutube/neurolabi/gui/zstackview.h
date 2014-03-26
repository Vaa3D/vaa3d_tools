/**@file zstackview.h
 * @brief Stack view
 * @author Ting Zhao
 */

#ifndef _ZSTACKVIEW_H_
#define _ZSTACKVIEW_H_

#include <QImage>
#include <QWidget>

#include "zstackframe.h"
#include "zparameter.h"
#include <vector>
#include "tz_image_lib_defs.h"
#include "neutube.h"
#ifdef __GLIBCXX__
#include <tr1/memory>
#else
#include <memory>
#endif
#include "zpaintbundle.h"

class ZStackPresenter;
class QSlider;
class ZImageWidget;
class QVBoxLayout;
class QHBoxLayout;
class QSpinBox;
class ZImage;
class QLabel;
class ZSlider;
class QMenu;
class QPushButton;
class QProgressBar;
class QRadioButton;
class ZStackDoc;
class ZStack;

class ZStackView : public QWidget {
  Q_OBJECT

public:
  ZStackView(ZStackFrame *parent = 0);
  virtual ~ZStackView();

  void reset();
  void updateImageScreen();
  void updateScrollControl();
  void hideThresholdControl();

  QSize sizeHint() const;
  inline ZImageWidget* imageWidget() const { return m_imageWidget; }
  inline std::tr1::shared_ptr<ZStackDoc> buddyDocument()
  { return (m_parent != NULL) ? m_parent->document() :
                                std::tr1::shared_ptr<ZStackDoc>(); }
  inline const std::tr1::shared_ptr<ZStackDoc> buddyDocument() const
  { return (m_parent != NULL) ? m_parent->document() :
                                std::tr1::shared_ptr<ZStackDoc>(); }

  inline ZStackPresenter* buddyPresenter()
  { return (m_parent != NULL) ? m_parent->presenter() : NULL; }
  inline ZImageWidget* screen() { return m_imageWidget; }
  inline QProgressBar* progressBar() { return m_progress; }

  int maxSliceIndex();
  int sliceIndex();
  void setSliceIndex(int slice);
  void stepSlice(int step);
  int threshold();
  ZStack *stackData();

  void connectSignalSlot();

  //set up the view after the document is ready
  void prepareDocument();

  virtual void resizeEvent(QResizeEvent *event);

  QStringList toStringList() const;

  void setImageWidgetCursor(const QCursor &cursor);
  void setScreenCursor(const QCursor &cursor);
  void resetScreenCursor();

  void setThreshold(int thre);
  int getIntensityThreshold();

  //void open3DWindow();
  void takeScreenshot(const QString &filename);

  bool isDepthChangable();

  void paintStackBuffer();
  void paintMaskBuffer();
  void paintObjectBuffer();
  void paintActiveDecorationBuffer();

  ZStack* getObjectMask(uint8_t maskValue);
  /*!
   * \brief Get object mask of a certain color
   */
  ZStack* getObjectMask(NeuTube::EColor color, uint8_t maskValue);
  ZStack* getStrokeMask(uint8_t maskValue);


  void exportObjectMask(const std::string &filePath);
  void exportObjectMask(NeuTube::EColor color, const std::string &filePath);

  inline void setSizeHintOption(NeuTube::ESizeHintOption option) {
    m_sizeHintOption = option;
  }

public slots:
  void updateView();
  void redraw();
  //void updateData(int nslice, int threshold = -1);
  //void updateData();
  //void updateSlice(int nslide);
  //void viewThreshold(int threshold);
  void updateThresholdSlider();
  void updateSlider();
  void updateChannelControl();

  void paintStack();
  void paintMask();
  void paintObject();
  void paintActiveDecoration();

  void mouseReleasedInImageWidget(QMouseEvent *event);
  void mousePressedInImageWidget(QMouseEvent *event);
  void mouseMovedInImageWidget(QMouseEvent *event);
  void mouseDoubleClickedInImageWidget(QMouseEvent *event);
  void mouseRolledInImageWidget(QWheelEvent *event);

  void popLeftMenu(const QPoint &pos);
  void popRightMenu(const QPoint &pos);

  void showContextMenu(QMenu *menu, const QPoint &pos);

  QMenu* leftMenu();
  QMenu* rightMenu();

  void setInfo(QString info);
  void autoThreshold();

  void displayActiveDecoration(bool display = true);

signals:
  void currentSliceChanged(int);

public:
  static QImage::Format stackKindToImageFormat(int kind);
  double getZoomRatio() const;
  void setInfo();

private:
  void updateCanvas();
  void updateMaskCanvas();
  void updateObjectCanvas();
  void updateActiveDecorationCanvas();

  //help functions
  void paintSingleChannelStackSlice(ZStack *stack, int slice);
  void paintMultipleChannelStackSlice(ZStack *stack, int slice);
  void paintSingleChannelStackMip(ZStack *stack);
  void paintMultipleChannelStackMip(ZStack *stack);

private:
  ZStackFrame *m_parent;
  ZSlider *m_depthControl;
  //QSpinBox *m_spinBox;
  QLabel *m_infoLabel;
  QLabel *m_activeLabel;
  ZImage *m_image;
  ZImage *m_imageMask;
  ZImage *m_objectCanvas;
  ZImage *m_activeDecorationCanvas;
  ZImageWidget *m_imageWidget;
  QVBoxLayout *m_layout;
  QHBoxLayout *m_topLayout;
  QHBoxLayout *m_secondTopLayout;
  QHBoxLayout *m_channelControlLayout;
  QHBoxLayout *m_ctrlLayout;
  bool m_scrollEnabled;

  QProgressBar *m_progress;
  ZSlider *m_thresholdSlider;
  QPushButton *m_autoThreButton;

  // used to turn on or off each channel
  std::vector<ZBoolParameter*> m_chVisibleState;

  NeuTube::ESizeHintOption m_sizeHintOption;

  ZPaintBundle m_paintBundle;
};

#endif
