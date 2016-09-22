#ifndef TILEINFO_H
#define TILEINFO_H
#include <QString>
#include <QStringList>
#include <QDebug>
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

    LocationSimple getPixelLocation();
    LocationSimple getStageLocation();
    LocationSimple getGalvoLocation();

    void setPixels(int pixelNumber);
    QStringList getTileInfoString();
    float getTileZoom();
    int getTilePixelsX();
    int getTilePixelsY();
    bool resOK;


private:
    LocationSimple pixelLocation;
    LocationSimple stageLocation;
    LocationSimple galvoLocation;
    int pixelsX;
    int pixelsY;
    float tileZoom;
    float zoomPixelsProduct;
    bool zoomSet;

};

#endif // TILEINFO_H
