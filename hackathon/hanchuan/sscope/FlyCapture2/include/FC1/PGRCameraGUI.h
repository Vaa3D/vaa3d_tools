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
// $Id: PGRCameraGUI.h,v 1.1 2009/04/30 18:12:25 soowei Exp $
//=============================================================================
#ifndef __PGRCAMERAGUI_H__
#define __PGRCAMERAGUI_H__

//=============================================================================
//
// PGRFlyCaptureGUI / DigiclopsGUI / LadybugGUI
//
//  Provides an API to display dialogs for:
//   - selecting a camera on the bus.
//   - altering camera properties.
//
//  This header file defines the API for all three of the "GUI" .DLLs  The
//  behaviour of the settings dialog is defined by the lib .DLL stub that is
//  linked to.  PGRFlyCaptureGUI.LIB will load PGRFlyCaptureGUI.DLL, 
//  DigiclopsGUI.LIB will load DigiclopsGUI.DLL and LadybugGUI.LIB will load
//  LadybugGUI.DLL.  PGRCameraGUI.LIB and PGRCameraGUI.DLL are no longer
//  distributed.  Please link to the appropriate SDK-specific .LIB file (and
//  .DLL), and use this header file.
//
//=============================================================================


#ifdef PGRCAMERAGUI_EXPORTS
#define PGRCAMERAGUI_API __declspec( dllexport )
#else
#define PGRCAMERAGUI_API __declspec( dllimport )
#endif

#ifdef __cplusplus
extern "C"
{
#endif
   
//=============================================================================
// Definitions and Enumerations
//=============================================================================

//
// Description:
//   A user level handle for the PGRGui object.
//
typedef void*  CameraGUIContext;


//
// Description:
//   A generic camera context to cast SDK contexts to before passing them in.
//
typedef void*  GenericCameraContext;


//
// Description:
//   Error codes returned from all PGRCameraGUI functions.
//
typedef enum CameraGUIError
{
   // Function completed successfully.
   PGRCAMGUI_OK,                      
   // Function failed.
   PGRCAMGUI_FAILED,                  
   // Was unable to create dialog.
   PGRCAMGUI_COULD_NOT_CREATE_DIALOG,         
   // An invalid argument was passed.
   PGRCAMGUI_INVALID_ARGUMENT,                
   // An invalid context was passed.
   PGRCAMGUI_INVALID_CONTEXT,       
   // Memory allocation error.
   PGRCAMGUI_MEMORY_ALLOCATION_ERROR,         
   // There has been an internal camera error - call getLastError()
   PGRCAMGUI_INTERNAL_CAMERA_ERROR,	   
      
} CameraGUIError;


//
// Description:
//   Type of PGRCameraGUI settings dialog to display.  
//
// Remarks:
//   This enum will be deprecated in the next version.  Please use the 
//   SDK-specific .LIB and .DLL (see note above) and the new 
//   pgrcamguiInitializeSettingsDialog() instead of pgrcamguiCreateSettingsDialog().
//
typedef enum CameraGUIType
{
   // PGRFlyCapture settings.
   PGRCAMGUI_TYPE_PGRFLYCAPTURE,
   // Digiclops Settings.
   PGRCAMGUI_TYPE_DIGICLOPS,
   // Ladybug settings.
   PGRCAMGUI_TYPE_LADYBUG,

} CameraGUIType;


//=============================================================================
// API Functions
//=============================================================================
//
//-----------------------------------------------------------------------------
//
// Name: pgrcamguiCreateContext()
//    
// Description:
//    Allocates a PGRCameraGUI handle to be used in all successive calls.
//    This function must be called before any other functions.
//
// Arguments:
//    pcontext - a pointer to a PGRCameraGUI context to be created.
//          
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiCreateContext(
		       CameraGUIContext* pcontext );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiDestroyContext()
//    
// Description:
//    Frees memory associated with a given context.
//
// Arguments:
//    context - context to destroy.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_INVALID_CONTEXT - if context is null.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiDestroyContext(
			CameraGUIContext context );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiShowCameraSelectionModal()
//    
// Description:
//    Displays the camera selection dialog.
//
// Arguments:
//    context           - The PGRCameraGUI context to access.
//    camcontext        - The SDK-specific context to use, casted appropriately.
//                        (ie, FlycaptureContext, DigiclopsContext, etc.)
//    pulSerialNumber   - Pointer to the returned serial number of the selected
//                        camera.
//    pipDialogStatus   - Status returned from the DoModal() call from the dialog.
//                        Use this to check for "Ok" or "Cancel."
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_FAILED - if the function failed.
//    PGRCAMGUI_INVALID_CONTEXT - if context is NULL.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiShowCameraSelectionModal(
				  CameraGUIContext       context,
				  GenericCameraContext   camcontext,
				  unsigned long*         pulSerialNumber,
				  INT_PTR*               pipDialogStatus );

//-----------------------------------------------------------------------------
//
// Name: pgrcamguiCreateSettingsDialog()
//    
// Description:
//   This function is DEPRECATED.  Please use 
//   pgrcamguiInitializeSettingsDialog()
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_FAILED - if the function failed.
//    PGRCAMGUI_INVALID_CONTEXT - if context is NULL.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiCreateSettingsDialog(
			      CameraGUIContext	   context,
			      CameraGUIType	   type,
			      GenericCameraContext camcontext );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiInitializeSettingsDialog()
//    
// Description:
//    Creates the settings dialog.  Call this before calling the either of the
//    other two functions dealing with the Settings dialog.
//
// Arguments:
//    context     - The PGRCameraGUI context to access.
//    camcontext  - The SDK-level context to use.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_FAILED - if the function failed.
//    PGRCAMGUI_INVALID_CONTEXT - if context is null.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiInitializeSettingsDialog(
                                  CameraGUIContext       context,
                                  GenericCameraContext   camcontext );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiToggleSettingsWindowState()
//    
// Description:
//    Displays or hides the modeless settings dialog.
//
// Arguments:
//    context     - The PGRCameraGUI context to access.
//    hwndParent  - Handle to the parent window.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_FAILED - if the function failed.
//    PGRCAMGUI_INVALID_CONTEXT - if context is null.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiToggleSettingsWindowState(  
				   CameraGUIContext   context,
				   HWND		      hwndParent );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiGetSettingsWindowState()
//    
// Description:
//    Retrieves the state of the settings dialog.
//
// Arguments:
//    context     - The PGRCameraGUI context to access.
//    pbShowing   - A pointer to the returned state of the settings dialog.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_FAILED - if the function failed.
//    PGRCAMGUI_INVALID_CONTEXT - if context is null.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiGetSettingsWindowState(
				CameraGUIContext   context,
				BOOL*		   pbShowing );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiSetSettingsWindowHelpPrefix()
//    
// Description:
//    Enables context sensitive help in the settings dialog by specifying the 
//    help prefix to append topic specific pages to. eg:
//    "..\\doc\\FlyCapture SDK help.chm::/FlyCap Demo Program/Camera Control Dialog"
//    to which strings like "/Look Up Table.html" will be appended.
//
// Arguments:
//    context       - The PGRCameraGUI context to access.
//    pszHelpPrefix - a pointer to a string containing the desired help prefix
//                    if NULL the context sensitive help will be unavailable.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    PGRCAMGUI_FAILED - if the function failed.
//    PGRCAMGUI_INVALID_CONTEXT - if context is null.
//    
PGRCAMERAGUI_API CameraGUIError
pgrcamguiSetSettingsWindowHelpPrefix(
                                     CameraGUIContext context,
                                     const char* pszHelpPrefix );


//-----------------------------------------------------------------------------
//
// Name: pgrcamguiShowInfoDlg()
//    
// Description:
//    Displays a modal dialog that displays PGR version information.
//
// Arguments:
//    context     - The PGRCameraGUI context to access.
//    camcontext  - The SDK-level context to use.
//    hwndParent  - Handle to the parent window.
//    pszAppName  - Pointer to a string that will be prepended to the version
//                  information and seperated by a newline.
//
// Returns:
//    PGRCAMGUI_OK - upon successful completion.
//    
PGRCAMERAGUI_API CameraGUIError 
pgrcamguiShowInfoDlg(
                     CameraGUIContext      context,
                     GenericCameraContext  camcontext,
                     HWND                  hwndParent,
                     char*                 pszAppName = NULL );

#ifdef __cplusplus
};
#endif

#endif // !__PGRCAMERAGUI_H__
