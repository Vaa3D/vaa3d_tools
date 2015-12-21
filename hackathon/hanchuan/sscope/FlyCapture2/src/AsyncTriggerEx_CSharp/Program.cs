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
// $Id: Program.cs,v 1.11 2009/09/08 23:56:40 soowei Exp $
//=============================================================================

using System;
using System.Collections.Generic;
using System.Text;

using FlyCapture2Managed;

namespace AsyncTriggerEx_CSharp
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

        static bool CheckSoftwareTriggerPresence(ManagedCamera cam)
        {
            const uint k_triggerInq = 0x530;
            uint regVal = cam.ReadRegister(k_triggerInq);

            if ((regVal & 0x10000) != 0x10000)
            {
                return false;
            }

            return true;
        }

        static bool PollForTriggerReady(ManagedCamera cam)
        {
            const uint k_softwareTrigger = 0x62C;

            uint regVal = 0;

            do
            {
                regVal = cam.ReadRegister(k_softwareTrigger);
            }
            while ((regVal >> 31) != 0);

            return true;
        }

        static bool FireSoftwareTrigger(ManagedCamera cam)
        {
            const uint k_softwareTrigger = 0x62C;
            const uint k_fireVal = 0x80000000;

            cam.WriteRegister(k_softwareTrigger, k_fireVal);

            return true;
        }

        static void Main(string[] args)
        {
            PrintBuildInfo();

            const int k_numImages = 10;
            bool useSoftwareTrigger = true;

            ManagedBusManager busMgr = new ManagedBusManager();
            uint numCameras = busMgr.GetNumOfCameras();

            Console.WriteLine("Number of cameras detected: {0}", numCameras);

            ManagedPGRGuid guid = busMgr.GetCameraFromIndex(0);

            ManagedCamera cam = new ManagedCamera();

            cam.Connect(guid);

            // Get the camera information
            CameraInfo camInfo = cam.GetCameraInfo();

            PrintCameraInfo(camInfo);

            if (!useSoftwareTrigger)
            {
                // Check for external trigger support
                TriggerModeInfo triggerModeInfo = cam.GetTriggerModeInfo();
                if (triggerModeInfo.present != true)
                {
                    Console.WriteLine("Camera does not support external trigger! Exiting...\n");
                    return;
                }
            }

            // Get current trigger settings
            TriggerMode triggerMode = cam.GetTriggerMode();

            // Set camera to trigger mode 0
            // A source of 7 means software trigger
            triggerMode.onOff = true;
            triggerMode.mode = 0;
            triggerMode.parameter = 0;

            if (useSoftwareTrigger)
            {
                // A source of 7 means software trigger
                triggerMode.source = 7;
            }
            else
            {
                // Triggering the camera externally using source 0.
                triggerMode.source = 0;
            }

            // Set the trigger mode
            cam.SetTriggerMode(triggerMode);

            // Poll to ensure camera is ready
            bool retVal = PollForTriggerReady(cam);
            if (retVal != true)
            {
                return;
            }

            // Get the camera configuration
            FC2Config config = cam.GetConfiguration();

            // Set the grab timeout to 5 seconds
            config.grabTimeout = 5000;

            // Set the camera configuration
            cam.SetConfiguration(config);

            // Camera is ready, start capturing images
            cam.StartCapture();

            if (useSoftwareTrigger)
            {
                if (CheckSoftwareTriggerPresence(cam) == false)
                {
                    Console.WriteLine("SOFT_ASYNC_TRIGGER not implemented on this camera!  Stopping application\n");
                    return;
                }
            }
            else
            {
                Console.WriteLine("Trigger the camera by sending a trigger pulse to GPIO%d.\n",
                  triggerMode.source);
            }

            ManagedImage image = new ManagedImage();
            for (int iImageCount = 0; iImageCount < k_numImages; iImageCount++)
            {
                if (useSoftwareTrigger)
                {

                    // Check that the trigger is ready
                    retVal = PollForTriggerReady(cam);

                    Console.WriteLine("Press the Enter key to initiate a software trigger.\n");
                    Console.ReadLine();

                    // Fire software trigger
                    retVal = FireSoftwareTrigger(cam);
                    if (retVal != true)
                    {
                        Console.WriteLine("Error firing software trigger!");
                        return;
                    }
                }

                // Grab image
                cam.RetrieveBuffer(image);

                Console.WriteLine(".\n");
            }

            Console.WriteLine("Finished grabbing images");

            // Stop capturing images
            cam.StopCapture();

            // Turn off trigger mode
            triggerMode.onOff = false;
            cam.SetTriggerMode(triggerMode);

            // Disconnect the camera
            cam.Disconnect();

            Console.WriteLine("Done! Press any key to exit...");
            Console.ReadKey();
        }
    }
}
