//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#include "CMergeTiles.h"
#include "CImport.h"
#include <new>
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include "StackStitcher.h"
#include "ProgressBar.h"
#include "IM_config.h"

using namespace terastitcher;

CMergeTiles* CMergeTiles::uniqueInstance = NULL;

void CMergeTiles::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CMergeTiles::~CMergeTiles()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CMergeTiles destroyed\n", this->thread()->currentThreadId());
    #endif
}

//automatically called when current thread is started
void CMergeTiles::run()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CMergeTiles::run() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //pointer to the <Image4DSimple> object which stores the volume to be shown into Vaa3D
        Image4DSimple* img = 0;

        //checking that a volume has been imported first
        vm::VirtualVolume* volume = CImport::instance()->getVolume();
        if(!volume)
            throw MyException("Unable to start this step. A volume must be properly imported first.");

        if(pMergeTiles != 0)
        {
            //merging
            StackStitcher stitcher(volume);
            int sliceheight = pMergeTiles->volumeformat_cbox->currentIndex() != 0 ? pMergeTiles->block_height_field->value() : -1;
            int slicewidth = pMergeTiles->volumeformat_cbox->currentIndex() != 0  ? pMergeTiles->block_width_field->value() : -1;
            int slicedepth = pMergeTiles->volumeformat_cbox->currentIndex() == 2 ? pMergeTiles->block_depth_field->value() : -1;
            bool restoreSPIM = pMergeTiles->restoreSPIM_cbox->currentIndex() != 0;
            std::string volumedir = pMergeTiles->savedir_field->text().toStdString();
            bool excludenonstitchables = pMergeTiles->excludenonstitchables_cbox->isChecked();
            int row0 = pMergeTiles->row0_field->value();
            int row1 = pMergeTiles->row1_field->value();
            int col0 = pMergeTiles->col0_field->value();
            int col1 = pMergeTiles->col1_field->value();
            int slice0 = pMergeTiles->slice0_field->value();
            int slice1 = pMergeTiles->slice1_field->value();
            int restore_direction = pMergeTiles->restoreSPIM_cbox->currentIndex();
            int blending_algo = pMergeTiles->blendingalbo_cbox->currentIndex();
            std::string img_format = pMergeTiles->imgformat_cbox->currentText().toStdString().c_str();
            int img_depth = pMergeTiles->imgdepth_cbox->currentText().toInt();

            if(pMergeTiles->volumeformat_cbox->currentText().toStdString().compare(tsp::IMAGE_FORMAT_TILED_3D_ANY) == 0)
            {
                // @FIXED by Alessandro on 2014-06-25: mergeTilesVaa3DRaw needs the generic format instead of file extension
                if(img_format.compare("tif") == 0 || img_format.compare("tiff") == 0)
                    img_format = "Tiff3D";
                else if(img_format.compare("v3draw") == 0)
                    img_format = "Vaa3DRaw";
                stitcher.mergeTilesVaa3DRaw(volumedir, sliceheight, slicewidth, slicedepth, resolutions,excludenonstitchables, row0, row1, col0, col1,
                                    slice0, slice1,restoreSPIM,restore_direction, blending_algo, false, true, img_format.c_str(), img_depth );
            }
            else
                stitcher.mergeTiles(volumedir, sliceheight, slicewidth, resolutions,excludenonstitchables, row0, row1, col0, col1,
                                    slice0, slice1,restoreSPIM,restore_direction, blending_algo, false, true, img_format.c_str(), img_depth );

            //checking that a volume with non-zero dimensions has been produced
            if(stitcher.getV1()-stitcher.getV0() <= 0 || stitcher.getH1()-stitcher.getH0() <= 0 || stitcher.getD1()-stitcher.getD0() <= 0)
                throw MyException("Empty volume selected");

            //if a resolution has be selected to be shown in Vaa3D, it is necessary to load each slice and to create a new Image4DSimple object
            if(resolution_index_vaa3D != -1)
            {
                //updating progress bar message
                ProgressBar::getInstance()->start("Loading volume into Vaa3D...");

                //retrieving the directory path where the selected volume is stored
                QString volpath = pMergeTiles->savedir_field->text();
                int height = (stitcher.getV1()-stitcher.getV0())/pow(2.0f,resolution_index_vaa3D);
                int width = (stitcher.getH1()-stitcher.getH0())/pow(2.0f,resolution_index_vaa3D);
                int depth = (stitcher.getD1()-stitcher.getD0())/pow(2.0f,resolution_index_vaa3D);
                volpath.append("/RES(").append(QString::number(height));
                volpath.append("x").append(QString::number(width));
                volpath.append("x").append(QString::number(depth));
                volpath.append(")");
                QDir voldir(volpath);
                QStringList first_level_list = voldir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
                voldir.setPath(voldir.path().append("/").append(first_level_list.first()));
                QStringList second_level_list = voldir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
                voldir.setPath(voldir.path().append("/").append(second_level_list.first()));
                QStringList slices_list = voldir.entryList(QDir::Files);

                //allocation of image data
                img = new Image4DSimple();
                img->setFileName(voldir.path().toStdString().c_str());
                uint8* img_data = new uint8[width*height*depth];    //images with any depth are loaded in 8 bit mode with OpenCV

                //loading slices and storing them into img_data
                for (int k = 0; k < slices_list.size(); ++k)
                {
                    const char* slice_path = QString(voldir.path().append("/").append(slices_list.at(k).toLocal8Bit().constData())).toStdString().c_str();
                    IplImage* slice_img = cvLoadImage(slice_path, CV_LOAD_IMAGE_GRAYSCALE);
                    if(!slice_img)
                        throw MyException(QString("Unable to load slice \"").append(slice_path).append("\" to be shown into Vaa3D").toStdString().c_str());
                    if(slice_img->height != height || slice_img->width != width)
                        throw MyException("An error occurred when loading slices to be shown into Vaa3D");

                    int slice_img_step = slice_img->widthStep/sizeof(uchar);
                    for(int i=0; i<height; i++)
                    {
                        uint8* slice_img_data = ((uint8*)slice_img->imageData)+i*slice_img_step;
                        for(int j=0; j<width; j++)
                            img_data[k*height*width + i*width +j] = slice_img_data[j];
                    }
                    cvReleaseImage(&slice_img);
                }
                img->setData(img_data, width, height, depth, 1, V3D_UINT8);

                //setting image attributes
                img->setRezX(volume->getVXL_H()*pow(2.0f,resolution_index_vaa3D));
                img->setRezY(volume->getVXL_V()*pow(2.0f,resolution_index_vaa3D));
                img->setRezZ(volume->getVXL_D()*pow(2.0f,resolution_index_vaa3D));
                img->setOriginX(stitcher.getMultiresABS_H(resolution_index_vaa3D, stitcher.getH0())/1000.0f);
                img->setOriginY(stitcher.getMultiresABS_V(resolution_index_vaa3D, stitcher.getV0())/1000.0f);
                img->setOriginZ(stitcher.getMultiresABS_D(resolution_index_vaa3D, stitcher.getD0())/1000.0f);
            }
        }

        //everything went OK
        emit sendOperationOutcome(0, img);
    }
    catch( iim::IOException& exception)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new MyException(exception.what()), 0);
    }
    catch( MyException& exception)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new MyException(exception.what()), 0);
    }
    catch(const char* error)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", error).c_str());
        emit sendOperationOutcome(new MyException(error), 0);
    }
    catch(std::bad_alloc& ba)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", ba.what()).c_str());
        emit sendOperationOutcome(new MyException(ba.what()), 0);
    }
    catch(...)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", "Generic error").c_str());
        emit sendOperationOutcome(new MyException("Unable to determine error's type"), 0);
    }
}

//reset method
void CMergeTiles::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CMergeTiles::reset()\n", this->thread()->currentThreadId());
    #endif

    for(int i=0; i<S_MAX_MULTIRES; i++)
        resolutions[i] = i==0;
    resolution_index_vaa3D = -1;
    pMergeTiles = 0;
}

void CMergeTiles::setResolutionToShow(int index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CMergeTiles::setResolutionToShow(%d)\n", this->thread()->currentThreadId(), index);
    #endif

    resolution_index_vaa3D = index;
}
