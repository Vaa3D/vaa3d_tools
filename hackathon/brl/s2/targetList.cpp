#include "targetList.h"

TargetList::TargetList(QWidget *parent) :
    QWidget(parent)
{

    targetTable = new QTableWidget(1,3,this);
    scanLocTable = new QTableWidget(1,5,this);
    QLabel* targetTableLabel = new QLabel("&Targets");
    QLabel* scanLocTableLabel = new QLabel("ROI locations");
    QStringList ttHeaders;
    ttHeaders.append("Target Number");
    ttHeaders.append("x (um)");
    ttHeaders.append("y (um)");
    targetTable->setHorizontalHeaderLabels(ttHeaders);
    QStringList sltHeaders;
    sltHeaders.append("ROI Number");
    sltHeaders.append("x (um)");
    sltHeaders.append("y (um)");
    sltHeaders.append("x width");
    sltHeaders.append("y width");
    scanLocTable->setHorizontalHeaderLabels(sltHeaders);

    targetTableLabel->setBuddy(targetTable);


    connect(targetTable,SIGNAL(cellClicked(int,int)), this, SLOT(updateScanTable(int,int)));

    mainLayout = new QGridLayout;
    mainLayout->addWidget(targetTableLabel,0,0);
    mainLayout->addWidget(targetTable, 1,0,3,2);
    mainLayout->addWidget(scanLocTableLabel,0,2);
    mainLayout->addWidget(scanLocTable,1,2,3,2 );

    scanLocTable->adjustSize();
    targetTable->adjustSize();

    setLayout(mainLayout);
    qDebug()<<"constructed targettable";
    qDebug()<<QString::number(scanLocTable->isVisible());
    LandmarkList fakeTargetList;
    LocationSimple testLoc1;
    testLoc1.x = 1.0;
    testLoc1.y = 122.9;
    testLoc1.ev_pc1 = 230;
    testLoc1.ev_pc2 = 234;
    LocationSimple testLoc2;
    testLoc2.x = 81.0;
    testLoc2.y = 22.9;
    testLoc2.ev_pc1 = 20;
    testLoc2.ev_pc2 = 24;

    fakeTargetList.append(testLoc1);
    fakeTargetList.append(testLoc2);
    fakeTargetList.append(testLoc1);
    QList<LandmarkList> fakeScanLocations;
    fakeScanLocations.append(fakeTargetList);
    fakeTargetList.append(testLoc1);
    fakeScanLocations.append(fakeTargetList);

    updateTargetTable(fakeTargetList, fakeScanLocations);

}

void TargetList::updateTargetTable(LandmarkList inputTargetList, QList<LandmarkList> inputScanLocations){


    targetList = inputTargetList;
    scanLocations = inputScanLocations;
    if (inputTargetList.isEmpty()){
        qDebug()<<"no targets";
        return;
    }

    targetTable->clearContents();

    for (int i=0; i<inputTargetList.length(); i++){
        targetTable->insertRow(i);

        QTableWidgetItem *cell =   targetTable->item(i,0);
        if (!cell){
            cell = new QTableWidgetItem;
            targetTable->setItem(i,0,cell);
        }
        cell->setText(QString::number(i));



        cell =   targetTable->item(i,1);
        if (!cell){
            cell = new QTableWidgetItem;
            targetTable->setItem(i,1,cell);
        }
        cell->setText(QString::number(inputTargetList.value(i).x));

        cell =   targetTable->item(i,2);
        if (!cell){
            cell = new QTableWidgetItem;
            targetTable->setItem(i,2,cell);
        }
        cell->setText(QString::number(inputTargetList.value(i).y));



    }
    targetTable->setCurrentCell(0,0);
    targetTable->adjustSize();
}

void TargetList::addTarget(const LocationSimple& newTarget){

}
void TargetList::addScanLoc(int row, LocationSimple newScanLoc, float xWidth, float yWidth){
if (scanLocTable->rowCount()<=row){
scanLocTable->insertRow(row);
}
    QTableWidgetItem *cell = scanLocTable->item(row, 0);
    if ( !cell) {
        cell = new QTableWidgetItem;
    }
    cell->setText(QString::number(row));
    scanLocTable->setItem(row, 0,cell);


    cell = scanLocTable->item(row, 1);
    if ( !cell) {
        cell = new QTableWidgetItem;
    }
    cell->setText(QString::number(newScanLoc.x));
    scanLocTable->setItem(row, 1,cell);


    cell = scanLocTable->item(row, 2);

    if ( !cell) {
        cell = new QTableWidgetItem;
    }
    cell->setText(QString::number(newScanLoc.y));
    scanLocTable->setItem(row, 2,cell);

    cell = scanLocTable->item(row, 3);
    if ( !cell) {
        cell = new QTableWidgetItem;
    }
    cell->setText(QString::number(xWidth));
    scanLocTable->setItem(row, 3,cell);

    cell = scanLocTable->item(row, 4);
    if ( !cell) {
        cell = new QTableWidgetItem;
    }
    cell->setText(QString::number(yWidth));
    scanLocTable->setItem(row, 4,cell);



}

void TargetList::updateScanTable(int newIndex, int ignore){
    qDebug()<<"updatingScanTable";
        scanLocTable->clearContents();
        if ((newIndex+1)>scanLocations.length()){return;}
        for (int i=0; i<scanLocations[newIndex].length();i++){
            addScanLoc(i,scanLocations[newIndex].value(i), scanLocations[newIndex].value(i).ev_pc1,  scanLocations[newIndex].value(i).ev_pc1);
        }
}

