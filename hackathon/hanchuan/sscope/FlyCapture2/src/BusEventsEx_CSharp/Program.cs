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
// $Id: Program.cs,v 1.1 2010/07/06 18:58:37 soowei Exp $
//=============================================================================

using System;
using System.Collections.Generic;
using System.Text;

using FlyCapture2Managed;

namespace BusEventsEx_CSharp
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

        void OnBusReset(System.IntPtr ptr, uint serialNumber)
        {
            Console.WriteLine("{0} - *** BUS RESET ***", DateTime.Now.ToString());
        }

        void OnBusArrival(System.IntPtr ptr, uint serialNumber)
        {
            Console.WriteLine("{0} - *** BUS ARRIVAL ({1})***", DateTime.Now.ToString(), serialNumber);
        }

        void OnBusRemoval(System.IntPtr ptr, uint serialNumber)
        {
            Console.WriteLine("{0} - *** BUS REMOVAL ({1})***", DateTime.Now.ToString(), serialNumber);
        }

        void BusResetLoop()
        {
            ManagedBusManager busMgr = new ManagedBusManager();

            List<IntPtr> callbackHandles = new List<IntPtr>();

            // Register bus events
            IntPtr busResetHandle = busMgr.RegisterCallback(OnBusReset, ManagedCallbackType.BusReset, IntPtr.Zero);
            IntPtr busArrivalHandle = busMgr.RegisterCallback(OnBusArrival, ManagedCallbackType.Arrival, IntPtr.Zero);
            IntPtr busRemovalHandle = busMgr.RegisterCallback(OnBusRemoval, ManagedCallbackType.Removal, IntPtr.Zero);

            callbackHandles.Add(busResetHandle);
            callbackHandles.Add(busArrivalHandle);
            callbackHandles.Add(busRemovalHandle);

            // Prevent exit if CTL+C is pressed.
            Console.TreatControlCAsInput = true;

            Console.WriteLine("Press any key to exit...\n");
            ConsoleKeyInfo cki = Console.ReadKey();

            // Unregister bus events
            foreach (IntPtr currHandle in callbackHandles)
            {
                busMgr.UnregisterCallback(currHandle);
            }
        }

        static void Main(string[] args)
        {
            PrintBuildInfo();

            Program program = new Program();
            program.BusResetLoop();      

            Console.WriteLine("Done! Press any key to exit...");
            Console.ReadKey();
        }
    }
}
