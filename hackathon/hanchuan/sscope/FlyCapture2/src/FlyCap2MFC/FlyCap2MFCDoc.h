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
// $Id: FlyCap2MFCDoc.h,v 1.15 2010/09/22 22:49:37 soowei Exp $
//=============================================================================

#include "FrameRateCounter.h"

//
// Size of the window when it the application first starts.
//
#define _DEFAULT_WINDOW_X  640
#define _DEFAULT_WINDOW_Y  480

#pragma once

class CFlyCap2MFCDoc : public CDocument
{
protected: // create from serialization only
	CFlyCap2MFCDoc();
	DECLARE_DYNCREATE(CFlyCap2MFCDoc)

public:
    virtual ~CFlyCap2MFCDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // Critical section to protect access to the processed image
    CCriticalSection m_csData;

    // Structure used to draw to the screen.
    BITMAPINFO        m_bitmapInfo;  

    // Get the processed frame rate
    double GetProcessedFrameRate();

    // Get the data pointer to the image
    unsigned char* GetProcessedPixels();

    // Get the dimensions of the image
    void GetImageSize( unsigned int* pWidth, unsigned int* pHeight );

    // Initialize the bitmap struct used for drawing.
    void InitBitmapStruct( int cols, int rows );

    // The image grab thread.
    static UINT ThreadGrabImage( void* pparam );

    // The object grab image loop.  Only executed from within the grab thread.
    UINT DoGrabLoop();

    // Redraw all the views in the application
    void RedrawAllViews();

    virtual BOOL OnNewDocument();
    virtual void OnCloseDocument(void);

protected:
    FlyCapture2::CameraBase* m_pCamera;  
    FlyCapture2::CameraInfo m_cameraInfo;
    FlyCapture2::Image m_rawImage;
    FlyCapture2::Image m_saveImage;
    FlyCapture2::Image m_processedImage;

    FlyCapture2::CameraControlDlg m_camCtlDlg;

    bool m_continueGrabThread;

    bool m_beingSaved;

    HANDLE m_heventThreadDone;

    FrameRateCounter m_processedFrameRate;

private:
    unsigned int m_prevWidth;
    unsigned int m_prevHeight;

    // Keeps track of the last filter index used for image saving.
    unsigned int m_uiFilterIndex;    

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnToggleCameraControl();
    afx_msg void OnFileSaveAs();
};


