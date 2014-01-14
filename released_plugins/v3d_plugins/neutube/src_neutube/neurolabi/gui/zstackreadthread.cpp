#include "zstackreadthread.h"

#include <iostream>

#include "zstack.hxx"
#include "zstackfile.h"

ZStackReadThread::ZStackReadThread(QObject *parent) :
  QThread(parent), m_stackFile(NULL), m_stack(NULL)
{
}

void ZStackReadThread::run()
{
  if (m_stackFile != NULL) {
    m_stack = m_stackFile->readStack();

#ifdef _DEBUG_
    std::cout << "Stack read done" << std::endl;
    std::cout << m_stack << std::endl;
#endif
  }
}
