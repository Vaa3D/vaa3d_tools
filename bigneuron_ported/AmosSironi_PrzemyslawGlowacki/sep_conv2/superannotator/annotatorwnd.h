#ifndef ANNOTATORWND_H
#define ANNOTATORWND_H

//Qt4
#include <QMainWindow>
//Qt5
//#include <QtWidgets>

#include "Matrix3D.h"
#include "ColorLists.h"

#include "Region3D.h"
#include "CommonTypes.h"

#include <QFileInfo>
//#include <stddef.h>
#include "brush.h"

#include "overlay.h"


namespace Ui {
    class AnnotatorWnd;
}

struct SupervoxelSelection;

class AnnotatorWnd : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnnotatorWnd(QWidget *parent = 0);
    ~AnnotatorWnd();

private:
    // SETTINGS
    QString m_sSettingsFile;

    struct {
        QString savePath;
        QString saveFilePath;
        QString loadPath;   // these are for annotation

        QString savePathScores; // for score volume
        QFileInfo saveFileInfoScores; // for annotation volume
        QString loadPathScores; // for score volume
        QString loadPathVolume; // for main volume
        QString fijiExePath;

        unsigned maxVoxForSVox;
        unsigned sliceJump;
    } mSettingsData;

    void loadSettings();
    void saveSettings();

protected:
    void closeEvent(QCloseEvent *);

    // if importAsLabel is positive, then it asks for a threshold and sets
    //   the values > threshold as label "importAsLabel"
    bool loadAnnotation(const QString& fileName, int importAsLabel = -1, LabelType threshold = 0);

    bool saveAnnotation(const QString& fileName);

private:
    Ui::AnnotatorWnd *ui;
    int mCurZSlice;
    int mCurX;
    int mCurY;
    // if label file shoul be saved on exit, and which would be the path
    bool        mSaveLabelsOnExit;
    QString     mSaveLabelsOnExitPath;

    QString mFileTypeFilter;

    // colors for labels + icons
    LabelColorList   mLblColorList;
    OverlayColorList mOverlayColorList;
    SelectionColor   mSelectionColor;
    ScoreColor       mScoreColor;

    // returns a region3d object according to current viewport
    //  and the slice spin box in the form
    Region3D getViewportRegion3D();

    Matrix3D<PixelType>  mVolumeData;    // loaded data (volume), whole volume
    Matrix3D<LabelType>  mVolumeLabels;  // labels for each pixel in original volume

    Matrix3D<PixelType>  mCroppedVolumeData;    // cropped version of data, to keep sizes manageable


    // score image (if loaded), only for aid in labeling
    Matrix3D<ScoreType> mScoreImage;
    bool                mScoreImageEnabled;

    CubeBrush cubeBrush;
    SphereBrush sphereBrush;
    PixelBrush pixelBrush;

    void updateCursorPixelInfo( int x, int y, int z );   // shows current pixel position

    bool   mOverlayLabelImage;    // if true, then an overlay is drawn on top of the image, showing color-coded pixel labels

    void annotateSupervoxel( const SupervoxelSelection &SV, LabelType label, bool onlyCurrentSlice = false );

    // scans for plugins and adds them.
    void scanPlugins( const QString &pluginFolder );


    void runConnectivityCheck( const Region3D &reg );

    void labelRegion(Matrix3D<LabelType> &data, uint regionIdx, uint labelValue);

public:

    std::vector< Overlay * >  mOverlayInfo;

    // called by the plugin to update the display
    void pluginUpdateDisplay();

    QMenu *getPluginMenuPtr();

    // more for plugins
    Matrix3D<PixelType> &   getVolumeVoxelData() {  return mVolumeData; }
    Matrix3D<LabelType> &   getLabelVoxelData()  {  return mVolumeLabels; }
    Matrix3D<ScoreType> &   getScoreVoxelData()  {  return mScoreImage; }

    Matrix3D<OverlayType> & getOverlayVoxelData( unsigned int num );
    Matrix3D<OverlayType> *getSelectedOverlayData();
    void                    setOverlayVisible( unsigned int num, bool visible );

    // returns one string per class name
    void                    getLabelClassList( QStringList &sList );


private:
    QTimer  *mConstraintsDisplayTimer;  // to keep track of a timeout to show some overlays

    // called before redrawing, should ckec if the timer hasn't expired
    //  and do necessary drawing. returns true if the image was modified,
    //  so that nothing else is drawn
    bool    constraintsUpdateImagesliceCallback(QImage &slice);

public slots:
    void    constraintsTimerCallback(); // timer callback
    void    constraintsChangedCallback();   // called everytime constraints are changed
    void    constraintsChangedCallback(int) { constraintsChangedCallback(); }

public slots:

    void statusBarMsg( const QString &str, int timeout = 1200 );

    void updateImageSlice();    //updates the label widget with mCurZSlice slice
    void updateImageSlice(int);    //same as above, discards parameter int

    void showPreferencesDialog();

    // called whenever the user has modified a single label supervoxel
    void userModifiedSupervoxelLabel();
    void butRunConnectivityCheckNowClicked();

    void labelImageWheelEvent(QWheelEvent * e);
    void labelImageMouseMoveEvent(QMouseEvent * e);
    void labelImageMouseReleaseEvent(QMouseEvent * e);

    void zSliderMoved( int );

    void chkLabelOverlayStateChanged(int state);
    void actionLabelOverlayTriggered();

    void annotVis3DClicked();

    void dialOverlayTransparencyMoved( int );

    // fills the labels combo box with the requested number of labels + the 'unlabeled' one
    void fillLabelComboBox( int numLabels );

    void genSupervoxelClicked();
    void genSuperVoxelWholeVolumeClicked();
    void loadSuperVoxelWholeVolumeClicked();
    void saveSuperVoxelWholeVolumeClicked();

    void actionSaveAnnotTriggered();
    void actionLoadAnnotTriggered();

    void actionImportAnnotTriggered();

    void actionLoadScoreImageTriggered();
    void actionEnableScoreImageTriggered();

    // called when the region changes in the region info window
    void regionListFrameIndexChanged(int);
    void regionListFrameLabelRegion(uint,uint);
    void clearSVSelection();

    // called when an image wants to be loaded in an overlay layer
    void overlayLoadTriggered();
    void overlayReloadTriggered();
    void selectPaintingLabel();
    void selectOverlay();
    void selectedOverlayChanged(int idx);
    void overlayChooseColorTriggered();

    void overlaySaveAsTriggered();
    void overlaySaveTriggered();
    void overlaySave(Matrix3D<OverlayType> *overlay, QString filePath);

    void overlayRescaleTriggered();

    void on_cubeBrushSizeX_valueChanged(int width);
    void on_cubeBrushSizeY_valueChanged(int height);
    void on_cubeBrushSizeZ_valueChanged(int depth);

};

#endif // ANNOTATORWND_H
