#ifndef ZDVIDCLIENT_H
#define ZDVIDCLIENT_H

#include <QString>
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QFile>
#include <QVariant>

/*!
 * \brief The class if DVID client
 */
class ZDvidClient : public QObject
{
  Q_OBJECT

public:
  ZDvidClient(const QString &server, QObject *parent = NULL);

  enum EDvidRequest {
    DVID_GET_OBJECT
  };

  inline void setServer(const QString &server) {
    m_serverAddress = server;
  }
  inline const QString &getServer() {
    return m_serverAddress;
  }

  /*!
   * \brief Send a request to DVID
   */
  bool postRequest(EDvidRequest request, const QVariant &parameter);

private slots:
  bool writeObject();
  void finishRequest();

private:
  QString m_serverAddress; //Server address
  QNetworkAccessManager *m_networkManager;
  QNetworkReply *m_networkReply;
  QString m_targetDirectory;
  QFile *m_file;
};

#endif // ZDVIDCLIENT_H
