#include "zprocessprogressbase.h"
#include "QsLog.h"
#include <QThread>
#include "QsLog/QsLogDest.h"

ZProcessProgressBase::ZProcessProgressBase(QObject *parent)
  : QObject(parent)
  , m_cancelFlag(NULL)
  , m_totalWeight(0.0)
{
#ifdef _USE_ITK_
  m_CallbackCommand = CommandType::New();
  m_CallbackCommand->SetCallbackFunction(this, &ZProcessProgressBase::processITKEvent);
  m_CallbackCommand->SetCallbackFunction(this, &ZProcessProgressBase::constProcessITKEvent);
#endif
}

void ZProcessProgressBase::setCancelFlag(bool *flag)
{
  m_cancelFlag = flag;
  for (size_t i=0; i<m_subOperations.size(); ++i)
    m_subOperations[i]->setCancelFlag(flag);
}

void ZProcessProgressBase::run()
{
  QsLogging::DestinationPtr fileDestination;
  QsLogging::Logger& logger = QsLogging::Logger::instance();
  if (!m_logFile.isEmpty()) {
    fileDestination = QsLogging::DestinationFactory::MakeFileDestination(m_logFile);
    logger.addDestination(fileDestination);
  }
  try {
    LDEBUG() << "run " << QThread::currentThreadId();
    doWork();
    emit finished();
  }
#ifdef _USE_ITK_
  catch (itk::ProcessAborted const & e) {
    LERROR() << "Process Aborted by User." << e.GetDescription();
    emit canceled();
    if (isSubOperation()) {
      LERROR() << "notifying parent operation..";
      if (!m_logFile.isEmpty())
        logger.removeDestination(fileDestination);
      throw;  // notify parent
    }
  }
  catch (itk::ExceptionObject const & excp) {
    LERROR() << "Caught itk exception" << excp.GetDescription();
    emit processError(QString(excp.GetDescription()));
    if (isSubOperation()) {
      LERROR() << "notifying parent operation..";
      if (!m_logFile.isEmpty())
        logger.removeDestination(fileDestination);
      throw;  // notify parent
    }
  }
#endif
  catch (ProcessAbortException const & e) {
    LERROR() << "Process Aborted by User." << e.what();
    emit canceled();
    if (isSubOperation()) {
      LERROR() << "notifying parent operation..";
      if (!m_logFile.isEmpty())
        logger.removeDestination(fileDestination);
      throw;  // notify parent
    }
  }
  //  catch (std::exception const & e) {
  //    LERROR() << "Caught std exception:" << e.what();
  //    emit processError(QString(e.what()));
  //    if (isSubOperation()) {
  //      LERROR() << "notifying parent operation..";
  //      if (!m_logFile.isEmpty())
  //        logger.removeDestination(fileDestination);
  //      throw;  // notify parent
  //    }
  //  }
  //  catch (...) {
  //    LERROR() << "Unknown Exception";
  //    emit processError("Unknown Exception");
  //    if (isSubOperation()) {
  //      LERROR() << "notifying parent operation..";
  //      if (!m_logFile.isEmpty())
  //        logger.removeDestination(fileDestination);
  //      throw;  // notify parent
  //    }
  //  }
  if (!m_logFile.isEmpty())
    logger.removeDestination(fileDestination);
}

void ZProcessProgressBase::registerOperation(void *reporter, double weight)
{
  LDEBUG() << "register operation";
  if (weight <= 0 || reporter == NULL)
    return;
  m_processToWeight[reporter] = weight;
  m_processToProgress[reporter] = 0.0;
}

#ifdef _USE_ITK_
void ZProcessProgressBase::registerOperation(itk::ProcessObject *filter, double weight)
{
  LDEBUG() << "register itk operation";
  if (weight <= 0 || filter == NULL)
    return;
  m_processToWeight[filter] = weight;
  m_processToProgress[filter] = 0.0;
  m_itkProcessers.push_back(filter);
  filter->AddObserver(itk::ProgressEvent(), m_CallbackCommand);
}
#endif

void ZProcessProgressBase::registerOperation(ZProcessProgressBase *reporter, double weight)
{
  LDEBUG() << "register sub operation";
  if (weight <= 0 || reporter == NULL)
    return;
  m_processToWeight[reporter] = weight;
  m_processToProgress[reporter] = 0.0;
  if (reporter != this) {  // sub operation
    reporter->addParentOperation(this);
    m_subOperations.push_back(reporter);
    reporter->setCancelFlag(m_cancelFlag);
  }
}

void ZProcessProgressBase::reportProgress(const void *reporter, double progress)
{
  if (isCanceled()) {
#ifdef _USE_ITK_
    itk::ProcessObject* itkProcess = getITKProcess(reporter);
    if (itkProcess)
      itkProcess->AbortGenerateDataOn();
    else
#endif
      throw ProcessAbortException();
  } else {
    if (m_processToProgress.find(reporter) != m_processToProgress.end()) {
      m_processToProgress[reporter] = clamp(progress);
      sendProgressSignal();
    } else {
      LERROR() << "reporter not registered!";
    }
  }
}

//void ZProcessProgressBase::cancelOperation()
//{
//#ifdef _USE_ITK_
//  for (size_t i=0; i<m_itkProcessers.size(); ++i)
//    m_itkProcessers[i]->AbortGenerateDataOn();
//#endif
//}

void ZProcessProgressBase::clearRegisteredOperations()
{
  m_processToProgress.clear();
  m_processToWeight.clear();
#ifdef _USE_ITK_
  m_itkProcessers.clear();
#endif
  m_subOperations.clear();
}

void ZProcessProgressBase::sendProgressSignal()
{
  std::map<const void*,double>::iterator wit = m_processToWeight.begin();
  std::map<const void*,double>::iterator pit = m_processToProgress.begin();
  double currentWeight = 0.0;
  while (wit != m_processToWeight.end()) {
    double weight = wit->second;
    currentWeight += pit->second * weight;
    ++wit;
    ++pit;
  }
  if (m_totalWeight > 0) {
    for (size_t i=0; i<m_parentOperations.size(); ++i)
      m_parentOperations[i]->reportProgress(this, currentWeight / m_totalWeight);
    emit progressChanged(currentWeight / m_totalWeight);
    emit progressChanged(static_cast<int>(100*currentWeight/m_totalWeight));
  }
}

void ZProcessProgressBase::addParentOperation(ZProcessProgressBase *parentOp)
{
  m_parentOperations.push_back(parentOp);
}

#ifdef _USE_ITK_
void ZProcessProgressBase::processITKEvent(itk::Object *caller, const itk::EventObject &event)
{
  if (typeid(itk::ProgressEvent) == typeid(event)) {
    itk::ProcessObject* process =  dynamic_cast<itk::ProcessObject*>(caller);
    reportProgress(process, clamp(process->GetProgress()));
  }
}

void ZProcessProgressBase::constProcessITKEvent(const itk::Object *caller, const itk::EventObject &event)
{
  if (typeid(itk::ProgressEvent) == typeid(event)) {
    const itk::ProcessObject* process =  dynamic_cast<const itk::ProcessObject*>(caller);
    reportProgress(process, clamp(process->GetProgress()));
  }
}
#endif

double ZProcessProgressBase::clamp(double progress, double min, double max)
{
  if (progress < min)
    return min;
  else if (progress > max)
    return max;
  else
    return progress;
}

bool ZProcessProgressBase::isCanceled() const
{
  if (m_cancelFlag && *m_cancelFlag)
    return true;
  else
    return false;
}

#ifdef _USE_ITK_
itk::ProcessObject *ZProcessProgressBase::getITKProcess(const void *reporter)
{
  for (size_t i=0; i<m_itkProcessers.size(); ++i)
    if (reporter == m_itkProcessers[i])
      return m_itkProcessers[i];
  return NULL;
}
#endif

bool ZProcessProgressBase::isSubOperation() const
{
  return !m_parentOperations.empty();
}
