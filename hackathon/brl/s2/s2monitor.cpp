#include "s2monitor.h"


S2ScanData::S2ScanData(){
    s2ScanImage = QImage(1,1,QImage::Format_RGB888);
    s2ScanImage.setPixel(0,0,0);
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    currentRepoHash = GIT_CURRENT_SHA1;
#endif
}

void S2ScanData::addNewTile(TileInfo newTileInfo){
    allTiles.append(newTileInfo);
    s2ScanDir = QFileInfo(newTileInfo.getFileString()).absoluteDir();
    QList<LocationSimple> pixelLocations;
    QList<LocationSimple> galvoLocations;
    QList<long> scanNumbers;
    for (int i = 0; i<allTiles.length(); i++){
        pixelLocations.append(allTiles.at(i).getPixelLocation());
        galvoLocations.append(allTiles.at(i).getGalvoLocation());
        scanNumbers.append(allTiles.at(i).getScanIndex());
    }
    if (allTiles.length()>0) {
        qDebug()<<"not updating our scan image...";
        //       S2ScanImageUpdater newUpdater;
        // newUpdater.initialize(pixelLocations,galvoLocations,scanNumbers,s2ScanDir);
        //  QTimer::singleShot(10,&newUpdater,SLOT(createScanImage()));
    }
    //updateS2Summary();
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
    /*

    QPainter myPainter;
    myPainter.begin(&s2ScanImage);
    myPainter.setPen(Qt::yellow);
    for (int i = 0; i<imageLocations.length(); i++){
        numberPPList.at(i).moveTo(imageLocations.at(i).x+30, imageLocations.at(i).y+10);
        myPainter.drawPath(numberPPList.at(i));
    }
    */
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





/*
S2ScanImageUpdater::S2ScanImageUpdater(){}



void S2ScanImageUpdater::initialize(QList<LocationSimple> pixelLocations, QList<LocationSimple> galvoLocations, QList<long> scanNumbers, QDir saveDirectory){
    this->pixelLocations = pixelLocations;
    this->galvoLocations = galvoLocations;
    this->scanNumbers = scanNumbers;
    saveDir = saveDirectory;
}

void S2ScanImageUpdater::createScanImage(QImage blankInputImage, QPainter testPainter){
    float minx = 1000000.0;
    float maxx = -1000000.0;
    float miny = 1000000.0;
    float maxy = -1000000.0;
    long sizex=0;
    long sizey=0;
    float boundingBoxX;
    float boundingBoxY;
    float totalTileArea;
    float imagedArea;
    QList<LocationSimple> imageLocations;
    totalTileArea=0;
    for (int i = 0; i<pixelLocations.length(); i++){
        LocationSimple locationi = pixelLocations.at(i);
        locationi.ev_pc1 = galvoLocations.at(i).ev_pc1;
        locationi.ev_pc2 = galvoLocations.at(i).ev_pc2;
        if (locationi.x<minx) minx = locationi.x;
        if (locationi.x+locationi.ev_pc1>maxx) maxx = locationi.x+locationi.ev_pc1;
        if (locationi.y<miny) miny = locationi.y;
        if (locationi.y+ locationi.ev_pc2 >maxy) maxy = locationi.y+locationi.ev_pc1;

        sizex = (long)  (maxx-minx+1.);
        sizey = (long)  (maxy-miny+1.);
        totalTileArea = totalTileArea+ (float) locationi.ev_pc2 * (float) locationi.ev_pc1;
    }



    for (int i = 0; i<pixelLocations.length(); i++){
        LocationSimple locationi = pixelLocations.at(i);
        locationi.x = locationi.x-minx;
        locationi.y = locationi.y-miny;
        imageLocations.append(locationi);
    }
    blankInputImage  = QImage(sizex,sizey,QImage::Format_RGB888);
    blankInputImage.fill(0);

    for (int i = 0; i<imageLocations.length(); i++){
        for (long k = imageLocations.at(i).x; k< (imageLocations.at(i).x+imageLocations.at(i).ev_pc1); k++){
            for (long j = imageLocations.at(i).y; j< (imageLocations.at(i).y+imageLocations.at(i).ev_pc2); j++){
                QRgb pixelValue =  blankInputImage.pixel(k,j);
                pixelValue = ((uint) pixelValue ) + (uint) 1;
                blankInputImage.setPixel(k,j,pixelValue);



            }
        }
    }





    boundingBoxX = (float) sizex;
    boundingBoxY = (float) sizey;

    imagedArea = 0;
    for (long imi=0; imi<sizex; imi++){
        for (long imj=0; imj<sizey; imj++){
            int imijpixel =  qBlue( blankInputImage.pixel(imi, imj));

            if ( imijpixel > 0) imagedArea++;
        }
    }


    testPainter.begin(&blankInputImage);
    testPainter.setPen(Qt::yellow);
    for (int i = 0; i<imageLocations.length(); i++){
        testPainter.drawText( QRectF(imageLocations.at(i).x+30, imageLocations.at(i).y+10, imageLocations.at(i).ev_pc1, imageLocations.at(i).ev_pc2), QString::number(scanNumbers.at(i)));
    }
    blankInputImage.save(saveDir.absolutePath().append(QDir::separator()).append("S2Image.tif"));


}




*/











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

TileInfoMonitor::TileInfoMonitor(){
    tileInfoList.clear();
    seedListList.clear();
    intList.clear();
    tracingMethodStrings.clear();
    running = false;
}

void TileInfoMonitor::addTileData(TileInfo incomingTile, LandmarkList seedList, int tileStatus, int correctX, int correctY, QString tracingMethod){
    tileInfoList.append(incomingTile);
    seedListList.append(seedList);
    QList<int> triplet;
    triplet.append(tileStatus);
    triplet.append(correctX);
    triplet.append(correctY);
    intList.append(triplet);
    tracingMethodStrings.append(tracingMethod);
    if (!running)   {
        running = true;
        QTimer::singleShot(0,this,SLOT(searchForTiles()));
    }
    qDebug()<<"ADD TILE TO TILEMONITOR: x ="<<correctX<<" y="<<correctY;
}

QList<TileInfo> TileInfoMonitor::getTileInfoList() const{
    return tileInfoList;
}
void TileInfoMonitor::searchForTiles(){
    running = true;
    QString realFileString;
    QDir saveDir;
    QStringList fileFilter;
    QFileInfoList fileInfoList;
    QString fileFinder;
    bool foundIt =false;
    bool useNewXY =false;
    int correctX;
    int correctY;


    for (int i =0; i<tileInfoList.length(); i++){
        useNewXY = false;
        // if the desired tile hasn't been imaged yet, the actual filename is unknown.
        if (tileInfoList.at(i).getFileString().contains("*")){
            // need to check for +/- 4 due to vaguely rounded tile locations.
            saveDir = QFileInfo(tileInfoList.at(i).getFileString()).absoluteDir();
            int incomingX = intList.at(i).at(1);
            int incomingY = intList.at(i).at(2);
            foundIt = false;
            for (int jj = incomingX-4; jj<=incomingX+4; jj++){
                for (int kk = incomingY-4; kk<= incomingY+4; kk++){
                    fileFinder = QString("x_").append(QString::number(jj)).append("_y_").append(QString::number(kk)).append("*.v3draw");
                    fileFilter.clear();
                    fileFilter.append(fileFinder);
                    fileInfoList = saveDir.entryInfoList(fileFilter);
                    if (!fileInfoList.isEmpty()){
                        correctX = jj;
                        correctY = kk;
                        foundIt=true;
                        break;
                    }else{
                        correctX = incomingX;
                        correctY = incomingY;
                    }
                }
                if ((foundIt)&((incomingX!=correctX)|(incomingY!=correctY))) {
                    qDebug()<<"original xy location "<<incomingX<<" "<<incomingY<<" final location "<<correctX<<" "<<correctY;
                    useNewXY = true;
                    break;}
                if (foundIt) break;
            }
            if (foundIt){
                realFileString = fileInfoList.at(0).absoluteFilePath();
            }else{
                realFileString = "";
                continue;
            }
        }else{
            realFileString = tileInfoList.at(i).getFileString();
        }

        QFileInfo iFileInfo(realFileString);

        QString swcString = iFileInfo.absolutePath().append(QDir::separator()).append(iFileInfo.completeBaseName()).append(".swcX");
        iFileInfo = QFileInfo(swcString);
        if (iFileInfo.isReadable()){
            TileInfo toEmit= tileInfoList.at(i);
            toEmit.setFileString(realFileString);
            if (useNewXY){
                emit foundTile(toEmit, seedListList.at(i), 1, correctX, correctY) ;

            }else{
                emit foundTile(toEmit, seedListList.at(i), 1, intList.at(i).at(1), intList.at(i).at(2)) ;
            }
            qDebug()<<"REMOVE tilE FROM TILEMONITOR: "<<toEmit.getFileString();

            seedListList.removeAt(i);
            intList.removeAt(i);
            tracingMethodStrings.removeAt(i);
            tileInfoList.removeAt(i);
            break;
        }

    }


    if (tileInfoList.isEmpty()){
        running = false;
        return;
    }else{
        QTimer::singleShot(100,this,SLOT(searchForTiles()));
    }
}
