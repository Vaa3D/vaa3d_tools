#include "tileInfo.h"

TileInfo::TileInfo(){
    qDebug()<<"TileInfo constructor...";
}

TileInfo::TileInfo(float zoomPixelsProduct)
{
    this->zoomPixelsProduct = zoomPixelsProduct;
    zoomSet = false;
    resOK = false;
    setZoomPos(6.0);// set default zoom so this constructor actually populates everything needed in getTileInfoString
}


void TileInfo::setZoomPos( float tileZoom){

    this->tileZoom = tileZoom;

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
    a.append(QString::number(resOK));
    }
return a;
}
float TileInfo::getTileZoom()const{if (zoomSet){return tileZoom;}else{return -1;}}
int TileInfo::getTilePixelsX()const{if (zoomSet){return pixelsX;}else{return -1;}}
int TileInfo::getTilePixelsY()const{if (zoomSet){return pixelsY;}else{return -1;}}
LocationSimple TileInfo::getPixelLocation()const{return pixelLocation;}
LocationSimple TileInfo::getStageLocation()const{return stageLocation;}
LocationSimple TileInfo::getGalvoLocation()const{return galvoLocation;}



void TileInfo::setPixelLocation(LocationSimple loc){pixelLocation=loc;}
void TileInfo::setStageLocation(LocationSimple loc){stageLocation=loc;}
void TileInfo::setGalvoLocation(LocationSimple loc){galvoLocation=loc;}

