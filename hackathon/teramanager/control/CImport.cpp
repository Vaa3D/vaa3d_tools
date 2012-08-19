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
#include <algorithm>

using namespace teramanager;

CImport* CImport::uniqueInstance = NULL;
bool sortVolumesDescendingSize (StackedVolume* i,StackedVolume* j) { return (i->getMVoxels() >= j->getMVoxels()); }

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

    for(int k=0; k<volumes.size(); k++)
        if(volumes[k])
            delete volumes[k];
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
        /********************* 1) IMPORTING CURRENT VOLUME ***********************
        If metadata binary file doesn't exist or the volume has to be re-imported,
        further informations must be provided to the constructor.
        *************************************************************************/
        string mdata_fpath = path;
        mdata_fpath.append("/");
        mdata_fpath.append(IM_METADATA_FILE_NAME);
        if(!StackedVolume::fileExists(mdata_fpath.c_str()) || reimport)
        {
            //checking current members validity
            if(AXS_1 != axis_invalid && AXS_2 != axis_invalid && AXS_3 != axis_invalid && VXL_1 != 0 && VXL_2 != 0 && VXL_3 != 0)
                volumes.push_back(new StackedVolume(path.c_str(), ref_sys(AXS_1,AXS_2,AXS_3),VXL_1,VXL_2,VXL_3, reimport));
            else
            {
                char errMsg[IM_STATIC_STRINGS_SIZE];
                sprintf(errMsg, "in CImport::run(): invalid parameters AXS_1(%s), AXS_2(%s), AXS_3(%s), VXL_1(%.4f), VXL_2(%.4f), VXL_3(%.4f)",
                        axis_to_str(AXS_1), axis_to_str(AXS_2), axis_to_str(AXS_3), VXL_1, VXL_2, VXL_3);
                throw MyException(errMsg);
            }
        }
        else
            volumes.push_back(new StackedVolume(path.c_str(), ref_sys(axis_invalid,axis_invalid,axis_invalid),0,0,0));

        /********************* 2) IMPORTING OTHER VOLUMES ***********************
        If multiresolution mode is enabled, importing all the available resolutions
        within the current volume's parent directory.
        *************************************************************************/
        if(multiresMode)
        {
            //detecting candidate volumes
            vector<StackedVolume*> candidateVols;
            QDir curParentDir(path.c_str());
            curParentDir.cdUp();
            QStringList otherDirs = curParentDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
            for(int k=0; k<otherDirs.size(); k++)
            {
                try
                {
                    StackedVolume* candidate_vol = new StackedVolume(curParentDir.absolutePath().append("/").append(otherDirs.at(k).toLocal8Bit().constData()).toStdString().c_str(),
                                                                     ref_sys(volumes[0]->getAXS_1(),volumes[0]->getAXS_2(),volumes[0]->getAXS_3()),
                                                                     volumes[0]->getVXL_1(),volumes[0]->getVXL_2(),volumes[0]->getVXL_3(), false, false);
                    candidateVols.push_back(candidate_vol);
                }
                catch(...){}
            }

            //importing candidate volumes
            for(int k=0; k<candidateVols.size(); k++)
            {
                //current volume (now stored in volumes[0]) should be discarded
                if(candidateVols[k]->getMVoxels() != volumes[0]->getMVoxels())
                {
                    int ratio = pow((volumes[0]->getMVoxels() / candidateVols[k]->getMVoxels()),(1/3.0f)) + 0.5;
                    volumes.push_back(new StackedVolume(candidateVols[k]->getSTACKS_DIR(),
                                                        ref_sys(volumes[0]->getAXS_1(),volumes[0]->getAXS_2(),volumes[0]->getAXS_3()),
                                                        volumes[0]->getVXL_1()*ratio,volumes[0]->getVXL_2()*ratio,volumes[0]->getVXL_3()*ratio, reimport));
                    delete candidateVols[k];
                }
            }

            //sorting volumes by descending size
            std::sort(volumes.begin(), volumes.end(), sortVolumesDescendingSize);
        }

        /********************** 3) GENERATING VOLUME 3D MAP ***********************
        If multiresolution mode is enabled, it could be convenient to generate once
        for all a volume map from a low-resolution volume.
        *************************************************************************/
        Image4DSimple* volMapImage = 0;
        if(multiresMode)
        {
            //searching for an already existing map, if not available we try to generate it from the lower resolutions
            string volMapPath = path;
            volMapPath.append("/");
            volMapPath.append(TMP_VMAP_FNAME);
            if(!StackedVolume::fileExists(volMapPath.c_str()) || reimport || regenerateVolMap)
            {
                //searching for the highest resolution available which size is less then the maximum allowed
                int volMapIndex = -1;
                for(int k=0; k<volumes.size(); k++)
                {
                    if(volumes[k]->getMVoxels() < volMapMaxSize)
                        volMapIndex = k;
                }

                //if found, generating and saving the corresponding map, otherwise throwing an exception
                if(volMapIndex != -1)
                {
                    uint8* vmap_raw = volumes[volMapIndex]->loadSubvolume_to_UINT8();
                    volMapHeight = volumes[volMapIndex]->getDIM_V();
                    volMapWidth  = volumes[volMapIndex]->getDIM_H();
                    volMapDepth  = volumes[volMapIndex]->getDIM_D();
                    FILE *volMapBin = fopen(volMapPath.c_str(), "wb");
                    fwrite(&volMapHeight, sizeof(uint32), 1, volMapBin);
                    fwrite(&volMapWidth,  sizeof(uint32), 1, volMapBin);
                    fwrite(&volMapDepth,  sizeof(uint32), 1, volMapBin);
                    fwrite(vmap_raw, volMapHeight*volMapWidth*volMapDepth, 1, volMapBin);
                    fclose(volMapBin);
                }
                else
                    throw MyException(QString("Can't generate 3D volume map: a resolution of at most ").
                                      append(QString::number(volMapMaxSize)).
                                      append(" MVoxels must be available in the volume's parent directory\n").toStdString().c_str());
            }

            //at this point we should have the volume map stored in the volume's directory
            FILE *volMapBin = fopen(volMapPath.c_str(), "rb");
            fread(&volMapHeight, sizeof(uint32), 1, volMapBin);
            fread(&volMapWidth,  sizeof(uint32), 1, volMapBin);
            fread(&volMapDepth,  sizeof(uint32), 1, volMapBin);
            volMapData = new uint8[volMapHeight*volMapWidth*volMapDepth];
            fread(volMapData, volMapHeight*volMapWidth*volMapDepth, 1, volMapBin);
            fclose(volMapBin);
            volMapImage = new Image4DSimple();
            volMapImage->setFileName("VolumeMap");
            volMapImage->setData(volMapData, volMapWidth, volMapHeight, volMapDepth, 1, V3D_UINT8);
        }

        //everything went OK
        emit sendOperationOutcome(0, volMapImage);
    }
    catch( MyException& exception)  {emit sendOperationOutcome(&exception, 0);}
    catch(const char* error)        {emit sendOperationOutcome(new MyException(error), 0);}
    catch(...)                      {emit sendOperationOutcome(new MyException("Unknown error occurred"), 0);}
}
