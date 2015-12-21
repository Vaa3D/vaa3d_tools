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
// $Id: FlyCapture2GUI_C.h,v 1.4 2010/04/13 21:31:12 hirokim Exp $
//=============================================================================

#ifndef PGR_FC2_FLYCAPTURE2GUI_C_H
#define PGR_FC2_FLYCAPTURE2GUI_C_H

//=============================================================================
// Global C header file for FlyCapture2. 
//
// This file defines the C API for FlyCapture2 GUI
//=============================================================================

#include "FlyCapture2Defs_C.h"
#include "FlyCapture2Platform_C.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Create a GUI context.
 *
 * @param pContext Pointer to context to be created.
 *
 * @return An Error indicating the success or failure of the function.
 */ 
FLYCAPTURE2_C_API fc2Error 
fc2CreateGUIContext(
    fc2GuiContext* pContext );

/**
 * Destroy a GUI context.
 *
 * @param context Context to be destroyed.
 *
 * @return An Error indicating the success or failure of the function.
 */
FLYCAPTURE2_C_API fc2Error 
fc2DestroyGUIContext(
    fc2GuiContext context );

/**
 * Connect GUI context to a camera context.
 *
 * @param context GUI context to connect.
 * @param cameraContext Camera context to connect.
 *
 * @return An Error indicating the success or failure of the function.
 */
FLYCAPTURE2_C_API void 
fc2GUIConnect( 
    fc2GuiContext context, 
    fc2Context cameraContext );

/**
 * Disconnect GUI context from camera.
 *
 * @param context GUI context to disconnect.
 *
 * @return An Error indicating the success or failure of the function.
 */
FLYCAPTURE2_C_API void 
fc2Disonnect( 
    fc2GuiContext context);

/**
 * Show the GUI.
 *
 * @param context Pointer to context to show.
 *
 * @return An Error indicating the success or failure of the function.
 */
FLYCAPTURE2_C_API void 
fc2Show( 
    fc2GuiContext context);

/**
 * Hide the GUI.
 *
 * @param context Pointer to context to hide.
 *
 * @return An Error indicating the success or failure of the function.
 */
FLYCAPTURE2_C_API void 
fc2Hide( 
    fc2GuiContext context);

/**
 * Check if the GUI is visible.
 *
 * @param context Pointer to context to show.
 *
 * @return Whether the GUI is visible.
 */
FLYCAPTURE2_C_API BOOL 
fc2IsVisible( 
    fc2GuiContext context);

/**
 * Show the camera selection dialog.
 *
 * @param context Pointer to context to show.
 * @param pOkSelected Whether Ok (true) or Cancel (false) was clicked.
 * @param guidArray Array of PGRGuids containing the selected cameras.
 * @param size Size of PGRGuid array.
 */
FLYCAPTURE2_C_API void 
fc2ShowModal( 
    fc2GuiContext context, 
    BOOL* pOkSelected, 
    fc2PGRGuid* guidArray, 
    unsigned int* size );

#ifdef __cplusplus
};
#endif

#endif // PGR_FC2_FLYCAPTURE2GUI_C_H

