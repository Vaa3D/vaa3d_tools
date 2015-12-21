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
// $Id: FlyCapture2GUI.h,v 1.15 2010/03/23 22:17:37 soowei Exp $
//=============================================================================

#ifndef PGR_FC2_FLYCAPTURE2GUI_H
#define PGR_FC2_FLYCAPTURE2GUI_H

#include "FlyCapture2Platform.h"
#include "FlyCapture2Defs.h"

namespace FlyCapture2 
{   
    // Forward declaration
    class CameraBase;
 
    /**
     * The CameraControlDlg object represents a GTKmm dialog that provides
     * a graphical interface to a specified camera.
     */ 
    class FLYCAPTURE2_API CameraControlDlg
    {
    public:

        /**
         * Default constructor.
         */
        CameraControlDlg();

        /**
         * Default destructor.
         */
        ~CameraControlDlg();

        /**
         * Connect dialog to a camera.
         *
         * @param pCamera Camera object to connect the dialog to.
         */ 
        void Connect( CameraBase* pCamera );

        /**
         * Disconnect a connected camera from the dialog.
         */ 
        void Disconnect();

        /**
         * Show the dialog.
         */ 
        void Show();

        /**
         * Hide the dialog
         */ 
        void Hide();

        /**
         * Get the visibility of the dialog.
         *
         * @return Whether the dialog is visible.
         */ 
        bool IsVisible();

    private:
        struct CamCtlData; // Forward declaration

        CamCtlData* m_pCamCtlData;

        CameraControlDlg( const CameraControlDlg& );
        CameraControlDlg& operator=( const CameraControlDlg& );
    };   
  
    /**
     * The CameraSelectionDlg object represents a GTKmm dialog that provides
     * a graphical interface that lists the number of cameras available to
     * the library.
     */ 
    class FLYCAPTURE2_API CameraSelectionDlg
    {
    public:
        /**
         * Default constructor.
         */
        CameraSelectionDlg();

        /**
         * Default destructor.
         */
        ~CameraSelectionDlg();

        /**
         * Show the CameraSelectionDlg.
         *
         * @param pOk Whether Ok (true) or Cancel (false) was clicked.
         * @param pGuid Array of PGRGuids containing the selected cameras.
         * @param pSize Size of PGRGuid array.
         */ 
        void ShowModal( bool* pOk, PGRGuid* pGuid, unsigned int* pSize );

    private:
        struct CamSelectionData; // Forward declaration

        CamSelectionData* m_pCamSelectionData;

        CameraSelectionDlg( const CameraSelectionDlg& );
        CameraSelectionDlg& operator=( const CameraSelectionDlg& );
    };
}

#endif // PGR_FC2_FLYCAPTURE2GUI_H
