#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QWidget>

class HttpClient : public QWidget
{
    Q_OBJECT
public:
    explicit HttpClient(QWidget *parent = nullptr);

signals:

};

#endif // HTTPCLIENT_H
