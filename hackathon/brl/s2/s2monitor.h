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
#include <tileInfo.h>
#include <QImage>


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
private:
QList<TileInfo> allTiles;
QImage s2ScanImage;
QDir s2ScanDir;
QString currentRepoHash;


};



class S2ScanImageUpdater: public QObject
{
    Q_OBJECT
public:
    S2ScanImageUpdater();
    void initialize(QList<LocationSimple> pixelLocations, QList<LocationSimple> galvoLocations, QList<long> scanNumbers, QDir saveDirectory);
public slots:
    void createScanImage();
private:
QImage scanImage;
QList<LocationSimple> pixelLocations;
QList<LocationSimple> galvoLocations;
QList<long> scanNumbers;
QDir saveDir;
};







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
    void updateScanData(int scanNumber, TileInfo newTileInfo);
    void startNewScan();
    void addNewTile(TileInfo newTileInfo);
private:
QDir saveDir;
    int currentScanNumber;
};

#endif // S2MONITOR_H
