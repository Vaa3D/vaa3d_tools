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
// $Id: FlyCap2MFCView.h,v 1.5 2009/10/23 20:28:11 soowei Exp $
//=============================================================================

#pragma once

#include "FrameRateCounter.h"

#include <GL/gl.h>
#include <GL/glu.h>

class CFlyCap2MFCView : public CView
{
public:
	CFlyCap2MFCDoc* GetDocument() const;
    double GetDisplayedFrameRate();
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual ~CFlyCap2MFCView();
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    FrameRateCounter m_displayedFrameRate;

    bool m_openGLInitialized;
    bool m_enableOpenGL;

    /** OpenGL rendering context. */
    HGLRC m_hRC;

    /** Device context for drawing. */
    CDC* m_pDC;

    static const unsigned int sk_maxNumTextures = 16;
    GLuint m_imageTextures[sk_maxNumTextures];

    bool InitializeOpenGL();
    bool SetupPixelFormat();

    void BindGL( );
    void UnbindGL( );

    CFlyCap2MFCView();
    DECLARE_DYNCREATE(CFlyCap2MFCView)

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
    virtual void OnInitialUpdate();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // debug version in FlyCap2MFCView.cpp
inline CFlyCap2MFCDoc* CFlyCap2MFCView::GetDocument() const
   { return reinterpret_cast<CFlyCap2MFCDoc*>(m_pDocument); }
#endif

