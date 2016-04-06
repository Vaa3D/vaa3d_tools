#ifndef TILEINFO_H
#define TILEINFO_H
#include <QString>
#include <QStringList>

class TileInfo
{

public:
    TileInfo();
    explicit TileInfo(float zoomPixelsProduct);
    void setZoomPos( float tileZoom, float xPos, float yPos);
    void setPixels(int pixelNumber);
    QStringList getTileInfoString();
    float getTileZoom();
    int getTilePixelsX();
    int getTilePixelsY();
    bool resOK;


private:
    int pixelsX;
    int pixelsY;
    float tileZoom;
    float xPos;
    float yPos;
    float zoomPixelsProduct;
    bool zoomSet;

};

#endif // TILEINFO_H
