#include "zqtmessagereporter.h"

ZQtMessageReporter::ZQtMessageReporter()
{
}

ZQtMessageReporter::~ZQtMessageReporter()
{

}

void ZQtMessageReporter::report(
    const std::string &title, const std::string &message, EMessageType msgType)
{
  switch (msgType) {
  case Error:
    m_box.setIcon(QMessageBox::Critical);
    break;
  case Warning:
    m_box.setIcon(QMessageBox::Warning);
    break;
  case Information:
    m_box.setIcon(QMessageBox::Information);
    break;
  case Debug:
    ZMessageReporter::report(title, message, msgType);
    break;
  }

  if (msgType != Debug) {
    m_box.setWindowTitle(title.c_str());
    m_box.setText(message.c_str());
    m_box.exec();
  }
}
