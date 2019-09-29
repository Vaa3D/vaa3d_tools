#ifndef BASICINFO_H
#define BASICINFO_H
#include <QtGui>
using namespace std;

struct SourceDataInfo{
    QString SdataID;//Image data ID, like BrainID
    QString SDSize;
    QString SDResolution;
    QString SDBit;
    QString SDState;
    QString SDComments;
    QString SDPriority;
    QString SDLabeledSomaNumber;
};
struct SomaConfInfo:SourceDataInfo
{
    QString SomaID;
    QString SDID;
    QString CellName;
    QString Zprojection;
    QString Xcoordinate;
    QString SomaComments;
    QString Ycoordinate;
    QString SomaState;
    QString SomaLocation;
    QString CellType;
    QString SomaPriority;
};
struct ReconstructionInfo:SomaConfInfo
{
//    QString SdataID;//Image data ID, like BrainID
//    QString dataID;//NeuronID
    QString levelID;
    QString author;
    QString checkers;
    QString updateTime;
    QString fatherDirName; //dir name for reconstructions
    QString filePath;
};
#endif // BASICINFO_H
