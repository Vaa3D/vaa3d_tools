#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "cannyEdgeDetection.cxx"


using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pbUpdate,SIGNAL(clicked()),this,SLOT(updateEdgeMap()));
    connect(ui->hsLow, SIGNAL( valueChanged( int ) ), ui->lbLowTh, SLOT(setNum(int)));
    connect(ui->hsHigh, SIGNAL( valueChanged( int ) ), ui->lbHighTh, SLOT(setNum(int)));
    connect(ui->hsVar, SIGNAL( valueChanged( int ) ), ui->lbVar, SLOT(setNum(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

double MainWindow::getLowTh()
{
   return ui->hsLow->value();
}

double MainWindow::getHighTh()
{
   return ui->hsHigh->value();
}

double MainWindow::getVar()
{
   return ui->hsVar->value();
}

void MainWindow::updateEdgeMap()
{
    unsigned char lowerTh = getLowTh();
    unsigned char upperTh = getHighTh();
    float gaussianVariance = getVar();

    Matrix3D<PixelType>& volData = mPluginServices->getVolumeVoxelData();
    long nx = volData.width();
    long ny = volData.height();
    long nz = volData.depth();
    long nc = 1;
    uchar* rawDataPtr = volData.data();

    printf("Computing canny map\n");
    float* cannyMap = 0;
    canny<unsigned char, float>(rawDataPtr,
                                (long)nx,(long)ny,(long)nz,(long)nc,
                                lowerTh, upperTh,
                                cannyMap,
                                gaussianVariance);
    printf("Canny map computed. Copying data...\n");

    Matrix3D<OverlayType> &ovMatrix = mPluginServices->getOverlayVolumeData(volume_idx);

    // MUST BE RESIZED!
    ovMatrix.reallocSizeLike(volData);

    LabelType *dPtr = ovMatrix.data();
    assert(ovMatrix.width() == nx);
    assert(ovMatrix.height() == ny);
    assert(ovMatrix.depth() == nz);
    ulong cubeSize = nx*ny*nz;
    for(ulong i = 0; i < cubeSize; i++) {
        dPtr[i] = (cannyMap[i]!=0)?255:0;
    }
    delete[] cannyMap;
    printf("Done\n");

    // set enabled
    mPluginServices->setOverlayVisible( volume_idx, true );

    mPluginServices->updateDisplay();
}
