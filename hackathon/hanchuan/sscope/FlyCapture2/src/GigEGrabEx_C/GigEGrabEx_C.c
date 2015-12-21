//=============================================================================
// Copyright © 2010 Point Grey Research, Inc. All Rights Reserved.
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
// $Id: GigEGrabEx_C.c,v 1.2 2010/05/05 16:15:27 soowei Exp $
//=============================================================================


#if defined(WIN32) || defined(WIN64)
#define _CRT_SECURE_NO_WARNINGS		
#endif

#include "C/FlyCapture2_C.h"
#include <stdio.h>

void PrintBuildInfo()
{
    fc2Version version;
    char versionStr[512];
    char timeStamp[512];

    fc2GetLibraryVersion( &version );

    sprintf( 
        versionStr, 
        "FlyCapture2 library version: %d.%d.%d.%d\n", 
        version.major, version.minor, version.type, version.build );

    printf( "%s", versionStr );

    sprintf( timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__ );

    printf( "%s", timeStamp );
}

void PrintCameraInfo( fc2CameraInfo *pCamInfo )
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

    if ( pCamInfo->interfaceType == FC2_INTERFACE_GIGE)
    {
        fprintf(
            stdout,
            "GigE major version - %u\n"
            "GigE minor version - %u\n"
            "User-defined name - %s\n"
            "XML URL1 - %s\n"
            "XML URL2 - %s\n"
            "MAC address - %02X:%02X:%02X:%02X:%02X:%02X\n"
            "IP address - %u.%u.%u.%u\n"
            "Subnet mask - %u.%u.%u.%u\n"
            "Default gateway - %u.%u.%u.%u\n\n",
            pCamInfo->gigEMajorVersion,
            pCamInfo->gigEMinorVersion,
            pCamInfo->userDefinedName,
            pCamInfo->xmlURL1,
            pCamInfo->xmlURL2,
            pCamInfo->macAddress.octets[0],
            pCamInfo->macAddress.octets[1],
            pCamInfo->macAddress.octets[2],
            pCamInfo->macAddress.octets[3],
            pCamInfo->macAddress.octets[4],
            pCamInfo->macAddress.octets[5],
            pCamInfo->ipAddress.octets[0],
            pCamInfo->ipAddress.octets[1],
            pCamInfo->ipAddress.octets[2],
            pCamInfo->ipAddress.octets[3],
            pCamInfo->subnetMask.octets[0],
            pCamInfo->subnetMask.octets[1],
            pCamInfo->subnetMask.octets[2],
            pCamInfo->subnetMask.octets[3],
            pCamInfo->defaultGateway.octets[0],
            pCamInfo->defaultGateway.octets[1],
            pCamInfo->defaultGateway.octets[2],
            pCamInfo->defaultGateway.octets[3]);            
    }
}


void PrintStreamChannelInfo( fc2GigEStreamChannel* pStreamChannel )
{
    char ipAddress[32];
    sprintf( 
        ipAddress, 
        "%u.%u.%u.%u", 
        pStreamChannel->destinationIpAddress.octets[0],
        pStreamChannel->destinationIpAddress.octets[1],
        pStreamChannel->destinationIpAddress.octets[2],
        pStreamChannel->destinationIpAddress.octets[3]);

    printf(
        "Network interface: %u\n"
        "Host post: %u\n"
        "Do not fragment bit: %s\n"
        "Packet size: %u\n"
        "Inter packet delay: %u\n"
        "Destination IP address: %s\n"
        "Source port (on camera): %u\n\n",
        pStreamChannel->networkInterfaceIndex,
        pStreamChannel->hostPost,
        pStreamChannel->doNotFragment == TRUE ? "Enabled" : "Disabled",
        pStreamChannel->packetSize,
        pStreamChannel->interPacketDelay,
        ipAddress,
        pStreamChannel->sourcePort );
}

int RunSingleCamera( fc2Context context, fc2PGRGuid guid)
{
    const int k_numImages = 1000;

    fc2Error error;
    fc2CameraInfo camInfo;
    unsigned int numStreamChannels = 0;
    fc2GigEImageSettingsInfo imageSettingsInfo;
    fc2Image rawImage, convertedImage;    
    fc2GigEImageSettings imageSettings;
    int imageCnt;
    unsigned int i;
    char filename[512];

    printf( "Connecting to camera...\n" );

    // Connect to a camera
    error = fc2Connect( context, &guid );
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2Connect: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    // Get the camera information
    error = fc2GetCameraInfo( context, &camInfo);
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2GetCameraInfo: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    PrintCameraInfo(&camInfo);        

    error = fc2GetNumStreamChannels( context, &numStreamChannels );
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2GetNumStreamChannels: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    for ( i = 0; i < numStreamChannels; i++)
    {
        fc2GigEStreamChannel streamChannel;
        error = fc2GetGigEStreamChannelInfo( context, i, &streamChannel );
        if ( error != FC2_ERROR_OK )
        {
            printf( "Error in fc2GetGigEStreamChannelInfo: %s\n", fc2ErrorToDescription(error) );
            return -1;
        }

        printf( "\nPrinting stream channel information for channel %u:\n", i );
        PrintStreamChannelInfo( &streamChannel );
    }    

    printf( "Querying GigE image setting information...\n" );

    error = fc2GetGigEImageSettingsInfo( context, &imageSettingsInfo );
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2GetGigEImageSettingsInfo: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    imageSettings.offsetX = 0;
    imageSettings.offsetY = 0;
    imageSettings.height = imageSettingsInfo.maxHeight;
    imageSettings.width = imageSettingsInfo.maxWidth;
    imageSettings.pixelFormat = FC2_PIXEL_FORMAT_MONO8;

    printf( "Setting GigE image settings...\n" );

    error = fc2SetGigEImageSettings( context, &imageSettings );
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2SetGigEImageSettings: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    printf( "Starting image capture...\n" );

    // Start capturing images
    error = fc2StartCapture( context);
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2StartCapture: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    // Prepare images
    error = fc2CreateImage( &rawImage );
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2CreateImage: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    error = fc2CreateImage( &convertedImage );
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2CreateImage: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    for ( imageCnt=0; imageCnt < k_numImages; imageCnt++ )
    {              
        // Retrieve an image
        error = fc2RetrieveBuffer( context, &rawImage );
        if ( error != FC2_ERROR_OK )
        {
            printf( "Error in fc2RetrieveBuffer: %s\n", fc2ErrorToDescription(error) );
            return -1;
        }

        printf( "Grabbed image %d\n", imageCnt );
        
        // Convert the raw image
        error = fc2ConvertImageTo( FC2_PIXEL_FORMAT_MONO8, 
                                &rawImage,
                                &convertedImage );
        if ( error != FC2_ERROR_OK )
        {
            printf( "Error in fc2ConvertImage: %s\n", fc2ErrorToDescription(error) );
            return -1;
        }

        // Create a unique filename
        sprintf( filename, "GigEGrabEx-%u-%d.pgm", camInfo.serialNumber, imageCnt );

        /*
        // Save the image. If a file format is not passed in, then the file
        // extension is parsed to attempt to determine the file format.
        error = fc2SaveImage( &convertedImage, filename, FC2_PGM );
        if ( error != FC2_ERROR_OK )
        {
            printf( "Error in fc2SaveImage: %s\n", fc2ErrorToDescription(error) );
            return -1;
        }
        */
    }         

    printf( "Stopping capture...\n" );

    // Stop capturing images
    error = fc2StopCapture( context);
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2StopCapture: %s\n", fc2ErrorToDescription(error) );
    }

    // Disconnect the camera
    error = fc2Disconnect( context);
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2Disconnect: %s\n", fc2ErrorToDescription(error) );
    }

    error = fc2DestroyImage( &rawImage );
    error = fc2DestroyImage( &convertedImage );

    error = fc2DestroyContext( context );

    return 0;
}

int main(int argc, char** argv)
{        
    fc2Error error;
    fc2Context context;
    fc2PGRGuid guid;
    unsigned int numCameras = 0;    
    fc2CameraInfo camInfo[128];
    unsigned int numCamInfo = 128;
    FILE* tempFile;
    unsigned int i;

    PrintBuildInfo();

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
    tempFile = fopen("test.txt", "w+");
    if (tempFile == NULL)
    {
        printf("Failed to create file in current folder.  Please check permissions.\n");
        return -1;
    }
    fclose(tempFile);
    remove("test.txt");    

    error = fc2CreateGigEContext( &context );
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2CreateContext: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }        

    error = fc2DiscoverGigECameras( context, camInfo, &numCamInfo);
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2DiscoverGigECameras: %s\n", fc2ErrorToDescription(error) );
        return -1;
    }

    printf( "Number of cameras discovered: %u\n", numCamInfo );

    for ( i=0; i < numCamInfo; i++)
    {
        PrintCameraInfo( &camInfo[i] );
    }

    error = fc2GetNumOfCameras( context, &numCameras );
    if ( error != FC2_ERROR_OK )
    {
        printf( "Error in fc2GetNumOfCameras: %s\n", fc2ErrorToDescription(error) );
        return 0;
    }

    if ( numCameras == 0 )
    {
        // No cameras detected
        printf( "No cameras detected.\n");
        return 0;
    }

    for (i=0; i < numCameras; i++)
    {
        fc2InterfaceType interfaceType;

        error = fc2GetCameraFromIndex( context, i, &guid);
        if ( error != FC2_ERROR_OK )
        {
            printf( "Error in fc2GetCameraFromIndex: %s\n", fc2ErrorToDescription(error) );
            return -1;
        }    

        error = fc2GetInterfaceTypeFromGuid( context, &guid, &interfaceType );
        if ( error != FC2_ERROR_OK )
        {
            printf( "Error in fc2GetInterfaceTypFromGuid: %s\n", fc2ErrorToDescription(error) );
            return -1;
        }    

        if ( interfaceType == FC2_INTERFACE_GIGE )
        {
            RunSingleCamera( context, guid);
        }
    }

    printf( "Done! Press Enter to exit...\n" );
    getchar();

	return 0;
}

