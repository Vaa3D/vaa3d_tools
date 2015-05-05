#include "GraphCutsPlugin.h"
#include "graphCut.h"
#include "gaussianFilter.cxx"

#include <QInputDialog>
#include "gcdialog.h"
#include "settingsdialog.h"
#include <vector>
#include "utils.h"

#include <QDebug>

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"
#include "itkImportImageFilter.h"
#include "itkConnectedComponentFunctorImageFilter.h"
#include "itkLabelObject.h"
#include "itkLabelMap.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include "itkShapeLabelObject.h"
#include "itkLabelMapToBinaryImageFilter.h"

extern "C" Q_DECL_EXPORT PluginBase* createPlugin();

PluginBase *createPlugin()
{
    return new GraphCutsPlugin();
}

GraphCutsPlugin::GraphCutsPlugin(QObject *parent) : PluginBase(parent)
{
    activeOverlay = 0;
    mouseEventDetected = false;
    brushSizeX = 3;
    brushSizeY = 3;
    brushSizeZ = 3;
    maxWidth = 100;
    maxHeight = 100;
    maxDepth = 100;
    gaussianVariance = 1.0;
    sigma = 100.0;

    outputWeightImage = 0;
    cache_gaussianVariance = -1;

    timer = new QTimer;
    timer->setInterval(40);
    timer->setSingleShot(false);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateOverlay()));
    timer->start();
}

GraphCutsPlugin::~GraphCutsPlugin()
{    
    timer->stop();
    delete timer;
    if(outputWeightImage) {
        delete[] outputWeightImage;
    }
}

void GraphCutsPlugin::changeSettings()
{
    settingsDialog *window = new settingsDialog(0, activeOverlay, brushSizeX, brushSizeY, brushSizeZ, maxWidth, maxHeight, maxDepth);
    if(window->exec() == QDialog::Accepted) {
        activeOverlay = window->getActiveOverlay();
        brushSizeX = window->getBrushSizeX()-1;
        brushSizeY = window->getBrushSizeY()-1;
        brushSizeZ = window->getBrushSizeZ()-1;
        maxWidth = window->getMaxWidth();
        maxHeight = window->getMaxHeight();
        maxDepth = window->getMaxDepth();
        gaussianVariance = window->getGaussianVariance();
        sigma = window->getEdgeWeight();
    }
    delete window;
}

void GraphCutsPlugin::runGraphCuts()
{
    //const int seedRadius = 3;
    const int seedRadius = 0;
    const bool outputOverlays = false;

    QAction *action = qobject_cast<QAction *>(sender());
    unsigned int idx = action->data().toUInt();

    Matrix3D<OverlayType> &scoreImage = mPluginServices->getOverlayVolumeData(idx_bindata_overlay);
    if(scoreImage.isEmpty()) {
        printf("Error: No score image loaded in overlay %d\n", idx_bindata_overlay);
        return;
    }

    qDebug() << "Running graphcuts with variance " << gaussianVariance << " and sigma " << sigma;

    // generate list of seeds
    Matrix3D<ScoreType> &seedOverlay = mPluginServices->getOverlayVolumeData(idx_seed_overlay);
    std::vector<Point> sinkPoints;
    std::vector<Point> sourcePoints;

    if(idx == GC_DATA) {
        for(int x = 0; x < seedOverlay.width(); ++x) {
            for(int y = 0; y < seedOverlay.height(); ++y) {
                for(int z = 0; z < seedOverlay.depth(); ++z) {
                    Point p(x,y,z);
                    if(seedOverlay(x,y,z) == label_seed) {
                        sourcePoints.push_back(p);
                    } else {
                        if(seedOverlay(x,y,z) == label_sink) {
                           sinkPoints.push_back(p);
                        }
                    }
                }
            }
        }
    } else {
        for(int x = 0; x < seedOverlay.width(); ++x) {
            for(int y = 0; y < seedOverlay.height(); ++y) {
                for(int z = 0; z < seedOverlay.depth(); ++z) {
                    Point p(x,y,z);
                    if(seedOverlay(x,y,z) == label_seed && scoreImage(x,y,z) != 0) {
                        sourcePoints.push_back(p);
                    } else {
                        if(seedOverlay(x,y,z) == label_sink && scoreImage(x,y,z) != 0) {
                           sinkPoints.push_back(p);
                        }
                    }
                }
            }
        }
    }

    if(sourcePoints.size() == 0) {
        fprintf(stderr,"Error: 0 source points\n");
        return;
    }
    if(sinkPoints.size() == 0) {
        fprintf(stderr,"Error: 0 sink points\n");
        return;
    }

    Matrix3D<PixelType>& volData = mPluginServices->getVolumeVoxelData();
    ulong cubeSize = volData.numElem();

    // get weight image
    if(outputWeightImage == 0 || cache_gaussianVariance != gaussianVariance) {
        cache_gaussianVariance = gaussianVariance;
        float* foutputWeightImage = 0;
        gradientMagnitude<unsigned char, float>(volData.data(), volData.width(), volData.height(), volData.depth(), 1, gaussianVariance, foutputWeightImage);

        cubeFloat2Uchar(foutputWeightImage,outputWeightImage,volData.width(), volData.height(), volData.depth());
        delete[] foutputWeightImage;
    }

    if(outputOverlays) {
        // copy weight image to overlay
        Matrix3D<OverlayType> &weightMatrix = mPluginServices->getOverlayVolumeData(idx_weight_overlay);
        weightMatrix.reallocSizeLike(volData);
        LabelType *dPtr = weightMatrix.data();
        for(ulong i = 0; i < cubeSize; i++) {
            dPtr[i] = outputWeightImage[i];
        }
    }

    Cube cGCWeight;
    cGCWeight.width = volData.width();
    cGCWeight.height = volData.height();
    cGCWeight.depth = volData.depth();
    cGCWeight.data = outputWeightImage;
    cGCWeight.wh = volData.width()*volData.height();

    bool use_histograms = false;
    int ccId = -1;
    ulong nObjects = 0;
    LabelImageType::Pointer labelInput = 0;
    LabelImageType* ptrLabelInput = 0;

    eUnaryWeights unaryType = UNARY_NONE;
    if(idx == GC_SCORES) {
        unaryType = UNARY_SCORE;
        use_histograms = false;
    } else {
        if(idx == GC_DATA) {
            use_histograms = true;
        }
    }

    GraphCut g;

    if(!scoreImage.isEmpty() && idx == GC_DEFAULT) {
        labelInput = getLabelImage<uchar,LabelImageType>(scoreImage.data(),scoreImage.width(),scoreImage.height(),scoreImage.depth(),&nObjects);

        // check that seed points belong to the same connected component
        std::vector<Point>::iterator it = sinkPoints.begin();
        ptrLabelInput = labelInput.GetPointer();
        LabelImageType::IndexType index;
        index[0] = it->x; index[1] = it->y; index[2] = it->z;
        ccId = ptrLabelInput->GetPixel(index);
        for(; it != sinkPoints.end(); ++it) {
            index[0] = it->x; index[1] = it->y; index[2] = it->z;
            int _ccId = ptrLabelInput->GetPixel(index);
            // _ccId = 0 means background
            if(_ccId != 0 && ccId != _ccId) {
                printf("Info: seed points belong to different connected components %d %d.\n", ccId, _ccId);
                use_histograms = false;
                ccId = -1;
                break;
            }
        }
        if(!use_histograms) {
            for(it = sourcePoints.begin(); it != sourcePoints.end(); ++it) {
                index[0] = it->x; index[1] = it->y; index[2] = it->z;
                int _ccId = ptrLabelInput->GetPixel(index);
                // _ccId = 0 means background
                if(_ccId != 0 && ccId != _ccId) {
                    printf("Info: seed points belong to different connected components %d %d.\n", ccId, _ccId);
                    use_histograms = false;
                    ccId = -1;
                    break;
                }
            }
        }

        printf("[Main] Extracting sub-cube using conncted component\n");
        g.extractSubCube(scoreImage.data(),
                         cGCWeight.data,
                         labelInput,
                         sourcePoints,sinkPoints,
                         cGCWeight.width,cGCWeight.height,cGCWeight.depth);
    } else {
        labelInput = getLabelImage<uchar,LabelImageType>(scoreImage.data(),scoreImage.width(),scoreImage.height(),scoreImage.depth(),&nObjects);

        // check that seed points belong to the same connected component
        std::vector<Point>::iterator it = sinkPoints.begin();
        ptrLabelInput = labelInput.GetPointer();
        LabelImageType::IndexType index;
        index[0] = it->x; index[1] = it->y; index[2] = it->z;
        ccId = ptrLabelInput->GetPixel(index);
        for(; it != sinkPoints.end(); ++it) {
            index[0] = it->x; index[1] = it->y; index[2] = it->z;
            int _ccId = ptrLabelInput->GetPixel(index);
            // _ccId = 0 means background
            if(_ccId != 0 && ccId != _ccId) {
                printf("Info: seed points belong to different connected components %d %d.\n", ccId, _ccId);
                use_histograms = false;
                ccId = -1;
                break;
            }
        }
        if(!use_histograms) {
            for(it = sourcePoints.begin(); it != sourcePoints.end(); ++it) {
                index[0] = it->x; index[1] = it->y; index[2] = it->z;
                int _ccId = ptrLabelInput->GetPixel(index);
                // _ccId = 0 means background
                if(_ccId != 0 && ccId != _ccId) {
                    printf("Info: seed points belong to different connected components %d %d.\n", ccId, _ccId);
                    use_histograms = false;
                    ccId = -1;
                    break;
                }
            }
        }

        printf("[Main] Extracting sub-cube (fixed size)\n");
        g.extractSubCube(cGCWeight.data,
                         sourcePoints,sinkPoints,
                         cGCWeight.width,cGCWeight.height,cGCWeight.depth,
                         maxWidth, maxHeight, maxDepth);
    }

    if(use_histograms) {
        unaryType = UNARY_HISTOGRAMS;
    }

    printf("ccId = %d, use_histograms = %d, unaryType = %d\n", ccId, (int)use_histograms, (int)unaryType);    

    if(outputOverlays) {
        if(ptrLabelInput != 0) {
            // copy label image to overlay
            LabelImageType::IndexType index;
            LabelImageType::PixelType pixel;

            if(nObjects < 255) {
                Matrix3D<OverlayType> &labelMatrix = mPluginServices->getOverlayVolumeData(idx_label_overlay);
                labelMatrix.reallocSizeLike(volData);
                LabelType * dPtr = labelMatrix.data();
                ulong cubeIdx = 0;
                float objToIndex = 255.0/nObjects;
                for(ulong z = 0; z < volData.depth(); z++) {
                    for(ulong y = 0; y < volData.height(); y++) {
                        for(ulong x = 0; x < volData.width(); x++) {
                            index[0] = x; index[1] = y; index[2] = z;
                            pixel = ptrLabelInput->GetPixel(index);
                            dPtr[cubeIdx] = (uchar)pixel*objToIndex;
                            ++cubeIdx;
                        }
                    }
                }
                mPluginServices->setOverlayVisible( idx_label_overlay, true );
            }
            /*
            else {
                ulong cubeSize = volData.depth()*volData.height()*volData.width()*3;
                uchar* dPtr = new uchar[cubeSize];
                ulong cubeIdx = 0;
                for(ulong z = 0; z < volData.depth(); z++) {
                    for(ulong y = 0; y < volData.height(); y++) {
                        for(ulong x = 0; x < volData.width(); x++) {
                            index[0] = x; index[1] = y; index[2] = z;
                            pixel = ptrLabelInput->GetPixel(index);
                            dPtr[cubeIdx] = (uchar)pixel&0xff;
                            dPtr[cubeIdx+1] = (uchar)pixel&0xff00;
                            dPtr[cubeIdx+2] = (uchar)pixel&0xff0000;
                            cubeIdx += 3;
                        }
                    }
                }
                exportColorTIFCube(dPtr, "cc", volData.depth(), volData.height(), volData.width());
            }
            */
        }
    }

    Cube originalCube;
    originalCube.width = volData.width();
    originalCube.height = volData.height();
    originalCube.depth = volData.depth();
    if(scoreImage.isEmpty()) {
        originalCube.data = 0;
    } else {
        originalCube.data = scoreImage.data();
    }
    originalCube.wh = originalCube.width*originalCube.height;

    printf("Running max-flow with %ld sources and %ld sinks\n", sourcePoints.size(), sinkPoints.size());
    g.run_maxflow(&cGCWeight, sourcePoints, sinkPoints, sigma, seedRadius, unaryType, &originalCube);

    unsigned char* output_data1d = new uchar[cubeSize];
    if(!use_histograms) {
        memcpy(output_data1d,scoreImage.data(),cubeSize);
    }

    printf("Copy output\n");
    if(idx == GC_DATA) {
        g.getOutput(&originalCube, output_data1d);
    } else {
        g.getOutputGivenSeeds(&originalCube, output_data1d);
    }
    if(ccId != -1) {
        printf("Applying cut\n");
        g.applyCut(ptrLabelInput, &originalCube, output_data1d, ccId, scoreImage.data());
    }

    // copy output to a new overlay    
    Matrix3D<OverlayType> &ovMatrix = mPluginServices->getOverlayVolumeData(idx_output_overlay);
    ovMatrix.reallocSizeLike(volData);
    LabelType *dPtr = ovMatrix.data();
    for(ulong i = 0; i < cubeSize; i++) {
        dPtr[i] = output_data1d[i];
    }

    // set enabled    
    mPluginServices->setOverlayVisible( idx_weight_overlay, true );
    mPluginServices->setOverlayVisible( idx_output_overlay, true );
    mPluginServices->updateDisplay();

    printf("Cleaning\n");
    delete[] output_data1d;    
    printf("Done\n");
}

void GraphCutsPlugin::cleanSeedOverlay()
{
    Matrix3D<ScoreType> &seedOverlay = mPluginServices->getOverlayVolumeData(idx_seed_overlay);

    if (seedOverlay.isEmpty())
    {
        seedOverlay.reallocSizeLike( mPluginServices->getVolumeVoxelData() );
    }

    seedOverlay.fill(0);
    mPluginServices->setOverlayVisible(idx_seed_overlay, true );
    mPluginServices->updateDisplay();
}

void GraphCutsPlugin::transferOverlay()
{
    Matrix3D<ScoreType> &outputOverlay = mPluginServices->getOverlayVolumeData(idx_output_overlay);

    if (!outputOverlay.isEmpty())
    {
        Matrix3D<ScoreType> &inputOverlay = mPluginServices->getOverlayVolumeData(idx_bindata_overlay);
        //inputOverlay.copyFrom(outputOverlay);
        LabelType *dPtrInput = inputOverlay.data();
        LabelType *dPtrOutput = outputOverlay.data();
        ulong cubeSize = outputOverlay.numElem();
        for(ulong i = 0; i < cubeSize; i++) {
            dPtrInput[i] = (dPtrOutput[i]==0)?0:76; // 76 corresponds to 30% transparent
        }
        mPluginServices->setOverlayVisible(idx_bindata_overlay, true );
        mPluginServices->updateDisplay();

        cleanSeedOverlay();
    }    
}
