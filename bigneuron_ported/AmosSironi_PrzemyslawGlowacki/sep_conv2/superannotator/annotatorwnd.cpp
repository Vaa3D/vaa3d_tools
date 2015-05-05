#include "annotatorwnd.h"
#include "ui_annotatorwnd.h"

#include <QPixmap>
#include <QImage>
#include <QWheelEvent>
#include <QTimer>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include "textinfodialog.h"

#include "MiscUtils.h"

#include "FijiHelper.h"

#include "SuperVoxeler.h"
#include "regionlistframe.h"

#include "RegionGrowing.h"

#include "PluginBase.h"
#include <QColorDialog>
#include <QInputDialog>

#include <QThread>
#include "extras/waitform.h"

#include "preferencesdialog.h"
/** ---- these variables here are a bit dirty, but it is to avoid putting them in the .h file
 ** even though it prevents multiple instances
 */
static SuperVoxeler<unsigned char> mSVoxel;

/** Supervoxel selection **/
struct SupervoxelSelection
{
    bool  valid;    // if it contains valid selection information

    unsigned int             svIdx;     // supervoxel ID
    SlicMapType::value_type  pixelList; // pixels that are inside the selected supervoxel

} static mSelectedSV ;

static Region3D mSVRegion;

// this holds a pointer to the regionListFrame window (if there is one)
// and other info
struct
{
    RegionListFrame                 *pFrame;
    Matrix3D<unsigned int>          lblMatrix;
    unsigned int                    lblCount;
    std::vector<ShapeStatistics<> >    shapeInfo;
    Region3D                        region3D;  // region used at computation time, to convert back to whole image coordinate sytem
    SlicMapType                     labelToPixelMap;    // to speed up processing

} static mLabelListData;


static std::vector<PluginBase *> mPluginBaseList;
static PluginServicesList       mPluginServList;
static std::vector<QLibrary *>  mPluginLibList; // to free them before exiting

// list of overlay volumes, we use ptrs because it has no copy xtor
std::vector< Matrix3D<OverlayType> * >  mOverlayVolumeList;
std::vector<QAction *>                mOverlayMenuActions;
std::vector<QMenu *>                  mOverlayMenus; // choose color menu action

/** -------- Class begin ------------ **/

AnnotatorWnd::AnnotatorWnd(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AnnotatorWnd)
{
    ui->setupUi(this);

    mLabelListData.pFrame = 0;
    mSaveLabelsOnExit = false;

    mConstraintsDisplayTimer = new QTimer(this);
    connect( ui->groupBoxRestrictPixLabels, SIGNAL(toggled(bool)), this, SLOT(constraintsChangedCallback()) );
    connect( ui->spinPixMax, SIGNAL(valueChanged(int)), this, SLOT(constraintsChangedCallback(int)) );
    connect( ui->spinPixMin, SIGNAL(valueChanged(int)), this, SLOT(constraintsChangedCallback(int)) );
    connect( mConstraintsDisplayTimer, SIGNAL(timeout()), this, SLOT(constraintsTimerCallback()) );

    connect( ui->cubeBrushSizeX, SIGNAL(valueChanged(int)), this, SLOT(on_cubeBrushSizeX_valueChanged(int)) );
    connect( ui->cubeBrushSizeY, SIGNAL(valueChanged(int)), this, SLOT(on_cubeBrushSizeY_valueChanged(int)) );
    connect( ui->cubeBrushSizeZ, SIGNAL(valueChanged(int)), this, SLOT(on_cubeBrushSizeZ_valueChanged(int)) );

    // settings
    m_sSettingsFile = QApplication::applicationDirPath() + "/settings.ini";
    qDebug() << m_sSettingsFile;
    loadSettings();

    mFileTypeFilter = "TIF (*.tif *.tiff)";


    mScoreImageEnabled = false;

    mSelectedSV.valid = false;  // no valid selection so far

    ui->centralWidget->setLayout( ui->horizontalLayout );


    qDebug("Args: %d", (int)qApp->arguments().size());
    for (unsigned i=0; i < qApp->arguments().size(); i++)
        qDebug("Arg %d: %s", i, qApp->arguments()[i].toStdString().c_str());

    // ask user to open raw file
    std::string stdFName;
    if (qApp->arguments().size() >= 2 && QFileInfo( qApp->arguments().at(1) ).exists())
        stdFName = qApp->arguments().at(1).toStdString();

    if (stdFName.empty())
    {
        QString fileName = QFileDialog::getOpenFileName( 0, "Load image", mSettingsData.loadPathVolume, mFileTypeFilter );

        if (fileName.isEmpty()) {
            QTimer::singleShot(1, qApp, SLOT(quit()));
            return;
        }

        stdFName = fileName.toLocal8Bit().constData();
        mSettingsData.loadPathVolume = QFileInfo( QString::fromStdString( stdFName) ).absolutePath();
    }

    try {
        mVolumeData.load( stdFName );
    } catch (std::exception &e)
    {
        QMessageBox::critical( this, "Cannot open image file", "Could not open the specified image, quitting.." );
        QTimer::singleShot(1, qApp, SLOT(quit()));
        return;
    }
    this->saveSettings();

    // allocate label volume (per pixel)
    mVolumeLabels.reallocSizeLike( mVolumeData );
    mVolumeLabels.fill(0);

    /** Parse remaining possible args **/
    if (qApp->arguments().size() >= 3)
    {
        if ( loadAnnotation( qApp->arguments().at(2) ) )
        {
            if ( qApp->arguments().size() >= 5 && (qApp->arguments().at(4) == "yes") )
            {
                mSaveLabelsOnExit = true;
                mSaveLabelsOnExitPath = qApp->arguments().at(2);
            }
        }
    }

    mCurZSlice = 0;
    if (qApp->arguments().size() >= 4) {
        bool ok = false;
        unsigned int z = qApp->arguments().at(3).toUInt(&ok);

        if (ok)
            mCurZSlice = z;
    }

    //Brushes
    cubeBrush.setSize(ui->cubeBrushSizeX->value(),ui->cubeBrushSizeY->value(),ui->cubeBrushSizeZ->value());
    sphereBrush.setSize(ui->cubeBrushSizeX->value(),ui->cubeBrushSizeY->value(),ui->cubeBrushSizeZ->value());

    //mVolumeData.load( "/data/phd/synapses/Rat/layer2_3stak_red_reg_z1.5_example_cropped.tif" );
    //mVolumeData.load( "/data/phd/synapses/Rat/layer2_3stak_red_reg_z1.5_firstquarter.tif" );
    //mVolumeData.load( "/data/phd/synapses/Rat/layer2_3stak_red_reg_z1.5.tif" );
    //mVolumeData.save("/tmp/test.tif" );
    //mVolumeData.load("/data/phd/twoexamples/00147.tif");
    qDebug("Volume size: %dx%dx%d\n", mVolumeData.width(), mVolumeData.height(), mVolumeData.depth());

    ui->zSlider->setMinimum(0);
    ui->zSlider->setMaximum( mVolumeData.depth()-1 );
    ui->zSlider->setSingleStep(1);
    ui->zSlider->setPageStep(10);
    ui->zSlider->setValue( mCurZSlice );

    updateImageSlice();

    // events
    connect(ui->labelImg,SIGNAL(wheelEventSignal(QWheelEvent*)),this,SLOT(labelImageWheelEvent(QWheelEvent*)));

    connect(ui->labelImg,SIGNAL(mouseMoveEventSignal(QMouseEvent*)),this,SLOT(labelImageMouseMoveEvent(QMouseEvent*)));
    connect(ui->labelImg,SIGNAL(mouseReleaseEventSignal(QMouseEvent*)),this,SLOT(labelImageMouseReleaseEvent(QMouseEvent*)));

    ui->labelImg->setMouseTracking(true);

    connect(ui->zSlider,SIGNAL(sliderMoved(int)),this,SLOT(zSliderMoved(int)));
    connect(ui->zSlider,SIGNAL(valueChanged(int)),this,SLOT(zSliderMoved(int)));

    connect( ui->actionZoom_fit, SIGNAL(triggered()), ui->labelImg, SLOT(zoomFit()) );
    connect( ui->actionOverlay_labels, SIGNAL(triggered()), this, SLOT(actionLabelOverlayTriggered()) );

    connect( ui->actionSave_annotation, SIGNAL(triggered()), this, SLOT(actionSaveAnnotTriggered()) );
    connect( ui->actionLoad_annotation, SIGNAL(triggered()), this, SLOT(actionLoadAnnotTriggered()) );

    connect( ui->actionImport_annotation, SIGNAL(triggered()), this, SLOT(actionImportAnnotTriggered()) );

    connect( ui->actionScoreImageLoad, SIGNAL(triggered()), this, SLOT(actionLoadScoreImageTriggered()) );
    connect( ui->actionScoreImageEnabled, SIGNAL(triggered()), this, SLOT(actionEnableScoreImageTriggered()) );

    connect(ui->chkLabelOverlay,SIGNAL(stateChanged(int)),this,SLOT(chkLabelOverlayStateChanged(int)));

    connect(ui->dialLabelOverlayTransp,SIGNAL(valueChanged(int)),this,SLOT(dialOverlayTransparencyMoved(int)));

    //connect(ui->butGenSV, SIGNAL(clicked()), this, SLOT(genSupervoxelClicked()));

    // ---- Create supervoxel menu
    QMenu *testMenu = new QMenu(this);

    connect( testMenu->addAction("Local (current view)"), SIGNAL(triggered()), this, SLOT(genSupervoxelClicked()) );
    connect( testMenu->addAction("Global"), SIGNAL(triggered()), this, SLOT(genSuperVoxelWholeVolumeClicked()) );
    connect( testMenu->addAction("Save global to file..."), SIGNAL(triggered()), this, SLOT(saveSuperVoxelWholeVolumeClicked()) );
    connect( testMenu->addAction("Load global from file..."), SIGNAL(triggered()), this, SLOT(loadSuperVoxelWholeVolumeClicked()) );


    ui->butGenSV->setMenu(testMenu);

    connect( ui->butConnectivityRun, SIGNAL(clicked()), this, SLOT(butRunConnectivityCheckNowClicked()) );

    connect(ui->butAnnotVis3D, SIGNAL(clicked()), this, SLOT(annotVis3DClicked()));

    connect( ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferencesDialog()) );

    connect( ui->spinScoreThrAbove, SIGNAL(valueChanged(int)), this, SLOT(updateImageSlice(int)) );
    connect( ui->spinScoreThrBelow, SIGNAL(valueChanged(int)), this, SLOT(updateImageSlice(int)) );

    connect( ui->actionHide_volume, SIGNAL(changed()), this, SLOT(updateImageSlice()) );

    ui->chkLabelOverlay->setChecked(true);

    //TODO the row is changed before triggering the slot so this does not work
    connect( ui->layersDisplay, SIGNAL(currentRowChanged(int)), this, SLOT(selectedOverlayChanged(int)));

    mOverlayLabelImage = ui->chkLabelOverlay->checkState() == Qt::Checked;

    //Label combobox
    fillLabelComboBox(3);

    dialOverlayTransparencyMoved( 0 );
    updateImageSlice();

    // add as many overlay volumes (initially empty) as needed
    {
        ui->menuView->addSeparator();

        //Save active overlay
        QAction *save = ui->menuView->addAction("Save active overlay");
        //TODO check if that's the proper way to assign the key
        save->setShortcut( QKeySequence( QString("S") ) );
        connect( save, SIGNAL(triggered()), this, SLOT(overlaySaveTriggered()) );

        //Reload active overlay
        QAction *reload = ui->menuView->addAction("Reload active overlay");
        reload->setShortcut( QKeySequence( QString("R") ) );
        connect( reload, SIGNAL(triggered()), this, SLOT(overlayReloadTriggered()) );

        // create objects + add overlay visibility menus/shortcuts
        for (int i=0; i < (int)PluginServices::getMaxOverlayVolumes(); i++ )
        {
            mOverlayVolumeList.push_back( new Matrix3D<OverlayType>() );

            mOverlayInfo.push_back( new Overlay() );

            QString name = QString("Overlay %1").arg(i+1);
            ui->layersDisplay->addItem(name);

            // add group for each one
            QMenu *subMenu = ui->menuView->addMenu( mOverlayColorList.getIcon(i), name );
            mOverlayMenus.push_back(subMenu);

            // enable check box
            QAction *selectOverlay = subMenu->addAction( "Select " + name );
            selectOverlay->setShortcut( QKeySequence( QString("Ctrl+%1").arg(i+1) ) );
            selectOverlay->setData( i ); // use data as index
            connect( selectOverlay, SIGNAL(triggered()), this, SLOT(selectOverlay()) );

            QAction *a = subMenu->addAction( "Enable " + name );
            a->setCheckable(true);
            a->setChecked(false);
            a->setEnabled(false);
            //a->setShortcut( QKeySequence( QString("%1").arg(i+1) ) );
            connect( a, SIGNAL(triggered()), this, SLOT(updateImageSlice()) );

            mOverlayMenuActions.push_back( a );

            // add color chooser
            QAction *aColor = subMenu->addAction( "Choose color..." );
            aColor->setEnabled(true);

            connect( aColor, SIGNAL(triggered()), this, SLOT(overlayChooseColorTriggered()) );

            // add button to rescale overlay
            QAction *rescale = subMenu->addAction("Rescale...");
            rescale->setCheckable(false);
            rescale->setEnabled(true);
            rescale->setData( i ); // use data as index

            connect( rescale, SIGNAL(triggered()), this, SLOT(overlayRescaleTriggered()) );

            // and a load button too
            QAction *load = subMenu->addAction("Load from file...");
            load->setCheckable(false);
            load->setEnabled(true);
            load->setData( i ); // use data as index

            connect( load, SIGNAL(triggered()), this, SLOT(overlayLoadTriggered()) );

            // and a save button too
            QAction *saveAs = subMenu->addAction("Save to file...");
            saveAs->setCheckable(false);
            saveAs->setEnabled(true);
            saveAs->setData( i ); // use data as index

            connect( saveAs, SIGNAL(triggered()), this, SLOT(overlaySaveAsTriggered()) );
        }

    }

    //TODO this default transparency is hardcoded
    mOverlayInfo[0]->alpha = 0.80;
    mOverlayInfo[1]->alpha = 0.40;

    //TODO assuming there will always be at least one overlay
    ui->layersDisplay->setCurrentRow(0);

    scanPlugins( qApp->applicationDirPath() + "/plugins/" );

    //TODO allow them to be unhidden
    ui->groupBoxRestrictPixLabels->hide();
    ui->groupBoxSupervoxels->hide();

    this->showMaximized();
}

void AnnotatorWnd::showPreferencesDialog()
{
    PreferencesDialog dialog(this);

    dialog.setFijiExePath( mSettingsData.fijiExePath );
    dialog.setMaxVoxelsForSV( mSettingsData.maxVoxForSVox );

    if (dialog.exec() == QDialog::Rejected)
        return;

    mSettingsData.fijiExePath = dialog.getFijiExePath();
    mSettingsData.maxVoxForSVox = dialog.getMaxVoxelsForSV();
    mSettingsData.sliceJump = dialog.getSliceJump();

    this->saveSettings();
}

void AnnotatorWnd::overlayChooseColorTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int idx = action->data().toInt();

    QColor c = QColorDialog::getColor( mOverlayColorList.getColor(idx), this );

    if (!c.isValid())   return;

    mOverlayColorList.replaceColor( idx, c );

    mOverlayMenus.at(idx)->setIcon( mOverlayColorList.getIcon(idx) );
    updateImageSlice();
}

void AnnotatorWnd::selectPaintingLabel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int idx = action->data().toInt();

    ui->comboLabel->setCurrentIndex(idx);
}

void AnnotatorWnd::selectOverlay()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int idx = action->data().toInt();

    int debugRow = ui->layersDisplay->currentRow();
    if(ui->layersDisplay->currentRow() != idx){
        ui->layersDisplay->setCurrentRow(idx);
        setOverlayVisible(idx, true);
    }else
        setOverlayVisible(idx,!mOverlayMenuActions[idx]->isEnabled());

    ui->dialLabelOverlayTransp->setValue(mOverlayInfo[idx]->alpha*100);


}

void AnnotatorWnd::selectedOverlayChanged(int idx)
{
    int debugRow = ui->layersDisplay->currentRow();
    ui->layersDisplay->setCurrentRow(idx);
    setOverlayVisible(idx, true);
    ui->dialLabelOverlayTransp->setValue(mOverlayInfo[idx]->alpha*100);

}

void AnnotatorWnd::overlayLoadTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int idx = action->data().toInt();

    QString fileName = QFileDialog::getOpenFileName( this, "Load overlay image", mSettingsData.loadPathScores, mFileTypeFilter );

    if (fileName.isEmpty())
        return;

    qDebug() << fileName;

    std::string stdFName = fileName.toLocal8Bit().constData();

    if (!mOverlayVolumeList[idx]->load( stdFName ))
        QMessageBox::critical(this, "Cannot open file", QString("%1 could not be read.").arg(fileName));


    if ( !mOverlayVolumeList[idx]->isSizeLike( mVolumeData ) )
    {
        QMessageBox::critical(this, "Dimensions do not match", "Image does not match original volume dimensions. Disabling this overlay.");

        mOverlayMenuActions[idx]->setChecked(false);
        mOverlayMenuActions[idx]->setEnabled(false);

        updateImageSlice();
        return;
    }

    // enable and show ;)
    mOverlayMenuActions[idx]->setChecked(true);
    mOverlayMenuActions[idx]->setEnabled(true);

    updateImageSlice();
    statusBarMsg("Overlay image loaded successfully.");

    mSettingsData.loadPathScores = QFileInfo(fileName).absolutePath();
    mSettingsData.saveFileInfoScores = QFileInfo(fileName);
    this->saveSettings();
}

void AnnotatorWnd::overlayReloadTriggered()
{

    int idx = ui->layersDisplay->currentIndex().row();
    if(!mSettingsData.saveFileInfoScores.exists()){
        QMessageBox::critical(this, "Cannot open file", QString("FileInfo is not set"));
        return;
    }

    //TODO move this to another function called by both triggers
    //TODO handle errors correctly of empty saveFileInfos here
    QString fileName(mSettingsData.saveFileInfoScores.absolutePath() + "/"
                   + mSettingsData.saveFileInfoScores.baseName() + "."
                   + mSettingsData.saveFileInfoScores.completeSuffix());

    std::string stdFName = fileName.toLocal8Bit().constData();

    if (!mOverlayVolumeList[idx]->load( stdFName )){
        QMessageBox::critical(this, "Cannot open file", QString("%1 could not be read.").arg(fileName));
        return;
    }

    if ( !mOverlayVolumeList[idx]->isSizeLike( mVolumeData ) )
    {
        QMessageBox::critical(this, "Dimensions do not match", "Image does not match original volume dimensions. Disabling this overlay.");

        mOverlayMenuActions[idx]->setChecked(false);
        mOverlayMenuActions[idx]->setEnabled(false);

        updateImageSlice();
        return;
    }

    // enable and show ;)
    mOverlayMenuActions[idx]->setChecked(true);
    mOverlayMenuActions[idx]->setEnabled(true);

    updateImageSlice();
    statusBarMsg("Overlay image loaded successfully.");

    mSettingsData.loadPathScores = QFileInfo(fileName).absolutePath();
    mSettingsData.saveFileInfoScores = QFileInfo(fileName);
    this->saveSettings();

}

void AnnotatorWnd::overlaySaveAsTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int idx = action->data().toInt();

    QString fileName = QFileDialog::getSaveFileName( this, "Save overlay image", mSettingsData.savePathScores, mFileTypeFilter );

    if (fileName.isEmpty()){
        //TODO say something!
        qDebug() << "FileName is empty";
        return;
    }

    if (!fileName.endsWith(".tif"))
        fileName += ".tif";

    qDebug() << fileName;

    overlaySave(mOverlayVolumeList[idx],fileName);

    mSettingsData.savePathScores = QFileInfo(fileName).absolutePath();
    mSettingsData.saveFileInfoScores = QFileInfo(fileName);
    this->saveSettings();
}

void AnnotatorWnd::overlaySaveTriggered()
{

    //TODO is saving the selected layer ok?
    int idx = ui->layersDisplay->currentIndex().row();
    if(idx < 0 || idx > ui->layersDisplay->count()){
        qDebug() << " unselected layer";
        QMessageBox::critical(this, "Cannot open file", QString("Select a layer first"));
        return;
    }

    Matrix3D<OverlayType> *overlay = mOverlayVolumeList.at(idx);
    if(overlay == NULL || overlay->isEmpty()){
        qDebug() << " overlay empty";
        QMessageBox::critical(this, "Cannot open file", QString("Overlay is empty"));
        return;
    }

    if(!mSettingsData.saveFileInfoScores.exists()){
        qDebug() << "FileInfo does not exist";
        //TODO put this somewhere else
        QString fileName = QFileDialog::getSaveFileName( this, "Save overlay image", mSettingsData.savePathScores, mFileTypeFilter );

        if (fileName.isEmpty()){
            qDebug() << "FileName is empty";
            return;
        }

        if (!fileName.endsWith(".tif"))
            fileName += ".tif";

        qDebug() << fileName;

        overlaySave(mOverlayVolumeList[idx],fileName);

        mSettingsData.savePathScores = QFileInfo(fileName).absolutePath();
        mSettingsData.saveFileInfoScores = QFileInfo(fileName);
        this->saveSettings();
        return;
    }

    overlaySave(overlay, mSettingsData.saveFileInfoScores.absolutePath() + "/"
              + mSettingsData.saveFileInfoScores.baseName() + "."
              + mSettingsData.saveFileInfoScores.completeSuffix());
}

void AnnotatorWnd::overlaySave(Matrix3D<OverlayType> *overlay, QString filePath)
{

    std::string stdFName = filePath.toLocal8Bit().constData();

    if (!overlay->save( stdFName )) {
        statusBarMsg(QString("Error saving ") + filePath, 0 );
        return;
    }
    else
        statusBarMsg("Annotation saved successfully.");

}

void AnnotatorWnd::overlayRescaleTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int idx = action->data().toInt();

    // ask for gaussian variance
    bool ok = false;
    float scale = QInputDialog::getDouble(0, "Scale", "Specify value used to rescale overlay", 0.5, 0.1, 1.0f, 1, &ok);
    if (!ok) return;

    // generate list of seeds
    Matrix3D<OverlayType>* dataOverlay = mOverlayVolumeList[idx];

    for(int x = 0; x < dataOverlay->width(); ++x) {
        for(int y = 0; y < dataOverlay->height(); ++y) {
            for(int z = 0; z < dataOverlay->depth(); ++z) {
                (*dataOverlay)(x,y,z) *= scale;
            }
        }
    }

    updateImageSlice();
}

Matrix3D<OverlayType> &  AnnotatorWnd::getOverlayVoxelData( unsigned int num )
{
    return *mOverlayVolumeList.at(num);  // note the assert on num!
}

Matrix3D<OverlayType> * AnnotatorWnd::getSelectedOverlayData( )
{
    int overlayindex = ui->layersDisplay->currentIndex().row();
    if(overlayindex >= 0 && overlayindex < mOverlayVolumeList.size() )
    {
        Matrix3D<OverlayType> *annotationData = mOverlayVolumeList.at(overlayindex);
        if (annotationData->isEmpty())
        {
            annotationData->reallocSizeLike( getVolumeVoxelData() );
            annotationData->fill(0);

            mOverlayMenuActions[overlayindex]->setChecked(true);
            mOverlayMenuActions[overlayindex]->setEnabled(true);
        }
        return annotationData;
    } else
    {
        throw std::runtime_error("Invalid overlay index. Is any selected?");
    }
}


void AnnotatorWnd::setOverlayVisible( unsigned int num, bool visible )
{
    // we will only set it visible if it contains valid info
    if (!mOverlayVolumeList.at(num)->isSizeLike( mVolumeData )) {
        qDebug() << "Overlay visible command ignored because overlay is not valid yet.";
        mOverlayMenuActions.at(num)->setEnabled(false);
        mOverlayMenuActions.at(num)->setChecked(false);
        return;
    }

    mOverlayMenuActions.at(num)->setEnabled(visible);
    mOverlayMenuActions.at(num)->setChecked(visible);

    updateImageSlice();
}

void AnnotatorWnd::scanPlugins( const QString &pluginFolder )
{
    // list files
    QDir dir(pluginFolder);
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name);

    qDebug() << "Plugin folder: " << pluginFolder;

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);

        QString absFilePath = fileInfo.absoluteFilePath();
        if ( !QLibrary::isLibrary( absFilePath ) )
            continue;

        QLibrary *library = new QLibrary( absFilePath, this );
        if (!library->load()) {
            qDebug() << "Could not load plugin " << absFilePath << ":" << library->errorString();
            continue;
        }

        typedef PluginBase*(*PluginCreateFunction)(void);

        PluginCreateFunction createPlugin = (PluginCreateFunction) library->resolve("createPlugin");

        if (createPlugin == 0) {
            qDebug() << "Could not resolve entry point for plugin " << absFilePath;
            continue;
        }

        PluginBase *newPlugin = createPlugin();
        mPluginBaseList.push_back( newPlugin );

        mPluginServList.append( PluginServices( newPlugin->pluginName(), this ) );
        bool ret = newPlugin->initializePlugin( mPluginServList.last() );

        mPluginLibList.push_back( library );

        if (!ret)
        {
            qDebug() << "Initialization failed for " << absFilePath;
            continue;
        }

    }


    if ( mPluginServList.isEmpty() )
        ui->menuPlugins->addAction( "No plugins loaded" )->setEnabled(false);
}

void AnnotatorWnd::loadSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    mSettingsData.savePath = settings.value("savePath", ".").toString();
    mSettingsData.loadPath = settings.value("loadPath", ".").toString();
    mSettingsData.loadPathScores = settings.value("loadPathScores", ".").toString();
    mSettingsData.loadPathVolume = settings.value("loadPathVolume", ".").toString();
    mSettingsData.fijiExePath = settings.value("fijiExePath", "Not set").toString();
    mSettingsData.maxVoxForSVox = settings.value("maxVoxForSVox", 28000000).toUInt();

    ui->spinSVCubeness->setValue( settings.value("spinSVCubeness", 40).toInt() );
    ui->spinSVSeed->setValue( settings.value("spinSVSeed", 20).toInt() );
    ui->spinSVZ->setValue( settings.value("spinSVZ", 100).toInt() );
}

void AnnotatorWnd::saveSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    settings.setValue( "savePath", mSettingsData.savePath );
    settings.setValue( "loadPath", mSettingsData.loadPath );
    settings.setValue( "loadPathScores", mSettingsData.loadPathScores );
    settings.setValue( "loadPathVolume", mSettingsData.loadPathVolume );

    settings.setValue("spinSVCubeness", ui->spinSVCubeness->value());
    settings.setValue("spinSVSeed", ui->spinSVSeed->value());
    settings.setValue( "spinSVZ", ui->spinSVZ->value() );
    settings.setValue( "fijiExePath", mSettingsData.fijiExePath );
    settings.setValue( "maxVoxForSVox", mSettingsData.maxVoxForSVox );
    settings.setValue( "sliceJump", mSettingsData.sliceJump );


    qDebug() << m_sSettingsFile;
}

void AnnotatorWnd::actionLoadScoreImageTriggered()
{
    QString fileName = QFileDialog::getOpenFileName( this, "Load score image", mSettingsData.loadPathScores, mFileTypeFilter );

    if (fileName.isEmpty())
        return;

    qDebug() << fileName;

    std::string stdFName = fileName.toLocal8Bit().constData();

    if (!mScoreImage.load( stdFName ))
        QMessageBox::critical(this, "Cannot open file", QString("%1 could not be read.").arg(fileName));


    if ( !mScoreImage.isSizeLike( mVolumeData ) )
    {
        QMessageBox::critical(this, "Dimensions do not match", "Score image does not match original volume dimensions. Disabling score visualization.");

        mScoreImageEnabled = false;
        ui->actionScoreImageEnabled->setChecked(mScoreImageEnabled);
        ui->chkConnectivityScoreImg->setEnabled(mScoreImageEnabled);
        ui->chkScoreEnable->setEnabled(mScoreImageEnabled);

        updateImageSlice();
        return;
    }

    // enable and show ;)
    mScoreImageEnabled = true;
    ui->actionScoreImageEnabled->setChecked(mScoreImageEnabled);
    ui->chkConnectivityScoreImg->setEnabled(mScoreImageEnabled);
    ui->chkScoreEnable->setEnabled(mScoreImageEnabled);

    updateImageSlice();
    statusBarMsg("Score image loaded successfully.");

    mSettingsData.loadPathScores = QFileInfo(fileName).absolutePath();
    this->saveSettings();
}

void AnnotatorWnd::actionEnableScoreImageTriggered()
{
    if ( !mScoreImage.isSizeLike( mVolumeData ) ) {
        ui->actionScoreImageEnabled->setChecked(false);
        return;
    }

    mScoreImageEnabled = ui->actionScoreImageEnabled->isChecked();
    updateImageSlice();
}

bool AnnotatorWnd::saveAnnotation(const QString& fileName_)
{
    QString fileName(fileName_);
    if (!fileName.endsWith(".tif"))
        fileName += ".tif";

    qDebug() << fileName;

    std::string stdFName = fileName.toLocal8Bit().constData();

    if (!mVolumeLabels.save( stdFName )) {
        statusBarMsg(QString("Error saving ") + fileName, 0 );
        return false;
    }
    else
        statusBarMsg("Annotation saved successfully.");

    return true;
}

void AnnotatorWnd::actionSaveAnnotTriggered()
{
    QString fileName;

    if(mSettingsData.saveFilePath.isEmpty())
        fileName = QFileDialog::getSaveFileName( this, "Save annotation", mSettingsData.savePath, mFileTypeFilter );
    else
        fileName = mSettingsData.saveFilePath;

    if (fileName.isEmpty())
        return;

    if (!saveAnnotation(fileName))
        return;

    mSettingsData.savePath = QFileInfo(fileName).absolutePath();
    mSettingsData.saveFilePath = QFileInfo(fileName).absoluteFilePath();
    this->saveSettings();
}

bool AnnotatorWnd:: loadAnnotation(const QString& fileName, int importAsLabel, LabelType threshold)
{
    qDebug() << fileName;

    std::string stdFName = fileName.toLocal8Bit().constData();

    if (!mVolumeLabels.load( stdFName )) {
        QMessageBox::critical(this, "Cannot open file", QString("%1 could not be read.").arg(fileName));
        return false;
    }

    if ( (mVolumeLabels.width() != mVolumeData.width()) || (mVolumeLabels.height() != mVolumeData.height()) || (mVolumeLabels.depth() != mVolumeData.depth()) )
    {
        QMessageBox::critical(this, "Dimensions do not match", "Annotation volume does not match original volume dimensions. Re-setting labels.");
        mVolumeLabels.reallocSizeLike( mVolumeData );
        mVolumeLabels.fill(0);
        updateImageSlice();
        return false;
    }

    // check if we have to import it
    if ( importAsLabel >= 0 )
    {
        const unsigned numEl = mVolumeLabels.numElem();
        const LabelType label = (unsigned char) importAsLabel;

        for (unsigned i=0; i < numEl; i++)
        {
            if ( mVolumeLabels.data()[i] >= threshold )
                mVolumeLabels.data()[i] = label;
            else
                mVolumeLabels.data()[i] = 0;
        }
    }

    updateImageSlice();
    statusBarMsg("Annotation loaded successfully.");

    return true;
}

void AnnotatorWnd::actionImportAnnotTriggered()
{
    QString fileName = QFileDialog::getOpenFileName( this, "Import annotation", mSettingsData.loadPath, mFileTypeFilter );

    if (fileName.isEmpty())
        return;

    bool ok = false;

    // ask for threshold
    int threshold = QInputDialog::getInt( 0, "Threshold value", "Specify the thresholding value:",
                              128, 0, 255, 1, &ok );
    if (!ok) return;

    // prepare string list, without 'not-labeled' item
    QStringList items;
    for (unsigned i=1; i < ui->comboLabel->count(); i++)
        items.append( ui->comboLabel->itemText(i) );

    QString selectedItem = QInputDialog::getItem( 0, "Select label", "Label to assign to values higher than threshold:",
                           items, 0, false, &ok);
    if (!ok)    return;

    int importAsLabel = items.indexOf( selectedItem ) + 1;

    if (!loadAnnotation(fileName, importAsLabel, threshold))
        return;

    mSettingsData.loadPath = QFileInfo(fileName).absolutePath();
    this->saveSettings();
}

void AnnotatorWnd::actionLoadAnnotTriggered()
{
    QString fileName = QFileDialog::getOpenFileName( this, "Load annotation", mSettingsData.loadPath, mFileTypeFilter );

    if (fileName.isEmpty())
        return;

    if (!loadAnnotation(fileName))
        return;

    mSettingsData.loadPath = QFileInfo(fileName).absolutePath();

    this->saveSettings();
}

Region3D AnnotatorWnd::getViewportRegion3D()
{
    // prepare Z range
    int zMin = mCurZSlice - ui->spinSVZ->value();
    int zMax = mCurZSlice + ui->spinSVZ->value();

    if (zMin < 0)   zMin = 0;
    if (zMax >= mVolumeData.depth())    zMax = mVolumeData.depth() - 1;

    // selected x,y region + whole z range
    return Region3D( ui->labelImg->getViewableRect(), zMin, zMax - zMin + 1 );
}

// this is a helper for genSupervoxelClicked()
class SupervoxelThread : public QThread
{

 public:
    typedef SuperVoxeler<PixelType>  SupervoxelerType;
    typedef Matrix3D<PixelType>      VolumeType;

protected:
    SupervoxelerType  &mSVox;
    const VolumeType  &mRawVolume;
    int mSeed;
    unsigned int mCubeness;
    AnnotatorWnd *mParent;

public:

    SupervoxelThread(AnnotatorWnd *parent, SupervoxelerType &svox, const VolumeType &raw,
             int seed, unsigned int cubeness) : QThread(parent), mSVox(svox), mRawVolume(raw),
                                                mSeed(seed), mCubeness(cubeness), mParent(parent)
    {
    }

 public:
     void run()
     {
         mSVox.apply( mRawVolume, mSeed, mCubeness );

         QMetaObject::invokeMethod( mParent, "statusBarMsg", Qt::QueuedConnection, Q_ARG( QString, QString("Done: %1 supervoxels generated.").arg( mSVox.numLabels() ) ) );
     }

};

void AnnotatorWnd::loadSuperVoxelWholeVolumeClicked()
{
    QString fileName = QFileDialog::getOpenFileName( this, "Load supervoxel data", mSettingsData.loadPathScores, "nrrd (*.nrrd)" );

    if (fileName.isEmpty())
        return;

    std::string stdFName = fileName.toLocal8Bit().constData();

    if (!mSVoxel.load( stdFName )) {
        QMessageBox::critical(this, "Cannot open file", QString("%1 could not be read.").arg(fileName));
        return;
    }

    if ( (mSVoxel.pixelToVoxel().width() != mVolumeData.width()) || (mSVoxel.pixelToVoxel().height() != mVolumeData.height()) || (mSVoxel.pixelToVoxel().depth() != mVolumeData.depth()) )
    {
        QMessageBox::critical(this, "Dimensions do not match", "Supervoxel volume does not match original volume dimensions. Re-setting supervoxels.");
        mSVRegion.valid = false;
        updateImageSlice();
        return;
    }

    mSVRegion.valid = true;
    mSVRegion.corner.x = mSVRegion.corner.y = mSVRegion.corner.z = 0;

    mSVRegion.size.x = mVolumeData.width();
    mSVRegion.size.y = mVolumeData.height();
    mSVRegion.size.z = mVolumeData.depth();

    updateImageSlice();
    statusBarMsg("Supervoxel data loaded successfully.");
}

void AnnotatorWnd::saveSuperVoxelWholeVolumeClicked()
{
    bool isOk = true;

    if (!mSVRegion.valid)   isOk = false;
    if ( (mSVRegion.corner.x != 0) || (mSVRegion.corner.y != 0) || (mSVRegion.corner.z != 0) ) isOk = false;
    if ( mSVRegion.size.x != mVolumeData.width() )  isOk = false;
    if ( mSVRegion.size.y != mVolumeData.height() )  isOk = false;
    if ( mSVRegion.size.z != mVolumeData.depth() )  isOk = false;

    if (!isOk)
    {
        QMessageBox::critical(this, "Error", "Global supervoxels must be computed before saving them.");
        return;
    }


    QString fileName = QFileDialog::getSaveFileName( this, "Save supervoxel data", mSettingsData.savePath, "nrrd (*.nrrd)" );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".nrrd"))
        fileName += ".nrrd";

    std::string stdFName = fileName.toLocal8Bit().constData();

    if (!mSVoxel.save( stdFName )) {
        statusBarMsg(QString("Error saving to ") + fileName, 0 );
        return;
    }
    else
        statusBarMsg("Supervoxel data saved successfully.");
}

void AnnotatorWnd::genSuperVoxelWholeVolumeClicked()
{
    // set region to whole cube
    mSVRegion.valid = true;
    mSVRegion.corner.x = mSVRegion.corner.y = mSVRegion.corner.z = 0;

    mSVRegion.size.x = mVolumeData.width();
    mSVRegion.size.y = mVolumeData.height();
    mSVRegion.size.z = mVolumeData.depth();


    mSelectedSV.valid = false;

    SupervoxelThread *thread = new SupervoxelThread( this, mSVoxel, mVolumeData, ui->spinSVSeed->value(), ui->spinSVCubeness->value() );

    connect( thread, SIGNAL(finished()), this, SLOT(updateImageSlice()) );

    runThreadWithProgress<SupervoxelThread>( this, thread );
}

void AnnotatorWnd::genSupervoxelClicked()
{
    //qDebug() << "Pos:  " << ui->labelImg->x() << " " <<  ui->labelImg->y();
    //qDebug() << "Size: " << ui->labelImg->width() << " " <<  ui->labelImg->height();

    // prepare Z range
    //int zMin = mCurZSlice - ui->spinSVZ->value();
    //int zMax = mCurZSlice + ui->spinSVZ->value();

    //if (zMin < 0)   zMin = 0;
    //if (zMax >= mVolumeData.depth())    zMax = mVolumeData.depth() - 1;

    // selected x,y region + whole z range
    mSVRegion = getViewportRegion3D();

    if ( mSVRegion.totalVoxels() > mSettingsData.maxVoxForSVox )
    {
        QMessageBox::critical( this, "Region too large",
                               QString("The current region contains %1 supervoxels, exceeding the limit of %2."
                                       "You can change this limit in the preferences dialog.").arg(mSVRegion.totalVoxels()).arg(mSettingsData.maxVoxForSVox) );
        mSVRegion.valid = false;
        updateImageSlice();
        return;
    }

    // do checking on cubeness and seed to avoid crashing while running supervoxel code
    {
        const unsigned minLength = mSVRegion.minSideLength();

        const unsigned maxSeed = ceil( minLength / 1.5 );

        qDebug("Min seed: %d", (int)maxSeed);

        if ( ui->spinSVSeed->value() > maxSeed )
        {
             QMessageBox::critical( this, "Seed too large", QString("Selected seed size is too large. For this region it has to be at most %1.").arg(maxSeed) );
             mSVRegion.valid = false;
             updateImageSlice();
             return;
        }
    }

    // crop data
    mSVRegion.useToCrop( mVolumeData, &mCroppedVolumeData );

    //mSVoxel.apply( mCroppedVolumeData, ui->spinSVSeed->value(), ui->spinSVCubeness->value() );
    mSelectedSV.valid = false;

    // save supervoxel parameters
    saveSettings();

    SupervoxelThread *thread = new SupervoxelThread( this, mSVoxel, mCroppedVolumeData, ui->spinSVSeed->value(), ui->spinSVCubeness->value() );

    connect( thread, SIGNAL(finished()), this, SLOT(updateImageSlice()) );

    runThreadWithProgress<SupervoxelThread>( this, thread );
}

void AnnotatorWnd::runConnectivityCheck( const Region3D &reg )
{
    const bool showInfo = ui->chkShowRegionInfo->isChecked();

    if ( reg.valid )
    {
        Matrix3D<LabelType> croppedImg;
        if ( !ui->chkConnectivityScoreImg->isChecked() )    // score image or label image?
            reg.useToCrop( mVolumeLabels, &croppedImg );
        else{
            if(mOverlayVolumeList[ui->layersDisplay->currentRow()]->isSizeLike(mVolumeLabels))
                reg.useToCrop( *mOverlayVolumeList[ui->layersDisplay->currentRow()], &croppedImg );
            else{
                qDebug() << "Selected layer does not match volume size. Aborting.";
                return;
            }
        }

        // now only keep the label set in the combo box in the case of label image
        const LabelType lbl = ui->comboLabel->currentIndex();

        unsigned int lblCount;

        // score image or label image?
        PixelType minThr = lbl; PixelType maxThr = lbl;
        if ( ui->chkConnectivityScoreImg->isChecked() ) {
            if ( ui->groupBoxRestrictPixLabels->isChecked() ){
                minThr = ui->spinScoreThreshold->value();
            } else {
                minThr = 1;
            }
            maxThr = 255;
        }
        croppedImg.createLabelMap( minThr, maxThr,
                                   &mLabelListData.lblMatrix, ui->chkConnectivityfull->isChecked(), &lblCount, showInfo?(&mLabelListData.shapeInfo):0,
                                   ui->chkConnectivityDetailedAnalysis->isChecked());

        mLabelListData.lblCount = lblCount;

        qDebug() << "Number of connected regions: " << lblCount;

        // save the region for which this was computed for offset computation later on
        mLabelListData.region3D = reg;

        // create inverse list
        createSlicMapT<true>( mLabelListData.lblMatrix, lblCount, mLabelListData.labelToPixelMap );

        if (showInfo && (lblCount > 0))
        {
            // sort by size
            std::sort( mLabelListData.shapeInfo.begin(), mLabelListData.shapeInfo.end(), ShapeStatistics<>::greaterThan );

            if (mLabelListData.pFrame != 0)
                delete mLabelListData.pFrame;

            mLabelListData.pFrame = new RegionListFrame(0, this);

            connect( mLabelListData.pFrame, SIGNAL(currentRegionChanged(int)),
                     this, SLOT(regionListFrameIndexChanged(int)) );

            connect( mLabelListData.pFrame, SIGNAL(labelRegion(uint,uint)),
                     this, SLOT(regionListFrameLabelRegion(uint,uint)));

            connect( mLabelListData.pFrame, SIGNAL(widgetClosed()),
                     this, SLOT(clearSVSelection()));

            mLabelListData.pFrame->setRegionData( mLabelListData.shapeInfo );

            mLabelListData.pFrame->show();
            mLabelListData.pFrame->moveToBottomLeftCorner();
        }

        // add to current message
        QString curMsg = this->statusBar()->currentMessage();
        if (!curMsg.isEmpty())
            curMsg += " | ";

        statusBarMsg( curMsg + QString("Region count: %1").arg(lblCount), 2000 );

        //TODO autolabel each region
        if( ui->autoLabel->isChecked() ){

            Matrix3D<LabelType> *overlayData = getSelectedOverlayData();
            for(int i=0; i < lblCount; i++){

                //TODO fix the labeling color
                labelRegion( *overlayData, i, i*205/lblCount + 50 );

            }

        }
    }
}

void AnnotatorWnd::labelRegion(Matrix3D<LabelType> &data, uint regionIdx, uint labelValue)
{
    //TODO how are regions stored?
    unsigned int pixLabelIdx = mLabelListData.shapeInfo[regionIdx].labelIdx();
    PixelInfoList pixelList;
    PixelInfo pixelInfo;
    mLabelListData.region3D.croppedToWholePixList( data,
                                                   mLabelListData.labelToPixelMap.at( pixLabelIdx - 1 ),
                                                   pixelList );
    for( int i=0; i<pixelList.size(); i++ )
    {
        pixelInfo = pixelList[i];
        pixelBrush.paint(*getSelectedOverlayData(), pixelInfo.coords.x, pixelInfo.coords.y, pixelInfo.coords.z, labelValue);
    }

    updateImageSlice();
}

void AnnotatorWnd::regionListFrameLabelRegion(uint regionIdx, uint labelId)
{

    unsigned int pixLabelIdx = mLabelListData.shapeInfo[regionIdx].labelIdx();

    mLabelListData.region3D.croppedToWholePixList( mVolumeLabels,
                                                   mLabelListData.labelToPixelMap.at( pixLabelIdx - 1 ),
                                                   mSelectedSV.pixelList );

    mSelectedSV.valid = true;

    annotateSupervoxel(mSelectedSV, labelId);

    mSelectedSV.valid = false;

    updateImageSlice();
}

void AnnotatorWnd::regionListFrameIndexChanged(int newRegionIdx)
{
    //qDebug() << newRegionIdx;
    // find pixels and set as highlighted supervoxel
    unsigned int pixLabelIdx = mLabelListData.shapeInfo[newRegionIdx].labelIdx();
    mLabelListData.region3D.croppedToWholePixList( mVolumeLabels,
                                                   mLabelListData.labelToPixelMap.at( pixLabelIdx - 1 ),
                                                   mSelectedSV.pixelList );

    // compute centroid so that we can focus on the area we want
    UIntPoint3D centerPix;    // will hold centroid
    centerPix.x = centerPix.y = centerPix.z = 0;
    for (unsigned int i=0; i < mSelectedSV.pixelList.size(); i++)
        centerPix.add( mSelectedSV.pixelList[i].coords );

    centerPix.divideBy( mSelectedSV.pixelList.size() );

    ui->zSlider->setValue( centerPix.z );

    mSelectedSV.valid = true;

    updateImageSlice();
}

void AnnotatorWnd::clearSVSelection()
{
    mSelectedSV.valid = false;

    updateImageSlice();
}

void AnnotatorWnd::butRunConnectivityCheckNowClicked()
{
    Region3D reg = getViewportRegion3D();
    runConnectivityCheck( reg );
}

void AnnotatorWnd::userModifiedSupervoxelLabel()
{
    if ( ui->chkConnectivityEnableOnline->isChecked() )
        butRunConnectivityCheckNowClicked();
}

void AnnotatorWnd::getLabelClassList( QStringList &sList )
{
    sList.clear();
    for(int i=0; i < ui->comboLabel->count(); i++)
        sList += ui->comboLabel->itemText(i);
}

void AnnotatorWnd::annotVis3DClicked()
{
    QMenu menu("Source", this);

    QAction *aLabel = menu.addAction("Labels volume");

    QAction *aScore = menu.addAction("Score volume");
    aScore->setEnabled( mScoreImage.isSizeLike( mVolumeData ) );

    // add overlays, if available
    std::vector< QAction * > overlayActions;
    for (unsigned int i=0; i < mOverlayMenuActions.size(); i++) {
        bool enabled = true;

        if ( !mOverlayVolumeList[i]->isSizeLike( mVolumeData ) )
            enabled = false;

        QAction *a = menu.addAction( mOverlayMenuActions[i]->text() );
        a->setEnabled(enabled);

        overlayActions.push_back(a);
    }

    QAction *res = menu.exec( ui->butAnnotVis3D->mapToGlobal( QPoint(0,0) ) );

    // this assumes that labeltype = pixeltype = scoretype!
    Matrix3D<PixelType> *srcPtr;
    bool isLabel = false;
    if ( res == aLabel ) {
        srcPtr = &mVolumeLabels;
        isLabel = true;
    } else if( res == aScore )
        srcPtr = &mScoreImage;
    else
    {
        bool found =  false;
        for (unsigned int i=0; i < overlayActions.size(); i++)
            if ( res == overlayActions[i] ) {
                found = true;
                srcPtr = mOverlayVolumeList[i];
                break;
            }

        if (!found)
            return;
    }

    // now we can process it
    Region3D reg = getViewportRegion3D();

    // warn the user about memory usage
    {
        int mult = 1;
        if (isLabel)    mult = 3;

        const double memUsg = (reg.totalVoxels() * mult) / (1024.0*1024.0);
        QMessageBox::StandardButton res =
                QMessageBox::question( this, "Memory usage",
                               QString("This operation needs %1 MB of RAM/hard drive space. Continue?").arg(memUsg),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );

        if (res != QMessageBox::Yes)
            return;
    }

    FijiConfig  cfg;
    cfg.fijiExe = mSettingsData.fijiExePath;

    if ( !QFile::exists(cfg.fijiExe) ) {
        QMessageBox::critical(this, "Error", "Fiji executable not found. Please go to the preferences menu and specify its path.");
        return;
    }

    FijiShow3D  s3d;
    s3d.setConfig(cfg);

    if (!reg.valid)
        return;

    Matrix3D<LabelType> lblCropped;
    reg.useToCrop( *srcPtr, &lblCropped );

    if (isLabel)    // then put some nice coloring
    {
        Matrix3D<PixelType> vR, vG, vB;
        vR.reallocSizeLike( lblCropped );
        vG.reallocSizeLike( lblCropped );   // alloc for each color channel
        vB.reallocSizeLike( lblCropped );

        vR.fill(0); vG.fill(0); vB.fill(0);

        const unsigned int N = lblCropped.numElem();
        const unsigned int maxLbl = mLblColorList.count();
        for (unsigned int i=0; i < N; i++)
        {
            LabelType lbl = lblCropped.data()[i];
            if (lbl <= 0)   continue;

            if (lbl > maxLbl) continue;

            vR.data()[i] = mLblColorList.getColor(lbl-1).red();
            vG.data()[i] = mLblColorList.getColor(lbl-1).green();
            vB.data()[i] = mLblColorList.getColor(lbl-1).blue();
        }

        // color
        if (!s3d.run( vR, vG, vB ))
            QMessageBox::critical(this, "Error", "There was an error when trying to execute Fiji. Please go to the preferences menu and check its path.");
    }
    else
    {
        if (!s3d.run( *srcPtr ))    // 1-channel
            QMessageBox::critical(this, "Error", "There was an error when trying to execute Fiji. Please go to the preferences menu and check its path.");
    }
}

void AnnotatorWnd::statusBarMsg( const QString &str, int timeout )
{
    statusBar()->showMessage( str, timeout );
}

void AnnotatorWnd::fillLabelComboBox( int numLabels )
{
    if (numLabels > mLblColorList.count()) {
        qWarning("fillLabelComboBox: numLabels exceeds hueList size");
        numLabels = mLblColorList.count();
    }

    ui->comboLabel->clear();

    // first none class
    ui->comboLabel->addItem("Unlabeled");
    QAction *unlabel = ui->menuView->addAction("Use unlabel");
    unlabel->setData( 0 ); // use data as index
    unlabel->setShortcut(QKeySequence( QString("0") ) );
    connect( unlabel, SIGNAL(triggered()), this, SLOT(selectPaintingLabel()) );

    for (int i=0; i < mLblColorList.count(); i++){
        ui->comboLabel->addItem( mLblColorList.getIcon(i), QString("Class %1").arg(i+1) );
        //TODO process keystrokes differently
        QString name = QString("label %1").arg(i+1);
        QAction *label = ui->menuView->addAction("Use " + name);

        label->setShortcut( QKeySequence( QString("%1").arg(i+1) ) );
        label->setData( i+1 ); // use data as index
        connect( label, SIGNAL(triggered()), this, SLOT(selectPaintingLabel()) );
    }
    //TODO maybe with something like QList<QKeySequence> shortcuts;

    ui->comboLabel->setCurrentIndex(1);
}

void AnnotatorWnd::dialOverlayTransparencyMoved( int value )
{

    int debugRow = ui->layersDisplay->currentRow();
    if(ui->layersDisplay->currentRow() >= 0 )
        mOverlayInfo[ui->layersDisplay->currentRow()]->alpha = value * 1.0 / 100;

    updateImageSlice();
}


void AnnotatorWnd::actionLabelOverlayTriggered()
{
    ui->chkLabelOverlay->setChecked( ui->actionOverlay_labels->isChecked() );
}

void AnnotatorWnd::chkLabelOverlayStateChanged(int state)
{
    mOverlayLabelImage = ui->chkLabelOverlay->checkState() == Qt::Checked;
    updateImageSlice();
}

void AnnotatorWnd::updateCursorPixelInfo(int x, int y, int z)
{
    if (!mVolumeData.pixIsInImage(x,y,z))   return; // something invalid

    QString pixPos = QString("Pixel (%1, %2, %3): %4").arg(x).arg(y).arg(z).arg( mVolumeData(x,y,z), 3 );
    ui->labelPixInfoTop->setText( pixPos );


#define CELLA(x) \
    QString("<td align=\"left\">") + x + QString("</td>")

#define CELLB(x) \
    QString("<td align=\"right\">") + x + QString("</td>")

    // SCORE
    QString scorePos = CELLA("Score");
    if (!mScoreImage.isSizeLike(mVolumeData))
        scorePos += CELLB("N/A");
    else {
        ScoreType val = mScoreImage(x,y,z);
        scorePos += CELLB( QString().sprintf( "%.3d %.2f", val, val/255.0 ) );
    }

    // overlays
    std::vector<QString> overlayStr( mOverlayVolumeList.size() );
    for (unsigned int i=0; i < overlayStr.size(); i++)
    {
        overlayStr[i] = CELLA(QString("Overlay %1").arg(i+1));
        if ( !mOverlayVolumeList[i]->isSizeLike(mVolumeData) )
        {
            overlayStr[i] += CELLB("N/A");
            continue;
        }

        OverlayType val = (*mOverlayVolumeList[i])(x,y,z);

        overlayStr[i] += CELLB( QString().sprintf( "%.3d %.2f", val, val/255.0 ) );
    }

#undef CELLA
#undef CELLB

    QString s = "<html><table width=\"100%\">";
    s += "<tr>" + scorePos + "</tr>";

    for (unsigned int i=0; i < overlayStr.size(); i++)
        s += "<tr>" + overlayStr[i] + "</tr>";

    ui->labelPixInfoMore->setText( s );
}

void AnnotatorWnd::zSliderMoved(int newPos)
{
    mCurZSlice = newPos;

    if (mCurZSlice < 0)
        mCurZSlice = 0;

    if (mCurZSlice >= mVolumeData.depth())
        mCurZSlice = mVolumeData.depth() - 1;

    updateImageSlice();
}

void AnnotatorWnd::updateImageSlice(int)
{
    updateImageSlice();
}

bool AnnotatorWnd::constraintsUpdateImagesliceCallback(QImage &slice)
{
    if (!mConstraintsDisplayTimer->isActive())
        return false;

    if ( !ui->groupBoxRestrictPixLabels->isChecked() )
        return false;

    const PixelType *imgPtr = mVolumeData.sliceData( mCurZSlice );
    unsigned int *pixPtr = (unsigned int *) slice.constBits(); // trick!
    unsigned int sz = mVolumeData.width() * mVolumeData.height();

    const unsigned char minThr = ui->spinPixMin->value();
    const unsigned char maxThr = ui->spinPixMax->value();

    overlayRGBThresholded( pixPtr, imgPtr, pixPtr, sz, mScoreColor, minThr, maxThr, false );

    return true;
}

void AnnotatorWnd::constraintsChangedCallback()
{
    if (!ui->groupBoxRestrictPixLabels->isChecked())
        return;

    mConstraintsDisplayTimer->setSingleShot(true);
    mConstraintsDisplayTimer->start( 600 );
    updateImageSlice();
}

void AnnotatorWnd::constraintsTimerCallback() // timer callback
{
    // then process an update so that it 'resumes normal operation'
    updateImageSlice();
}

void AnnotatorWnd::on_cubeBrushSizeX_valueChanged(int width){
    cubeBrush.width = width;
    sphereBrush.width = width;
}

void AnnotatorWnd::on_cubeBrushSizeY_valueChanged(int height){
    cubeBrush.height = height;
    sphereBrush.height = height;
}

void AnnotatorWnd::on_cubeBrushSizeZ_valueChanged(int depth){
    cubeBrush.depth = depth;
    sphereBrush.depth = depth;
}

void AnnotatorWnd::updateImageSlice()
{
    QImage qimg;

    mVolumeData.QImageSlice( mCurZSlice, qimg );

    // hide?
    if ( ui->actionHide_volume->isChecked() )
        qimg.fill( Qt::black );

    if (constraintsUpdateImagesliceCallback(qimg))
    {
        // just update and return
        ui->labelImg->setImage(qimg);
        return;
    }


    // score overlay?
    if ( mScoreImageEnabled )
    {
        // sanity check
        if ( !mScoreImage.isSizeLike( mVolumeData ) ) {
            qWarning("mScoreImageEnabled == true but score image is not of valid size");
        }
        else
        {
            // use as red channel
            const PixelType *scorePtr = mScoreImage.sliceData( mCurZSlice );
            unsigned int *pixPtr = (unsigned int *) qimg.constBits(); // trick!
            unsigned int sz = mVolumeLabels.width() * mVolumeLabels.height();

            const unsigned char minThr = ui->spinScoreThrAbove->value();
            const unsigned char maxThr = ui->spinScoreThrBelow->value();

            if (minThr == 0 && maxThr==255)
                overlayRGB( pixPtr, scorePtr, pixPtr, sz, mScoreColor );
            else
                overlayRGBThresholded( pixPtr, scorePtr, pixPtr, sz, mScoreColor, minThr, maxThr, ui->chkHardThreshold->isChecked() );
        }
    }

    // user-overlays
    for (unsigned int overlayIdx=0; overlayIdx < mOverlayMenuActions.size(); overlayIdx++)
    {
        if ( !mOverlayVolumeList[overlayIdx]->isSizeLike( mVolumeData ) )
            continue;
        if ( !mOverlayMenuActions[overlayIdx]->isChecked() )
            continue;

        //int floatTransp = 256 * mOverlayInfo[overlayIdx]->alpha;
        //int floatTranspInv = 256 - floatTransp;
        const OverlayType *scorePtr = mOverlayVolumeList[overlayIdx]->sliceData( mCurZSlice );
        unsigned int *pixPtr = (unsigned int *) qimg.constBits(); // trick!
        unsigned int sz = mVolumeLabels.width() * mVolumeLabels.height();


        const QColor &color = mOverlayColorList.getColor(overlayIdx);

        overlayRGB( pixPtr, scorePtr, pixPtr, sz, color, mOverlayInfo[overlayIdx]->alpha );

    }

    // check ground truth slice and draw it
    /*
    if (mOverlayLabelImage)
    {
        //int intTransp = 255*mOverlayLabelImageTransparency;
        int floatTransp = 256 * mOverlayLabelImageTransparency;
        int floatTranspInv = 256 - floatTransp;

        const LabelType *lblPtr = mVolumeLabels.sliceData( mCurZSlice );
        unsigned int *pixPtr = (unsigned int *) qimg.constBits(); // trick!

        unsigned int sz = mVolumeLabels.width() * mVolumeLabels.height();
        int maxLabel = (int) mLblColorList.count();

        for (unsigned int i=0; i < sz; i++)
        {
            if ( lblPtr[i] == 0 )
                continue;   // unlabeled, we don't care

            if (lblPtr[i] > maxLabel) {
                qDebug("Label exceed possible colors: %d", (int)lblPtr[i]);
                continue;
            }

            int vv = (floatTransp * (pixPtr[i] & 0xFF))/256;

            const QColor &color = mLblColorList.getColor(lblPtr[i] - 1);

            int g = vv + (floatTranspInv * color.green()) / 256;
            int b = vv + (floatTranspInv * color.blue()) / 256;
            int r = vv + (floatTranspInv * color.red()) / 256;

            QColor c = QColor::fromRgb( r,g,b );

            pixPtr[i] = c.rgb();
        }
    }
    */

    if (mSelectedSV.valid)  //if selection is valid, draw highlight
    {
        const qreal opacity = 0.6;
        const qreal invOpacity = 0.99 - opacity;

        const qreal cRd = mSelectionColor.redF() * opacity;
        const qreal cGr = mSelectionColor.greenF() * opacity;
        const qreal cBl = mSelectionColor.blueF() * opacity;

        for (int i=0; i < mSelectedSV.pixelList.size(); i++)
        {
            unsigned int z = mSelectedSV.pixelList[i].coords.z;
            if ( mCurZSlice != z )
                continue;

            unsigned int x = mSelectedSV.pixelList[i].coords.x;
            unsigned int y = mSelectedSV.pixelList[i].coords.y;

            QColor pixColor = QColor::fromRgb( qimg.pixel(x, y) );

            qreal r = pixColor.redF() * invOpacity + cRd;
            qreal g = pixColor.greenF() * invOpacity + cGr;
            qreal b = pixColor.blueF() * invOpacity + cBl;

            qimg.setPixel( x, y, QColor::fromRgbF( r, g, b ).rgb() );
        }
    }else{ //if mouse point valid

        if (ui->brushToolCube->isChecked()){
            cubeBrush.paint(qimg, mCurX, mCurY, mSelectionColor);
        }else if( ui->brushToolSphere->isChecked())
            sphereBrush.paint(qimg, mCurX, mCurY, mSelectionColor);
        else
            cubeBrush.paint(qimg, mCurX, mCurY, mSelectionColor);
     }

    ui->labelImg->setImage( qimg );
}

void AnnotatorWnd::annotateSupervoxel( const SupervoxelSelection &SV, LabelType label, bool onlyCurrentSlice )
{
    if (!SV.valid)
        return;

    // then mark it according to the GT
    if (!onlyCurrentSlice)
    {
        for (int i=0; i < SV.pixelList.size(); i++) {
            mVolumeLabels.data()[ SV.pixelList[i].index ] = label;
        }
    } else
    {
        for (int i=0; i < SV.pixelList.size(); i++)
        {
            if ( SV.pixelList[i].coords.z == mCurZSlice )
                mVolumeLabels.data()[ SV.pixelList[i].index ] = label;
        }
    }

    //// this should go on a status bar, and disappear after a while
    statusBarMsg(QString("%1 pixels labeled with Label %2").arg(SV.pixelList.size()).arg(label));

    // callback
    userModifiedSupervoxelLabel();
}

void AnnotatorWnd::labelImageMouseReleaseEvent(QMouseEvent * e)
{
    QPoint pt = ui->labelImg->screenToImage( e->pos() );

    //TODO copy stuff from moved to released

    int x = pt.x();
    int y = pt.y();

    // call plugin mouse move event
    //for (unsigned i=0; i < mPluginBaseList.size(); i++)
    //    mPluginBaseList[i]->mouseReleaseEvent( e, x, y, mCurZSlice );

    // select supervoxel for labeling?
    if ( e->button() == Qt::LeftButton )
    {
        if ( !mSelectedSV.valid )
            return; // no superpixel valid

        // try to find a pixel of the current supervoxel where the mouse is
        // otherwise we will not do anything
        bool found = false;
        for (int i=0; i < mSelectedSV.pixelList.size(); i++)
        {
            if (mSelectedSV.pixelList[i].coords.z != mCurZSlice)
                continue;

            if ( (mSelectedSV.pixelList[i].coords.x == pt.x()) && (mSelectedSV.pixelList[i].coords.y == pt.y())) {
                found = true;
                break;
            }
        }

        if (!found)
            return;

        if(0)   // try region growing
        {
            // compute region mean
            unsigned int mean = 0;
            for ( unsigned int i=0; i < mSelectedSV.pixelList.size(); i++ )
                mean += mVolumeData.data()[ mSelectedSV.pixelList[i].index ];

            mean /= mSelectedSV.pixelList.size();
            double fMean = mean;

            double var = 0;
            for ( unsigned int i=0; i < mSelectedSV.pixelList.size(); i++ ) {
                double dv = mVolumeData.data()[mSelectedSV.pixelList[i].index] - fMean;
                var += dv*dv;
            }

            double stdDev = sqrt( var / mSelectedSV.pixelList.size() );

            double minVal = fMean - stdDev;
            double maxVal = fMean + stdDev;

            if ( minVal < 0 )   minVal = 0;
            if ( maxVal > 255 )   maxVal = 255;
            qDebug() << "Limits " << minVal << "  " << maxVal;

            PixelInfoList pixListResult;
            unsigned int maxRegionSize = 4 * mSelectedSV.pixelList.size();
            RegionGrow<PixelType>( mVolumeData, mSelectedSV.pixelList, (PixelType)minVal, (PixelType)maxVal, maxRegionSize,  &pixListResult );

            qDebug() << "New: " << pixListResult.size();

            mSelectedSV.pixelList = pixListResult;
        }

        annotateSupervoxel( mSelectedSV, ui->comboLabel->currentIndex(), ui->chkOnlyCurSlice->isChecked() );
        mSelectedSV.valid = false;

        updateImageSlice();
    }

    if ( e->button() == Qt::RightButton )  // iterate through possible labels
    {
        int incr = 1;
        if ( e->modifiers() == Qt::ShiftModifier )
            incr = -1;

        int newLbl = ui->comboLabel->currentIndex() + incr;
        if ( newLbl < 0 )
            newLbl = 0;
        if (newLbl >= ui->comboLabel->count())
            newLbl = ui->comboLabel->count() - 1;

        ui->comboLabel->setCurrentIndex( newLbl );
    }
}

//TODO change this to support plain annotation as well as SV annotation
void AnnotatorWnd::labelImageMouseMoveEvent(QMouseEvent * e)
{
    //qDebug("Mouse move: %d %d", e->x(), e->y());

    QPoint pt = ui->labelImg->screenToImage( e->pos() );


    int x = pt.x();
    int y = pt.y();

    mCurX = x;
    mCurY = y;

    bool invalid = false;
    if (x < 0)  invalid = true;
    if (y < 0)  invalid = true;
    if (x >= mVolumeData.width())   invalid = true;
    if (y >= mVolumeData.height())  invalid = true;

    if (!invalid)
        updateCursorPixelInfo( x, y, mCurZSlice );

    if( invalid )
        return;

    // call plugin mouse move event if control is not pressed
    if (ui->brushToolPlugin->isChecked()){
        for (unsigned i=0; i < mPluginBaseList.size(); i++)
            mPluginBaseList[i]->mouseMoveEvent( e, x, y, mCurZSlice );
        return;
    }

    if(ui->superAnnotation->isChecked()){
        if(!mSVRegion.valid)
            return;
        //TODO move this somwhere else
        // convert to cropped region coordinates
        UIntPoint3D croppedCoords;
        if (!mSVRegion.inRegion( UIntPoint3D(x, y, mCurZSlice), &croppedCoords ))
            return; // nothing to do, outside cropped area

        // find supervoxel idx
        unsigned int slicIdx = mSVoxel.pixelToVoxel() (croppedCoords.x, croppedCoords.y, croppedCoords.z);
        //qDebug("Slic IDX: %u", slicIdx);

        // find corresponding pixels and copy them to the local structure
        mSVRegion.croppedToWholePixList( mVolumeData, mSVoxel.voxelToPixel().at(slicIdx), mSelectedSV.pixelList );

        // if restricted pixel values is checked..
        if ( ui->groupBoxRestrictPixLabels->isChecked() )
        {
            unsigned char thrMin = ui->spinPixMin->value();
            unsigned char thrMax = ui->spinPixMax->value();
            // make copy
            SlicMapType::value_type oldList = mSelectedSV.pixelList;

            const bool dontOverwriteLabeledPixs = ui->chkDontOverwriteLabeledPIxs->isChecked();

            mSelectedSV.pixelList.clear();

            for (int i=0; i < (int)oldList.size(); i++)
            {
                PixelType val = mVolumeData.data()[ oldList[i].index ];

                if ( (val < thrMin) || (val > thrMax) )
                    continue;   //ignore

                if ( dontOverwriteLabeledPixs ) {
                    bool alreayLabeled = mVolumeLabels.data()[ oldList[i].index ] != 0;
                    if ( alreayLabeled )
                        continue;
                }

                mSelectedSV.pixelList.push_back( oldList[i] );
            }
        }

        // if score image present + score thresholding selected
        if ( ui->chkScoreEnable->isChecked() && mScoreImage.isSizeLike(mVolumeData) )
        {
            // make copy
            SlicMapType::value_type oldList = mSelectedSV.pixelList;

            mSelectedSV.pixelList.clear();

            unsigned char thrVal = ui->spinScoreThreshold->value();

            for (int i=0; i < (int)oldList.size(); i++)
            {
                PixelType val = mScoreImage.data()[ oldList[i].index ];

                if ( val < thrVal )
                    continue;   //ignore

                mSelectedSV.pixelList.push_back( oldList[i] );
            }
        }

        mSelectedSV.svIdx = slicIdx;
        mSelectedSV.valid = true;

        // if mouse is pressed, then automatically annotate it
        if ( e->buttons() == Qt::LeftButton ) {
            annotateSupervoxel( mSelectedSV, ui->comboLabel->currentIndex(), ui->chkOnlyCurSlice->isChecked() );
            mSelectedSV.valid = false;
        }

    }else{
        int label = ui->comboLabel->currentIndex();
        int color = 0;
        if ( e->buttons() == Qt::LeftButton ) {
            //Shift to erase //TODO we want this?
            if (e->modifiers() == Qt::ShiftModifier)
                label = 0;

            Matrix3D<LabelType> *annotationData;

            int overlayindex = ui->layersDisplay->currentIndex().row();
            if(overlayindex >= 0 && overlayindex < mOverlayVolumeList.size() ) {
                annotationData = mOverlayVolumeList.at(overlayindex);
                if (annotationData->isEmpty())
                {
                    annotationData->reallocSizeLike( getVolumeVoxelData() );
                    annotationData->fill(0);

                    mOverlayMenuActions[overlayindex]->setChecked(true);
                    mOverlayMenuActions[overlayindex]->setEnabled(true);
                }
            } else
                annotationData = &mVolumeLabels;

            if(label != 0){
                //TODO fix this
                //color = mLblColorList.getColor(label-1).rgb();
                switch(label) {
                case 1:
                    color = 128;
                    break;
                case 2:
                    color = 255;
                    break;
                default:
                    color = 255;
                }
            }else
                color = 0;

            if (ui->brushToolCube->isChecked())
                cubeBrush.paint(*annotationData, x, y, mCurZSlice, color );
            else if( ui->brushToolSphere->isChecked())
                sphereBrush.paint(*annotationData, x, y, mCurZSlice, color);
            else
                cubeBrush.paint(*annotationData, x, y, mCurZSlice, color);
        }

    }

    //TODO check if I want this
    updateImageSlice();

}

void AnnotatorWnd::labelImageWheelEvent(QWheelEvent * e)
{
    enum Operation { opNone, opZDive, opZoom, opHScroll, opVScroll };

    Operation op = opNone;

    if ( e->modifiers() == Qt::AltModifier )
        op = opZoom;
    if ( e->modifiers() == Qt::NoModifier )
        op = opZDive;
    if ( e->modifiers() == Qt::ControlModifier )
        op = opHScroll;
    if ( e->modifiers() == Qt::ShiftModifier )
        op = opVScroll;

    QPoint pt = ui->labelImg->screenToImage( e->pos() );

    switch(op)
    {
        case opZDive:
        {
            int zPos = ui->zSlider->value();

            if (e->delta() > 0)
                ui->zSlider->setValue( zPos + 1  );
            else
                ui->zSlider->setValue( zPos - 1  );

            // force handler call
            zSliderMoved( ui->zSlider->value() );

            updateCursorPixelInfo( pt.x(), pt.y(), mCurZSlice );

            break;
        }

        case opZoom:
        {
            const double zoomFactor = 1/0.9;
            double toZoom = zoomFactor;
            if (e->delta() < 0)
                toZoom = 1.0/toZoom;

            ui->labelImg->scale( toZoom );
            statusBarMsg( QString().sprintf("Zoom: %.1f", ui->labelImg->scaleFactor()) );

            break;
        }

        case opVScroll:
        case opHScroll:
        {
            const int toScroll = -e->delta()/4;

            if (op == opHScroll)
                ui->labelImg->pan( toScroll, 0 );
            else
                ui->labelImg->pan( 0, toScroll );

            break;
        }

    }

    //qDebug("Wheel signal: %d", e->delta());
}

void AnnotatorWnd::pluginUpdateDisplay()
{
    updateImageSlice();
}

QMenu *AnnotatorWnd::getPluginMenuPtr()
{
    return ui->menuPlugins;
}

AnnotatorWnd::~AnnotatorWnd()
{
            delete ui;
}

void AnnotatorWnd::closeEvent(QCloseEvent *evt)
{
    //TODO not sure this is working
//    if (mSaveLabelsOnExit)
//        saveAnnotation( mSaveLabelsOnExitPath );

    qApp->quit();
    QMainWindow::closeEvent(evt);
}
