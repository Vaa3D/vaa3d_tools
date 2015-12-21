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
// $Id: main.cpp,v 1.3 2009/05/12 17:34:44 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include <iostream>

#include "FlyCapture2.h"
#include "FlyCapture2GUI.h"

#include "SerialPortWindow.h"

int main(int /*argc*/, char** /*argv*/)
{
    // Create a Gtk instance
    Gtk::Main kit = Gtk::Main( 0, NULL );  

    // Initialize the thread system
    if( !Glib::thread_supported() ) 
    {
        Glib::thread_init();
    }  

    // Display the camera selection dialog
    CameraSelectionDlg camSlnDlg;
    PGRGuid guid[64];
    unsigned int size = 64;

    bool ok;
    camSlnDlg.ShowModal( &ok, guid, &size );

    if ( ok != true || size <= 0 )
    {
        return -1;
    }      

    bool success;
    SerialPortWindow serialPortWin;
    success = serialPortWin.Initialize();
    if ( success != true )
    {
        return -1;
    }

    serialPortWin.Run( guid[0] );

    return 0;
}
