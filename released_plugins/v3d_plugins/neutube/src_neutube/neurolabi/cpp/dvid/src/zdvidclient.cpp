#include "zdvidclient.h"
#include <QNetworkReply>

ZDvidClient::ZDvidClient(QObject *parent) :
  QObject(parent)
{
}

void ZDvidClient::print(const QNetworkReply *reply) const
{
  //reply.
}
