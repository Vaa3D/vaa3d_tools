#ifndef REGIONLISTFRAME_H
#define REGIONLISTFRAME_H

#include <QFrame>
#include <QStringList>
#include<QListWidgetItem>
#include "ShapeStatistics.h"
#include <QModelIndex>

namespace Ui {
    class RegionListFrame;
}

class AnnotatorWnd;

class RegionListFrame : public QFrame
{
    Q_OBJECT

private:
    std::vector< ShapeStatistics<> > mRegionDescriptions;
    AnnotatorWnd *mAnnotatorWnd;

public:
    explicit RegionListFrame(QWidget *parent, AnnotatorWnd *annWnd);
    ~RegionListFrame();

    void setRegionData( const std::vector< ShapeStatistics<> > &info );

    void moveToBottomLeftCorner();

public slots:
    void listCurrentItemChanged( QListWidgetItem *, QListWidgetItem * );
    void saveAsClicked();
    void listClickedSignal(QModelIndex mIdx);
    void butLabelRegionClicked();

protected:
    void keyReleaseEvent(QKeyEvent *);

    void closeEvent(QCloseEvent *event);

signals:
    // this is emitted when the current region selection is changed
    void currentRegionChanged(int newRegionIdx);

    // emitted when the user clicked to label a region
    void labelRegion( unsigned int regionIdx , unsigned int labelIdx );

    void widgetClosed();

private:
    Ui::RegionListFrame *ui;
};

#endif // REGIONLISTFRAME_H
