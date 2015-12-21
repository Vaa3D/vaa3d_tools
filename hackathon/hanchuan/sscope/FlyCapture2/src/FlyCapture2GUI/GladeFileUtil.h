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
// $Id: GladeFileUtil.h,v 1.2 2009/06/12 18:55:05 hirokim Exp $
//=============================================================================

#ifndef GLADEFILEUTIL_H_INCLUDED
#define GLADEFILEUTIL_H_INCLUDED

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

namespace FlyCapture2
{      	
	class GladeFileUtil
	{    
    public:
        static Glib::ustring GetGladeFilePath( void)
        {
            static const char* sk_GUI_GTK_Glade = "FlyCapture2GUI_GTK.glade";

#if defined(WIN32) || defined(WIN64)
            //
            // For Windows, this function returns the full path to the glade file.
            // The relative path won't work if the glade file is not in the 
            // current directory.
            //
            char fileName[ MAX_PATH];
            
            HMODULE module = ::GetModuleHandleA( "FlyCapture2GUI.dll");
            if ( module == NULL)
            {
                // If failed, try to get the debug version DLL.
                module = ::GetModuleHandleA( "FlyCapture2GUId.dll");
                if ( module == NULL)
                {
                    // This shouldn't happen.
                    ::MessageBoxA( NULL, "Failed to get module handle", "GladeFileUtil", MB_OK);
                    return sk_GUI_GTK_Glade;
                }
            }
     
            DWORD result = ::GetModuleFileNameA( module, fileName, sizeof(fileName));
            if ( result == 0)
                return sk_GUI_GTK_Glade;

            char *backslashPos = strrchr( fileName, '\\');
            if ( backslashPos == NULL)
                return sk_GUI_GTK_Glade;

            *(backslashPos+1) = 0;
            strcat( fileName, sk_GUI_GTK_Glade);
            return fileName;
#else
            //
            // For non-Windows, this just returns the relative path to the file.
            //
            return sk_GUI_GTK_Glade;
#endif

        };
    };
}
#endif // GLADEFILEUTIL_H_INCLUDED
