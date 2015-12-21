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
// $Id: Program.cs,v 1.3 2010/09/20 23:02:10 oscarp Exp $
//=============================================================================

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

using FlyCapture2Managed;

namespace GigEGrabEx_CSharp
{
    class Program
    {
        static void PrintBuildInfo()
        {
            FC2Version version = ManagedUtilities.libraryVersion;

            StringBuilder newStr = new StringBuilder();
            newStr.AppendFormat(
                "FlyCapture2 library version: {0}.{1}.{2}.{3}\n",
                version.major, version.minor, version.type, version.build);

            Console.WriteLine(newStr);
        }

        static void PrintCameraInfo(CameraInfo camInfo)
        {
            StringBuilder newStr = new StringBuilder();
            newStr.Append("\n*** CAMERA INFORMATION ***\n");
            newStr.AppendFormat("Serial number - {0}\n", camInfo.serialNumber);
            newStr.AppendFormat("Camera model - {0}\n", camInfo.modelName);
            newStr.AppendFormat("Camera vendor - {0}\n", camInfo.vendorName);
            newStr.AppendFormat("Sensor - {0}\n", camInfo.sensorInfo);
            newStr.AppendFormat("Resolution - {0}\n", camInfo.sensorResolution);
            newStr.AppendFormat("Firmware version - {0}\n", camInfo.firmwareVersion);
            newStr.AppendFormat("Firmware build time - {0}\n", camInfo.firmwareBuildTime);
            newStr.AppendFormat("GigE version - {0}.{1}\n", camInfo.gigEMajorVersion, camInfo.gigEMinorVersion);
            newStr.AppendFormat("User defined name - {0}\n", camInfo.userDefinedName);
            newStr.AppendFormat("XML URL 1 - {0}\n", camInfo.xmlURL1);
            newStr.AppendFormat("XML URL 2 - {0}\n", camInfo.xmlURL2);
            newStr.AppendFormat("MAC address - {0}\n", camInfo.macAddress.ToString());
            newStr.AppendFormat("IP address - {0}\n", camInfo.ipAddress.ToString());
            newStr.AppendFormat("Subnet mask - {0}\n", camInfo.subnetMask.ToString());
            newStr.AppendFormat("Default gateway - {0}\n", camInfo.defaultGateway.ToString());

            Console.WriteLine(newStr);
        }

        static void PrintStreamChannelInfo(GigEStreamChannel streamChannelInfo)
        {
            StringBuilder newStr = new StringBuilder();
            newStr.Append("\n*** STREAM CHANNEL INFORMATION ***\n");
            newStr.AppendFormat("Network interface - {0}\n", streamChannelInfo.networkInterfaceIndex);
            newStr.AppendFormat("Host post - {0}\n", streamChannelInfo.hostPost);
            newStr.AppendFormat("Do not fragment bit - {0}\n", streamChannelInfo.doNotFragment == true ? "Enabled" : "Disabled");
            newStr.AppendFormat("Packet size - {0}\n", streamChannelInfo.packetSize);
            newStr.AppendFormat("Inter packet delay - {0}\n", streamChannelInfo.interPacketDelay);
            newStr.AppendFormat("Destination IP address - {0}\n", streamChannelInfo.destinationIpAddress);
            newStr.AppendFormat("Source port (on camera) - {0}\n\n", streamChannelInfo.sourcePort);

            Console.WriteLine(newStr);
        }

        void RunSingleCamera(ManagedPGRGuid guid)
        {
            const int k_numImages = 10;

            ManagedGigECamera cam = new ManagedGigECamera();

            // Connect to a camera
            cam.Connect(guid);

            // Get the camera information
            CameraInfo camInfo = cam.GetCameraInfo();
            PrintCameraInfo(camInfo);

            uint numStreamChannels = cam.GetNumStreamChannels();
            for (uint i=0; i < numStreamChannels; i++)
            {
                PrintStreamChannelInfo(cam.GetGigEStreamChannelInfo(i));
            }

            GigEImageSettingsInfo imageSettingsInfo = cam.GetGigEImageSettingsInfo();

            GigEImageSettings imageSettings = new GigEImageSettings();
            imageSettings.offsetX = 0;
            imageSettings.offsetY = 0;
            imageSettings.height = imageSettingsInfo.maxHeight;
            imageSettings.width = imageSettingsInfo.maxWidth;
            imageSettings.pixelFormat = PixelFormat.PixelFormatMono8;

            cam.SetGigEImageSettings(imageSettings);

            // Get embedded image info from camera
            EmbeddedImageInfo embeddedInfo = cam.GetEmbeddedImageInfo();

            // Enable timestamp collection	
            if (embeddedInfo.timestamp.available == true)
            {
                embeddedInfo.timestamp.onOff = true;
            }

            // Set embedded image info to camera
            cam.SetEmbeddedImageInfo(embeddedInfo);

            // Start capturing images
            cam.StartCapture();

            ManagedImage rawImage = new ManagedImage();
            for (int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
            {
                // Retrieve an image
                cam.RetrieveBuffer(rawImage);

                // Get the timestamp
                TimeStamp timeStamp = rawImage.timeStamp;

                Console.WriteLine(
                   "Grabbed image {0} - {1} {2} {3}",
                   imageCnt,
                   timeStamp.cycleSeconds,
                   timeStamp.cycleCount,
                   timeStamp.cycleOffset);

                // Create a converted image
                ManagedImage convertedImage = new ManagedImage();

                // Convert the raw image
                rawImage.Convert(PixelFormat.PixelFormatBgr, convertedImage);

                // Create a unique filename
                string filename = String.Format(
                   "GigEGrabEx_CSharp-{0}-{1}.bmp",
                   camInfo.serialNumber,
                   imageCnt);

                // Get the Bitmap object. Bitmaps are only valid if the
                // pixel format of the ManagedImage is RGB or RGBU.
                System.Drawing.Bitmap bitmap = convertedImage.bitmap;

                // Save the image
                bitmap.Save(filename);
            }

            // Stop capturing images
            cam.StopCapture();

            // Disconnect the camera
            cam.Disconnect();
        }

        static void Main(string[] args)
        {
            PrintBuildInfo();

            Program program = new Program();

            // Since this application saves images in the current folder
            // we must ensure that we have permission to write to this folder.
            // If we do not have permission, fail right away.
            FileStream fileStream;
            try
            {
                fileStream = new FileStream(@"test.txt", FileMode.Create);
                fileStream.Close();
                File.Delete("test.txt");
            }
            catch
            {
                Console.WriteLine("Failed to create file in current folder.  Please check permissions.\n");
                return;
            }

            ManagedBusManager busMgr = new ManagedBusManager();

            CameraInfo[] camInfos = ManagedBusManager.DiscoverGigECameras();
            Console.WriteLine("Number of cameras discovered: {0}", camInfos.Length);
            foreach (CameraInfo camInfo in camInfos)
            {
                PrintCameraInfo(camInfo);
            }

            uint numCameras = busMgr.GetNumOfCameras();
            Console.WriteLine("Number of cameras enumerated: {0}", numCameras);

            for (uint i = 0; i < numCameras; i++)
            {
                ManagedPGRGuid guid = busMgr.GetCameraFromIndex(i);
                if ( busMgr.GetInterfaceTypeFromGuid(guid) != InterfaceType.GigE )
                {
                    continue;
                }

                try
                {
                    program.RunSingleCamera(guid);
                }
                catch (FC2Exception ex)
                {                    
                    Console.WriteLine(
                        String.Format(
                        "Error running camera {0}. Error: {1}", 
                        i, ex.Message));
                }                
            }

            Console.WriteLine("Done! Press any key to exit...");
            Console.ReadKey();
        }
    }
}
