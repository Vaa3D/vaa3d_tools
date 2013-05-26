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

using namespace terastitcher;

CImport* CImport::uniqueInstance = NULL;

void CImport::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
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

    if(     axs1.compare("Y")==0)
        AXS_1 = axis(1);
    else if(axs1.compare("-Y")==0)
        AXS_1 = axis(-1);
    else if(axs1.compare("X")==0)
        AXS_1 = axis(2);
    else if(axs1.compare("-X")==0)
        AXS_1 = axis(-2);
    else if(axs1.compare("Z")==0)
        AXS_1 = axis(3);
    else if(axs1.compare("-Z")==0)
        AXS_1 = axis(-3);

    if(     axs2.compare("Y")==0)
        AXS_2 = axis(1);
    else if(axs2.compare("-Y")==0)
        AXS_2 = axis(-1);
    else if(axs2.compare("X")==0)
        AXS_2 = axis(2);
    else if(axs2.compare("-X")==0)
        AXS_2 = axis(-2);
    else if(axs2.compare("Z")==0)
        AXS_2 = axis(3);
    else if(axs2.compare("-Z")==0)
        AXS_2 = axis(-3);

    if(     axs3.compare("Y")==0)
        AXS_3 = axis(1);
    else if(axs3.compare("-Y")==0)
        AXS_3 = axis(-1);
    else if(axs3.compare("X")==0)
        AXS_3 = axis(2);
    else if(axs3.compare("-X")==0)
        AXS_3 = axis(-2);
    else if(axs3.compare("Z")==0)
        AXS_3 = axis(3);
    else if(axs3.compare("-Z")==0)
        AXS_3 = axis(-3);
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
        //if a volume's directory path has been provided, searching for metadata file
        if(path.find(".xml")==std::string::npos && path.find(".XML")==std::string::npos)
        {
            string mdata_fpath = path;
            mdata_fpath.append("/");
            mdata_fpath.append(VM_BIN_METADATA_FILE_NAME);

             //if metadata binary file doesn't exist or the volume has to be re-imported, further informations must be provided to the constructor
            if(!StackedVolume::fileExists(mdata_fpath.c_str()) || reimport)
            {
                //checking current members validity
                if(AXS_1 != axis_invalid && AXS_2 != axis_invalid && AXS_3 != axis_invalid && VXL_1 != 0 && VXL_2 != 0 && VXL_3 != 0)
                    volume = new StackedVolume(path.c_str(), ref_sys(AXS_1,AXS_2,AXS_3),VXL_1,VXL_2,VXL_3, reimport);
                else
                {
                    char errMsg[VM_STATIC_STRINGS_SIZE];
                    sprintf(errMsg, "in CImport::run(): invalid parameters AXS_1(%s), AXS_2(%s), AXS_3(%s), VXL_1(%.4f), VXL_2(%.4f), VXL_3(%.4f)",
                            axis_to_str(AXS_1), axis_to_str(AXS_2), axis_to_str(AXS_3), VXL_1, VXL_2, VXL_3);
                    throw MyException(errMsg);
                }
            }
            else
                volume = new StackedVolume(path.c_str(), ref_sys(axis_invalid,axis_invalid,axis_invalid),0,0,0);
        }
        else
            volume = new StackedVolume(path.c_str());

        //everything went OK
        emit sendOperationOutcome(0);
    }
    catch( MyException& exception)  {emit sendOperationOutcome(&exception);}
    catch(const char* error)        {emit sendOperationOutcome(new MyException(error));}
    //catch(...)                      {emit sendOperationOutcome(new MyException("Unable to determine error's type"));}
}
