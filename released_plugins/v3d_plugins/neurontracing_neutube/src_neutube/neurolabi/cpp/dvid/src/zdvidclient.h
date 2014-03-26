#ifndef ZDVIDCLIENT_H
#define ZDVIDCLIENT_H

#include <QObject>

class QNetworkReply;

class ZDvidClient : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidClient(QObject *parent = 0);

signals:

public slots:
  void print(const QNetworkReply *reply) const;
};

#endif // ZDVIDCLIENT_H
