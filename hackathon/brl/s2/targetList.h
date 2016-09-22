#ifndef TARGETLIST_H
#define TARGETLIST_H

#include <QWidget>
#include <QTableWidget>
#include "v3d_message.h"
#include <v3d_interface.h>
#include "tileInfo.h"
class TargetList : public QWidget
{
    Q_OBJECT
public:
    explicit TargetList(QWidget *parent = 0);


signals:
    
public slots:
    void updateTargetTable(QList<TileInfo> inputTargetList, QList<LandmarkList> inputScanLocations);
    void addTarget(const LocationSimple &newTarget);
    void addScanLoc(int row, LocationSimple newScanLoc, float xWidth, float yWidth);
    void updateScanTable(int newIndex, int ignore);

private:
    QTableWidget* targetTable;
    QTableWidget* scanLocTable;
    QGridLayout* mainLayout;
    QList<LandmarkList> scanLocations;
};

#endif // TARGETLIST_H
