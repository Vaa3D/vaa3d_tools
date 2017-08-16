#include "tileInfo.h"

TileInfo::TileInfo(){
}

TileInfo::TileInfo(float zoomPixelsProduct)
{
    this->zoomPixelsProduct = zoomPixelsProduct;
    zoomSet = false;
    resOK = false;
    setZoomPos(6.0);// set default zoom so this constructor actually populates everything needed in getTileInfoString
    tilev3drawFileString = "noFileStringYet.v3draw";
    timeStampCategory = 0;
    tileTimes.clear();
    elapsedTimes.clear();
    scanIndex =0;
    // 1st timestamp is when tile is added to queue  S2UI::handleNewLocation or S2UI::startingSmartScan
    // 2nd timestamp when tile is sent to the microscope for imaging S2UI::s2ROIMonitor
    // 3rd timestamp is when the tile  is done imaging and sent for analysis.
    //
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

int TileInfo::setTimeStamp(QDateTime timeNow){
    tileTimes.append(timeNow);
    if (tileTimes.length()==1){
        elapsedTimes.append(0.0);
    }else{
        quint64 timeSinceLast = tileTimes.at(timeStampCategory).toMSecsSinceEpoch() - tileTimes.at(timeStampCategory-1).toMSecsSinceEpoch();
        elapsedTimes.append((float) timeSinceLast);
    }
    timeStampCategory++;
	qDebug() << "tileTimeLengeth " << tileTimes.length();
return tileTimes.length();
}

QList<float> TileInfo::getElapsedTimes()const{
    return elapsedTimes;
}

QStringList TileInfo::getTimeStrings()const{
    QStringList outputTimeStrings;
    for (int i=0; i<tileTimes.length(); i++){
        outputTimeStrings.append(tileTimes.at(i).toString("yyyy_MM_dd_ddd_hh_mm_ss_zzz"));
    }
return outputTimeStrings;
}

float TileInfo::getTileZoom()const{if (zoomSet){return tileZoom;}else{return -1;}}
int TileInfo::getTilePixelsX()const{if (zoomSet){return pixelsX;}else{return -1;}}
int TileInfo::getTilePixelsY()const{if (zoomSet){return pixelsY;}else{return -1;}}
LocationSimple TileInfo::getPixelLocation()const{return pixelLocation;}
LocationSimple TileInfo::getStageLocation()const{return stageLocation;}
LocationSimple TileInfo::getGalvoLocation()const{return galvoLocation;}
QString TileInfo::getFileString()const{return tilev3drawFileString;}
QList<QDateTime> TileInfo::getTileTimes()const{return tileTimes;}
long TileInfo::getScanIndex()const{return scanIndex;}

void TileInfo::setPixelLocation(LocationSimple loc){pixelLocation=loc;}
void TileInfo::setStageLocation(LocationSimple loc){stageLocation=loc;}
void TileInfo::setGalvoLocation(LocationSimple loc){galvoLocation=loc;}
void TileInfo::setFileString(QString inputString){tilev3drawFileString=inputString;}
void TileInfo::setScanIndex(long sIndex){scanIndex = sIndex;}
