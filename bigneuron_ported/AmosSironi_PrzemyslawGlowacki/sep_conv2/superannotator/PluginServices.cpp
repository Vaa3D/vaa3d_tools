#include "PluginServices.h"
#include "annotatorwnd.h"

// to generate num of overlays according to this
#include "ColorLists.h"

Q_DECL_EXPORT Matrix3D<PixelType> &  PluginServices::getVolumeVoxelData() const
{
    return mAnnWnd->getVolumeVoxelData();
}

Q_DECL_EXPORT Matrix3D<ScoreType> &  PluginServices::getScoreVoxelData() const
{
    return mAnnWnd->getScoreVoxelData();
}

Q_DECL_EXPORT Matrix3D<LabelType> &  PluginServices::getLabelVoxelData() const
{
    return mAnnWnd->getLabelVoxelData();
}

// updates image display
Q_DECL_EXPORT void  PluginServices::updateDisplay() const
{
    mAnnWnd->pluginUpdateDisplay();
}

// menu to which plugins can add items and submenus
//  and connect signals/slots to
Q_DECL_EXPORT QMenu * PluginServices::getPluginMenu() const
{
    return mPluginMenu;
}

Q_DECL_EXPORT QWidget *PluginServices::getMainWindow() const
{
    return dynamic_cast<QWidget *>( mAnnWnd );
}

Q_DECL_EXPORT unsigned int PluginServices::getMaxOverlayVolumes()
{
    static bool firstTime = true;
    static unsigned int maxOverlayVolumes;

    if (firstTime) {
        OverlayColorList cList;
        maxOverlayVolumes = cList.count();
        firstTime = false;
    }

    return maxOverlayVolumes;
}

Q_DECL_EXPORT Matrix3D<ScoreType> & PluginServices::getOverlayVolumeData( unsigned int num ) const
{
    //TODO that's kind of cheating, as it returns the wrong overlay:
    if (num >= getMaxOverlayVolumes())
        return getOverlayVolumeData( getMaxOverlayVolumes() - 1 ) ;

    return mAnnWnd->getOverlayVoxelData(num);
}

// enables/disables the visualization of a given overlay
Q_DECL_EXPORT void PluginServices::setOverlayVisible( unsigned int num, bool visible ) const
{
    mAnnWnd->setOverlayVisible(num, visible);
}

// this is not exported because we call it inside the app
PluginServices::PluginServices( const QString &pluginName, AnnotatorWnd *annWnd )
{
    mPluginName = pluginName;
    mAnnWnd = annWnd;

    // add a menu
    mPluginMenu = annWnd->getPluginMenuPtr()->addMenu( pluginName );
}
