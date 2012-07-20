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
#include "presentation/PDialogImport.h"
#include "presentation/PMain.h"
#include <sstream>
#include <limits>

using namespace teramanager;

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
    AXS_1 = axis(atoi(axs1.c_str()));
    AXS_2 = axis(atoi(axs2.c_str()));
    AXS_3 = axis(atoi(axs3.c_str()));
}
void CImport::setVoxels(std::string vxl1, std::string vxl2, std::string vxl3)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CImport setVoxels(%s, %s, %s) launched\n", this->thread()->currentThreadId(), vxl1.c_str(), vxl2.c_str(), vxl3.c_str());
    #endif

    std::istringstream tmp1(vxl1), tmp2(vxl2), tmp3(vxl3);
    tmp1.imbue(std::locale("C"));
    tmp2.imbue(std::locale("C"));
    tmp3.imbue(std::locale("C"));
    tmp1 >> VXL_1;
    tmp2 >> VXL_2;
    tmp3 >> VXL_3;
}

//automatically called when current thread is started
void CImport::run()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CImport::run() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //if metadata binary file doesn't exist or the volume has to be re-imported, further informations must be provided to the constructor
        string mdata_fpath = path;
        mdata_fpath.append("/");
        mdata_fpath.append(IM_METADATA_FILE_NAME);
        if(!StackedVolume::fileExists(mdata_fpath.c_str()) || reimport)
        {
            //checking current members validity
            if(AXS_1 != axis_invalid && AXS_2 != axis_invalid && AXS_3 != axis_invalid && VXL_1 != 0 && VXL_2 != 0 && VXL_3 != 0)
                volume = new StackedVolume(path.c_str(), ref_sys(AXS_1,AXS_2,AXS_3),VXL_1,VXL_2,VXL_3, reimport);
            else
            {
                char errMsg[IM_STATIC_STRINGS_SIZE];
                sprintf(errMsg, "in CImport::run(): invalid parameters AXS_1(%s), AXS_2(%s), AXS_3(%s), VXL_1(%.4f), VXL_2(%.4f), VXL_3(%.4f)",
                        axis_to_str(AXS_1), axis_to_str(AXS_2), axis_to_str(AXS_3), VXL_1, VXL_2, VXL_3);
                throw MyException(errMsg);
            }
        }
        else
            volume = new StackedVolume(path.c_str(), ref_sys(axis_invalid,axis_invalid,axis_invalid),0,0,0);

        //if "Generate and show 3D volume map" checkbox has been checked
        vmap_data = 0;
        Image4DSimple* vmap_image = 0;
        if(genmap)
        {
            //searching for an already existing map, if not available we try to generate it from the lower resolutions
            string vmap_fpath = path;
            vmap_fpath.append("/");
            vmap_fpath.append(TMP_VMAP_FNAME);
            if(!StackedVolume::fileExists(vmap_fpath.c_str()))
            {
                //searching for the highest resolution available which size is less then the maximum allowed ("good for map")
                QDir resdir(path.c_str());
                resdir.cdUp();
                QStringList resolutions_dirs = resdir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Size);
                int res_good4map = -1;
                float res_good4map_highest = -std::numeric_limits<float>::max();
                for(int k=0; k<resolutions_dirs.size(); k++)
                {
                    int width=0, height=0, depth=0;
                    int scanres = sscanf(resolutions_dirs.at(k).toLocal8Bit().constData(), "RES(%dx%dx%d)", &height, &width, &depth);
                    float res_size = (width/1024.0f)*(height/1024.0f)*depth;
                    if(scanres == 3 && res_size < TMP_VMAP_MAXSIZE && res_size > res_good4map_highest)
                    {
                        res_good4map_highest = res_size;
                        res_good4map = k;
                    }
                }

                //if one "good for map" resolution is available, generating and saving the corresponding map, otherwise throwing an exception
                if(res_good4map != -1)
                {
                    StackedVolume vol_good4map(resdir.absolutePath().append("/").append(resolutions_dirs.at(res_good4map).toLocal8Bit().constData()).toStdString().c_str(),
                                               ref_sys(AXS_1,AXS_2,AXS_3),VXL_1,VXL_2,VXL_3, false, false);
                    uint8* vmap_raw = vol_good4map.loadSubvolume_to_UINT8();
                    int vmap_height = vol_good4map.getDIM_V();
                    int vmap_width  = vol_good4map.getDIM_H();
                    int vmap_depth  = vol_good4map.getDIM_D();
                    FILE *vmap_bin = fopen(vmap_fpath.c_str(), "wb");
                    fwrite(&vmap_height, sizeof(uint32), 1, vmap_bin);
                    fwrite(&vmap_width,  sizeof(uint32), 1, vmap_bin);
                    fwrite(&vmap_depth,  sizeof(uint32), 1, vmap_bin);
                    fwrite(vmap_raw, vmap_height*vmap_width*vmap_depth, 1, vmap_bin);
                    fclose(vmap_bin);
                }
                else
                    throw MyException(QString("Can't generate 3D volume map: a resolution of at most ").
                                      append(QString::number(TMP_VMAP_MAXSIZE)).
                                      append(" MVoxels must be available in the volume's parent directory\n").toStdString().c_str());
            }

            //at this point we should have the volume map stored in the volume's directory
            FILE *vmap_bin = fopen(vmap_fpath.c_str(), "rb");
            fread(&vmap_height, sizeof(uint32), 1, vmap_bin);
            fread(&vmap_width,  sizeof(uint32), 1, vmap_bin);
            fread(&vmap_depth,  sizeof(uint32), 1, vmap_bin);
            vmap_data = new uint8[vmap_height * vmap_width * vmap_depth];
            fread(vmap_data, vmap_height*vmap_width*vmap_depth, 1, vmap_bin);
            fclose(vmap_bin);
            vmap_image = new Image4DSimple();
            vmap_image->setFileName("Volume map");
            vmap_image->setData(vmap_data, vmap_width, vmap_height, vmap_depth, 1, V3D_UINT8);
        }

        //everything went OK
        emit sendOperationOutcome(0, vmap_image);
    }
    catch( MyException& exception)  {emit sendOperationOutcome(&exception, 0);}
    catch(const char* error)        {emit sendOperationOutcome(new MyException(error), 0);}
    catch(...)                      {emit sendOperationOutcome(new MyException("Unknown error occurred"), 0);}
}
