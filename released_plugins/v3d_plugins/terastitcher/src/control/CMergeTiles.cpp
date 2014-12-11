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
#include "StackStitcher.h"
#include "ProgressBar.h"
#include "IM_config.h"
#include "vmBlockVolume.h"

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
            throw iom::exception("Unable to start this step. A volume must be properly imported first.");

        if(pMergeTiles != 0)
        {
            // retrieve user's input
            StackStitcher stitcher(volume);
            int sliceheight =  pMergeTiles->block_height_field->value();
            int slicewidth = pMergeTiles->block_width_field->value();
            int slicedepth =pMergeTiles->block_depth_field->value();
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
            std::string img_format = pMergeTiles->img_format_cbox->currentText().toStdString().c_str();
            int img_depth = pMergeTiles->imgdepth_cbox->currentText().section(" ", 0, 0).toInt();

            // launch merging
            if ( vm::VOLUME_OUTPUT_FORMAT_PLUGIN.compare(BlockVolume::id)==0 )
                stitcher.mergeTilesVaa3DRaw(volumedir, sliceheight, slicewidth, slicedepth, resolutions,excludenonstitchables, row0, row1, col0, col1,
                                    slice0, slice1,restoreSPIM,restore_direction, blending_algo, false, true, img_format.c_str(), img_depth );
            else if ( vm::VOLUME_OUTPUT_FORMAT_PLUGIN.compare(StackedVolume::id)==0 )
                stitcher.mergeTiles(volumedir, sliceheight, slicewidth, resolutions,excludenonstitchables, row0, row1, col0, col1,
                                    slice0, slice1,restoreSPIM,restore_direction, blending_algo, false, true, img_format.c_str(), img_depth );
            else
                throw iom::exception(vm::strprintf("Unsupported output volume format plugin \"%s\"", vm::VOLUME_OUTPUT_FORMAT_PLUGIN.c_str()).c_str());

            // check that a volume with non-zero dimensions has been produced
            if(stitcher.getV1()-stitcher.getV0() <= 0 || stitcher.getH1()-stitcher.getH0() <= 0 || stitcher.getD1()-stitcher.getD0() <= 0)
                throw iom::exception("Empty volume selected");

            //if a resolution has be selected to be shown in Vaa3D, it is necessary to load each slice and to create a new Image4DSimple object
            if(resolution_index_vaa3D != -1)
            {
                //updating progress bar message
                ProgressBar::instance()->start("Loading volume into Vaa3D...");

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

                // use Vaa3D to load all slices
                std::vector<Image4DSimple*> slices;
                for (int k = 0; k < slices_list.size(); k++)
                {
                    slices.push_back(new Image4DSimple());
                    slices.back()->loadImage(const_cast<char*>(QString(voldir.path().append("/").append(slices_list.at(k).toLocal8Bit().constData())).toStdString().c_str()), false);
                }

                // allocate image data assuming all slices have the same X, Y, C dimensions and bitdepth
                img = new Image4DSimple();
                img->setFileName(voldir.path().toStdString().c_str());
                V3DLONG slice_dims = slices[0]->getXDim()*slices[0]->getYDim()*slices[0]->getCDim()*slices[0]->getUnitBytes();
                unsigned char* img_data = new iom::uint8[slice_dims * slices_list.size()];

                // copy each loaded slice into the volume
                for (int k = 0; k < slices.size(); k++)
                    for(V3DLONG pc = 0; pc < slice_dims; pc++)
                        img_data[k*slice_dims + pc] = slices[k]->getRawData()[pc];

                // set image data
                img->setData(img_data, slices[0]->getXDim(), slices[0]->getYDim(), slices_list.size(), slices[0]->getCDim(), slices[0]->getDatatype());

                // deallocate data
                for (int k = 0; k < slices.size(); k++)
                    delete slices[k];

                // set image attributes
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
        emit sendOperationOutcome(new iom::exception(exception.what()), 0);
    }
    catch( iom::exception& exception)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new iom::exception(exception.what()), 0);
    }
    catch(const char* error)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", error).c_str());
        emit sendOperationOutcome(new iom::exception(error), 0);
    }
    catch(std::bad_alloc& ba)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", ba.what()).c_str());
        emit sendOperationOutcome(new iom::exception(ba.what()), 0);
    }
    catch(...)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", "Generic error").c_str());
        emit sendOperationOutcome(new iom::exception("Unable to determine error's type"), 0);
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
