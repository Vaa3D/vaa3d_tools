#ifndef PLUGINSERVICES_H
#define PLUGINSERVICES_H

#include <Matrix3D.h>
#include <QObject>
#include <QMenu>
#include <QList>
#include "CommonTypes.h"

class AnnotatorWnd;
class QWidget;

class PluginServices
{
private:
    AnnotatorWnd *mAnnWnd;
    QString       mPluginName;
    QMenu        *mPluginMenu;

public:
    // returns ref to volume voxel data
    Matrix3D<PixelType> &  getVolumeVoxelData() const;
    Matrix3D<ScoreType> &  getScoreVoxelData() const;
    Matrix3D<LabelType> &  getLabelVoxelData() const;

    // updates image display
    void  updateDisplay() const;

    // menu to which plugins can add items and submenus
    //  and connect signals/slots to
    QMenu * getPluginMenu() const;

    // returns main window pointer. Useful to show messageboxes, etc..
    QWidget *getMainWindow() const;

    // returns maximum number of overlay volumes
    static unsigned int getMaxOverlayVolumes();

    // returns a reference to a given overlay volume data
    Matrix3D<ScoreType> & getOverlayVolumeData( unsigned int num ) const;

    // enables/disables the visualization of a given overlay
    void setOverlayVisible( unsigned int num, bool visible ) const;

    /** constructor only called by the main app **/
    PluginServices( const QString &pluginName, AnnotatorWnd *annWnd );
};

typedef QList<PluginServices>   PluginServicesList;

#endif // PLUGINSERVICES_H
