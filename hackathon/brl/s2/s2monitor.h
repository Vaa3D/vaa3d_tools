#ifndef S2MONITOR_H
#define S2MONITOR_H

#include <QWidget>
#include <QTextEdit>
#include <QString>
#include <QFileInfo>
#include <QPushButton>
#include <QGridLayout>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QList>
#include "tileInfo.h"
#include <QImage>
#include <QMutex>


class S2ScanData{

public:
S2ScanData();
QList<TileInfo> getAllTileInfo()const;
QImage getS2ScanImage()const;
float boundingBoxX;
float boundingBoxY;
float totalTileArea;
float imagedArea;
void addNewTile(TileInfo newTileInfo);
void updateS2ScanImage();
void updateS2Summary();

QList<TileInfo> allTiles;
private:
QImage s2ScanImage;
QDir s2ScanDir;
QString currentRepoHash;
QList<QPainterPath> numberPPList;

};



//class S2ScanImageUpdater: public QObject
//{
//    Q_OBJECT
//public:
//    S2ScanImageUpdater();
//    void initialize(QList<LocationSimple> pixelLocations, QList<LocationSimple> galvoLocations, QList<long> scanNumbers, QDir saveDirectory);
//public slots:
//    void createScanImage(QImage blankInputImage, QPainter testPainter);
//private:
//QList<LocationSimple> pixelLocations;
//QList<LocationSimple> galvoLocations;
//QList<long> scanNumbers;
//QDir saveDir;
//};







class S2Monitor : public QWidget
{
    Q_OBJECT
public:
    explicit S2Monitor(QWidget *parent = 0);
    QList<S2ScanData> allScanData;

signals:

public slots:
    void writeScanData(int scanNumber);
    void writeAllScanData();
    void setSaveDir(QDir directory);
//    void updateScanData(int scanNumber, TileInfo newTileInfo);
    void startNewScan();
    void addNewTile(TileInfo newTileInfo);
private:
QDir saveDir;
    int currentScanNumber;
};


class TileInfoMonitor : public QObject{
    Q_OBJECT
public:
    TileInfoMonitor();
signals:
    void foundTile(TileInfo duplicateTile, LandmarkList seedList, int tileStatus, int correctX, int correctY);
public slots:
    void addTileData(TileInfo incomingTile, LandmarkList seedList, int tileStatus, int correctX, int correctY, QString tracingMethod);
    QList<TileInfo> getTileInfoList()const;
private slots:
    void searchForTiles();
private:
    QList<TileInfo> tileInfoList;
    QList<LandmarkList> seedListList;
    QList<QList<int> > intList;
    QList<QString> tracingMethodStrings;
    bool running;
    QMutex myMutex;
};


#endif // S2MONITOR_H
