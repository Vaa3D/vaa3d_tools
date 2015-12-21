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
// $Id: Program.cs,v 1.21 2010/09/20 23:02:21 oscarp Exp $
//=============================================================================

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

using FlyCapture2Managed;

namespace CustomImageEx_CSharp
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

            Console.WriteLine(newStr);
        }

        static void PrintFormat7Capabilities(Format7Info fmt7Info)
        {
            StringBuilder newStr = new StringBuilder();
            newStr.AppendFormat("Max image pixels: ({0}, {1})\n", fmt7Info.maxWidth, fmt7Info.maxHeight);
            newStr.AppendFormat("Image Unit size: ({0}, {1})\n", fmt7Info.imageHStepSize, fmt7Info.imageVStepSize);
            newStr.AppendFormat("Offset Unit size: ({0}, {1})", fmt7Info.offsetHStepSize, fmt7Info.offsetVStepSize);

            Console.WriteLine(newStr);
        }

        static void Main(string[] args)
        {
            PrintBuildInfo();

            const Mode k_fmt7Mode = Mode.Mode0;
            const PixelFormat k_fmt7PixelFormat = PixelFormat.PixelFormatMono8;
            const int k_numImages = 10;

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
            uint numCameras = busMgr.GetNumOfCameras();

            Console.WriteLine("Number of cameras detected: {0}", numCameras);

            ManagedPGRGuid guid = busMgr.GetCameraFromIndex(0);

            ManagedCamera cam = new ManagedCamera();

            cam.Connect(guid);

            // Get the camera information
            CameraInfo camInfo = cam.GetCameraInfo();

            PrintCameraInfo(camInfo);

            // Query for available Format 7 modes
            bool supported = false;
            Format7Info fmt7Info = cam.GetFormat7Info(k_fmt7Mode, ref supported);

            PrintFormat7Capabilities(fmt7Info);

            if ((k_fmt7PixelFormat & (PixelFormat)fmt7Info.pixelFormatBitField) == 0)
            {
                // Pixel format not supported!
                return;
            }

            Format7ImageSettings fmt7ImageSettings = new Format7ImageSettings();
            fmt7ImageSettings.mode = k_fmt7Mode;
            fmt7ImageSettings.offsetX = 0;
            fmt7ImageSettings.offsetY = 0;
            fmt7ImageSettings.width = fmt7Info.maxWidth;
            fmt7ImageSettings.height = fmt7Info.maxHeight;
            fmt7ImageSettings.pixelFormat = k_fmt7PixelFormat;

            // Validate the settings to make sure that they are valid
            bool settingsValid = false;
            Format7PacketInfo fmt7PacketInfo = cam.ValidateFormat7Settings(
                fmt7ImageSettings,
                ref settingsValid);

            if (settingsValid != true)
            {
                // Settings are not valid
                return;
            }

            // Set the settings to the camera
            cam.SetFormat7Configuration(
               fmt7ImageSettings,
               fmt7PacketInfo.recommendedBytesPerPacket);

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

            // Retrieve frame rate property
            CameraProperty frmRate = cam.GetProperty(PropertyType.FrameRate);

            Console.WriteLine("Frame rate is {0:F2} fps", frmRate.absValue);

            Console.WriteLine("Grabbing {0} images", k_numImages);

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
                   "CustomImageEx_CSharp-{0}-{1}.bmp",
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

            Console.WriteLine("Done! Press any key to exit...");
            Console.ReadKey();
        }
    }
}
