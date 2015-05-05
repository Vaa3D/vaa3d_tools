#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

/** We shouldn't need a .h file
  * but it is necessary for the Q_OBJECT correctness (vtable)
  */

#include <PluginBase.h>
#include <cstdlib>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include "Colormap.h"

// for color image
#include <itkImage.h>
#include <itkRGBPixel.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkExceptionObject.h>
#include <QMouseEvent>

class SegmentationColorifier : public PluginBase
{
    Q_OBJECT
private:
    const PluginServices* mPluginServices;

public:
    SegmentationColorifier(QObject *parent = 0) : PluginBase(parent) {}

    bool    initializePlugin( const PluginServices &pServices )
    {
        mPluginServices = &pServices;

        /** Add a menu item **/
        QAction *action = mPluginServices->getPluginMenu()->addAction( QString("Colorify score image") );
        connect( action, SIGNAL(triggered()), this, SLOT(colorifyClicked()) );

        return true;
    }

    // must return the plugin's name
    QString pluginName() {
        return "Segmentation Colorifier";
    }

    virtual void  mouseMoveEvent( QMouseEvent *evt, unsigned int imgX, unsigned int imgY, unsigned int imgZ )
    {
        /*if ( (evt->buttons() & Qt::LeftButton) == 0 )
            return;

        Matrix3D<ScoreType> &scoreMatrix = mPluginServices->getOverlayVolumeData(0);

        if (scoreMatrix.isEmpty())
            scoreMatrix.reallocSizeLike( mPluginServices->getVolumeVoxelData() );

        scoreMatrix( imgX, imgY, imgZ ) = 255;

        mPluginServices->setOverlayVisible( 0, true );
        mPluginServices->updateDisplay();*/
    }

public slots:
    void colorifyClicked()
    {
        /*QAction *action = qobject_cast<QAction *>(sender());
        unsigned int idx = action->data().toUInt();*/

        Matrix3D<ScoreType> &scoreMatrix = mPluginServices->getScoreVoxelData();
        Matrix3D<PixelType> &imgMatrix = mPluginServices->getVolumeVoxelData();

        if (!scoreMatrix.isSizeLike(imgMatrix))
        {
            QMessageBox::critical( 0, "Invalid score volume", "Please load a valid score volume before calling this plugin." );
            return;
        }

        // ask the user for threshold
        bool ok = false;
        int thr = QInputDialog::getInteger( 0, "Threshold value", "Specify the thresholding value:",
                                  128, 0, 255, 1, &ok );
        if (!ok) return;


        int minCCSize = QInputDialog::getInteger( 0, "Minimum CC Size", "Specify minimum CC size:",
                                  1000, 0, 1000000, 1, &ok );
        if (!ok) return;


        // ---- Begin ITK processing: Threshold + CC
        typedef unsigned int LabelScalarType;

        Matrix3D<LabelScalarType> CCMatrix;
        LabelScalarType labelCount;

        // we need to store info to remove small regions
        std::vector<ShapeStatistics<itk::ShapeLabelObject<LabelScalarType, 3> > >   shapeDescr;

        scoreMatrix.createLabelMap<LabelScalarType>( thr, 255, &CCMatrix,
                                                  false, &labelCount, &shapeDescr );

        qDebug("CC regions: %d", (int)labelCount);

        if (labelCount == 0)
        {
            QMessageBox::critical( 0, "No regions found", "No connected regions were found." );
            return;
        }

        // now create RGB image
        {
            typedef itk::RGBPixel< unsigned char > RGBPixelType;
            typedef itk::Image< RGBPixelType, 3 > RGBImageType;
            typedef itk::ImageFileWriter<RGBImageType> WriterType;
            typedef Matrix3D<LabelScalarType>::ItkImageType LabelImageType;

            // original image
            LabelImageType::Pointer  labelsImage = CCMatrix.asItkImage();

            RGBImageType::Pointer rgbImage = RGBImageType::New();

            rgbImage->SetRegions( labelsImage->GetLargestPossibleRegion() );
            rgbImage->Allocate();

            // get colormap
            Colormap::itkRGBPixelVector  rgbCMap;
            Colormap  cmap( Colormap::Lines );
            cmap.get(rgbCMap);

            const unsigned cmapSize = rgbCMap.size();

            // null color
            RGBPixelType nullRGB;
            nullRGB.Set(0,0,0); // black

            // now copy pixel values, get an iterator for each
            itk::ImageRegionConstIterator<LabelImageType> labelsIterator(labelsImage, labelsImage->GetLargestPossibleRegion());
            itk::ImageRegionIterator<RGBImageType> rgbIterator(rgbImage, rgbImage->GetLargestPossibleRegion());

            // WARNING: assuming same searching order-- could be wrong!!
            while( (! labelsIterator.IsAtEnd()) && (!rgbIterator.IsAtEnd()) )
            {
                if ((labelsIterator.Value() == 0) || ( shapeDescr[labelsIterator.Value() - 1].numVoxels() < minCCSize ) )
                    rgbIterator.Set( nullRGB );
                else
                {
                    const unsigned cIdx = (labelsIterator.Value() - 1) % cmapSize;
                    rgbIterator.Set( rgbCMap[cIdx] );
                }

                ++labelsIterator;
                ++rgbIterator;
            }


            // write to file
            QString destFile = QFileDialog::getSaveFileName( 0, "Choose destination file", QString(), "*.nrrd" );

            WriterType::Pointer writer = WriterType::New();
            writer->SetInput(rgbImage);
            writer->SetFileName( destFile.toLocal8Bit().constData() );

            try {
                writer->Update();
            }
            catch( itk::ExceptionObject & err )
            {
                QMessageBox::critical( 0, "Error saving file", QString("Error saving file: %1.").arg( err.GetDescription() ) );
                return;
            }
        }

/*

        // MUST BE RESIZED!
        ovMatrix.reallocSizeLike( mPluginServices->getVolumeVoxelData() );

        LabelType *dPtr = ovMatrix.data();

        for (unsigned int i=0; i < ovMatrix.numElem(); i++)
            dPtr[i] = rand() % 128;

        // set enabled
        mPluginServices->setOverlayVisible( idx, true );

        mPluginServices->updateDisplay();*/
    }
};

#endif // TESTPLUGIN_H
