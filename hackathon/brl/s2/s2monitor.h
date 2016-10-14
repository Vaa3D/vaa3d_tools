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


class S2ScanData : public QObject{
    Q_OBJECT
public:
S2ScanData();
QList<TileInfo> getAllTileInfo()const;
QImage getS2ScanImage()const;

public slots:
void addNewTile(TileInfo newTileInfo);
void updateS2ScanImage();
private:
QList<TileInfo> allTiles;
QImage s2ScanImage;



};











class S2Monitor : public QWidget
{
    Q_OBJECT
public:
    explicit S2Monitor(QWidget *parent = 0);
    
signals:

public slots:
    void writeScanData(int scanNumber);
    void writeAllScanData();
    void setSaveDir(QDir directory);
    void updateScanData(int scanNumber, TileInfo newTileInfo);
private:
QList<S2ScanData> allScanData;
QDir saveDir;
    
};

#endif // S2MONITOR_H
