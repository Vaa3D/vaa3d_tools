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
// $Id: HighDynamicRangeEx.cpp,v 1.18 2010/04/01 20:34:47 oscarp Exp $
//=============================================================================

#include "stdafx.h"

#include "FlyCapture2.h"

using namespace FlyCapture2;

const unsigned int k_HDRCtrl = 0x1800;

const unsigned int k_HDRShutter1 = 0x1820;
const unsigned int k_HDRShutter2 = 0x1840;
const unsigned int k_HDRShutter3 = 0x1860;
const unsigned int k_HDRShutter4 = 0x1880;

const unsigned int k_HDRGain1 = 0x1824;
const unsigned int k_HDRGain2 = 0x1844;
const unsigned int k_HDRGain3 = 0x1864;
const unsigned int k_HDRGain4 = 0x1884;

const unsigned int k_HDROn = 0x82000000;
const unsigned int k_HDROff = 0x80000000;

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

bool IsHDRSupported( Camera* pCam )
{
    Error error;
    unsigned int RegVal;
    error = pCam->ReadRegister( k_HDRCtrl, &RegVal );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return false;
    }

    return ( !0x80000000 & RegVal ) ? false : true;
}

void ToggleHDRMode( Camera* pCam, bool hdrOn )
{
    Error error;
    error = pCam->WriteRegister( k_HDRCtrl, hdrOn ? k_HDROn : k_HDROff );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
    }

    printf( "HDR mode turned to %s\n", hdrOn ? "on" : "off" );
}

void InitializeHDRRegisters( Camera* pCam )
{
    Error error;
    error = pCam->WriteRegister( k_HDRShutter1, 0x000 );
    error = pCam->WriteRegister( k_HDRShutter2, 0x120 );
    error = pCam->WriteRegister( k_HDRShutter3, 0x240 );
    error = pCam->WriteRegister( k_HDRShutter4, 0x360 );

    error = pCam->WriteRegister( k_HDRGain1, 0x000 );
    error = pCam->WriteRegister( k_HDRGain2, 0x0E3 );
    error = pCam->WriteRegister( k_HDRGain3, 0x1C6 );
    error = pCam->WriteRegister( k_HDRGain4, 0x2A9 );

    printf( "Initialized HDR registers...\n" );
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

    printf( "Number of cameras detected: %u\n", numCameras );

    if ( numCameras < 1 )
    {
        printf( "Insufficient number of cameras... exiting\n" );
        return -1;
    }

    PGRGuid guid;
    error = busMgr.GetCameraFromIndex(0, &guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

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

    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    bool supported = IsHDRSupported( &cam );
    if ( supported != true )
    {
        printf( "HDR is not supported! Exiting...\n" );
        return -1;
    }

    InitializeHDRRegisters( &cam ); 

    ToggleHDRMode( &cam, true );

    // Grab 4 images
    Image rawImage[4];
    for ( int imageCnt=0; imageCnt < 4; imageCnt++ )
    {
        error = cam.RetrieveBuffer( &rawImage[ imageCnt] );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        printf( "Grabbed image %d\n", imageCnt );
    }

    // Process and save the 4 images
    for ( int imageCnt=0; imageCnt < 4; imageCnt++ )
    {
        // Create a converted image
        Image convertedImage;

        // Convert the raw image
        error = rawImage[ imageCnt].Convert( PIXEL_FORMAT_BGRU, &convertedImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }  

        printf( "Converted image %d\n", imageCnt );

        // Create a unique filename
        char filename[512];
        sprintf( filename, "HDR-%u-%d.ppm", camInfo.serialNumber, imageCnt );

        // Save the image. If a file format is not passed in, then the file
        // extension is parsed to attempt to determine the file format.
        error = convertedImage.Save( filename );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        } 

        printf( "Saved image %d\n", imageCnt );
    }

    printf( "\nFinished grabbing images\n" );

    // Stop capturing images
    error = cam.StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }      

    ToggleHDRMode( &cam, false );

    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    printf( "Done! Press Enter to exit...\n" );
    getchar();

	return 0;
}
