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
// $Id: HelpLauncher.h,v 1.1 2009/08/28 23:37:36 soowei Exp $
//=============================================================================

#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#include <HtmlHelp.h>

#if defined( WIN64 ) && (_MSC_VER >= 1400)
#define REGISTRY_HIVE   KEY_WOW64_64KEY
#elif defined( WIN32 ) && (_MSC_VER >= 1400)
#define REGISTRY_HIVE   KEY_WOW64_32KEY
#else
#define REGISTRY_HIVE   0
#endif

#else

// All other platforms

#endif

std::string GetHelpFilePath()
{
#if defined(WIN32) || defined(WIN64)

    // Default to the assumption that the app is being run out of the default
    // directory structure
    const char* defaultHelpPath = "..\\doc\\FlyCapture2 SDK Help.chm"; 

    char helpPath[sk_maxStringLength];
    memset( helpPath, 0x0, sk_maxStringLength );

    HKEY hkey = NULL;
    LONG retVal = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        "Software\\Point Grey Research, Inc.\\PGRFlyCapture\\",
        0,
        KEY_QUERY_VALUE | REGISTRY_HIVE,
        &hkey );
    if( retVal != ERROR_SUCCESS || hkey == NULL )
    {
        sprintf( helpPath, defaultHelpPath ); 
    }
    else
    {
        char installDir[sk_maxStringLength];
        memset( installDir, 0x0, sk_maxStringLength );

        DWORD dwBufSize = sk_maxStringLength;
        retVal = ::RegQueryValueEx( 
            hkey, 
            "InstallDir", 
            0,
            NULL,
            (LPBYTE)installDir,
            &dwBufSize );
        if ( retVal != ERROR_SUCCESS )
        {
            sprintf( helpPath, defaultHelpPath ); 
        }
        else
        {
            retVal = ::RegCloseKey( hkey );
            if ( retVal != ERROR_SUCCESS )
            {
                // Error
            }

            sprintf( helpPath, "%sdoc\\FlyCapture2 SDK Help.chm", installDir );
        }        
    }   

    std::string helpFilePath( helpPath );
    return helpFilePath;

#else

    // All other platforms
    std::string helpFilePath("");
    return helpFilePath;

#endif
}

void LaunchHelp()
{
#if defined(WIN32) || defined(WIN64)

    std::string helpFilePath = GetHelpFilePath();

    HWND result = HtmlHelp(
        NULL,
        helpFilePath.c_str(),
        HH_DISPLAY_TOC,
        NULL );
    if( result == NULL )
    {      
        Gtk::MessageDialog dialog( 
            "Error locating help file", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK );       

        Glib::ustring secondaryTxt;
        secondaryTxt = "Unable to locate help file at ";
        secondaryTxt += helpFilePath;
        dialog.set_secondary_text( secondaryTxt );

        dialog.run();	
    }

#else

    // All other platforms

#endif
}

//=============================================================================
// $Log: HelpLauncher.h,v $
// Revision 1.1  2009/08/28 23:37:36  soowei
// [1] Adding loading of help file as mentioned in bug 8035
//
//=============================================================================
