#ifndef BASICINFO_H
#define BASICINFO_H
#include <QtGui>
using namespace std;
struct ReconstructionInfo{
    QString SdataID;//Image data ID, like BrainID
    QString dataID;//NeuronID
    QString levelID;
    QString author;
    QString checkers;
    QString updateTime;
    QString fatherDirName; //dir name for reconstructions
    QString filePath;
};
#endif // BASICINFO_H
