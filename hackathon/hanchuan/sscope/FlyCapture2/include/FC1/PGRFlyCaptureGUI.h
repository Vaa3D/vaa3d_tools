//=============================================================================
// Copyright © 2004 Point Grey Research, Inc. All Rights Reserved.
// 
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with Point Grey Research, Inc. (PGR).
// 
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: PGRFlyCaptureGUI.h,v 1.1 2009/04/30 17:29:31 soowei Exp $
//=============================================================================
#ifndef _PGRFLYCAPTUREGUI_H
#define _PGRFLYCAPTUREGUI_H
//#include <pgrcameragui.h>
#include "pgrcameragui.h"

//=============================================================================
//
// This header allows access to the FlyCapture Image Utility window.
//
//=============================================================================

#ifdef __cplusplus
extern "C"
{
#endif
   
//-----------------------------------------------------------------------------
//
// Name: pgrcamguiCreateGraphWindow()
//    
// Description:
//    Create the FlyCapture specific Image Utility window.  Call this function
//    before any other calls involving the GraphWindow.
//
// Arguments:
//    context     - The PGRCameraGUI context to access.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiCreateGraphWindow(
                               CameraGUIContext context );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiToggleGraphWindowState()
//    
// Description:
//    Displays or hides the modeless Image Utility window.
//
// Arguments:
//    context     - The PGRCameraGUI context to access.
//    hwndParent  - A handle to the dialog's parent. Must not be NULL.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_INVALID_ARGUMENT - if context or hwndParent is NULL
//    PGRCAMGUI_COULD_NOT_CREATE_DIALOG - if dialog could not be created
//    PGRCAMGUI_FAILED - if dialog could not be shown or destroyed
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiToggleGraphWindowState(
                               CameraGUIContext context,
                               HWND hwndParent );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiUpdateGraphWindowImage()
//    
// Description:
//    Used to pass a new image to the Image Utility window.
//
// Arguments:
//    context     - The PGRCameraGUI context to access.
//    fcContext   - A valid FlyCaptureContext. Must not be NULL.
//    pImage      - A valid FlyCaptureImage. Must not be NULL.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_INVALID_ARGUMENT - if any argument is NULL.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiUpdateGraphWindowImage(
                              CameraGUIContext context,
                              FlyCaptureContext fcContext,
                              FlyCaptureImage* pImage );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiGetGraphWindowState()
//    
// Description:
//    Determines whether the Graph Window is currently displayed.
//
// Arguments:
//    context     - The PGRCameraGUI context to access.
//    pShowing    - BOOL to be filled with state information. Must not be NULL.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_INVALID_ARGUMENT - if pShowing is NULL.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiGetGraphWindowState(
                                 CameraGUIContext context,
                                 BOOL* pShowing );
#ifdef __cplusplus
};
#endif

#endif // #ifndef _PGRFLYCAPTUREGUI_H