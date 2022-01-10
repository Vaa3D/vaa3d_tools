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

#include "CImport.h"
#include "src/presentation/PTabImport.h"
#include "src/presentation/PMain.h"
#include "../core/volumemanager/vmBlockVolume.h"
#include "../core/volumemanager/vmStackedVolume.h"
#include "IM_config.h"

using namespace terastitcher;

CImport* CImport::uniqueInstance = 0;

void CImport::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

CImport::~CImport()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CImport destroyed\n", this->thread()->currentThreadId());
    #endif

    if(volume)
        delete volume;
}

//SET methods
void CImport::setAxes(string axs1, string axs2, string axs3)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CImport setAxes(%s, %s, %s) launched\n", this->thread()->currentThreadId(), axs1.c_str(), axs2.c_str(), axs3.c_str());
    #endif

    AXS_1 = vm::str2axis(axs1);
    AXS_2 = vm::str2axis(axs2);
    AXS_3 = vm::str2axis(axs3);
}
void CImport::setVoxels(float vxl1, float vxl2, float vxl3)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CImport setVoxels(%.1f, %.1f, %.1f) launched\n", this->thread()->currentThreadId(), vxl1, vxl2, vxl3);
    #endif

    VXL_1 = vxl1;
    VXL_2 = vxl2;
    VXL_3 = vxl3;
}

//automatically called when current thread is started
void CImport::run()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CImport::run() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        // import volume
        if(QString(path.c_str()).endsWith(".xml", Qt::CaseInsensitive))
            volume = vm::VirtualVolumeFactory::createFromXML(vm::VOLUME_INPUT_FORMAT_PLUGIN, path.c_str(), reimport);
        else
            volume = vm::VirtualVolumeFactory::createFromData(vm::VOLUME_INPUT_FORMAT_PLUGIN, path.c_str(), vm::ref_sys(AXS_1,AXS_2,AXS_3),VXL_1,VXL_2,VXL_3, reimport);

        // save updated descriptor
        volume->saveXML("xml_import");

        // everything went OK
        emit sendOperationOutcome(0);
    }
    catch( iim::IOException& exception)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new iom::exception(exception.what()));
    }
    catch( iom::exception& exception)
    {
        /**/tsp::warning(strprintf("exception thrown in CImport::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new iom::exception(exception.what()));
    }
    catch(const char* error)
    {
        /**/tsp::warning(strprintf("exception thrown in CImport::run(): \"%s\"", error).c_str());
        emit sendOperationOutcome(new iom::exception(error));
    }
    catch(...)
    {
        /**/tsp::warning(strprintf("exception thrown in CImport::run(): \"%s\"", "Generic error").c_str());
        emit sendOperationOutcome(new iom::exception("Unable to determine error's type"));
    }
}
