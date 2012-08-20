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

#include "CVolume.h"
#include "CImport.h"

using namespace teramanager;

CVolume* CVolume::uniqueInstance = NULL;

void CVolume::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CVolume::~CVolume()
{
    #ifdef TMP_DEBUG
    printf("teramanager plugin [thread %d] >> CVolume destroyed\n", this->thread()->currentThreadId());
    #endif
}

//automatically called when current thread is started
void CVolume::run()
{
    #ifdef TMP_DEBUG
    printf("teramanager plugin [thread %d] >> CVolume::run() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        StackedVolume* volume = CImport::instance()->getVolume(voiResIndex);

        //checking subvolume interval
        if(voiV1 - voiV0 <=0 || voiH1 - voiH0 <=0 || voiD1 - voiD0 <=0)
            throw MyException("Invalid subvolume intervals inserted.");

        //checking for an imported volume
        if(volume)
            voiData = volume->loadSubvolume_to_UINT8(voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
        else
            throw MyException("No volume has been imported yet.");

        //everything went OK
        emit sendOperationOutcome(0, sourceObject);
    }
    catch( MyException& exception)  {emit sendOperationOutcome(&exception, sourceObject);}
    catch(const char* error)        {emit sendOperationOutcome(new MyException(error), sourceObject);}
    catch(...)                      {emit sendOperationOutcome(new MyException("Unknown error occurred"), sourceObject);}
}

