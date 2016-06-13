#include "tileInfo.h"

TileInfo::TileInfo(){
    qDebug()<<"TileInfo constructor...";
}

TileInfo::TileInfo(float zoomPixelsProduct)
{
    this->zoomPixelsProduct = zoomPixelsProduct;
    zoomSet = false;
    resOK = false;
}


void TileInfo::setZoomPos( float tileZoom, float xPos, float yPos){

    this->tileZoom = tileZoom;
    this->xPos = xPos;
    this->yPos = yPos;
    this->pixelsX = zoomPixelsProduct/tileZoom;
    this->pixelsY = pixelsX;
    if (tileZoom<=13.1){resOK = true;}else{resOK =false;}
    zoomSet = true;
}

void TileInfo::setPixels(int pixelNumber){
    this->pixelsX = pixelNumber;
    this->pixelsY = pixelNumber;
    this->tileZoom = zoomPixelsProduct/pixelNumber;
    if (tileZoom<=13.0){resOK = true;}else{resOK =false;}
    zoomSet = true;
}
QStringList TileInfo::getTileInfoString(){
    QStringList a= QStringList();
    if (zoomSet){
    a.append(QString::number(pixelsX));
    a.append(QString::number(pixelsY));
    a.append(QString::number(tileZoom));
    a.append(QString::number(xPos));
    a.append(QString::number(yPos));
    a.append(QString::number(resOK));
    }
return a;
}
float TileInfo::getTileZoom(){if (zoomSet){return tileZoom;}else{return -1;}}
int TileInfo::getTilePixelsX(){if (zoomSet){return pixelsX;}else{return -1;}}
int TileInfo::getTilePixelsY(){if (zoomSet){return pixelsY;}else{return -1;}}



