#include "eventLogger.h"
#include <QDebug>



S2Event::S2Event(){
    eventName = "";
    msEventTime = 0;
    timeString = "";
}

S2Event::S2Event(QString eString, qint64 msTime){
   eventName = eString;
   msEventTime = msTime;
   timeString = QDateTime::fromMSecsSinceEpoch(msTime).toString("yyyy_MM_dd_ddd_hh_mm_ss_zzz");
}


QString S2Event::getEventName(){
return eventName;
}

QString S2Event::getTimeString(){
return timeString;
}

void S2Event::setEventString(QString eString){
eventName = eString;
}

void S2Event::setTime(qint64 msTime){
msEventTime = msTime;
timeString = QDateTime::fromMSecsSinceEpoch(msTime).toString("yyyy_MM_dd_ddd_hh_mm_ss_zzz");

}



EventLogger::EventLogger(QObject *parent) :
    QObject(parent)
{
}


//how to arrange this?  the eventlogger should have a
// list of events, with each event consisting of a string and a time (epoch ms)
//
void EventLogger::logEvent(QString eventString){
    eventList.append(S2Event(eventString, QDateTime::currentMSecsSinceEpoch()));
    for (int i =0; i<eventList.length(); i++){
        qDebug()<<eventList[i].getEventName()<<" at "<<eventList[i].getTimeString();
    }
}


void EventLogger::processEvents(){
    // log startsmartScan
    // finishedsmartScan

    // log startMultiTarget
    // log finishedMultiTarget
    // run through the list, looking for particular eventStrings

    // for each pair of startZStack - finishedZStack
    // or
    // startAnalysis -finishedAnalysis

    //  then for each pair, determine the elapsed time

    // calculate the total time imaging, total time analyzing
    // total time imaging while not analyzing and time analyzing while not imaging
    // calculate the total time for the whole process
}

