#ifndef TILEINFO_H
#define TILEINFO_H
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QDateTime>
#include <v3d_interface.h>

class TileInfo
{

public:
    TileInfo();
    explicit TileInfo(float zoomPixelsProduct);
    void setZoomPos(float tileZoom);
    void setPixelLocation(LocationSimple loc);
    void setStageLocation(LocationSimple loc);
    void setGalvoLocation(LocationSimple loc);

    LocationSimple getPixelLocation() const;
    LocationSimple getStageLocation() const;
    LocationSimple getGalvoLocation() const;

    void setPixels(int pixelNumber);
    QStringList getTileInfoString();
    float getTileZoom() const;
    int getTilePixelsX() const;
    int getTilePixelsY() const;
    void setFileString(QString inputString);
    QString getFileString() const;
    bool resOK;
    int setTimeStamp(QDateTime timeNow);
    QList<float> getElapsedTimes() const;
    QStringList getTimeStrings() const;
    QList<QDateTime> getTileTimes() const;

private:
    LocationSimple pixelLocation;
    LocationSimple stageLocation;
    LocationSimple galvoLocation;
    int pixelsX;
    int pixelsY;
    float tileZoom;
    float zoomPixelsProduct;
    bool zoomSet;
    QString tilev3drawFileString;
    int timeStampCategory;
    QList<QDateTime> tileTimes;
    QList<float> elapsedTimes;

};

#endif // TILEINFO_H
