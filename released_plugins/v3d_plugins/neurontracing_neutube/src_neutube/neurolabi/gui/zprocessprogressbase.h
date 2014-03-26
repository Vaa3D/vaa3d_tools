#ifndef ZPROCESSPROGRESSBASE_H
#define ZPROCESSPROGRESSBASE_H

#include <QObject>
#include <map>
#include <vector>

#ifdef _USE_ITK_
#include <itkCommand.h>
#include <itkProcessObject.h>
#endif

class ProcessAbortException : public std::exception {
public:
  ProcessAbortException(const std::string& what = "") : m_what(what) {}
  virtual ~ProcessAbortException() throw() {}

  virtual const char* what() const throw() {return m_what.c_str();}
protected:
  std::string m_what;
};

class ZProcessProgressBase : public QObject
{
  Q_OBJECT
public:  
  explicit ZProcessProgressBase(QObject *parent = 0);
  void setCancelFlag(bool *flag);

  void reportProgress(const void* reporter, double progress);

  // log output
  void setLogFile(const QString &logFile) { m_logFile = logFile; }

signals:
  // progress from 1 to 100, used for QProgressbar
  void progressChanged(int);
  // progress from 0.0 to 1.0
  void progressChanged(double);
  void canceled();
  void processError(QString);
  void finished();
  
public slots:
  void run();

protected:
  virtual void doWork() = 0;
  //must set before register operations
  // total weight can be changed during operation
  void setTotalWeight(double total) { m_totalWeight = total; sendProgressSignal(); }
  double getTotalWeight() const { return m_totalWeight; }

  void registerOperation(void *reporter, double weight);
#ifdef _USE_ITK_
  void registerOperation(itk::ProcessObject *filter, double weight);
#endif
  void registerOperation(ZProcessProgressBase* reporter, double weight);

  void clearRegisteredOperations();

private:
  void sendProgressSignal();
  void addParentOperation(ZProcessProgressBase *parentOp);
#ifdef _USE_ITK_
  typedef itk::MemberCommand<ZProcessProgressBase>  CommandType;
  typedef CommandType::Pointer CommandPointer;
  // call back function for ITK
  void processITKEvent(itk::Object *caller, const itk::EventObject &event);
  void constProcessITKEvent(const itk::Object *caller, const itk::EventObject &event);
  CommandPointer m_CallbackCommand;
#endif
  double clamp(double progress, double min=0.0, double max=1.0);
  bool isCanceled() const;
#ifdef _USE_ITK_
  // if reporter is itk process, return it, otherwise return NULL
  itk::ProcessObject* getITKProcess(const void *reporter);
#endif
  bool isSubOperation() const;

  bool *m_cancelFlag;
  double m_totalWeight;
  std::map<const void*,double> m_processToWeight;
  std::map<const void*,double> m_processToProgress;
#ifdef _USE_ITK_
  std::vector<itk::ProcessObject*> m_itkProcessers;
#endif
  std::vector<ZProcessProgressBase*> m_parentOperations;
  std::vector<ZProcessProgressBase*> m_subOperations;
  QString m_logFile;
};

#endif // ZPROCESSPROGRESSBASE_H
