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
#include "../core/imagemanager/TiledVolume.h"
#include "../core/imagemanager/TiledMCVolume.h"

using namespace teramanager;
using namespace iim;

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

//automatically called when current thread is started
void CVolume::run()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        VirtualVolume* volume = CImport::instance()->getVolume(voiResIndex);

        //---- Alessandro 2013-04-17: if VOI exceeds limits it is automatically adjusted. This is very useful in the cases the user is zooming-in
        //around peripheral regions
        voiV0 = voiV0 >=0                   ? voiV0 : 0;
        voiV1 = voiV1 <= volume->getDIM_V() ? voiV1 : volume->getDIM_V();
        voiH0 = voiH0 >=0                   ? voiH0 : 0;
        voiH1 = voiH1 <= volume->getDIM_H() ? voiH1 : volume->getDIM_H();
        voiD0 = voiD0 >=0                   ? voiD0 : 0;
        voiD1 = voiD1 <= volume->getDIM_D() ? voiD1 : volume->getDIM_D();
//        voiT0 = voiT0 >=0                   ? voiT0 : 0;
//        voiT1 = voiT1 <  volume->getDIM_T() ? voiT1 : volume->getDIM_T()-1;

        // check subvolume interval
        if(voiV1 - voiV0 <=0 || voiH1 - voiH0 <=0 || voiD1 - voiD0 <=0 || voiT1 - voiT0 <0)           
            throw RuntimeException(strprintf("Invalid subvolume intervals inserted: X=[%d, %d), Y=[%d, %d), Z=[%d, %d), T=[%d, %d]",
                                             voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, voiT0, voiT1));

        // check destination
        /**/itm::debug(itm::LEV2, "Check destination", __itm__current__function__);
        CExplorerWindow* destination = dynamic_cast<CExplorerWindow*>(source);
        if(!destination)
            throw RuntimeException("Destination type not supported");
        /**/itm::debug(itm::LEV2, strprintf("destination registered as \"%s\"", destination->title.c_str()).c_str(), __itm__current__function__);

        //checking for an imported volume
        if(volume)
        {
            if(streamingSteps == 1)
            {
                // 5D data with instant visualization of selected frame
                if(voiT0 != voiT1 && cur_t != -1)
                {
                    // load selected frame
                    QElapsedTimer timerIO;
                    timerIO.start();
                    volume->setActiveFrames(cur_t, cur_t);
                    /**/itm::debug(itm::LEV3, "load selected time frame", __itm__current__function__);
                    uint8* voiData = volume->loadSubvolume_to_UINT8(voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
                    qint64 elapsedTime = timerIO.elapsed();


                    // wait for GUI thread to update graphics
                    /**/itm::debug(itm::LEV3, "Waiting for updateGraphicsInProgress mutex", __itm__current__function__);
                    /**/ updateGraphicsInProgress.lock();
                    /**/itm::debug(itm::LEV3, "Access granted from updateGraphicsInProgress mutex", __itm__current__function__);


                    // send data
                    integer_array data_s = make_vector<int>() << voiH0        << voiV0        << voiD0        << 0                            << cur_t;
                    integer_array data_c = make_vector<int>() << voiH1-voiH0  << voiV1-voiV0  << voiD1-voiD0  << volume->getNACtiveChannels() << 1;
                    emit sendData(voiData, data_s, data_c, source, false, 0, elapsedTime,
                                strprintf("Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d), T=[%d, %d] loaded from res %d",
                                voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, cur_t, cur_t, voiResIndex).c_str());

                    // unlock updateGraphicsInProgress mutex
                    /**/itm::debug(itm::LEV3, strprintf("updateGraphicsInProgress.unlock()").c_str(), __itm__current__function__);
                    /**/ updateGraphicsInProgress.unlock();
                }
                {
                    // load data
                    QElapsedTimer timerIO;
                    timerIO.start();
                    volume->setActiveFrames(voiT0, voiT1);
                    /**/itm::debug(itm::LEV3, "load data", __itm__current__function__);
                    uint8* voiData = volume->loadSubvolume_to_UINT8(voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
                    qint64 elapsedTime = timerIO.elapsed();


                    // wait for GUI thread to update graphics
                    /**/itm::debug(itm::LEV3, "Waiting for updateGraphicsInProgress mutex", __itm__current__function__);
                    /**/ updateGraphicsInProgress.lock();
                    /**/itm::debug(itm::LEV3, "Access granted from updateGraphicsInProgress mutex", __itm__current__function__);


                    // send data
                    integer_array data_s = make_vector<int>() << voiH0        << voiV0        << voiD0        << 0                            << voiT0;
                    integer_array data_c = make_vector<int>() << voiH1-voiH0  << voiV1-voiV0  << voiD1-voiD0  << volume->getNACtiveChannels() << voiT1-voiT0+1;
                    emit sendData(voiData, data_s, data_c, source, true, 0, elapsedTime,
                                strprintf("Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d), T=[%d, %d] loaded from res %d",
                                voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, voiT0, voiT1, voiResIndex).c_str());
                    /**/itm::debug(itm::LEV3, "sendData signal emitted", __itm__current__function__);

                    // unlock updateGraphicsInProgress mutex
                    /**/itm::debug(itm::LEV3, strprintf("updateGraphicsInProgress.unlock()").c_str(), __itm__current__function__);
                    /**/ updateGraphicsInProgress.unlock();
                }
            }
            else
            {
                throw RuntimeException("Streaming has been temporarily disabled. Please contact the developer.");
//                    // precondition checks
//                    if(!buffer)
//                        throw RuntimeException("Buffer not initialized");
//                    CExplorerWindow* destination = dynamic_cast<CExplorerWindow*>(source);
//                    if(!destination)
//                        throw RuntimeException("Streaming not yet supported for this type of destination");
//                    if(streamingSteps != 2)
//                        throw RuntimeException("Only streaming steps = 2 supported for 5D data");

//                    for (int step = 1; step <= streamingSteps; step++)
//                    {
//                        // load current selected frame
//                        /**/itm::debug(itm::LEV3, strprintf("Time step %d/2: loading data", step).c_str(), __itm__current__function__);
//                        volume->setActiveFrames(step == 1 ? cur_t : voiT0, step == 1 ? cur_t : voiT1);
//                        QElapsedTimer timerIO;
//                        timerIO.start();
//                        uint8* data = volume->loadSubvolume_to_UINT8(voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
//                        qint64 elapsedTime = timerIO.elapsed();

//                        // copy frame to buffer ( ** CRITICAL SECTION ** )
//                        /**/itm::debug(itm::LEV3, strprintf("Time step %d/2: waiting for buffer mutex", step).c_str(), __itm__current__function__);
//                        /**/ bufferMutex.lock();
//                        /**/itm::debug(itm::LEV3, strprintf("Time step %d/2: access granted on buffer mutex", step).c_str(), __itm__current__function__);
//                        uint32 buf_dims[5]      = {voiH1-voiH0, voiV1-voiV0, voiD1-voiD0, volume->getDIM_C(), voiT1 - voiT0 +1};
//                        uint32 buf_offset[5]    = {0,           0,           0,           0,                            cur_t-voiT0};
//                        uint32 cur_t_dims[5]    = {voiH1-voiH0, voiV1-voiV0, voiD1-voiD0, volume->getDIM_C(), 1};
//                        uint32 cur_t_offset[5]  = {0,           0,           0,           0,                            0};
//                        uint32 cur_t_count[5]   = {voiH1-voiH0, voiV1-voiV0, voiD1-voiD0, volume->getDIM_C(), 1};
//                        CExplorerWindow::copyVOI(cur_t_data, cur_t_dims, cur_t_offset, cur_t_count, buffer, buf_dims, buf_offset);
//                        delete[] cur_t_data;
//                        /**/ bufferMutex.unlock();
//                        /**/itm::debug(itm::LEV3, strprintf("Time step %d/2: unlocked buffer mutex", step).c_str(), __itm__current__function__);


//                        // wait GUI thread to complete update graphics
//                        /**/itm::debug(itm::LEV3, strprintf("Time step %d/2: waiting for updateGraphicsInProgress mutex", step).c_str(), __itm__current__function__);
//                        /**/ destination->updateGraphicsInProgress.lock();
//                        /**/ destination->updateGraphicsInProgress.unlock();
//                        /**/itm::debug(itm::LEV3, strprintf("Time step %d/2: unlocked updateGraphicsInProgress mutex", step).c_str(), __itm__current__function__);

//                        // send data to GUI thread
//                        finished = false;
//                        /**/itm::debug(itm::LEV3, strprintf("Time step %d/2: sendOperationOutcome", step).c_str(), __itm__current__function__);
//                        sprintf(msg, "Streaming %d/%d: Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) loaded from res %d",
//                                1, 2, voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, voiResIndex);
//                        emit sendOperationOutcome(buffer, 0, destination, elapsedTime, msg, 1);
//                    }









//                    // load current selected frame
//                    /**/itm::debug(itm::LEV3, "Time step 1/2: loading data", __itm__current__function__);
//                    volume->setActiveFrames(cur_t, cur_t);
//                    QElapsedTimer timerIO;
//                    timerIO.start();
//                    uint8* cur_t_data = volume->loadSubvolume_to_UINT8(voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
//                    qint64 elapsedTime = timerIO.elapsed();


//                    /**/itm::debug(itm::LEV3, "Time step 2/2: waiting for buffer mutex", __itm__current__function__);
//                    /**/ bufferMutex.lock();
//                    /**/itm::debug(itm::LEV3, "Time step 2/2: access granted on buffer mutex", __itm__current__function__);
//                    timerIO.start();
//                    volume->setActiveFrames(voiT0 + (voiT1-voiT0)/2, voiT1);
//                    /**/itm::debug(itm::LEV3, "Time step 2/2: loading data", __itm__current__function__);
//                    buffer = volume->loadSubvolume_to_UINT8(voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
//                    elapsedTime = timerIO.elapsed();
//                    /**/ bufferMutex.unlock();
//                    /**/itm::debug(itm::LEV3, "Time step 2/2: unlocked buffer mutex", __itm__current__function__);

//                    sprintf(msg, "Streaming %d/%d: Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) loaded from res %d",
//                            2, 2, voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, voiResIndex);

//                    /**/itm::debug(itm::LEV3, "Time step 1/2: waiting for updateGraphicsInProgress mutex", __itm__current__function__);
//                    /**/ destination->updateGraphicsInProgress.lock();
//                    /**/ destination->updateGraphicsInProgress.unlock();
//                    /**/itm::debug(itm::LEV3, "Time step 1/2: unlocked updateGraphicsInProgress mutex", __itm__current__function__);

//                    finished = true;
//                    /**/itm::debug(itm::LEV3, "Time step 1/2: sendOperationOutcome", __itm__current__function__);
//                    emit sendOperationOutcome(buffer, 0, destination, elapsedTime, msg, 2);


//            //checking preconditions
//            TiledVolume* vaa3D_volume_RGB = dynamic_cast<TiledVolume*>(volume);
//            TiledMCVolume* vaa3D_volume_4D= dynamic_cast<TiledMCVolume*>(volume);
//            if(!vaa3D_volume_RGB && !vaa3D_volume_4D)
//                throw RuntimeException("Streaming not yet supported for the current format. Please restart the plugin.");
//            if(!buffer)
//                throw RuntimeException("Buffer not initialized");
//            CExplorerWindow* destination = dynamic_cast<CExplorerWindow*>(source);
//            if(!destination)
//                throw RuntimeException("Streaming not yet supported for this type of destination");

//            //reading/writing from/to the same buffer with MUTEX (see Producer-Consumer problem)
//            /**/itm::debug(itm::LEV3, "Calling streamedLoadSubvolume_open", __itm__current__function__);
//            void *stream_descr = 0;
//            if(vaa3D_volume_RGB)
//                stream_descr = vaa3D_volume_RGB->streamedLoadSubvolume_open(streamingSteps, buffer, voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
//            else
//                stream_descr = vaa3D_volume_4D->streamedLoadSubvolume_open(streamingSteps, buffer, voiV0, voiV1, voiH0, voiH1, voiD0, voiD1);
//            for (int currentStep = 1; currentStep <= streamingSteps; currentStep++)
//            {

//                /**/itm::debug(itm::LEV3, "Waiting for buffer mutex", __itm__current__function__);
//                /**/ bufferMutex.lock();
//                /**/itm::debug(itm::LEV3, "Access granted to buffer mutex, locking", __itm__current__function__);
//                QElapsedTimer timerIO;
//                timerIO.start();
//                if(vaa3D_volume_RGB)
//                    buffer = vaa3D_volume_RGB->streamedLoadSubvolume_dostep(stream_descr);
//                else if(vaa3D_volume_4D)
//                    buffer = vaa3D_volume_4D->streamedLoadSubvolume_dostep(stream_descr);
//                qint64 elapsedTime = timerIO.elapsed();
//                /**/ bufferMutex.unlock();
//                /**/itm::debug(itm::LEV3, "Unlocked buffer mutex", __itm__current__function__);

//                sprintf(msg, "Streaming %d/%d: Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) loaded from res %d",
//                        currentStep, streamingSteps, voiH0, voiH1, voiV0, voiV1, voiD0, voiD1, voiResIndex);

//                /**/itm::debug(itm::LEV3, "Waiting for updateGraphicsInProgress mutex", __itm__current__function__);
//                /**/ destination->updateGraphicsInProgress.lock();
//                /**/ destination->updateGraphicsInProgress.unlock();
//                /**/itm::debug(itm::LEV3, "Unlocked updateGraphicsInProgress mutex", __itm__current__function__);


//                finished = currentStep == streamingSteps;

//                if(finished)
//                {
//                    /**/itm::debug(itm::LEV3, "Calling streamedLoadSubvolume_close", __itm__current__function__);
//                    if(vaa3D_volume_RGB)
//                        buffer = vaa3D_volume_RGB->streamedLoadSubvolume_close(stream_descr);
//                    else if(vaa3D_volume_4D)
//                        buffer = vaa3D_volume_4D->streamedLoadSubvolume_close(stream_descr);
//                }

//                /**/itm::debug(itm::LEV3, strprintf("sendOperationOutcome, step %d", currentStep).c_str(), __itm__current__function__);
//                emit sendOperationOutcome(buffer, 0, destination, elapsedTime, msg, currentStep);
            }
        }
        else
            throw RuntimeException("No volume has been imported yet.");

        /**/itm::debug(itm::LEV1, "EOF", __itm__current__function__);
    }
    catch( iim::IOException& exception)
    {
        // before emit signal, it is necessary to wait for updateGraphicsInProgress mutex
        CExplorerWindow* dest = dynamic_cast<CExplorerWindow*>(source);
        /**/ updateGraphicsInProgress.lock();
        reset();
        //bufferMutex.unlock();
        itm::warning(exception.what(), "CVolume");
        /**/ updateGraphicsInProgress.unlock();

        emit sendData(0, make_vector<int>(), make_vector<int>(), dest, true, new RuntimeException(exception.what()), 0, "");
    }
    catch( RuntimeException& exception)
    {
        // before emit signal, it is necessary to wait for updateGraphicsInProgress mutex
        CExplorerWindow* dest = dynamic_cast<CExplorerWindow*>(source);
        /**/ updateGraphicsInProgress.lock();
        reset();
        //bufferMutex.unlock();
        itm::warning(exception.what(), "CVolume");
        /**/ updateGraphicsInProgress.unlock();

        emit sendData(0, make_vector<int>(), make_vector<int>(), dest, true, new RuntimeException(exception.what()), 0, "");
    }
    catch(const char* error)
    {
        // before emit signal, it is necessary to wait for updateGraphicsInProgress mutex
        CExplorerWindow* dest = dynamic_cast<CExplorerWindow*>(source);
        /**/ updateGraphicsInProgress.lock();
        reset();
        //bufferMutex.unlock();
        itm::warning(error, "CVolume");
        /**/ updateGraphicsInProgress.unlock();

        emit sendData(0, make_vector<int>(), make_vector<int>(), dest, true, new RuntimeException(error), 0, "");
    }
    catch(...)
    {
        // before emit signal, it is necessary to wait for updateGraphicsInProgress mutex
        CExplorerWindow* dest = dynamic_cast<CExplorerWindow*>(source);
        /**/ updateGraphicsInProgress.lock();
        reset();
        //bufferMutex.unlock();
        itm::warning("Unknown error occurred", "CVolume");
        /**/ updateGraphicsInProgress.unlock();

        emit sendData(0, make_vector<int>(), make_vector<int>(), dest, true, new RuntimeException("Unknown error occurred"), 0, "");
    }
}

