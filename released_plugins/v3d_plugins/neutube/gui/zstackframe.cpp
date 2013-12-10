#include "zstackframe.h"
#include <QUndoCommand>
#include <iostream>
#include "tz_error.h"
#include "zstackview.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zimagewidget.h"
#include "settingdialog.h"
#include "zlocsegchain.h"
#include "tz_xml_utils.h"
#include "tz_string.h"
#include "ztraceproject.h"
#include "zstack.hxx"
#include "zcurve.h"
#include "z3dapplication.h"
#include "z3dwindow.h"
#include "zstackfile.h"
#include "zdoublevector.h"
#include "zfiletype.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include "zobjsmanagerwidget.h"
#include "neutubeconfig.h"
#include "zstackviewlocator.h"
#include "zstackstatistics.h"
#include "tz_stack_bwmorph.h"
#include "zobject3dscan.h"
#include "tz_stack_math.h"
#include "z3dcompositor.h"

using namespace std;

ZStackFrame::ZStackFrame(QWidget *parent, bool preparingModel) :
  QMdiSubWindow(parent), m_parentFrame(NULL),
  m_traceProject(NULL), m_isClosing(false),
  m_3dWindow(NULL)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  setAcceptDrops(true);
  m_settingDlg = new SettingDialog(this);
  m_manageObjsDlg = NULL;

  //m_presenter = new ZStackPresenter(this);
  //m_view = new ZStackView(this);
  if (preparingModel) {
    constructFrame();
  }
}

ZStackFrame::~ZStackFrame()
{
#ifdef _DEBUG_
  std::cout << "Frame " << this << " deconstructed" << std::endl;
#endif

  cleanUp();
}

void ZStackFrame::constructFrame()
{
  createPresenter();
  createView();
  createDocument();

  setView(m_view);
  m_view->prepareDocument();
  m_presenter->prepareView();
}

void ZStackFrame::detach3DWindow()
{
  m_3dWindow = NULL;
}

void ZStackFrame::createDocument()
{
  setDocument(tr1::shared_ptr<ZStackDoc>(new ZStackDoc));
}

void ZStackFrame::createPresenter()
{
  m_presenter = new ZStackPresenter(this);
}

void ZStackFrame::createView()
{
  m_view = new ZStackView(this);
}

void ZStackFrame::setDocument(tr1::shared_ptr<ZStackDoc> doc)
{
  m_doc = doc;
  m_doc->setParentFrame(this);
  connectSignalSlot();

  m_doc->updateTraceWorkspace(traceEffort(), traceMasked(),
                              xResolution(), yResolution(), zResolution());
  m_doc->updateConnectionTestWorkspace(xResolution(), yResolution(),
                                       zResolution(), unit(),
                                       reconstructDistThre(),
                                       reconstructSpTest(),
                                       crossoverTest());

  if (m_doc->hasStackData()) {
    m_presenter->optimizeStackBc();
    m_view->reset();
  }

  m_progressReporter.setProgressBar(m_view->progressBar());
  m_doc->setProgressReporter(&m_progressReporter);
}

void ZStackFrame::connectSignalSlot()
{
  connect(m_doc.get(), SIGNAL(locsegChainSelected(ZLocsegChain*)),
      this, SLOT(setLocsegChainInfo(ZLocsegChain*)));
  connect(m_doc.get(), SIGNAL(stackLoaded()), this, SIGNAL(stackLoaded()));
  connect(this, SIGNAL(stackLoaded()), this, SLOT(setupDisplay()));

  connect(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateChannelControl()));
  connect(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateThresholdSlider()));
  connect(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateSlider()));
  connect(m_doc.get(), SIGNAL(stackModified()),
          m_presenter, SLOT(updateStackBc()));
  connect(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateView()));

  connect(m_doc.get(), SIGNAL(objectModified()), m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(chainModified()),
          m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(swcModified()),
          m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(punctaModified()),
          m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(obj3dModified()),
          m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(strokeModified()), m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(cleanChanged(bool)),
          this, SLOT(changeWindowTitle(bool)));
  connect(m_doc.get(), SIGNAL(holdSegChanged()), m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(chainSelectionChanged(QList<ZLocsegChain*>,QList<ZLocsegChain*>)),
          m_view, SLOT(paintObject()));
  /*
  connect(m_doc.get(), SIGNAL(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)),
          m_view, SLOT(paintObject()));
          */
  connect(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged(
                                QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
          m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
          m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(chainVisibleStateChanged(ZLocsegChain*,bool)),
          m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(swcVisibleStateChanged(ZSwcTree*,bool)),
          m_view, SLOT(paintObject()));
  connect(m_doc.get(), SIGNAL(punctumVisibleStateChanged(ZPunctum*,bool)),
          m_view, SLOT(paintObject()));
  connect(m_view, SIGNAL(currentSliceChanged(int)),
          m_presenter, SLOT(processSliceChangeEvent(int)));
}

void ZStackFrame::disconnectAll()
{
  disconnect(m_doc.get(), SIGNAL(locsegChainSelected(ZLocsegChain*)),
      this, SLOT(setLocsegChainInfo(ZLocsegChain*)));
  disconnect(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateThresholdSlider()));
  disconnect(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateSlider()));
  disconnect(m_doc.get(), SIGNAL(stackModified()),
          m_presenter, SLOT(updateStackBc()));
  disconnect(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateView()));
  disconnect(m_doc.get(), SIGNAL(chainModified()),
          m_view, SLOT(updateView()));
  disconnect(m_doc.get(), SIGNAL(swcModified()),
          m_view, SLOT(updateView()));

  disconnect(m_doc.get(), SIGNAL(holdSegChanged()), m_view, SLOT(paintObject()));
  disconnect(m_doc.get(), SIGNAL(punctaModified()),
          m_view, SLOT(paintObject()));
  disconnect(m_doc.get(), SIGNAL(obj3dModified()),
             m_view, SLOT(paintObject()));
  disconnect(m_doc.get(), SIGNAL(cleanChanged(bool)),
             this, SLOT(changeWindowTitle(bool)));
  disconnect(m_doc.get(), SIGNAL(chainSelectionChanged(QList<ZLocsegChain*>,QList<ZLocsegChain*>)),
             m_view, SLOT(paintObject()));
  disconnect(m_doc.get(), SIGNAL(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)),
             m_view, SLOT(paintObject()));
  disconnect(m_doc.get(), SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
             m_view, SLOT(paintObject()));
  disconnect(m_doc.get(), SIGNAL(chainVisibleStateChanged(ZLocsegChain*,bool)),
             m_view, SLOT(paintObject()));
  disconnect(m_doc.get(), SIGNAL(swcVisibleStateChanged(ZSwcTree*,bool)),
             m_view, SLOT(paintObject()));
  disconnect(m_doc.get(), SIGNAL(punctumVisibleStateChanged(ZPunctum*,bool)),
             m_view, SLOT(paintObject()));
}

void ZStackFrame::takeScreenshot(const QString &filename)
{
  if (m_view != NULL)
    m_view->takeScreenshot(filename);
}

void ZStackFrame::cleanUp()
{
  disconnectAll();
  detachParentFrame();
  removeAllChildFrame();

  document()->setParentFrame(NULL);
  document()->setProgressReporter(NULL);

  // will be deleted by parent (this), so don't need, otherwise will crash
  if (m_view != NULL) {
    delete m_view;
    m_view = NULL;
  }

  if (m_presenter != NULL) {
    delete m_presenter;
    m_presenter = NULL;
  }

  if (m_settingDlg != NULL) {
    delete m_settingDlg;
    m_settingDlg = NULL;
  }
}

void ZStackFrame::loadStack(Stack *stack, bool isOwner)
{
  Q_ASSERT(m_doc != NULL);
  m_doc->loadStack(stack, isOwner);
  m_presenter->optimizeStackBc();
  m_view->reset();
}

void ZStackFrame::loadStack(ZStack *stack)
{
  Q_ASSERT(m_doc != NULL);
  m_doc->loadStack(stack);
  m_presenter->optimizeStackBc();
  m_view->reset();
  setWindowTitle(stack->sourcePath());
}

void ZStackFrame::setupDisplay()
{
  setWindowTitle(document()->stackSourcePath());
  m_statusInfo =  QString("%1 loaded").arg(document()->stackSourcePath());
  m_presenter->optimizeStackBc();
  m_view->reset();

  qDebug() << "ready(this) emitted";

  //To prevent strange duplcated signal emit
  disconnect(this, SIGNAL(stackLoaded()), this, SLOT(setupDisplay()));

  emit ready(this);
}

void ZStackFrame::setSizeHintOption(NeuTube::ESizeHintOption option)
{
  if (view() != NULL) {
    view()->setSizeHintOption(option);
  }
}

int ZStackFrame::readStack(const char *filePath)
{
  Q_ASSERT(m_doc != NULL);

  switch (ZFileType::fileType(filePath)) {
  case ZFileType::SWC_FILE:
    m_doc->readSwc(filePath);
    if (!m_doc->hasSwc()) {
      return ERROR_IO_READ;
    }

#ifdef _DEBUG_
    cout << "emit stackLoaded()" << endl;
#endif
    emit stackLoaded();
    break;
  case ZFileType::V3D_APO_FILE:
  case ZFileType::V3D_MARKER_FILE:
    m_doc->importPuncta(filePath);
#ifdef _DEBUG_
    cout << "emit stackLoaded()" << endl;
#endif
    emit stackLoaded();
    break;
  case ZFileType::LOCSEG_CHAIN_FILE: {
    QStringList files;
    files.push_back(filePath);
    m_doc->importLocsegChain(files);
#ifdef _DEBUG_
    cout << "emit stackLoaded()" << endl;
#endif
    emit stackLoaded();
    break;
  }
  case ZFileType::SWC_NETWORK_FILE:
    m_doc->loadSwcNetwork(filePath);
#ifdef _DEBUG_
    cout << "emit stackLoaded()" << endl;
#endif
    emit stackLoaded();
    break;
  case ZFileType::JSON_FILE:
    if (!m_doc->importSynapseAnnotation(filePath)) {
      return ERROR_IO_READ;
    }
    break;
  default:
    m_doc->readStack(filePath);
    break;
  }

  return SUCCESS;
}

int ZStackFrame::importImageSequence(const char *filePath)
{
  Q_ASSERT(m_doc != NULL);

  if (m_doc->importImageSequence(filePath)) {
    if (!m_doc->hasStackData()) {
      return ERROR_IO_READ;
    }
  }

  setWindowTitle(filePath);
  m_statusInfo =  QString("%1 loaded").arg(filePath);
  m_presenter->optimizeStackBc();
  m_view->reset();

  return SUCCESS;
}

int ZStackFrame::loadTraceProject(const char *filePath, QProgressBar *pb)
{
  xmlDocPtr doc;
  xmlNodePtr cur;

  doc = xmlParseFile(filePath);
  if (doc == NULL) {
    return 1;
  }

  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    xmlFreeDoc(doc);
    return 2;
  }

  if (Xml_Node_Is_Element(cur, "trace") == FALSE) {
    xmlFreeDoc(doc);
    return 3;
  }

  if (m_traceProject == NULL) {
    m_traceProject = new ZTraceProject(this);
  }

  m_traceProject->setProjFilePath(filePath);
  QString curdir = QString(filePath);
  curdir.truncate(curdir.lastIndexOf("/")+1);

  Stack_Document *stack_doc = NULL;

  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if (Xml_Node_Is_Element(cur, "data") == TRUE) {
      stack_doc = Xml_Stack_Document_R(doc, cur, curdir.toLocal8Bit().constData(), NULL);
      if (stack_doc != NULL) {
        Stack *stack = Import_Stack_Document(stack_doc);
        if (stack != NULL) {
          char *stackFile = Stack_Document_File_Path(stack_doc);
          loadStack(stack, true);
          document()->stack()->setSource(stack_doc);
          if (stack_doc->channel == -1) {
            setWindowTitle(stackFile);
          } else {
            QString title(stackFile);
            title += QString("_Channel_%1").arg(stack_doc->channel+1);
            setWindowTitle(title);
          }
          m_statusInfo =  QString("%1 loaded").arg(stackFile);
        } else {
          Kill_Stack_Document(stack_doc);
        }
        if (stack == NULL) {
          xmlFreeDoc(doc);
          return 5;
        }
      } else {
        xmlFreeDoc(doc);
        return 4;
      }
      if (pb != NULL) {
        if (pb->value() < 50) {
          pb->setValue(50);
        }
        QApplication::processEvents();
      }
    } else if (Xml_Node_Is_Element(cur, "output")) {
      xmlNodePtr child = cur->xmlChildrenNode;
      while (child != NULL) {
        if (Xml_Node_Is_Element(child, "workdir")) {
          char *value = Xml_Node_String_Value(doc, child);
          QString workdir = QString(value);
          free(value);
          if (!QDir::isAbsolutePath(workdir)) {
            workdir = curdir + workdir;
          }
          m_doc->setWorkdir(workdir.toLocal8Bit().constData());
          m_traceProject->setWorkDir(workdir.toLocal8Bit().constData());
        } else if (Xml_Node_Is_Element(child, "tube")) {
          char *prefix = Xml_Node_String_Value(doc, child);
          m_doc->setTubePrefix(prefix);
          free(prefix);
          char *screen =
              Xml_String_To_String(xmlGetProp(child,
                                              CONST_XML_STRING("screen")));
          m_doc->setBadChainScreen(screen);
          if (screen != NULL) {
            free(screen);
          }
          m_doc->importGoodTube();
          updateView();
        }
        child = child->next;
      }
    } else if (Xml_Node_Is_Element(cur, "object")) {
      xmlNodePtr child = cur->xmlChildrenNode;
      while (child != NULL) {
        if (Xml_Node_Is_Element(child, "swc")) {
          ZSwcTree *tree = new ZSwcTree;
          char *value = Xml_Node_String_Value(doc, child);
          char *filePath = value;
          if (!QDir::isAbsolutePath(QString(filePath))) {
            filePath = fullpath(curdir.toLocal8Bit().constData(), value, NULL);
            free(value);
          }

          if (fexist(filePath)) {
            tree->load(filePath);
            //m_doc->addSwcTree(tree);
            m_presenter->addDecoration(tree);
            m_traceProject->addDecoration(filePath, "swc");
          } else {
            QMessageBox::warning(this, tr("Problem"),
                    tr("The file %1 cannot be found.").arg(filePath),
                    QMessageBox::Ok);
          }
          free(filePath);
        } else if (Xml_Node_Is_Element(child, "tube")) {
          char *dirpath = Xml_Node_String_Value(doc, child);
          const char *pattern = strsplit(dirpath, '/', -1);
          QStringList filters;

          if (pattern != NULL) {
            if (pattern[0] == '\0') {
              pattern = "*.tb";
            }
          } else {
            pattern = "*.tb";
          }

          filters << pattern;

          QDir dir(dirpath);
          QStringList fileList = dir.entryList(filters);

          if (!fileList.isEmpty()) {
            QStringList::Iterator file;
            for (file = fileList.begin(); file != fileList.end(); ++file) {
              *file = dir.filePath(*file);
            }
            m_doc->importLocsegChain(fileList, ZStackDoc::ALL_TUBE,
                                     ZStackDoc::APPEND_OBJECT);
          }

          m_traceProject->addDecoration(dirpath, "tube");

          free(dirpath);
        }
        child = child->next;
      }
      updateView();
    }

    if (pb != NULL) {
      if (pb->value() < 75) {
        pb->setValue(75);
      }
      QApplication::processEvents();
    }

    cur = cur->next;
  }
  xmlFreeDoc(doc);

  synchronizeSetting();
  m_doc->updateTraceWorkspace(traceEffort(), traceMasked(),
                              xResolution(), yResolution(), zResolution());
  m_doc->updateConnectionTestWorkspace(xResolution(), yResolution(),
                                       zResolution(), unit(),
                                       reconstructDistThre(),
                                       reconstructSpTest(),
                                       crossoverTest());

  return 0;
}

void ZStackFrame::saveTraceProject(const QString &filePath,
                                     const QString &output,
                                     const QString &prefix)
{
  QFile file(filePath);
  file.open(QIODevice::WriteOnly);
  QTextStream stream(&file);
  stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  stream << "<trace version=\"1.0\">\n";
  stream << "<data>\n";
  stream << "<image type=\"tif\">\n";
  stream << "<url>" << document()->stack()->source()->firstUrl().c_str()
         << "</url>\n";
  stream << "<resolution>" << "<x>" << xResolution() << "</x>"
      << "<y>" << yResolution() << "</y>" << "<z>" << zResolution() << "</z>"
      << "</resolution>\n";
  stream << "</image>\n";
  stream << "</data>\n";

  if (!(output.isEmpty()) || !(prefix.isEmpty())) {
    stream << "<output>\n";
    if (!output.isEmpty()) {
      stream << "<workdir>" << output << "</workdir>\n";
    }
    if (!prefix.isEmpty()) {
      stream << "<tube>" << prefix << "</tube>\n";
    }
    stream << "</output>\n";
  }
  stream << "</trace>\n";
  file.close();
}

void ZStackFrame::setView(ZStackView *view)
{
  setWidget(view);
}

void ZStackFrame::closeEvent(QCloseEvent *event)
{
  if (m_doc->isUndoClean()) {
    m_isClosing = true;
    event->accept();

    qDebug() << "emit closed(this)";
    emit closed(this);
  } else {
    int ans =  QMessageBox::question(this, tr("Confirm"), tr("There are unsaved changes. Close anyway?"),
                                      QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Ok);
    if (ans == QMessageBox::Ok) {
      m_isClosing = true;
      event->accept();

      qDebug() << "emit closed(this)";
      emit closed(this);
    } else {
      event->ignore();
    }
  }
}

void ZStackFrame::resizeEvent(QResizeEvent *event)
{
  QMdiSubWindow::resizeEvent(event);

  qDebug() << "emit infoChanged()";
  emit infoChanged();
}

void ZStackFrame::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void ZStackFrame::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();

  load(urls);

  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    open3DWindow(this->parentWidget(), Z3DWindow::EXCLUDE_VOLUME);
  }
}

void ZStackFrame::setViewInfo(const QString &info)
{
  view()->setInfo(info);
}

void ZStackFrame::setViewInfo()
{
  view()->setInfo();
}

QString ZStackFrame::briefInfo() const
{
  return m_statusInfo;
}

QString ZStackFrame::info() const
{
  if ((document() != NULL) && view() != NULL) {
    QString info = document()->stack()->sourcePath() +
      QString("\n %1 x %2 => %3 x %4").arg(document()->stack()->width()).
      arg(document()->stack()->height()).
      arg(view()->imageWidget()->screenSize().width()).
      arg(view()->imageWidget()->screenSize().height());
    info += QString("\n zoom ratio: %1").arg(presenter()->zoomRatio());
    info += QString("\n") + document()->toString();
    info += QString("\n") + m_statusInfo;
    return info;
  } else {
    return QString("");
  }
}

void ZStackFrame::showSetting()
{
  if (m_settingDlg->exec() == QDialog::Accepted) {
    synchronizeDocument();
  }
}

void ZStackFrame::showManageObjsDialog()
{
  if (m_manageObjsDlg) {
    m_manageObjsDlg->raise();
    m_manageObjsDlg->show();
  } else {
    m_manageObjsDlg = new QDialog(this);
    m_manageObjsDlg->setWindowTitle("Objects");
    m_manageObjsDlg->setSizeGripEnabled(true);
    ZObjsManagerWidget* omw = new ZObjsManagerWidget(m_doc.get(), NULL);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(omw);
    m_manageObjsDlg->setLayout(layout);

    m_manageObjsDlg->show();
  }
}

double ZStackFrame::xResolution()
{
  return m_settingDlg->xResolution();
}

double ZStackFrame::yResolution()
{
  return m_settingDlg->yResolution();
}

double ZStackFrame::zResolution()
{
  return m_settingDlg->zResolution();
}

/*
double ZStackFrame::xReconstructScale()
{
  return m_settingDlg->xScale();
}

double ZStackFrame::zReconstructScale()
{
  return m_settingDlg->zScale();
}
*/
int ZStackFrame::traceEffort()
{
  return m_settingDlg->traceEffort();
}

bool ZStackFrame::traceMasked()
{
  return m_settingDlg->traceMasked();
}

double ZStackFrame::traceMinScore()
{
  return m_settingDlg->traceMinScore();
}

char ZStackFrame::unit()
{
  return m_settingDlg->unit();
}

double ZStackFrame::reconstructDistThre()
{
  return m_settingDlg->distThre();
}

int ZStackFrame::reconstructRootOption()
{
  return m_settingDlg->rootOption();
}

BOOL ZStackFrame::reconstructSpTest()
{
  if (m_settingDlg->reconstructEffort() == 0) {
    return FALSE;
  } else {
    return TRUE;
  }
}

bool ZStackFrame::crossoverTest()
{
  return m_settingDlg->crossoverTest();
}

bool ZStackFrame::singleTree()
{
  return m_settingDlg->singleTree();
}

bool ZStackFrame::removeOvershoot()
{
  return m_settingDlg->removeOvershoot();
}

void ZStackFrame::setResolution(const double *res)
{
  m_settingDlg->setResolution(res);
}

void ZStackFrame::addDecoration(ZStackDrawable *obj)
{
  presenter()->addDecoration(obj, true);
}

void ZStackFrame::clearDecoration()
{
  presenter()->removeAllDecoration();

  updateView();
}

void ZStackFrame::setBc(double greyScale, double greyOffset, int channel)
{
  presenter()->setStackBc(greyScale, greyOffset, channel);
}

void ZStackFrame::synchronizeSetting()
{
  m_settingDlg->setResolution(document()->stack()->resolution().voxelSize());
  m_settingDlg->setUnit(document()->stack()->resolution().unit());
}

void ZStackFrame::synchronizeDocument()
{
  document()->stack()->setResolution(m_settingDlg->xResolution(),
                                     m_settingDlg->yResolution(),
                                     m_settingDlg->zResolution(),
                                     m_settingDlg->unit());
  document()->setTraceMinScore(m_settingDlg->traceMinScore());
  document()->setReceptor(m_settingDlg->receptor(), m_settingDlg->useCone());
  if (hasProject()) {
    document()->setWorkdir(m_traceProject->workspacePath().toLocal8Bit().constData());
  }
  m_doc->updateTraceWorkspace(traceEffort(), traceMasked(),
                              xResolution(), yResolution(), zResolution());
  m_doc->updateConnectionTestWorkspace(xResolution(), yResolution(),
                                       zResolution(), unit(),
                                       reconstructDistThre(),
                                       reconstructSpTest(),
                                       crossoverTest());
}

void ZStackFrame::setLocsegChainInfo(ZLocsegChain *chain, QString prefix,
                                     QString suffix)
{
  m_statusInfo += prefix;
  if (chain != NULL) {
    if (chain->heldNode() < 0) {
      m_statusInfo = QString("Chain %1 %2; %3 segments;")
                     .arg(chain->id())
                     .arg(chain->source())
                     .arg(chain->length());
    } else {
      m_statusInfo =  QString("Chain %1 [%2] %3").arg(chain->id()).
                      arg(chain->heldNode()).arg(chain->source());
    }
  }
  m_statusInfo += suffix;

  updateInfo();
}

void ZStackFrame::changeWindowTitle(bool clean)
{
  QString title = windowTitle();
  if (clean) {
    if (title.endsWith(" *")) {
      title.resize(title.size()-2);
      setWindowTitle(title);
    }
  } else {
    if (!title.endsWith(" *")) {
      title += " *";
      setWindowTitle(title);
    }
  }
}

void ZStackFrame::keyPressEvent(QKeyEvent *event)
{
  if (m_presenter != NULL) {
    m_presenter->processKeyPressEvent(event);
  }
}

void ZStackFrame::updateInfo()
{
  qDebug() << "emit infoChanged()";
  emit infoChanged();
}

ZCurve ZStackFrame::curveToPlot(PlotSettings *settings, int option) const
{
  switch (option) {
  case 0:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 500.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_DOT);

  case 1:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 1.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_CORRCOEF);

  case 2:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 255.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_MEAN_SIGNAL);

  case 3:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 255.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_OUTER_SIGNAL);

  default:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 1.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_CORRCOEF);
  }
}

QStringList ZStackFrame::toStringList() const
{
  QStringList list;

  list += view()->toStringList();
  list += presenter()->toStringList();
  list += document()->toStringList();

  return list;
}


void ZStackFrame::updateView()
{
  m_view->redraw();
}

void ZStackFrame::undo()
{
  m_doc->undoStack()->undo();
}
void ZStackFrame::redo()
{
  m_doc->undoStack()->redo();
}

void ZStackFrame::pushUndoCommand(QUndoCommand *command)
{
  m_doc->undoStack()->push(command);
}

void ZStackFrame::pushBinarizeCommand()
{
  m_doc->executeBinarizeCommand(view()->getIntensityThreshold());
  /*
  QUndoCommand *cmd = new ZStackDocBinarizeCommand(
        document().get(), view()->getIntensityThreshold());
  pushUndoCommand(cmd);
  */
}

void ZStackFrame::pushBwsolidCommand()
{
  m_doc->executeBwsolidCommand();
  /*
  QUndoCommand *cmd = new ZStackDocBwSolidCommand(document().get());
  pushUndoCommand(cmd);
  */
}

void ZStackFrame::pushEnhanceLineCommand()
{
  m_doc->executeEnhanceLineCommand();
  //pushUndoCommand(new ZStackDocEnhanceLineCommand(document().get()));
}

void ZStackFrame::executeSwcRescaleCommand(const ZRescaleSwcSetting &setting)
{
  document()->executeSwcRescaleCommand(setting);
}

void ZStackFrame::executeAutoTraceCommand()
{
  document()->executeAutoTraceCommand();
}

void ZStackFrame::executeAutoTraceAxonCommand()
{
  document()->executeAutoTraceAxonCommand();
}

void ZStackFrame::executeWatershedCommand()
{
  document()->executeWatershedCommand();
}

void ZStackFrame::executeAddObjectCommand(ZDocumentable *obj,
                                          NeuTube::EDocumentableType type)
{
  document()->executeAddObjectCommand(obj, type);
}

double ZStackFrame::displayGreyMin(int c) const
{
  if (presenter()->greyScale(c) == 0.0) {
    return 0.0;
  }

  return -presenter()->greyOffset(c) / presenter()->greyScale(c);
}

double ZStackFrame::displayGreyMax(int c) const
{
  if (presenter()->greyScale(c) == 0.0) {
    return 0.0;
  }

  return (255.0 - presenter()->greyOffset(c)) / presenter()->greyScale(c);
}

void ZStackFrame::saveProjectAs(const QString &path)
{
  if (m_traceProject == NULL) {
    m_traceProject = new ZTraceProject(this);
  }

  document()->setWorkdir(path);
  m_traceProject->saveAs(path);
  m_doc->undoStack()->setClean();
}

void ZStackFrame::saveProject()
{
  if (m_traceProject == NULL) {
    m_traceProject = new ZTraceProject(this);
  }

  view()->progressBar()->setRange(0, 100);
  view()->progressBar()->show();
  QApplication::processEvents();
  m_traceProject->save();
  view()->progressBar()->setValue(view()->progressBar()->maximum());
  QApplication::processEvents();
  view()->progressBar()->hide();
  view()->setInfo("Project saved.");
  m_doc->undoStack()->setClean();
}

void ZStackFrame::importSwcAsReference(const QStringList &pathList)
{
  if (m_traceProject == NULL) {
    m_traceProject = new ZTraceProject(this);
  }

  document()->importSwc(pathList, ZStackDoc::APPEND_OBJECT);
  foreach (QString path, pathList) {
    m_traceProject->addDecoration(path, "swc");
  }
}

bool ZStackFrame::isReadyToSave() const
{
  if (!hasProject()) {
    return false;
  }

  if (m_traceProject->workspacePath().isEmpty()) {
    return false;
  }

  return true;
}

void ZStackFrame::exportSwc(const QString &filePath)
{
  document()->exportSwc(filePath.toStdString().c_str());

  if (document()->stack()->isSwc()) {
    m_doc->undoStack()->setClean();
    setWindowTitle(filePath);
  }
}

void ZStackFrame::exportPuncta(const QString &filePath)
{
  document()->exportPuncta(filePath.toStdString().c_str());
}

int ZStackFrame::exportSwcReconstruct(const QString &filePath, bool multiple)
{
  int swcNumber = 0;

  if (!multiple) {
    document()->exportSwcTree(filePath.toStdString().c_str());
    swcNumber = 1;
  } else {
    swcNumber = document()->exportMultipleSwcTree(filePath);
  }

  return swcNumber;
}

void ZStackFrame::exportVrml(const QString &filePath)
{
  document()->exportVrml(filePath.toStdString().c_str());
}

void ZStackFrame::exportTube(const QString &filePath)
{
  document()->exportBinary(filePath.toStdString().c_str());
}

void ZStackFrame::exportChainFileList(const QString &filePath)
{
  document()->exportChainFileList(filePath.toStdString().c_str());
}

void ZStackFrame::exportChainConnection(const QString &filePath)
{
  document()->exportLocsegChainConn(filePath.toStdString().c_str());
}

void ZStackFrame::exportChainConnectionFeat(const QString &filePath)
{
  document()->exportLocsegChainConnFeat(filePath.toStdString().c_str());
}

ZStack* ZStackFrame::getObjectMask()
{
  return view()->getObjectMask(1);
}

ZStack* ZStackFrame::getObjectMask(NeuTube::EColor color)
{
  return view()->getObjectMask(color, 1);
}

ZStack* ZStackFrame::getStrokeMask()
{
  return view()->getStrokeMask(1);
}

void ZStackFrame::exportObjectMask(const QString &filePath)
{
  view()->exportObjectMask(filePath.toStdString());
}

void ZStackFrame::exportObjectMask(
    NeuTube::EColor color, const QString &filePath)
{
  view()->exportObjectMask(color, filePath.toStdString());
}


void ZStackFrame::saveStack(const QString &filePath)
{
  document()->stack()->save(filePath.toStdString());
  document()->setStackSource(filePath.toStdString().c_str());
}

void ZStackFrame::displayActiveDecoration(bool enabled)
{
  m_view->displayActiveDecoration(enabled);
}

void ZStackFrame::setViewMode(ZInteractiveContext::ViewMode mode)
{
  presenter()->interactiveContext().setViewMode(mode);
}

void ZStackFrame::setObjectDisplayStyle(ZStackDrawable::Display_Style style)
{
  presenter()->setObjectStyle(style);
}

void ZStackFrame::setViewPortCenter(int x, int y, int z)
{
  presenter()->setViewPortCenter(x, y, z);
}

void ZStackFrame::viewRoi(int x, int y, int z, int radius)
{
  ZStackViewLocator locator;
  locator.setCanvasSize(view()->imageWidget()->canvasSize().width(),
                        view()->imageWidget()->canvasSize().height());
  QRect viewPort = locator.getViewPort(x, y, radius);
  presenter()->setZoomRatio(
        locator.getZoomRatio(viewPort.width(), viewPort.height()));
  presenter()->setViewPortCenter(x, y, z);
}

void ZStackFrame::hideObject()
{
  presenter()->setObjectVisible(false);
}

void ZStackFrame::showObject()
{
  presenter()->setObjectVisible(true);
}

Z3DWindow* ZStackFrame::open3DWindow(QWidget *parent, Z3DWindow::EInitMode mode)
{
  if (Z3DApplication::app()->is3DSupported()) {
    if (m_3dWindow == NULL) {
      m_3dWindow = new Z3DWindow(document(), mode, false, parent);
      connect(m_3dWindow, SIGNAL(destroyed()), this, SLOT(detach3DWindow()));
      if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
        m_3dWindow->getCompositor()->setBackgroundFirstColor(glm::vec3(1, 1, 1));
        m_3dWindow->getCompositor()->setBackgroundSecondColor(glm::vec3(1, 1, 1));
      }
    }

    QRect screenRect = QApplication::desktop()->screenGeometry();
    m_3dWindow->setGeometry(screenRect.width() / 10, screenRect.height() / 10,
                            screenRect.width() - screenRect.width() / 5,
                            screenRect.height() - screenRect.height() / 5);
    m_3dWindow->show();
    m_3dWindow->raise();

    return m_3dWindow;
  } else {
    QMessageBox::critical(this, tr("3D functions are disabled"),
                          Z3DApplication::app()->getErrorMessage());
  }

  return NULL;
}

void ZStackFrame::load(const QList<QUrl> &urls)
{
  m_doc->loadFileList(urls);
}

void ZStackFrame::load(const QStringList &fileList)
{
  m_doc->loadFileList(fileList);
}

void ZStackFrame::load(const QString &filePath)
{
  m_doc->loadFile(filePath, true);
}

/*
void ZStackFrame::importStackMask(const string &filePath)
{
  ZStackFile file;
  file.import(filePath);
  setStackMask(file.readStack());
}

void ZStackFrame::setStackMask(ZStack *stack)
{
  m_doc->setStackMask(stack);
  updateView();
}
*/

ZStackFrame* ZStackFrame::spinoffStackSelection(const vector<int> &selected)
{
  if (document()->hasStackData()) {
    int channelNumber = document()->stack()->channelNumber();
    std::vector<std::vector<double> > selectedColor =
        ZDoubleVector::reshape(selected, channelNumber);

    return spinoffStackSelection(selectedColor);
  }

  return NULL;
}

ZStackFrame* ZStackFrame::spinoffStackSelection(
    const vector<vector<double> > &selected)
{
  ZStackFrame *frame = NULL;

  if (m_doc->hasStackData()) {
    frame = new ZStackFrame();
    ZStack *substack = m_doc->stack()->createSubstack(selected);

    frame->document()->loadStack(substack);
    frame->view()->reset();
  }

  return frame;
}

void ZStackFrame::invertStack()
{
  if (m_doc->hasStackData()) {
    m_doc->invert();
  }
}

void ZStackFrame::detachParentFrame()
{
  if (m_parentFrame != NULL) {
    m_parentFrame->removeChildFrame(this);
    m_parentFrame = NULL;
  }
}

void ZStackFrame::removeChildFrame(ZStackFrame *frame)
{
  if (m_childFrameList.removeOne(frame)) {
    frame->m_parentFrame = NULL;
  }
}

void ZStackFrame::removeAllChildFrame()
{
  foreach (ZStackFrame *childFrame, m_childFrameList) {
    childFrame->m_parentFrame = NULL;
  }
  m_childFrameList.clear();
}

void ZStackFrame::setParentFrame(ZStackFrame *frame)
{
  detachParentFrame();
  m_parentFrame = frame;
  if (m_parentFrame != NULL) {
    m_parentFrame->m_childFrameList.append(this);
  }
}

void ZStackFrame::findLoopInStack()
{
  document()->findLoop();
}

void ZStackFrame::bwthin()
{
  document()->bwthin();
}

void ZStackFrame::importMask(const QString &filePath)
{
  ZStack *stack = NULL;
  if (ZFileType::fileType(filePath.toStdString()) == ZFileType::PNG_FILE) {
    QImage image;
    image.load(filePath);
    stack = new ZStack(GREY, image.width(), image.height(), 1, 1);

    size_t offset = 0;
    for (int y = 0; y < image.height(); ++y) {
      for (int x = 0; x < image.width(); ++x) {
        QRgb rgb = image.pixel(x, y);
        if (qRed(rgb) == 255 && qGreen(rgb) == 255 && qBlue(rgb) == 255) {
          stack->array8()[offset] = 0;
        } else {
          if (image.hasAlphaChannel()) {
            stack->array8()[offset] = qAlpha(rgb);
          } else {
            stack->array8()[offset] = qRed(rgb);
          }
        }
        ++offset;
      }
    }
  } else {
    ZStackFile stackFile;
    stackFile.import(filePath.toStdString());
    stack = stackFile.readStack();
  }

  if (stack != NULL) {
    if (stack->channelNumber() == 1 && stack->kind() == GREY) {
      ZObject3d *obj = new ZObject3d;
      obj->setColor(QColor(255, 0, 0, 128));
      if (obj->loadStack(stack->c_stack(0))) {
        executeAddObjectCommand(obj, NeuTube::Documentable_OBJ3D);
      } else {
        delete obj;
        report("Loading mask failed", "Cannot convert the image into mask",
               ZMessageReporter::Error);
      }
    } else {
      report("Loading mask failed", "Must be single 8-bit image",
             ZMessageReporter::Error);
    }
    delete stack;
  }
}

void ZStackFrame::importSwc(const QString &filePath)
{
  load(filePath);
}

void ZStackFrame::importSobj(const QStringList &fileList)
{
  ZStackFile stackFile;
  vector<string> fileArray;
  foreach(QString file, fileList) {
    fileArray.push_back(file.toStdString());
  }

  stackFile.importSobjList(fileArray);

  ZStack *stack = stackFile.readStack();
  loadStack(stack);
}

void ZStackFrame::autoBcAdjust()
{
  document()->startProgress();
  for (int i = 0; i < document()->stack()->channelNumber(); ++i) {
    document()->advanceProgress(0.1);
    double lower, upper;
    ZStackStatistics::getGreyMapHint(*document()->stack(), i,
                                     &lower, &upper);
    document()->advanceProgress(0.2);
    double scale = 1.0;
    double offset = 0.0;
    if (upper > lower) {
      scale = 255.0 / (upper - lower);
    }

    offset = -scale * lower;

    setBc(scale, offset, i);
  }
  document()->endProgress();
  updateView();
}

void ZStackFrame::loadRoi(const QString &filePath)
{
  ZStackFile stackFile;
  stackFile.import(filePath.toStdString());

  ZStack *stack = stackFile.readStack();

  if (stack->kind() != GREY) {
    std::cerr << "Ignore invalid ROI" <<std::endl;
    delete stack;
    stack = NULL;
  }

  if (stack != NULL) {
    Stack *mask = C_Stack::clone(stack->c_stack());

    ZObject3dScan *obj = new ZObject3dScan;

    Stack_Binarize(mask);
    Stack_Not(mask, mask);

    obj->loadStack(mask);

#ifdef _DEBUG_
    obj->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

    //obj->print();
    obj->duplicateAcrossZ(document()->stack()->depth());

    obj->setColor(16, 16, 16, 64);

    obj->setTarget(ZStackDrawable::OBJECT_CANVAS);
    addDecoration(obj);
    updateView();

    C_Stack::kill(mask);

    delete stack;
  }
}
