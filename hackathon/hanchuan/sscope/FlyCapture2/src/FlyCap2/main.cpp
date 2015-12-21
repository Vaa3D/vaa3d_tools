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
// $Id: main.cpp,v 1.29 2009/07/21 17:59:31 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include <iostream>

#include "FlyCapture2.h"
#include "FlyCapture2GUI.h"

#include "FlycapWindow.h"

bool GetCameras( PGRGuid* pGuids, unsigned int* pSize )
{
    bool ok;
    CameraSelectionDlg camSlnDlg;
    camSlnDlg.ShowModal( &ok, pGuids, pSize );

    return ok;
}

int main(int /*argc*/, char** /*argv*/)
{
    Gtk::Main* pKit = Gtk::Main::instance();
    if ( pKit == NULL )
    {
        pKit = new Gtk::Main( 0, NULL );
    }

    // Initialize the thread system
    if( !Glib::thread_supported() ) 
    {
        Glib::thread_init();
    }  

    // Display the camera selection dialog
    PGRGuid guid[64];
    unsigned int size = 64;
    bool ok = GetCameras( &guid[0], &size );
    if ( ok != true || size <= 0 )
    {
        return -1;
    }      

    // Create the individual windows
    FlycapWindow* windows = new FlycapWindow[size];
    for ( unsigned int i = 0; i < size; i++ )
    {
        bool ret = windows[i].Run( guid[i] );
        if ( ret != true )
        {
            for ( int j=0; j < i; j++)
            {
                // Clean up all windows that are already created
                windows[j].Cleanup();
            }

            delete [] windows;
            return -1;
        }
    }

    // Start the main Gtk event loop
    pKit->run();

    // This point is reached when the last window alive kills the Gtk::Main loop

    // Perform cleanup on all the windows
    for ( unsigned int i = 0; i < size; i++ )
    {
        windows[i].Cleanup();
    }    

    delete [] windows;

    return 0;
}
