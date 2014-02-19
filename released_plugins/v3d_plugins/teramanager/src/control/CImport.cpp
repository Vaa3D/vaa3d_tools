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

#include "../presentation/PDialogImport.h"
#include "../presentation/PMain.h"
#include "../presentation/PLog.h"
#include "CImport.h"
#include "CPlugin.h"
#include <sstream>
#include <limits>
#include <algorithm>
#include "../core/ImageManager/StackedVolume.h"
#include "../core/ImageManager/TiledVolume.h"
#include "../core/ImageManager/TiledMCVolume.h"

using namespace teramanager;
using namespace iim;

CImport* CImport::uniqueInstance = 0;
bool sortVolumesAscendingSize (VirtualVolume* i,VirtualVolume* j) { return (i->getMVoxels() < j->getMVoxels()); }

void CImport::uninstance()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CImport::~CImport()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    for(int k=0; k<volumes.size(); k++)
        if(volumes[k])
            delete volumes[k];

    /**/itm::debug(itm::LEV1, "object successfully DESTROYED", __itm__current__function__);
}

//SET methods
void CImport::setAxes(string axs1, string axs2, string axs3)
{
    /**/itm::debug(itm::LEV1, strprintf("axes = (%s, %s, %s)", axs1.c_str(), axs2.c_str(), axs3.c_str()).c_str(), __itm__current__function__);

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
    /**/itm::debug(itm::LEV1, strprintf("voxels = (%.3f, %.3f, %.3f)", vxl1, vxl2, vxl3).c_str(), __itm__current__function__);

    VXL_1 = vxl1;
    VXL_2 = vxl2;
    VXL_3 = vxl3;
}

//automatically called when current thread is started
void CImport::run()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        timerIO.start();

        /********************* 1) IMPORTING CURRENT VOLUME ***********************
        PRECONDITIONS:
        reimport = true  ==> the volume cannot be directly imported (i.e., w/o the
        additional info provided by the user) or the user explicitly asked for re-
        importing the volume.
        reimport = false ==> the volume is directly importable
        *************************************************************************/
        /**/itm::debug(itm::LEV_MAX, strprintf("importing current volume at \"%s\"", path.c_str()).c_str(), __itm__current__function__);
        if(reimport)
            volumes.push_back(VirtualVolume::instance(path.c_str(), format, AXS_1, AXS_2, AXS_3, VXL_1, VXL_2, VXL_3));
        else
            volumes.push_back(VirtualVolume::instance(path.c_str()));




        /********************* 2) IMPORTING OTHER VOLUMES ***********************
        Importing all the available resolutions within the current volume's
        parent directory.
        *************************************************************************/
        /**/itm::debug(itm::LEV_MAX, "Importing other volumes of the multiresolution octree", __itm__current__function__);
        /* -------------------- detect candidate volumes -----------------------*/
        /**/itm::debug(itm::LEV_MAX, "Detecting volumes that CAN be loaded (let us call them CANDIDATE volumes: the correspondent structures will be destroyed after this step)", __itm__current__function__);
        vector<VirtualVolume*> candidateVols;
        QDir curParentDir(path.c_str());
        curParentDir.cdUp();
        QStringList otherDirs = curParentDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        for(int k=0; k<otherDirs.size(); k++)
        {
            string path_i = curParentDir.absolutePath().append("/").append(otherDirs.at(k).toLocal8Bit().constData()).toStdString();

            // skip volumes[0]
            if(path_i.compare(path) == 0)
                continue;

            /**/itm::debug(itm::LEV_MAX, strprintf("Checking for loadable volume at \"%s\"", path_i.c_str()).c_str(), __itm__current__function__);
            if( !reimport && VirtualVolume::isDirectlyImportable( path_i.c_str()) )
                candidateVols.push_back(VirtualVolume::instance(path_i.c_str()));
            else
                volumes.push_back(VirtualVolume::instance(path_i.c_str(), volumes[0]->getPrintableFormat(),
                                  volumes[0]->getAXS_1(), volumes[0]->getAXS_2(), volumes[0]->getAXS_3(),
                                  volumes[0]->getVXL_1(), volumes[0]->getVXL_2(), volumes[0]->getVXL_3()));
        }
        /* -------------------- import candidate volumes ------------------------*/
        /**/itm::debug(itm::LEV_MAX, "Importing loadable volumes (previously checked)", __itm__current__function__);
        for(int k=0; k<candidateVols.size(); k++)
        {
            int ratio = iim::round(  pow((volumes[0]->getMVoxels() / candidateVols[k]->getMVoxels()),(1/3.0f))  );

             /**/itm::debug(itm::LEV_MAX, strprintf("Importing loadable volume at \"%s\"", candidateVols[k]->getROOT_DIR()).c_str(), __itm__current__function__);
            if( !reimport && VirtualVolume::isDirectlyImportable( candidateVols[k]->getROOT_DIR()) )
                volumes.push_back(VirtualVolume::instance(candidateVols[k]->getROOT_DIR()));
            else
                volumes.push_back(VirtualVolume::instance(candidateVols[k]->getROOT_DIR(),    candidateVols[k]->getPrintableFormat(),
                              volumes[0]->getAXS_1(),       volumes[0]->getAXS_2(),       volumes[0]->getAXS_3(),
                              volumes[0]->getVXL_1()*ratio, volumes[0]->getVXL_2()*ratio, volumes[0]->getVXL_3()*ratio));
        }
        /* -------------------- destroy candidate volumes -----------------------*/
        /**/itm::debug(itm::LEV_MAX, "Destroying candidate volumes", __itm__current__function__);
        for(int k=0; k<candidateVols.size(); k++)
        {
            delete candidateVols[k];
            candidateVols[k] = 0;
        }
        candidateVols.clear();
        /* ------------- sort imported volumes by ascending size ---------------*/
        /**/itm::debug(itm::LEV_MAX, "Sorting volumes by ascending size", __itm__current__function__);
        std::sort(volumes.begin(), volumes.end(), sortVolumesAscendingSize);
        /* ---------------------- check imported volumes -----------------------*/
        if(volumes.size() < 2)
            throw RuntimeException("One resolution found only: at least two resolutions are needed for the multiresolution mode.");
        for(int k=0; k<volumes.size()-1; k++)
            if(volumes[k]->getPrintableFormat().compare( volumes[k+1]->getPrintableFormat() ) != 0)
                throw RuntimeException(strprintf("Volumes have different formats at \"%s\"", qPrintable(curParentDir.absolutePath())).c_str());

        for(int k=0; k<volumes.size(); k++)
            printf("[%d]:\t%s, %.0f MVoxels\n", k, volumes[k]->getROOT_DIR(), volumes[k]->getMVoxels());

        /********************** 3) GENERATING VOLUME 3D MAP ***********************
        It is convenient to generate once for all a volume map from one of the
        lowest-resolution volumes.
        *************************************************************************/
        Image4DSimple* volMapImage = 0;
        {
            //searching for an already existing map, if not available we try to generate it from the lower resolutions
            string volMapPath = path;
            volMapPath.append("/");
            volMapPath.append(VMAP_BIN_FILE_NAME.c_str());
            if(!iim::isFile(volMapPath.c_str()) || reimport || regenerateVMap)
            {
                /**/itm::debug(itm::LEV_MAX, "Entering volume's map generation section", __itm__current__function__);

                //searching for the highest resolution available which size is less then the maximum allowed
                int volMapIndex = -1;
                for(int k=0; k<volumes.size(); k++)
                {
                    if(volumes[k]->getMVoxels() < vmapMaxSize)
                        volMapIndex = k;
                }

                //if found, generating and saving the corresponding map, otherwise throwing an exception
                if(volMapIndex != -1)
                {
                    uint8* vmap_raw = volumes[volMapIndex]->loadSubvolume_to_UINT8(-1, -1, -1, -1, -1, -1, &vmapCDim);
                    vmapYDim = volumes[volMapIndex]->getDIM_V();
                    vmapXDim  = volumes[volMapIndex]->getDIM_H();
                    vmapZDim  = volumes[volMapIndex]->getDIM_D();
                    FILE *volMapBin = fopen(volMapPath.c_str(), "wb");

                    // --- Alessandro 2013-04-23: added exception when file can't be opened in write mode
                    if(!volMapBin)
                        throw RuntimeException(QString("Cannot write volume map at \"").append(volMapPath.c_str()).append("\"\n\nPlease check write permissions on this storage.").toStdString().c_str());

                    uint16 verstr_size = static_cast<uint16>(strlen(CPlugin::getMajorVersion().c_str()) + 1);
                    fwrite(&verstr_size, sizeof(uint16), 1, volMapBin);
                    fwrite(CPlugin::getMajorVersion().c_str(), verstr_size, 1, volMapBin);
                    fwrite(&vmapCDim, sizeof(int), 1, volMapBin);
                    fwrite(&vmapYDim, sizeof(uint32), 1, volMapBin);
                    fwrite(&vmapXDim,  sizeof(uint32), 1, volMapBin);
                    fwrite(&vmapZDim,  sizeof(uint32), 1, volMapBin);
                    fwrite(vmap_raw, vmapYDim*vmapXDim*vmapZDim*vmapCDim, 1, volMapBin);
                    fclose(volMapBin);
                }
                else
                {
                    QString msg = "Can't generate 3D volume map: the volume map size limit is "+
                                  QString::number(vmapMaxSize) + " MVoxels while the lower resolution found has size " +
                                  QString::number(volumes[0]->getMVoxels(), 'f', 0) + " MVoxels";

                    throw RuntimeException(msg.toStdString().c_str());
                }
            }

            //at this point we should have the volume map stored in the volume's directory
            /**/itm::debug(itm::LEV_MAX, "Entering volume's map loading section", __itm__current__function__);
            size_t fread_return_val;
            FILE *volMapBin = fopen(volMapPath.c_str(), "rb");

            // --- Alessandro 2013-04-23: added exception when file can't be opened in read mode
            if(!volMapBin)
                throw RuntimeException(QString("Cannot read volume map at \"").append(volMapPath.c_str()).append("\"\n\nPlease check read permissions on this storage or whether the path is correct.").toStdString().c_str());


            //checking plugin version
            /**/itm::debug(itm::LEV_MAX, "checking plugin version from vmap", __itm__current__function__);
            uint16 verstr_size;
            fread_return_val = fread(&verstr_size, sizeof(uint16), 1, volMapBin);
            if(fread_return_val != 1)
                throw RuntimeException("Unable to read volume map file (<version_size> field). It must be regenerated.");
            char *version = new char[verstr_size];
            fread_return_val = fread(version, verstr_size, 1, volMapBin);
            if(fread_return_val != 1)
                throw RuntimeException("Unable to read volume map file (<version> field). It must be regenerated.");
            if(atof(version) < 0.7f)
                throw RuntimeException(QString("Volume map file was generated with a plugin version"
                                  " (").append(version).append(") older than 0.7. Please check the \"Regenerate volume map\" option from \"File->Options\".").toStdString().c_str());
            delete[] version;

            //loading metadata and data            
            /**/itm::debug(itm::LEV_MAX, "loading metadata and data from vmap", __itm__current__function__);
            fread_return_val = fread(&vmapCDim, sizeof(int), 1, volMapBin);
            if(fread_return_val != 1)
                throw RuntimeException("Unable to read volume map file (<nchannels> field). It must be regenerated.");
            fread_return_val = fread(&vmapYDim, sizeof(uint32), 1, volMapBin);
            if(fread_return_val != 1)
                throw RuntimeException("Unable to read volume map file (<volMapHeight> field). It must be regenerated.");
            fread_return_val = fread(&vmapXDim,  sizeof(uint32), 1, volMapBin);
            if(fread_return_val != 1)
                throw RuntimeException("Unable to read volume map file (<volMapWidth> field). It must be regenerated.");
            fread_return_val = fread(&vmapZDim,  sizeof(uint32), 1, volMapBin);
            if(fread_return_val != 1)
                throw RuntimeException("Unable to read volume map file (<volMapDepth> field). It must be regenerated.");
            vmapData = new uint8[vmapYDim*vmapXDim*vmapZDim*vmapCDim];
            fread_return_val = fread(vmapData, vmapYDim*vmapXDim*vmapZDim*vmapCDim, 1, volMapBin);
            if(fread_return_val != 1)
                throw RuntimeException("Unable to read volume map file (<volMapData> field). It must be regenerated.");
            fclose(volMapBin);
            volMapImage = new Image4DSimple();
            volMapImage->setFileName("VolumeMap");
            volMapImage->setData(vmapData, vmapXDim, vmapYDim, vmapZDim, vmapCDim, V3D_UINT8);

            //--- Alessandro 29/09/2013: checking that the loaded vmap corresponds to one of the loaded volumes            
            /**/itm::debug(itm::LEV_MAX, "checking that the loaded vmap corresponds to one of the loaded volumes", __itm__current__function__);
            bool check_passed = false;
            for(int i=0; i<volumes.size() && !check_passed; i++)
                if(volumes[i]->getDIM_V() == vmapYDim &&
                   volumes[i]->getDIM_H() == vmapXDim  &&
                   volumes[i]->getDIM_D() == vmapZDim  &&
                   volumes[i]->getCHANS() == vmapCDim)
                    check_passed = true;
            if(!check_passed)
                throw RuntimeException(QString("Volume map stored at \"").append(volMapPath.c_str()).append("\" does not correspond to any of the loaded resolutions. Please delete or regenerate the volume map.").toStdString().c_str());

        }

        //everything went OK
        emit sendOperationOutcome(0, volMapImage, timerIO.elapsed());

        /**/itm::debug(itm::LEV1, "EOF", __itm__current__function__);
    }
    catch( iim::IOException& exception)  {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()), 0);}
    catch( RuntimeException& exception)  {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()), 0);}
    catch(const char* error)             {reset(); emit sendOperationOutcome(new RuntimeException(error), 0);}
    catch(...)                           {reset(); emit sendOperationOutcome(new RuntimeException("Unknown error occurred"), 0);}
}
