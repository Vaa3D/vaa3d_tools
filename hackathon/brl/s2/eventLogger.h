#ifndef EVENTLOGGER_H
#define EVENTLOGGER_H

#include <QObject>
#include <QTime>
#include <QList>

class S2Event
{
public:
    S2Event();
    S2Event(QString eString, qint64 msTime);
    QString getTimeString();
    QString getEventName();
    void setEventString(QString eString);
    void setTime(qint64 msTime);

 private:
    QString eventName;
    qint64 msEventTime;
    QString timeString;

};


class EventLogger : public QObject
{
    Q_OBJECT
public:
    explicit EventLogger(QObject *parent = 0);
    QList<S2Event> eventList;
signals:

public slots:
    void logEvent(QString eventString);
    void processEvents();

};


#endif // EVENTLOGGER_H
