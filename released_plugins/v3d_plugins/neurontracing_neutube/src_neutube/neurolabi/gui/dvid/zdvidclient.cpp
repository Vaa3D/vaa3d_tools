#include "zdvidclient.h"
#include <QFileInfo>
#include <QNetworkReply>
#include <QDebug>

#include "zerror.h"

ZDvidClient::ZDvidClient(const QString &server, QObject *parent) :
  QObject(parent), m_serverAddress(server),
  m_networkReply(NULL), m_targetDirectory("/tmp"), m_file(NULL)
{
  m_networkManager = new QNetworkAccessManager(this);
}

bool ZDvidClient::postRequest(EDvidRequest request, const QVariant &parameter)
{
  QUrl requestUrl;

  switch (request) {
  case DVID_GET_OBJECT:
    requestUrl.setUrl(QString("%1/api/node/f1/sp2body/sparsevol/%2").
                      arg(m_serverAddress).arg(parameter.toInt()));
    qDebug() << m_serverAddress;
    qDebug() << requestUrl.toString();
    break;
  default:
    RECORD_ERROR_UNCOND("Invalid request");
    return false;
    break;
  }

  m_networkReply = m_networkManager->get(QNetworkRequest(requestUrl));

  switch (request) {
  case DVID_GET_OBJECT:
  {
    m_file = new QFile(QString("%1/%2.dvid").
                       arg(m_targetDirectory).arg(parameter.toInt()));
    if (!m_file->open(QIODevice::WriteOnly)) {
      RECORD_ERROR_UNCOND("Unable to write object");
      delete m_file;
      m_file = NULL;
      return false;
    }

    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(writeObject()));
  }
  }

  return true;
}

bool ZDvidClient::writeObject()
{
  if (m_file != NULL) {
    m_file->write(m_networkReply->readAll());
    return true;
  }

  return false;
}

void ZDvidClient::finishRequest()
{
  if (m_file != NULL) {
    m_file->flush();
    m_file->close();
  }

  if (m_networkReply->error()) {
      m_file->remove();
      RECORD_ERROR_UNCOND(std::string("Unable to finish operation: ") +
                          m_networkReply->errorString().toStdString());
  }

  m_networkReply->deleteLater();
  m_networkReply = NULL;

  if (m_file != NULL) {
    delete m_file;
    m_file = NULL;
  }
}

