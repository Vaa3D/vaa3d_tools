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

#include <typeinfo>
#include "CVolume.h"
#include "CImport.h"
#include "../presentation/PLog.h"
#include "../core/ImageManager/TiledVolume.h"
#include "../core/ImageManager/TiledMCVolume.h"

using namespace teramanager;

CVolume* CVolume::uniqueInstance = 0;

void CVolume::uninstance()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

CVolume::~CVolume()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
}

int CVolume::scaleVCoord(int coord, int srcRes, int dstRes) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("coord = %d, srcRes = %d, dstRes = %d", coord, srcRes, dstRes).c_str(), __itm__current__function__);

    //checks
    if(srcRes < 0 || srcRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid source resolution of coordinate mapping operation");
    if(dstRes < 0 || dstRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid destination resolution of coordinate mapping operation");

    //computation
    if(srcRes == dstRes)
        return coord;
    // --- Alessandro added on August 8th, 2013: useful for exact boundary conversion ---
    else if(coord == CImport::instance()->getVolume(srcRes)->getDIM_V())
        return CImport::instance()->getVolume(dstRes)->getDIM_V();
    else
    {
        float ratio = (CImport::instance()->getVolume(dstRes)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(srcRes)->getDIM_V()-1.0f);
        return static_cast<int>(coord*ratio + 0.5f);
    }
}

int CVolume::scaleHCoord(int coord, int srcRes, int dstRes) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("coord = %d, srcRes = %d, dstRes = %d", coord, srcRes, dstRes).c_str(), __itm__current__function__);

    //checks
    if(srcRes < 0 || srcRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid source resolution of coordinate mapping operation");
    if(dstRes < 0 || dstRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid destination resolution of coordinate mapping operation");

    //computation
    if(srcRes == dstRes)
        return coord;
    else if(coord == CImport::instance()->getVolume(srcRes)->getDIM_H())
        return CImport::instance()->getVolume(dstRes)->getDIM_H();
    else
    {
        float ratio = (CImport::instance()->getVolume(dstRes)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(srcRes)->getDIM_H()-1.0f);
        return static_cast<int>(coord*ratio + 0.5f);
    }
}

int CVolume::scaleDCoord(int coord, int srcRes, int dstRes) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("coord = %d, srcRes = %d, dstRes = %d", coord, srcRes, dstRes).c_str(), __itm__current__function__);

    //checks
    if(srcRes < 0 || srcRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid source resolution of coordinate mapping operation");
    if(dstRes < 0 || dstRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid destination resolution of coordinate mapping operation");

    //computation
    if(srcRes == dstRes)
        return coord;
    else if(coord == CImport::instance()->getVolume(srcRes)->getDIM_D())
        return CImport::instance()->getVolume(dstRes)->getDIM_D();
    else
    {
        float ratio = (CImport::instance()->getVolume(dstRes)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(srcRes)->getDIM_D()-1.0f);
        return static_cast<int>(coord*ratio + 0.5f);
    }
}

float CVolume::scaleVCoord(float coord, int srcRes, int dstRes) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("coord = %.3f, srcRes = %d, dstRes = %d", coord, srcRes, dstRes).c_str(), __itm__current__function__);

    //checks
    if(srcRes < 0 || srcRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid source resolution of coordinate mapping operation");
    if(dstRes < 0 || dstRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid destination resolution of coordinate mapping operation");

    //computation
    if(srcRes == dstRes)
        return coord;
    else if(coord == static_cast<float>(CImport::instance()->getVolume(srcRes)->getDIM_V()))
        return static_cast<float>(CImport::instance()->getVolume(dstRes)->getDIM_V());
    else
    {
        float ratio = (CImport::instance()->getVolume(dstRes)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(srcRes)->getDIM_V()-1.0f);
        return coord*ratio;
    }
}

float CVolume::scaleHCoord(float coord, int srcRes, int dstRes) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("coord = %.3f, srcRes = %d, dstRes = %d", coord, srcRes, dstRes).c_str(), __itm__current__function__);

    //checks
    if(srcRes < 0 || srcRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid source resolution of coordinate mapping operation");
    if(dstRes < 0 || dstRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid destination resolution of coordinate mapping operation");

    //computation
    if(srcRes == dstRes)
        return coord;
    else if(coord == static_cast<float>(CImport::instance()->getVolume(srcRes)->getDIM_H()))
        return static_cast<float>(CImport::instance()->getVolume(dstRes)->getDIM_H());
    else
    {
        float ratio = (CImport::instance()->getVolume(dstRes)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(srcRes)->getDIM_H()-1.0f);
        return coord*ratio;
    }
}

float CVolume::scaleDCoord(float coord, int srcRes, int dstRes) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("coord = %.3f, srcRes = %d, dstRes = %d", coord, srcRes, dstRes).c_str(), __itm__current__function__);

    //checks
    if(srcRes < 0 || srcRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid source resolution of coordinate mapping operation");
    if(dstRes < 0 || dstRes >= CImport::instance()->getResolutions())
        throw RuntimeException("Invalid destination resolution of coordinate mapping operation");

    //computation
    if(srcRes == dstRes)
        return coord;
    else if(coord == static_cast<float>(CImport::instance()->getVolume(srcRes)->getDIM_D()))
        return static_cast<float>(CImport::instance()->getVolume(dstRes)->getDIM_D());
    else
    {
        float ratio = (CImport::instance()->getVolume(dstRes)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(srcRes)->getDIM_D()-1.0f);
        return coord*ratio;
    }
}

//automatically called when current thread is started
void CVolume::run()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        finished = false;
        VirtualVolume* volume = CImport::instance()->getVolume(voiResIndex);
        char msg[1024];

        //---- Alessandro 2013-04-17: if VOI exceeds limits it is automatically adjusted. This is very useful in the cases the user is zooming-in
        //around peripheral regions
        voiV0 = voiV0 >=0                   ? voiV0 : 0;
        voiV1 = voiV1 <= volume->getDIM_V() ? voiV1 : volume->getDIM_V();
        voiH0 = voiH0 >=0                   ? voiH0 : 0;
        voiH1 = voiH1 <= volume->getDIM_H() ? voiH1 : volume->getDIM_H();
        voiD0 = voiD0 >=0                   ? voiD0 : 0;
        voiD1 = voiD1 <= volume->getDIM_D() ? voiD1 : volume->getDIM_D();
        voiT0 = voiT0 >=0                   ? voiT0 : 0;
        voiT1 = voiT1 <  volume->getDIM_T() ? voiT1 : volume->getDIM_T()-1;

        //checking subvolume interval
        if(voiV1 - voiV0 <=0 || voiH1 - voiH0 <=0 || voiD1 - voiD0 <=0 || voiT1 - voiT0 <0)
        {
            sprintf(msg, "Invalid subvolume intervals inserted: X=[%d, %d), Y=[%d, %d), Z=[%d, %d), T=[%d, %d]", voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, voiT0, voiT1);
            throw RuntimeException(msg);
        }

        //checking for an imported volume
        if(volume)
        {
            if(streamingSteps == 1)
            {
                QElapsedTimer timerIO;
                timerIO.start();
                volume->setActiveFrames(voiT0, voiT1);
                uint8* voiData = volume->loadSubvolume_to_UINT8(voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);

                qint64 elapsedTime = timerIO.elapsed();
                sprintf(msg, "Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d), T=[%d, %d] loaded from res %d",
                        voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, voiT0, voiT1, voiResIndex);

                CExplorerWindow* destination = dynamic_cast<CExplorerWindow*>(source);
                if(destination)
                {
                    /**/itm::debug(itm::LEV3, "Waiting for updateGraphicsInProgress mutex", __itm__current__function__);
                    /**/ destination->updateGraphicsInProgress.lock();
                    /**/ destination->updateGraphicsInProgress.unlock();
                    /**/itm::debug(itm::LEV3, "Access granted from updateGraphicsInProgress mutex", __itm__current__function__);
                }
                finished = true;
                emit sendOperationOutcome(voiData, 0, source, elapsedTime, msg, 1);
            }
            else
            {
                //checking preconditions
                TiledVolume* vaa3D_volume_RGB = dynamic_cast<TiledVolume*>(volume);
                TiledMCVolume* vaa3D_volume_4D= dynamic_cast<TiledMCVolume*>(volume);
                if(!vaa3D_volume_RGB && !vaa3D_volume_4D)
                    throw RuntimeException("Streaming not yet supported for the current format. Please restart the plugin.");
                if(!buffer)
                    throw RuntimeException("Buffer not initialized");
                CExplorerWindow* destination = dynamic_cast<CExplorerWindow*>(source);
                if(!destination)
                    throw RuntimeException("Streaming not yet supported for this type of destination");

                //reading/writing from/to the same buffer with MUTEX (see Producer-Consumer problem)
                void *stream_descr = 0;
                if(vaa3D_volume_RGB)
                    stream_descr = vaa3D_volume_RGB->streamedLoadSubvolume_open(streamingSteps, buffer, voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
                else
                    stream_descr = vaa3D_volume_4D->streamedLoadSubvolume_open(streamingSteps, buffer, voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
                for (int currentStep = 1; currentStep <= streamingSteps; currentStep++)
                {
                    /**/ bufferMutex.lock();
                    QElapsedTimer timerIO;
                    timerIO.start();
                    if(vaa3D_volume_RGB)
                        buffer = vaa3D_volume_RGB->streamedLoadSubvolume_dostep(stream_descr);
                    else if(vaa3D_volume_4D)
                        buffer = vaa3D_volume_4D->streamedLoadSubvolume_dostep(stream_descr);
                    qint64 elapsedTime = timerIO.elapsed();
                    /**/ bufferMutex.unlock();

                    sprintf(msg, "Streaming %d/%d: Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) loaded from res %d",
                            currentStep, streamingSteps, voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, voiResIndex);

                    /**/ destination->updateGraphicsInProgress.lock();
                    /**/ destination->updateGraphicsInProgress.unlock();


                    finished = currentStep == streamingSteps;

                    emit sendOperationOutcome(buffer, 0, destination, elapsedTime, msg, currentStep);
                }
                if(vaa3D_volume_RGB)
                    buffer = vaa3D_volume_RGB->streamedLoadSubvolume_close(stream_descr);
                else if(vaa3D_volume_4D)
                    buffer = vaa3D_volume_4D->streamedLoadSubvolume_close(stream_descr);
                delete[] buffer;
                buffer = 0;
            }
        }
        else
            throw RuntimeException("No volume has been imported yet.");

        /**/itm::debug(itm::LEV1, "EOF", __itm__current__function__);
    }
    catch( iim::IOException& exception)  {reset(); emit sendOperationOutcome(0, new RuntimeException(exception.what()), source);}
    catch( RuntimeException& exception)  {emit sendOperationOutcome(0, new RuntimeException(exception.what()), source);}
    catch(const char* error)        {emit sendOperationOutcome(0, new RuntimeException(error), source);}
    catch(...)                      {emit sendOperationOutcome(0, new RuntimeException("Unknown error occurred"), source);}
}

