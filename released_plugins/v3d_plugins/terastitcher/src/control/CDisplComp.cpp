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

#include "CDisplComp.h"
#include "CImport.h"
#include "IM_config.h"

using namespace terastitcher;

CDisplComp* CDisplComp::uniqueInstance = NULL;

void CDisplComp::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CDisplComp::~CDisplComp()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CDisplComp destroyed\n", this->thread()->currentThreadId());
    #endif
}

//automatically called when current thread is started
void CDisplComp::run()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CDisplComp::run() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //checking that a volume has been imported first
        vm::VirtualVolume* volume = CImport::instance()->getVolume();
        if(!volume)
            throw MyException("Unable to start this step. A volume must be properly imported first.");

        //launching pairwise displacements computation
        StackStitcher stitcher(volume);
        stitcher.computeDisplacements(algo, row0, col0, row1, col1, Voverlap, Hoverlap, Vrad, Hrad, Drad, subvol_dim, restoreSPIM, 1, 1);

        //saving into XML project file
        volume->saveXML(0,saveproj_path.c_str());

        //everything went OK
        emit sendOperationOutcome(0);
    }
    catch( iim::IOException& exception)
    {
        /**/tsp::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new MyException(exception.what()));
    }
    catch( MyException& exception)
    {
        /**/tsp::warning(strprintf("exception thrown in CDisplComp::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new MyException(exception.what()));
    }
    catch(const char* error)
    {
        /**/tsp::warning(strprintf("exception thrown in CDisplComp::run(): \"%s\"", error).c_str());
        emit sendOperationOutcome(new MyException(error));
    }
    catch(...)
    {
        /**/tsp::warning(strprintf("exception thrown in CDisplComp::run(): \"%s\"", "Generic error").c_str());
        emit sendOperationOutcome(new MyException("Unable to determine error's type"));
    }
}

