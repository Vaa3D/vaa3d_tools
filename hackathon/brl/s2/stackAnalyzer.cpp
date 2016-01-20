#include "stackAnalyzer.h"

stackAnalyzer::stackAnalyzer(QObject *parent) :
    QObject(parent)
{
}


void stackAnalyzer::analysisDone(QList<int[3]> newTargets){
    // this signal emits newTargets, a QList of 3-ints [x,y,z] coordinates
    // for the next ROI.

    // exact format is TBD...  center of face with tips?
    //                         average location of tips on each face?
    //                         center of next ROI?

}

void stackAnalyzer::messageSignal(QString msg){
    // signal to send messages back to s2UI if needed
}

void stackAnalyzer::processStack(Image4DSimple * pInputImage){
    // process stack data


    // if needed
     emit(messageSignal(QString("message!")));


    // determine all ROI locations and put them in newTargetList, a QList of 3-ints [x,y,z] coordinates
    // for the next ROI.
    QList<int[3]> newTargets;
    // exact format is TBD...  center of face with tips?
    //                         average location of tips on each face?
    //



    //   then emit the new targets!
    emit(analysisDone(newTargetList));


}
