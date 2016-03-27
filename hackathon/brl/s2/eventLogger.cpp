#include "eventLogger.h"
#include <QDebug>
#include <QFile>


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

qint64 S2Event::getTime(){
    return msEventTime;
}




EventLogger::EventLogger(QObject *parent) :
    QObject(parent)
{
}


// EventLogger contains 3 lists:
// 1. list of S2Events
// 2. list of elapsed times between start and stop events in 1.
// 3. summary list with several quantities as S2Events

void EventLogger::logEvent(QString eventString){
    eventList.append(S2Event(eventString, QDateTime::currentMSecsSinceEpoch()));
    int startingEvent = 0;
    if (eventList.length() >5){startingEvent = eventList.length()-5;qDebug()<< "most recent events:";}

    for (int i = startingEvent; i<eventList.length(); i++){
        qDebug()<<eventList[i].getEventName()<<" at "<<eventList[i].getTimeString();
    }
}


void EventLogger::processEvents(QString saveFileString){

    QList<S2Event> eventListCopy = eventList;
    int   starti = eventListFirstContains(eventListCopy, "start");

    while (starti>=0){

        S2Event startEvent = eventListCopy.value(starti);

        QString startString= startEvent.getEventName().split("start").last();
        eventListCopy.removeAt(starti);
        int endj = eventListFirstContains(eventListCopy, "finished"+startString);
        S2Event endEvent = eventListCopy.value(endj);
        eventListCopy.removeAt(endj);
        elapsedList.append(S2Event("elapsed"+startString,endEvent.getTime()-startEvent.getTime())) ;


        starti = eventListFirstContains(eventListCopy, "start");
    }
    //

    //



    if (elapsedList.isEmpty()){qDebug()<<"empty elapsedList"; return;}

    QFile saveTextFile;
    saveTextFile.setFileName(saveFileString);// add currentScanFile
    bool okToSave = true;
    if (!saveTextFile.isOpen()){
        if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
            qDebug()<<"unable to save file!";
            okToSave = false;
        }
    }
    QList<qint64> stackTimeList;
    for (int i =0; i<elapsedList.length(); i++){
        if ((elapsedList.value(i).getEventName().contains("Scan"))||
                (elapsedList.value(i).getEventName().contains("MultiTarget"))){

            summaryList.append(elapsedList.value(i));
        }
        if (elapsedList.value(i).getEventName().contains("Stack")){
            stackTimeList.append(elapsedList.value(i).getTime());
        }

    }

    QTextStream outputStream;
    outputStream.setDevice(&saveTextFile);
    for (int i =0; i<elapsedList.length(); i++){




        QString toWrite = elapsedList[i].getEventName().append(" took ").append(QString::number(elapsedList[i].getTime())).append(" ms");

         if (okToSave){
                             outputStream<<toWrite<<"\n";
         }
        qDebug()<<toWrite;
    }




    if (okToSave){saveTextFile.close();}


    // calculate the total time imaging, total time analyzing
    // total time imaging while not analyzing and time analyzing while not imaging
    // calculate the total time for the whole process
}

int EventLogger::eventListFirstContains(QList<S2Event> inputEventList, QString stringToFind){
    if (inputEventList.isEmpty()){return -1;}
    for (int i=0; i<inputEventList.length(); i++){
        if (inputEventList.value(i).getEventName().contains(stringToFind)){
            return i;
        }
    }
    return -1;
}
