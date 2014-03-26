#ifndef ZSTACKREADTHREAD_H
#define ZSTACKREADTHREAD_H

#include <QThread>

class ZStackFile;
class ZStack;

class ZStackReadThread : public QThread
{
  Q_OBJECT
public:
  explicit ZStackReadThread(QObject *parent = 0);
  void run();
  inline ZStack* getStack() {
    return m_stack;
  }
  inline ZStackFile* getStackFile() {
    return m_stackFile;
  }
  
  inline void setStackFile(ZStackFile *stackFile) {
    m_stackFile = stackFile;
  }

signals:

public slots:

private:
  ZStackFile *m_stackFile;
  ZStack *m_stack;
};

#endif // ZSTACKREADTHREAD_H
