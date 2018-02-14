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

#include "CPreview.h"
#include "CImportUnstitched.h"
#include "PTabImport.h"
#include "PTeraStitcher.h"
#include "StackStitcher.h"
#include "S_config.h"
#include "VirtualVolume.h"

using namespace terastitcher;

CPreview* CPreview::uniqueInstance = NULL;

void CPreview::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CPreview::~CPreview()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CPreview destroyed\n", this->thread()->currentThreadId());
    #endif
}

//automatically called when current thread is started
void CPreview::run()
{
    /**/ts::debug(ts::LEV1, 0, __tsp__current__function__);

    try
    {
        // check precondition #1: valid volume handle
        if(!volume)
            throw iom::exception("Invalid volume handle");

        // initialize stitcher
        /**/ts::debug(ts::LEV_MAX, "Initialize stitcher", __tsp__current__function__);
        StackStitcher stitcher(volume);

        // stitch selected slice
        /**/ts::debug(ts::LEV_MAX, strprintf("stitch slice %d", slice_index).c_str(), __tsp__current__function__);
        stitcher.mergeTiles("", -1, -1, 0, false, -1, -1, -1, -1, slice_index, slice_index+1,
                            false, false, S_SHOW_STACK_MARGIN, HALVE_BY_MEAN, true, false, "tif", bitdepth);

        // load slice image with Vaa3D
        /**/ts::debug(ts::LEV_MAX, "load slice", __tsp__current__function__);

#ifdef VAA3D_TERASTITCHER
        Image4DSimple *img = V3D_env->loadImage(const_cast<char*>(iom::strprintf("%s/test_middle_slice.tif", CImportUnstitched::instance()->getVolume()->getSTACKS_DIR()).c_str()));
        emit sendOperationOutcome(0, img);
#else
		emit sendOperationOutcome(0);
#endif
    }
    catch( iim::IOException& exception)
    {
        /**/ts::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new iom::exception(exception.what()));
    }
    catch(iom::exception& exception)
    {
        /**/ts::warning(strprintf("exception thrown in CPreview::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new iom::exception(exception.what()));
    }
    catch(const char* error)
    {
        /**/ts::warning(strprintf("exception thrown in CPreview::run(): \"%s\"", error).c_str());
        emit sendOperationOutcome(new iom::exception(std::string(error).c_str()));
    }
    catch(...)
    {
        /**/ts::warning(strprintf("exception thrown in CPreview::run(): \"%s\"", "generic error").c_str());
        emit sendOperationOutcome(new iom::exception("Generic error occurred"));
    }
}
