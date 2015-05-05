#ifndef GRAPHCUTSPLUGIN_H
#define GRAPHCUTSPLUGIN_H

/** We shouldn't need a .h file
  * but it is necessary for the Q_OBJECT correctness (vtable)
  */

#include <PluginBase.h>
#include <cstdlib>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTimer>

using namespace std;

const int idx_seed_overlay = 0;
const int idx_bindata_overlay = 1;
const int idx_weight_overlay = 2;
const int idx_output_overlay = 3;
const int idx_label_overlay = 4;

const int label_background = 0;
const int label_seed = 128;
const int label_sink = 255;

enum eGCType
{
    GC_DEFAULT = 0,
    GC_LIMITED,
    GC_SCORES,
    GC_DATA
};

class GraphCutsPlugin : public PluginBase
{
    Q_OBJECT
private:
    const PluginServices* mPluginServices;

    QTimer* timer;

    // id of the active overlay
    int activeOverlay;

    int brushSizeX;
    int brushSizeY;
    int brushSizeZ;

    // max size for limited size graph-cut
    int maxWidth;
    int maxHeight;
    int maxDepth;

    float gaussianVariance;
    float sigma;

    bool mouseEventDetected;

    uchar* outputWeightImage;
    float cache_gaussianVariance;

public:
    GraphCutsPlugin(QObject *parent = 0);

    ~GraphCutsPlugin();

    int getActiveOverlay() { return activeOverlay; }
    int getBrushSizeX() { return brushSizeX; }
    int getBrushSizeY() { return brushSizeY; }
    int getBrushSizeZ() { return brushSizeZ; }

    bool    initializePlugin( const PluginServices &pServices )
    {
        mPluginServices = &pServices;

        /** Add a menu item **/
        {
            QAction *action = mPluginServices->getPluginMenu()->addAction( QString("Run") );
            action->setData(GC_DEFAULT);
            action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
            connect( action, SIGNAL(triggered()), this, SLOT(runGraphCuts()) );
        }

        /** Add a menu item **/
        {
            QAction *action = mPluginServices->getPluginMenu()->addAction( QString("Run (limited size)") );
            action->setData(GC_LIMITED);
            connect( action, SIGNAL(triggered()), this, SLOT(runGraphCuts()) );
        }

        /** Add a menu item **/
        {
            QAction *action = mPluginServices->getPluginMenu()->addAction( QString("Run (scores)") );
            action->setData(GC_SCORES);
            connect( action, SIGNAL(triggered()), this, SLOT(runGraphCuts()) );
        }

        /** Add a menu item **/
        {
            QAction *action = mPluginServices->getPluginMenu()->addAction( QString("Run (data)") );
            action->setData(GC_DATA);
            connect( action, SIGNAL(triggered()), this, SLOT(runGraphCuts()) );
        }

        /** Add a menu item **/
        {
            QAction *action = mPluginServices->getPluginMenu()->addAction( "Clean seed overlay" );
            action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
            connect( action, SIGNAL(triggered()), this, SLOT(cleanSeedOverlay()) );
        }

        /** Add a menu item **/
        {
            QAction *action = mPluginServices->getPluginMenu()->addAction( "Transfer overlay" );
            action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
            connect( action, SIGNAL(triggered()), this, SLOT(transferOverlay()) );
        }

        /** Add a menu item **/
        {
            QAction *action = mPluginServices->getPluginMenu()->addAction( "Settings" );
            action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
            connect( action, SIGNAL(triggered()), this, SLOT(changeSettings()) );
        }

        return true;
    }

    // must return the plugin's name
    QString pluginName() {
        return "GraphCut plugin";
    }

    virtual void mouseReleaseEvent(QMouseEvent *evt, unsigned int imgX, unsigned int imgY, unsigned int imgZ)
    {
        mouseEvent(evt, imgX, imgY, imgZ);
    }

    virtual void  mouseMoveEvent( QMouseEvent *evt, unsigned int imgX, unsigned int imgY, unsigned int imgZ )
    {
        if ( (evt->buttons() & Qt::LeftButton) == 0 && (evt->buttons() & Qt::RightButton) == 0)
           return;

        mouseEvent(evt, imgX, imgY, imgZ);
    }

    virtual void  mouseEvent( QMouseEvent *evt, unsigned int imgX, unsigned int imgY, unsigned int imgZ )
    {
        Matrix3D<ScoreType> &activeOverlayMatrix = mPluginServices->getOverlayVolumeData(activeOverlay);

        if (activeOverlayMatrix.isEmpty())
        {
            activeOverlayMatrix.reallocSizeLike( mPluginServices->getVolumeVoxelData() );
            activeOverlayMatrix.fill(0);
        }

        if (evt->modifiers() & Qt::ControlModifier || evt->buttons() & Qt::RightButton) {
            for(int x = max(0, (int)imgX - brushSizeX); x <= min(activeOverlayMatrix.width()-1, imgX + brushSizeX); ++x) {
                for(int y = max(0, (int)imgY - brushSizeY); y <= min(activeOverlayMatrix.height()-1, imgY + brushSizeY); ++y) {
                    for(int z = max(0, (int)imgZ - brushSizeZ); z <= min(activeOverlayMatrix.depth()-1, imgZ + brushSizeZ); ++z) {
                        activeOverlayMatrix(x,y,z) = 0;
                    }
                }
            }
        } else {
            if (evt->modifiers() & Qt::ShiftModifier) {
                for(int x = max(0, (int)imgX - brushSizeX); x <= min(activeOverlayMatrix.width()-1, imgX + brushSizeX); ++x) {
                    for(int y = max(0, (int)imgY - brushSizeY); y <= min(activeOverlayMatrix.height()-1, imgY + brushSizeY); ++y) {
                        for(int z = max(0, (int)imgZ - brushSizeZ); z <= min(activeOverlayMatrix.depth()-1, imgZ + brushSizeZ); ++z) {
                            activeOverlayMatrix(x,y,z) = 128;
                        }
                    }
                }
            }
            else {
                for(int x = max(0, (int)imgX - brushSizeX); x <= min(activeOverlayMatrix.width()-1, imgX + brushSizeX); ++x) {
                    for(int y = max(0, (int)imgY - brushSizeY); y <= min(activeOverlayMatrix.height()-1, imgY + brushSizeY); ++y) {
                        for(int z = max(0, (int)imgZ - brushSizeZ); z <= min(activeOverlayMatrix.depth()-1, imgZ + brushSizeZ); ++z) {
                            activeOverlayMatrix(x,y,z) = 255;
                        }
                    }
                }
            }
        }

        mouseEventDetected = true;
        //mPluginServices->setOverlayVisible(activeOverlay, true );
        //mPluginServices->updateDisplay();
    }

public slots:

    void changeSettings();

    void cleanSeedOverlay();

    void runGraphCuts();

    // transfer overlay created by plugin to input overlay
    void transferOverlay();

    void showMsgBoxClicked()
    {
        QMessageBox::information( mPluginServices->getMainWindow(), "Clicked me!", "You have just clicked me." );
    }

    void updateOverlay()
    {
        if(mouseEventDetected) {
            mouseEventDetected = false;
            mPluginServices->setOverlayVisible(activeOverlay, true );
            mPluginServices->updateDisplay();
        }
    }
};

#endif // GRAPHCUTSPLUGIN_H
