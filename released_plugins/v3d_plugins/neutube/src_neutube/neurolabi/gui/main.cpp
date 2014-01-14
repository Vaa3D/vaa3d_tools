#include <iostream>
#include <cstring>
#include <QApplication>
#include <QDir>
#include "mainwindow.h"
#include "QsLog/QsLog.h"
#include "QsLog/QsLogDest.h"
#include "zcommandline.h"
#include "zerror.h"
#include "z3dapplication.h"

#if !defined(QT_NO_DEBUG)
#include "ztest.h"
#endif

#include "tz_utilities.h"
#include "neutubeconfig.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStack>
#include <QPointer>
// thanks to Daniel Price for this workaround
struct MacEventFilter : public QObject
{
  QStack<QPointer<QWidget> > m_activationstack; // stack of widgets to re-active on dialog close.

  explicit MacEventFilter(QObject *parent = NULL)
    : QObject(parent)
  {}

  virtual bool eventFilter(QObject *anObject, QEvent *anEvent)
  {
    switch (anEvent->type()) {
    case QEvent::Show: {
      if ((anObject->inherits("QDialog") || anObject->inherits("QDockWidget")) && qApp->activeWindow()) {
        // Workaround for Qt bug where opened QDialogs do not re-activate previous window
        // when accepted or rejected. We cannot rely on the parent pointers so push the previous
        // active window onto a stack before the dialog is shown.
        // We have to use a stack in case a dialog opens another dialog.
        // NOTE: It's important to use QPointers so that any widgets deleted by Qt do not lead to
        // hanging pointers in the stack.
        m_activationstack.push(qApp->activeWindow());
      }
      break;
    }
    case QEvent::Hide: {
      if ((anObject->inherits("QDialog") || anObject->inherits("QDockWidget")) && !m_activationstack.isEmpty()) {
        QPointer<QWidget> widget = m_activationstack.pop();
        if (widget) {
          // Re-acivate widgets in the order as dialogs are closed. See Show case above.
          widget->activateWindow();
          widget->raise();
        }
      }
      break;
    }
    default:
      break;
    }

    return QObject::eventFilter(anObject, anEvent);
  }
};

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  switch (type) {
  case QtDebugMsg:
    LDEBUGF(context.file, context.line, context.function) << msg;
    break;
  case QtWarningMsg:
    LWARNF(context.file, context.line, context.function) << msg;
    break;
  case QtCriticalMsg:
    LERRORF(context.file, context.line, context.function) << msg;
    break;
  case QtFatalMsg:
    LFATALF(context.file, context.line, context.function) << msg;
    abort();
  }
}
#else
void myMessageOutput(QtMsgType type, const char *msg)
{
  switch (type) {
  case QtDebugMsg:
    LDEBUG_NLN() << msg;
    break;
  case QtWarningMsg:
    LWARN_NLN() << msg;
    break;
  case QtCriticalMsg:
    LERROR_NLN() << msg;
    break;
  case QtFatalMsg:
    LFATAL_NLN() << msg;
    abort();
  }
}
#endif    // qt version > 5.0.0

int main(int argc, char *argv[])
{

#ifndef _FLYEM_
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  qInstallMessageHandler(myMessageOutput);
#else
  qInstallMsgHandler(myMessageOutput);
#endif
#endif

  bool debugging = false;
  bool unitTest = false;
  bool runCommandLine = false;

#ifndef QT_NO_DEBUG
  if (argc > 1) {
    if (strcmp(argv[1], "d") == 0) {
      debugging = true;
    }
    if (strcmp(argv[1], "u") == 0 || QString(argv[1]).startsWith("--gtest")) {
      unitTest = true;
      debugging = true;
    }
    if (strcmp(argv[1], "--command") == 0) {
      runCommandLine = true;
    }
  }
#endif

  QApplication app(argc, argv);     // call first otherwise it will cause runtime warning: Please instantiate the QApplication object first

  //load config
  NeutubeConfig &config = NeutubeConfig::getInstance();
  std::cout << QApplication::applicationDirPath().toStdString() << std::endl;
  config.setApplicationDir(QApplication::applicationDirPath().toStdString());
  if (config.load(config.getConfigPath()) == false) {
    std::cout << "Unable to load configuration: "
              << config.getConfigPath() << std::endl;
  }
  //config.print();

  RECORD_INFORMATION("************* Start ******************");

  if (debugging == false && runCommandLine == false) {
#if defined __APPLE__        //use macdeployqt
#else
#if defined(QT_NO_DEBUG)
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
    dir.cdUp();
    dir.cd("lib");
    QApplication::addLibraryPath(dir.absolutePath());
#endif
#endif


    // init the logging mechanism
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    const QString sLogPath(QDir(app.applicationDirPath()).filePath("neuTube_log.txt"));
    QsLogging::DestinationPtr fileDestination(
          QsLogging::DestinationFactory::MakeFileDestination(sLogPath, QsLogging::EnableLogRotation, 1e7, 20));
    QsLogging::DestinationPtr debugDestination(
          QsLogging::DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);
#if defined _DEBUG_
    logger.setLoggingLevel(QsLogging::DebugLevel);
#else
    logger.setLoggingLevel(QsLogging::InfoLevel);
#endif

#if defined __APPLE__ && (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    app.setGraphicsSystem("raster");
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    qApp->installEventFilter(new MacEventFilter(qApp));
#endif

    // init 3D
    //std::cout << "Initializing 3D ..." << std::endl;
    RECORD_INFORMATION("Initializing 3D ...");
    Z3DApplication z3dApp(QCoreApplication::applicationDirPath());
    z3dApp.initialize();

    MainWindow *mainWin = new MainWindow();
    mainWin->config();
    mainWin->show();
    mainWin->initOpenglContext();

    int result =  app.exec();

    delete mainWin;
    z3dApp.deinitializeGL();
    z3dApp.deinitialize();
    return result;
  } else {
    if (runCommandLine) {
      ZCommandLine cmd;
      return cmd.run(argc, argv);
    }

    /********* for debugging *************/

#ifndef QT_NO_DEBUG
    /*
    std::cout << "Debugging ..." << std::endl;
    ZCurve curve;
    double *array = new double[10];
    for (int i = 0; i < 10; i++) {
      array[i] = -i * i;
    }
    curve.loadArray(array, 10);
    std::cout << curve.minY() << std::endl;
    */
    if (unitTest) {
      ZTest::runUnitTest(argc, argv);
    } else {
      ZTest::test(NULL);
    }

#else
    std::cout << "No debugging in the release version." << std::endl;
#endif

    return 1;
  }
}
