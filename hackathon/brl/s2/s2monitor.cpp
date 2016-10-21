#include "s2monitor.h"


S2ScanData::S2ScanData(){
    s2ScanImage = QImage(1,1,QImage::Format_RGB888);
    s2ScanImage.setPixel(0,0,0);
    currentRepoHash = GIT_CURRENT_SHA1;
}

void S2ScanData::addNewTile(TileInfo newTileInfo){
    allTiles.append(newTileInfo);
    s2ScanDir = QFileInfo(newTileInfo.getFileString()).absoluteDir();
    updateS2ScanImage();
    updateS2Summary();
}

void S2ScanData::updateS2ScanImage(){
    float minx = 1000000.0;
    float maxx = -1000000.0;
    float miny = 1000000.0;
    float maxy = -1000000.0;
    long sizex=0;
    long sizey=0;
    QList<LocationSimple> imageLocations;
    totalTileArea=0;
    for (int i = 0; i<allTiles.length(); i++){
        LocationSimple locationi = allTiles.at(i).getPixelLocation();
        locationi.ev_pc1 = allTiles.at(i).getGalvoLocation().ev_pc1;
        locationi.ev_pc2 = allTiles.at(i).getGalvoLocation().ev_pc2;
        if (locationi.x<minx) minx = locationi.x;
        if (locationi.x+locationi.ev_pc1>maxx) maxx = locationi.x+locationi.ev_pc1;
        if (locationi.y<miny) miny = locationi.y;
        if (locationi.y+ locationi.ev_pc2 >maxy) maxy = locationi.y+locationi.ev_pc1;

        sizex = (long)  (maxx-minx+1.);
        sizey = (long)  (maxy-miny+1.);
        totalTileArea = totalTileArea+ (float) locationi.ev_pc2 * (float) locationi.ev_pc1;
    }



    for (int i = 0; i<allTiles.length(); i++){
        LocationSimple locationi = allTiles.at(i).getPixelLocation();
        locationi.x = locationi.x-minx;
        locationi.y = locationi.y-miny;
        imageLocations.append(locationi);
    }
    s2ScanImage  = QImage(sizex,sizey,QImage::Format_RGB888);
    s2ScanImage.fill(0);

    for (int i = 0; i<imageLocations.length(); i++){
        for (long k = imageLocations.at(i).x; k< (imageLocations.at(i).x+imageLocations.at(i).ev_pc1); k++){
            for (long j = imageLocations.at(i).y; j< (imageLocations.at(i).y+imageLocations.at(i).ev_pc2); j++){
                QRgb pixelValue =  s2ScanImage.pixel(k,j);
                pixelValue = ((uint) pixelValue ) + (uint) 1;
                s2ScanImage.setPixel(k,j,pixelValue);



            }
        }
     }





    boundingBoxX = (float) sizex;
    boundingBoxY = (float) sizey;

    imagedArea = 0;
    for (long imi=0; imi<sizex; imi++){
        for (long imj=0; imj<sizey; imj++){
            int imijpixel =  qBlue( s2ScanImage.pixel(imi, imj));

            if ( imijpixel > 0) imagedArea++;
        }
    }


    QPainter myPainter;
    myPainter.begin(&s2ScanImage);
    myPainter.setPen(Qt::yellow);
    for (int i = 0; i<imageLocations.length(); i++){
        myPainter.drawText( QRectF(imageLocations.at(i).x+30, imageLocations.at(i).y+10, imageLocations.at(i).ev_pc1, imageLocations.at(i).ev_pc2), QString::number(allTiles.at(i).getScanIndex()));
    }
    s2ScanImage.save(s2ScanDir.absolutePath().append(QDir::separator()).append("S2Image.tif"));


}


void S2ScanData::updateS2Summary(){
    //


}


QImage S2ScanData::getS2ScanImage()const{
    return s2ScanImage;
}

QList<TileInfo> S2ScanData::getAllTileInfo()const{
    return allTiles;
}






S2Monitor::S2Monitor(QWidget *parent) :
    QWidget(parent)
{
    currentScanNumber =0;
}

void S2Monitor::setSaveDir(QDir directory){
    saveDir = directory;
}

void S2Monitor::writeAllScanData(){
    for (int i=0; i<allScanData.length();i++){
        writeScanData(i);
    }
}

void S2Monitor::writeScanData(int scanNumber){
    // write summary data for a single s2scan  to saveDir
}

void S2Monitor::startNewScan(){
    currentScanNumber++;
}


void S2Monitor::addNewTile(TileInfo newTileInfo){
    if (currentScanNumber>allScanData.length()) {
        S2ScanData newS2ScanData = S2ScanData();
        newS2ScanData.addNewTile(newTileInfo);
        allScanData.append(newS2ScanData);
    }else{
        allScanData.last().addNewTile(newTileInfo);
    }
}
