#ifndef ZQTMESSAGEREPORTER_H
#define ZQTMESSAGEREPORTER_H

#include <QMessageBox>
#include <QWidget>
#include "zmessagereporter.h"

class ZQtMessageReporter : public ZMessageReporter
{
public:
  ZQtMessageReporter();
  virtual ~ZQtMessageReporter();
  virtual void report(const std::string &title, const std::string &message,
                      EMessageType msgType);

private:
  QMessageBox m_box;
};

#endif // ZQTMESSAGEREPORTER_H
