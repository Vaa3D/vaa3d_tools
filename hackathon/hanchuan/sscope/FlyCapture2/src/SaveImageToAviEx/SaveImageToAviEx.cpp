//=============================================================================
// Copyright © 2009 Point Grey Research, Inc. All Rights Reserved.
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
// $Id: SaveImageToAviEx.cpp,v 1.10 2009/12/08 18:58:36 soowei Exp $
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

int RunCamera( PGRGuid guid )
{
    const int k_numImages = 100;

    Error error;
    Camera cam;

    // Connect to a camera
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    // Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    PrintCameraInfo(&camInfo);  

    // Start capturing images
    printf( "Start capturing... \n" );
    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    AVIRecorder aviRecorder;
    char AVIFileName[512];
    sprintf( AVIFileName, "AviTest%u.avi", camInfo.serialNumber );
    AVIOption option; 

    // Check if the camera supports the FRAME_RATE property
    PropertyInfo propInfo;
    propInfo.type = FRAME_RATE;
    error = cam.GetPropertyInfo( &propInfo );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    if ( propInfo.present == true )
    {
        // Get the frame rate
        Property prop;
        prop.type = FRAME_RATE;
        error = cam.GetProperty( &prop );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }
        // Set the frame rate.
        // Note that the actual recording frame rate may be slower,
        // depending on the bus speed and disk writing speed.
        option.frameRate = prop.absValue;
    }
    else
    {
       // Frame rate property does not appear to be supported.
       // Set to a proper value, 15.0fps for example.
       option.frameRate = 15.0;
    }
 
    // Open the AVI file for appending images
    error = aviRecorder.AVIOpen(AVIFileName, &option );
    if (error != PGRERROR_OK)
    {
       PrintError( error );
       return -1;
    }

    Image rawImage; 
    printf( "Grab %d images and Save as AVI file: %s ... \n", k_numImages, AVIFileName ); 
    for ( int imageCnt=0; imageCnt < k_numImages; imageCnt++ )
    {                
        // Retrieve an image
        error = cam.RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            continue;
        }

        // Append the image to AVI file
        error = aviRecorder.AVIAppend( &rawImage );
        if (error != PGRERROR_OK)
        {
           PrintError( error );
           return -1;
        }
        printf( "Grabbed and appended image %d\n", imageCnt ); 
    } 

    // Close the AVI file
    error = aviRecorder.AVIClose( );
    if (error != PGRERROR_OK)
    {
       PrintError( error );
       return -1;
    } 

    // Stop capturing images
    error = cam.StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }       
 
    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{    
    PrintBuildInfo();

    Error error;

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
    FILE* tempFile = fopen("test.txt", "w+");
    if (tempFile == NULL)
    {
       printf("Failed to create file in current folder.  Please check permissions.\n");
       return -1;
    }
    fclose(tempFile);
    remove("test.txt");

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    if ( numCameras < 1 )
    {
       printf( "No camera detected.\n" );
       return -1;
    }
    else
    {
       printf( "Number of cameras detected: %u\n", numCameras );
    }

    PGRGuid guid;
    error = busMgr.GetCameraFromIndex(0, &guid);
    if (error != PGRERROR_OK)
    {
       PrintError( error );
       return -1;
    }

    printf( "Running the first camera.\n" );
    RunCamera( guid );

    printf( "Done! Press Enter to exit...\n" );
    getchar();

    return 0;
}
