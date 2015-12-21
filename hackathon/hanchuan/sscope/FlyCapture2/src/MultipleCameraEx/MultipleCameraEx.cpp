//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: MultipleCameraEx.cpp,v 1.17 2010/02/26 01:00:50 soowei Exp $
//=============================================================================

#include "stdafx.h"

#include "FlyCapture2.h"

using namespace FlyCapture2;

void PrintBuildInfo()
{
    FC2Version fc2Version;
    Utilities::GetLibraryVersion( &fc2Version );
    char version[128];
    sprintf( 
        version, 
        "FlyCapture2 library version: %d.%d.%d.%d\n", 
        fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build );

    printf( "%s", version );

    char timeStamp[512];
    sprintf( timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__ );

    printf( "%s", timeStamp );
}

void PrintCameraInfo( CameraInfo* pCamInfo )
{
    printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}

void PrintError( Error error )
{
    error.PrintErrorTrace();
}

int main(int /*argc*/, char** /*argv*/)
{
    PrintBuildInfo();

    const int k_numImages = 100;
    Error error;

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    printf( "Number of cameras detected: %u\n", numCameras );

    if ( numCameras < 1 )
    {
        printf( "Insufficient number of cameras... press Enter to exit.\n" );
        getchar();
        return -1;
    }

    Camera** ppCameras = new Camera*[numCameras];

    // Connect to all detected cameras and attempt to set them to
    // a common video mode and frame rate
    for ( unsigned int i = 0; i < numCameras; i++)
    {
        ppCameras[i] = new Camera();

        PGRGuid guid;
        error = busMgr.GetCameraFromIndex( i, &guid );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        // Connect to a camera
        error = ppCameras[i]->Connect( &guid );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        // Get the camera information
        CameraInfo camInfo;
        error = ppCameras[i]->GetCameraInfo( &camInfo );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        PrintCameraInfo(&camInfo); 

        // Set all cameras to a specific mode and frame rate so they
        // can be synchronized
        error = ppCameras[i]->SetVideoModeAndFrameRate( 
            VIDEOMODE_640x480Y8, 
            FRAMERATE_30 );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            printf( 
                "Error starting cameras. \n"
                "This example requires cameras to be able to set to 640x480 Y8 at 30fps. \n"
                "If your camera does not support this mode, please edit the source code and recompile the application. \n"
                "Press Enter to exit. \n");
            getchar();
            return -1;
        }
    }
    
    error = Camera::StartSyncCapture( numCameras, (const Camera**)ppCameras );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        printf( 
            "Error starting cameras. \n"
            "This example requires cameras to be able to set to 640x480 Y8 at 30fps. \n"
            "If your camera does not support this mode, please edit the source code and recompile the application. \n"
            "Press Enter to exit. \n");
        getchar();
        return -1;
    }

    for ( int j = 0; j < k_numImages; j++ )
    {
        // Display the timestamps for all cameras to show that the image
        // capture is synchronized for each image
        for ( unsigned int i = 0; i < numCameras; i++ )
        {
            Image image;
            error = ppCameras[i]->RetrieveBuffer( &image );
            if (error != PGRERROR_OK)
            {
                PrintError( error );
                return -1;
            }

            TimeStamp timestamp = image.GetTimeStamp();
            printf( 
                "Cam %d - Frame %d - TimeStamp [%d %d]\n", 
                i, 
                j, 
                timestamp.cycleSeconds, 
                timestamp.cycleCount);
        }
    }

    for ( unsigned int i = 0; i < numCameras; i++ )
    {
        ppCameras[i]->StopCapture();
        ppCameras[i]->Disconnect();
        delete ppCameras[i];
    }

    delete [] ppCameras;

    printf( "Done! Press Enter to exit...\n" );
    getchar();

	return 0;
}
