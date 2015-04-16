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
#include "StackedVolume.h"
#include "TiledVolume.h"
#include "TiledMCVolume.h"
#include "iomanager.config.h"

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
        uniqueInstance = 0;
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

        // skip nonmatching entries
        QDir dir(path.c_str());
        if( dir.dirName().toStdString().substr(0,3).compare(itm::RESOLUTION_PREFIX) != 0)
            throw RuntimeException(strprintf("\"%s\" is not a valid resolution: the name of the folder does not start with \"%s\"",
                                             path.c_str(), itm::RESOLUTION_PREFIX.c_str() ).c_str());

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
            QDir dir_i(path_i.c_str());

            // skip volumes[0]
            if(dir.dirName() == dir_i.dirName())
                continue;

            // skip nonmatching entries
            if(dir_i.dirName().toStdString().substr(0,3).compare(itm::RESOLUTION_PREFIX) != 0)
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
            delete candidateVols[k];
        /* ------------- sort imported volumes by ascending size ---------------*/
        /**/itm::debug(itm::LEV_MAX, "Sorting volumes by ascending size", __itm__current__function__);
        std::sort(volumes.begin(), volumes.end(), sortVolumesAscendingSize);
        /* ---------------------- check imported volumes -----------------------*/
        if(volumes.size() < 2)
            throw RuntimeException(strprintf("%d resolution found at %s: at least two resolutions are needed for the multiresolution mode",
                                             volumes.size(), qPrintable(curParentDir.path()) ).c_str());
        for(int k=0; k<volumes.size()-1; k++)
        {
            if(volumes[k]->getPrintableFormat().compare( volumes[k+1]->getPrintableFormat() ) != 0)
                throw RuntimeException(strprintf("Volumes have different formats at \"%s\"", qPrintable(curParentDir.absolutePath())).c_str());
            if(volumes[k]->getDIM_T() != volumes[k+1]->getDIM_T())
                throw RuntimeException(strprintf("Volumes have different time frames at \"%s\"", qPrintable(curParentDir.absolutePath())).c_str());
        }



        /**************** 3) GENERATING / LOADING VOLUME 3D MAP *****************
        We generate once for all a volume map from lowest-resolution volume.
        *************************************************************************/
        string volMapPath = curParentDir.path().toStdString() + "/" + VMAP_BIN_FILE_NAME;
        if(hasVolumeMapToBeRegenerated(volMapPath.c_str(), "0.9.42", vmapTDimMax, volumes[0]->getDIM_T()) || reimport || regenerateVMap)
        {
            /**/itm::debug(itm::LEV_MAX, "Entering volume's map generation section", __itm__current__function__);

            // check that the lowest resolution does not exceed the maximum allowed size for the volume map
            VirtualVolume* lowestResVol = volumes[0];
            if(lowestResVol->getDIM_H() > vmapXDimMax ||
               lowestResVol->getDIM_V() > vmapYDimMax ||
               lowestResVol->getDIM_D() > vmapZDimMax)
                itm::warning("@TODO: resample along XYZ so as to match the volume map maximum size", __itm__current__function__);

            vmapXDim = lowestResVol->getDIM_H();
            vmapYDim = lowestResVol->getDIM_V();
            vmapZDim = lowestResVol->getDIM_D();
            vmapCDim = lowestResVol->getDIM_C();
            // if the number of time frames exceeds the maximum, we put only the first vmapTDimMax in the volume map
            vmapTDim = std::min(vmapTDimMax, lowestResVol->getDIM_T());

            // generate volume map
            lowestResVol->setActiveFrames(0, vmapTDimMax -1);
            vmapData = lowestResVol->loadSubvolume_to_UINT8();
            FILE *volMapBin = fopen(volMapPath.c_str(), "wb");
            if(!volMapBin)
                throw RuntimeException(strprintf("Cannot write volume map at \"%s\". Please check your write permissions.", volMapPath.c_str()).c_str());
            uint16 verstr_size = static_cast<uint16>(strlen(itm::version.c_str()) + 1);
            fwrite(&verstr_size, sizeof(uint16), 1, volMapBin);
            fwrite(itm::version.c_str(), verstr_size, 1, volMapBin);
            fwrite(&vmapTDim,  sizeof(uint32), 1, volMapBin);
            fwrite(&vmapCDim,  sizeof(uint32), 1, volMapBin);
            fwrite(&vmapYDim,  sizeof(uint32), 1, volMapBin);
            fwrite(&vmapXDim,  sizeof(uint32), 1, volMapBin);
            fwrite(&vmapZDim,  sizeof(uint32), 1, volMapBin);
            size_t vmapSize = ((size_t)vmapYDim)*vmapXDim*vmapZDim*vmapCDim*vmapTDim;
            fwrite(vmapData, vmapSize, 1, volMapBin);
            fclose(volMapBin);
        }
        else
        {
            /**/itm::debug(itm::LEV_MAX, "Entering volume's map loading section", __itm__current__function__);

            // load volume map
            FILE *volMapBin = fopen(volMapPath.c_str(), "rb");
            uint16 verstr_size;
            if(!volMapBin)
                throw RuntimeException(strprintf("Cannot read volume map at \"%s\". Please check your read permissions.", volMapPath.c_str()).c_str());
            if(!fread(&verstr_size, sizeof(uint16), 1, volMapBin))
                throw RuntimeException("Unable to read volume map file (<version_size> field). Please delete the volume map and re-open the volume.");
            char ver[1024];
            if(!fread(ver, verstr_size, 1, volMapBin))
                throw RuntimeException("Unable to read volume map file (<version> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapTDim, sizeof(uint32), 1, volMapBin) != 1)
                throw RuntimeException("Unable to read volume map file (<vmapTDim> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapCDim, sizeof(uint32), 1, volMapBin) != 1)
                throw RuntimeException("Unable to read volume map file (<vmapCDim> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapYDim, sizeof(uint32), 1, volMapBin) != 1)
                throw RuntimeException("Unable to read volume map file (<vmapYDim> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapXDim,  sizeof(uint32), 1, volMapBin)!= 1)
                throw RuntimeException("Unable to read volume map file (<vmapXDim> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapZDim,  sizeof(uint32), 1, volMapBin)!= 1)
                throw RuntimeException("Unable to read volume map file (<vmapZDim> field). Please delete the volume map and re-open the volume.");
            size_t vmapSize = ((size_t)vmapYDim)*vmapXDim*vmapZDim*vmapCDim*vmapTDim;
            vmapData = new uint8[vmapSize];
            if(fread(vmapData, vmapSize, 1, volMapBin) != 1)
                throw RuntimeException("Unable to read volume map file (<vmapData> field). Please delete the volume map and re-open the volume.");
            fclose(volMapBin);


            //--- Alessandro 29/09/2013: checking that the loaded vmap corresponds to one of the loaded volumes
//            /**/itm::debug(itm::LEV_MAX, "checking that the loaded vmap corresponds to one of the loaded volumes", __itm__current__function__);
//            bool check_passed = false;
//            for(int i=0; i<volumes.size() && !check_passed; i++)
//                if(volumes[i]->getDIM_V() == vmapYDim  &&
//                   volumes[i]->getDIM_H() == vmapXDim  &&
//                   volumes[i]->getDIM_D() == vmapZDim  &&
//                   volumes[i]->getDIM_C() == vmapCDim)
//                    check_passed = true;
//            if(!check_passed)
//                throw RuntimeException(QString("Volume map stored at \"").append(volMapPath.c_str()).append("\" does not correspond to any of the loaded resolutions. Please delete or regenerate the volume map.").toStdString().c_str());

        }

        // 2015-04-15. Alessandro. @FIXED (temporary) incorrect selection of plugin.
        if(!volumes.empty())
        {
            if(dynamic_cast<TiledVolume*>(volumes[0]))
                iom::IMIN_PLUGIN = "tiff3D";
            else if(dynamic_cast<StackedVolume*>(volumes[0]))
                iom::IMIN_PLUGIN = "tiff2D";
        }

        //everything went OK
        emit sendOperationOutcome(0, timerIO.elapsed());

        /**/itm::debug(itm::LEV1, "EOF", __itm__current__function__);
    }
    catch( iim::IOException& exception)  {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch( iom::exception& exception)    {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch( RuntimeException& exception)  {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch(const char* error)             {reset(); emit sendOperationOutcome(new RuntimeException(error));}
    catch(...)                           {reset(); emit sendOperationOutcome(new RuntimeException("Unknown error occurred"));}
}

// returns true if
// 1) the volume map does not exist OR
// 2) it is not compatible with the current version OR
// 3) contains a number of 'T' frames with T < maxDim && T < TDim
bool CImport::hasVolumeMapToBeRegenerated(std::string vmapFilepath,
                                          std::string min_required_version,
                                          int maxTDim, int TDim) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("vmapFilepath = \"%s\", min_required_version = \"%s\", maxTDim = %d, TDim = %d",
                                        vmapFilepath.c_str(), min_required_version.c_str(), maxTDim, TDim).c_str(), __itm__current__function__);

    // open volume map
    FILE* vmapFile = fopen(vmapFilepath.c_str(), "rb");
    if(!vmapFile)
    {
        itm::warning("volume map needs to be (re-)generated: cannot open existing vmap.bin", __itm__current__function__);
        return true;
    }

    // read version
    uint16 verstr_size;
    if(!fread(&verstr_size, sizeof(uint16), 1, vmapFile))
    {
        fclose(vmapFile);
        itm::warning("volume map needs to be (re-)generated: cannot read version from vmap.bin", __itm__current__function__);
        return true;
    }
    char ver[1024];
    if(!fread(ver, verstr_size, 1, vmapFile))
    {
        fclose(vmapFile);
        itm::warning("volume map needs to be (re-)generated: cannot read version from vmap.bin", __itm__current__function__);
        return true;
    }
    int T = 0;
    if(!fread(&T, sizeof(int), 1, vmapFile))
    {
        fclose(vmapFile);
        itm::warning("volume map needs to be (re-)generated: cannot read T dim from vmap.bin", __itm__current__function__);
        return true;
    }
    fclose(vmapFile);


    // check version
    if(!CPlugin::checkPluginVersion(ver, min_required_version))
    {
        itm::warning(itm::strprintf("volume map needs to be (re-)generated: check version failed from vmap.bin (current = \"%s\", required = \"%s\"", ver, min_required_version.c_str()).c_str(), __itm__current__function__);
        return true;
    }

    // check time size: can we load more frames?
    if(T < maxTDim &&  // we can load more frames
       T < TDim)       // there are more frames that can be loaded
    {
        itm::warning("volume map needs to be (re-)generated: check time size failed from vmap.bin", __itm__current__function__);
        return true;
    }

    // check time size: should we load less frames?
    if(T > maxTDim)
    {
        itm::warning("volume map needs to be (re-)generated: mismatch between T from vmap.bin and maxTDim from GUI", __itm__current__function__);
        return true;
    }

    // all checks passed: no need to regenerate volume map
    return false;
}
