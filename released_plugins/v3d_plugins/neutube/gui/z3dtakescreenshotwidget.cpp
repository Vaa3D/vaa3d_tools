#include "z3dtakescreenshotwidget.h"
#include "z3dgl.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include <QDateTime>
#include "zselectfilewidget.h"
#include "z3dgpuinfo.h"

Z3DTakeScreenShotWidget::Z3DTakeScreenShotWidget(bool group, QWidget *parent)
  : QWidget(parent)
  , m_group(group)
  , m_mode("Mode")
  , m_captureStereoImage("Capture Stereo Image", false)
  , m_stereoImageType("Stereo Image Type")
  , m_useWindowSize("Use Window Size as Image Size", true)
  , m_customSize("Use Custom Image Size", glm::ivec2(1920, 1080), glm::ivec2(6, 6),
                 glm::ivec2(Z3DGpuInfoInstance.getMaxTextureSize()))
  , m_namePrefix("filename prefix", "")
  , m_nextNumber(1)
  , m_axis("Rotate Around Axis")
  , m_clockwise("Clockwise Rotate", false)
  , m_timeInSecond("Total Time (seconds)", 30, 1, 600)
  , m_framePerSecond("Frame per Second", 30, 24, 60)
{
  m_customSize.setStyle("SPINBOX");
  QList<QString> names;
  names.push_back("Width:");
  names.push_back("Height:");
  m_customSize.setNameForEachValue(names);
  m_mode.addOptions("Capture Single Image", "Capture Rotating Image sequence");
  m_mode.select("Capture Single Image");
  m_stereoImageType.addOptions("Full Side-By-Side", "Half Side-By-Side");
  m_stereoImageType.select("Full Side-By-Side");
  m_axis.addOptions("X", "Y", "Z");
  m_axis.select("Y");
  QDateTime data = QDateTime::currentDateTime();
  QString prefix = QString("neuTubeCapture") + data.toString("yyyyMMdd") + QString("_");
  m_namePrefix.set(prefix);
  createWidget();
  connect(&m_mode, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));
  connect(&m_captureStereoImage, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));
  connect(&m_useWindowSize, SIGNAL(valueChanged()), this, SLOT(updateImageSizeWidget()));
  connect(&m_namePrefix, SIGNAL(valueChanged()), this, SLOT(prefixChanged()));
  connect(m_captureButton, SIGNAL(clicked()), this, SLOT(captureButtonPressed()));
  connect(m_captureSequenceButton, SIGNAL(clicked()), this, SLOT(captureSequenceButtonPressed()));
  adjustWidget();
  updateImageSizeWidget();
}

void Z3DTakeScreenShotWidget::captureButtonPressed()
{
  QString filepath;
  if (m_useManualName->isChecked()) {
    filepath = QFileDialog::getSaveFileName(
          this, "Save Capture", m_lastFName, "",
          NULL/*, QFileDialog::DontUseNativeDialog*/);
    if (!filepath.isEmpty()) {
      m_lastFName = filepath;
    }
  } else {  // auto generate file name based on folder and prefix
    if (m_folderWidget->getSelectedDirectory().isEmpty()) {
      QMessageBox::critical(this, "Output Folder do not exist", "Output Folder do not exist");
      return;
    }
    QDir dir(m_folderWidget->getSelectedDirectory());
    while (true) {
      QString filename = QString("%1%2.tif").arg(m_namePrefix.get()).arg(m_nextNumber++);
      if (dir.exists(filename))
        continue;
      filepath = dir.filePath(filename);
      break;
    }
  }

  Z3DScreenShotType sst;
  if (m_captureStereoImage.get()) {
    if (m_stereoImageType.isSelected("Half Side-By-Side"))
      sst = HalfSideBySideStereoView;
    else
      sst = FullSideBySideStereoView;
  } else
    sst = MonoView;

  if (m_useWindowSize.get())
    emit takeScreenShot(filepath, sst);
  else {
    glm::ivec2 size = m_customSize.get();
    emit takeScreenShot(filepath, size.x, size.y, sst);
  }
}

void Z3DTakeScreenShotWidget::captureSequenceButtonPressed()
{
  if (m_folderWidget->getSelectedDirectory().isEmpty()) {
    QMessageBox::critical(this, "Output Folder do not exist", "Output Folder do not exist");
    return;
  }
  QDir dir(m_folderWidget->getSelectedDirectory());
  glm::vec3 axis;
  if (m_axis.isSelected("X"))
    axis = glm::vec3(1.f, 0.f, 0.f);
  else if (m_axis.isSelected("Y"))
    axis = glm::vec3(0.f, 1.f, 0.f);
  else
    axis = glm::vec3(0.f, 0.f, 1.f);
  int numFrame = m_framePerSecond.get() * m_timeInSecond.get();

  Z3DScreenShotType sst;
  if (m_captureStereoImage.get()) {
    if (m_stereoImageType.isSelected("Half Side-By-Side"))
      sst = HalfSideBySideStereoView;
    else
      sst = FullSideBySideStereoView;
  } else
    sst = MonoView;

  if (m_useWindowSize.get())
    emit takeSeriesScreenShot(dir, m_namePrefix.get(), axis, m_clockwise.get(), numFrame, sst);
  else {
    glm::ivec2 size = m_customSize.get();
    emit takeSeriesScreenShot(dir, m_namePrefix.get(), axis, m_clockwise.get(), numFrame,
                              size.x, size.y, sst);
  }
}

void Z3DTakeScreenShotWidget::setFileNameSource()
{
  if (m_useManualName->isChecked()) {
    m_folderWidget->setEnabled(false);
    m_namePrefix.setEnabled(false);
  } else {
    m_folderWidget->setEnabled(true);
    m_namePrefix.setEnabled(true);
  }
}

void Z3DTakeScreenShotWidget::updateImageSizeWidget()
{
  if (m_useWindowSize.get())
    m_customSize.setEnabled(false);
  else
    m_customSize.setEnabled(true);
}

void Z3DTakeScreenShotWidget::prefixChanged()
{
  m_nextNumber = 1;
}

void Z3DTakeScreenShotWidget::adjustWidget()
{
  if (m_mode.isSelected("Capture Single Image")) {
    m_useAutoName->setVisible(true);
    m_useManualName->setVisible(true);
    m_captureButton->setVisible(true);
    m_axis.setVisible(false);
    m_clockwise.setVisible(false);
    m_timeInSecond.setVisible(false);
    m_framePerSecond.setVisible(false);
    m_captureSequenceButton->setVisible(false);
    setFileNameSource();
  } else {
    m_useAutoName->setVisible(false);
    m_useManualName->setVisible(false);
    m_captureButton->setVisible(false);
    m_axis.setVisible(true);
    m_clockwise.setVisible(true);
    m_timeInSecond.setVisible(true);
    m_framePerSecond.setVisible(true);
    m_captureSequenceButton->setVisible(true);
    m_folderWidget->setEnabled(true);
    m_namePrefix.setEnabled(true);
  }
  m_stereoImageType.setVisible(m_captureStereoImage.get());
}

void Z3DTakeScreenShotWidget::createWidget()
{
  QVBoxLayout *lo = new QVBoxLayout;

  QHBoxLayout *hlo = new QHBoxLayout;
  hlo->addWidget(m_mode.createNameLabel());
  QWidget* wg = m_mode.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->addWidget(m_captureStereoImage.createNameLabel());
  wg = m_captureStereoImage.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->addWidget(m_stereoImageType.createNameLabel());
  wg = m_stereoImageType.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->addWidget(m_useWindowSize.createNameLabel());
  wg = m_useWindowSize.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->addWidget(m_customSize.createNameLabel());
  wg = m_customSize.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  m_useManualName = new QRadioButton("Manual specify output name", this);
  connect(m_useManualName, SIGNAL(clicked()), this, SLOT(setFileNameSource()));
  lo->addWidget(m_useManualName);
  m_useAutoName = new QRadioButton("Auto generate output name", this);
  connect(m_useAutoName, SIGNAL(clicked()), this, SLOT(setFileNameSource()));
  lo->addWidget(m_useAutoName);

  hlo = new QHBoxLayout;
  hlo->addWidget(m_axis.createNameLabel());
  wg = m_axis.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->addWidget(m_clockwise.createNameLabel());
  wg = m_clockwise.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->addWidget(m_timeInSecond.createNameLabel());
  wg = m_timeInSecond.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->addWidget(m_framePerSecond.createNameLabel());
  wg = m_framePerSecond.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  int left;
  int top;
  int right;
  int bottom;
  hlo->getContentsMargins(&left, &top, &right, &bottom);
  hlo->setContentsMargins(left+20, top, right, bottom);
  m_folderWidget = new ZSelectFileWidget(ZSelectFileWidget::DIRECTORY, "output folder:", QString(),
                                         QBoxLayout::LeftToRight, this);
  hlo->addWidget(m_folderWidget);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->setContentsMargins(left+20, top, right, bottom);
  hlo->addWidget(m_namePrefix.createNameLabel());
  wg = m_namePrefix.createWidget();
  wg->setMinimumWidth(125);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  m_captureButton = new QPushButton(tr("Capture"), this);
  m_captureButton->setIcon(QIcon(":/images/screenshot.png"));
  m_captureButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  lo->addWidget(m_captureButton, 0, Qt::AlignHCenter | Qt::AlignVCenter);

  m_captureSequenceButton = new QPushButton(tr("Capture Sequence"), this);
  m_captureSequenceButton->setIcon(QIcon(":/images/videocam.png"));
  m_captureSequenceButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  lo->addWidget(m_captureSequenceButton, 0, Qt::AlignHCenter | Qt::AlignVCenter);

  m_useAutoName->click();

  if (m_group) {
    m_groupBox = new QGroupBox(tr("capture"), this);
    m_groupBox->setLayout(lo);
    hlo = new QHBoxLayout;
    hlo->addWidget(m_groupBox);
    setLayout(hlo);
  } else {
    setLayout(lo);
  }
}
