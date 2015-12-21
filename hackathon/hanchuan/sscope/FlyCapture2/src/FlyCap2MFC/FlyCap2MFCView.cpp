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
// $Id: FlyCap2MFCView.cpp,v 1.13 2010/09/22 22:48:53 soowei Exp $
//=============================================================================

#include "stdafx.h"
#include "FlyCap2MFC.h"

#include "FlyCap2MFCDoc.h"
#include "FlyCap2MFCView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int GetMinimumPowerOfTwo(int in)
{
    int i = 1;
    while ( i < in)
    {
        i *= 2;
    }

    return i;
}

void OutputGLError( char* pszLabel )
{
    GLenum errorno = glGetError();

    if ( errorno != GL_NO_ERROR )
    {
        char msg[256];
        sprintf( msg,
            "%s had error: #(%d) %s\r\n", 
            pszLabel, 
            errorno, 
            gluErrorString( errorno ) );
        AfxMessageBox( msg, MB_OK);
    }
}


// CFlyCap2MFCView

IMPLEMENT_DYNCREATE(CFlyCap2MFCView, CView)

BEGIN_MESSAGE_MAP(CFlyCap2MFCView, CView)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CFlyCap2MFCView construction/destruction

CFlyCap2MFCView::CFlyCap2MFCView()
{
    m_enableOpenGL = true;
    m_openGLInitialized = false;    
}

CFlyCap2MFCView::~CFlyCap2MFCView()
{
    glDeleteTextures( sk_maxNumTextures, m_imageTextures );
}

#ifdef _DEBUG
void CFlyCap2MFCView::AssertValid() const
{
    CView::AssertValid();
}

void CFlyCap2MFCView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CFlyCap2MFCDoc* CFlyCap2MFCView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFlyCap2MFCDoc)));
    return (CFlyCap2MFCDoc*)m_pDocument;
}

#endif //_DEBUG

BOOL CFlyCap2MFCView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

void CFlyCap2MFCView::OnDraw(CDC* pDC)
{
    CFlyCap2MFCDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    ((CMainFrame*)GetParentFrame())->ResizeToFit();
    ((CMainFrame*)GetParentFrame())->UpdateStatusBar();

    // Transfer the RGB buffer to graphics card.
    int width = pDoc->m_bitmapInfo.bmiHeader.biWidth;
    int height = ::abs( pDoc->m_bitmapInfo.bmiHeader.biHeight );

    CSingleLock dataLock( &pDoc->m_csData );
    dataLock.Lock();

    PUCHAR pImageData = pDoc->GetProcessedPixels();
    if( pImageData == NULL )
    {
        return;
    }
    
    if ( m_enableOpenGL)
    {
        double validTextureWidth = 1.0;
        double validTextureHeight = 1.0;
        bool useTiledTextures = false;
        GLenum errorno;

        BindGL( );
        glEnable( GL_TEXTURE_2D );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        glBindTexture( GL_TEXTURE_2D, m_imageTextures[0] );
        glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GL_RGB, 
            width, 
            height, 
            0, 
            GL_BGR_EXT, 
            GL_UNSIGNED_BYTE,
            pDoc->GetProcessedPixels() );
        errorno = glGetError();
        if ( errorno != GL_NO_ERROR)
        {
            // Attempt to fall back and use a power-of-two sized texture.
            // This is for older cards that don't support more arbitrary
            // texture sizes.

            const int textureWidth = GetMinimumPowerOfTwo(width);
            const int textureHeight = GetMinimumPowerOfTwo(height);
            validTextureWidth = (double)width / textureWidth;
            validTextureHeight = (double)height / textureHeight;

            glTexImage2D(
                GL_TEXTURE_2D, 
                0, 
                GL_RGB, 
                textureWidth, 
                textureHeight, 
                0, 
                GL_BGR_EXT, 
                GL_UNSIGNED_BYTE,
                NULL );
            errorno = glGetError();
            if ( errorno != GL_NO_ERROR)
            {
                // The graphics doesn't seem to support this texture size.
                // Images must be split and then tiled.
                useTiledTextures = true;
            }
            else
            {
                glTexSubImage2D( 
                    GL_TEXTURE_2D, 
                    0, 
                    0, 
                    0,
                    width, 
                    height,
                    GL_BGR_EXT, 
                    GL_UNSIGNED_BYTE, 
                    pDoc->GetProcessedPixels() );
                errorno = glGetError();
                if ( errorno != GL_NO_ERROR)
                {
                    // Error
                }
            }
        }

        if ( useTiledTextures)
        {
            //
            // The image is split into multiple textures.
            //
            int bytesPerPixel = 3;
            int tileSize = 1024;
            int horizResidual = width % tileSize;
            int vertResidual = height % tileSize;
            int numHorizTextures = width / tileSize + ( horizResidual > 0);
            int numVertTextures = height / tileSize + ( vertResidual > 0);

            unsigned char *tileBuffer = new unsigned char [ tileSize * tileSize * bytesPerPixel];
            for ( int tileY = 0; tileY < numVertTextures ; tileY++)
            {
                for ( int tileX = 0; tileX < numHorizTextures; tileX++)
                {
                    int subTexHeight = tileSize;
                    if (  tileY == numVertTextures - 1 && vertResidual > 0)
                        subTexHeight = vertResidual;

                    int subTexWidth = tileSize;
                    if ( tileX == numHorizTextures - 1 && horizResidual > 0)
                        subTexWidth = horizResidual;

                    // copy image buffer to the tile
                    for ( int line = 0; line < subTexHeight; line++)
                    {
                        memcpy( tileBuffer + line * tileSize * bytesPerPixel, 
                            pDoc->GetProcessedPixels() + ( ( line + tileSize * tileY) * width + tileSize * tileX) * bytesPerPixel, 
                            subTexWidth * bytesPerPixel);
                    }

                    int texId = tileY * numHorizTextures + tileX;
                    if ( texId >= sk_maxNumTextures)
                        continue;

                    glBindTexture( GL_TEXTURE_2D, m_imageTextures[ texId] );
                    glTexImage2D(
                        GL_TEXTURE_2D, 
                        0, 
                        GL_RGB, 
                        tileSize, 
                        tileSize, 
                        0, 
                        GL_BGR_EXT, 
                        GL_UNSIGNED_BYTE,
                        tileBuffer );

                    double x_begin = (double)tileSize / width * tileX;
                    double x_end = (double)tileSize / width * ( tileX + 1);
                    double y_begin = 1.0 - (double)tileSize / height * ( tileY + 1);
                    double y_end = 1.0 - (double)tileSize / height * tileY;

                    glBegin( GL_QUADS );

                    glTexCoord2d( 0.0, 1.0 );
                    glVertex2d( x_begin, y_begin );

                    glTexCoord2d( 1.0, 1.0 );
                    glVertex2d( x_end, y_begin );

                    glTexCoord2d( 1.0, 0.0 );
                    glVertex2d( x_end, y_end );

                    glTexCoord2d( 0.0, 0.0 );
                    glVertex2d( x_begin, y_end );

                    glEnd();
                }
            }

            delete [] tileBuffer;
        }
        else
        {
            // Just one texture
            glBegin( GL_QUADS );

            glTexCoord2d( 0.0, validTextureHeight );
            glVertex2d( 0.0, 0.0 );

            glTexCoord2d( validTextureWidth, validTextureHeight );
            glVertex2d( 1.0, 0.0 );

            glTexCoord2d( validTextureWidth, 0.0 );
            glVertex2d( 1.0, 1.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex2d( 0.0, 1.0 );

            glEnd();
        }

        glEnd();

        SwapBuffers( pDC->m_hDC );
        UnbindGL( );
    }
    else
    {
        // non-OpenGL drawing
        if( ::SetDIBitsToDevice(
            pDC->GetSafeHdc(),
            0, 
            0,
            pDoc->m_bitmapInfo.bmiHeader.biWidth, 
            ::abs( pDoc->m_bitmapInfo.bmiHeader.biHeight ),
            0, 
            0,
            0, 
            ::abs( pDoc->m_bitmapInfo.bmiHeader.biHeight ),
            pDoc->GetProcessedPixels(), 
            &pDoc->m_bitmapInfo, 
            DIB_RGB_COLORS )  == 0 )
        {
            // error.
        }
    }

    m_displayedFrameRate.NewFrame();

    dataLock.Unlock();
}

double CFlyCap2MFCView::GetDisplayedFrameRate()
{
    return m_displayedFrameRate.GetFrameRate();
}

void CFlyCap2MFCView::OnInitialUpdate()
{
    CView::OnInitialUpdate();

    CFlyCap2MFCDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    m_openGLInitialized = InitializeOpenGL();

    RECT clientRect;
    GetClientRect( &clientRect );

    // Resize the window to properly display the image
    GetParentFrame()->SetWindowPos( NULL,
        0, 0, clientRect.right, clientRect.right/2,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
}

int CFlyCap2MFCView::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    if ( m_enableOpenGL)
    {
        if( !InitializeOpenGL() )
        {
            return -1;
        }
        else
        {
            m_openGLInitialized = true;
        }
    }

    return 0;
}

void CFlyCap2MFCView::OnDestroy()
{
    CView::OnDestroy();

    if (m_openGLInitialized)
    {
        // Make the RC non-current
        UnbindGL( );

        // Delete the rendering context
        if ( ::wglDeleteContext( m_hRC ) == FALSE )
        {
            MessageBox("Could not Make RC non-Current.");
        }

        // Delete DC
        if ( m_pDC )
        {
            delete m_pDC;
            m_pDC = NULL;
        }        
    }
}

void CFlyCap2MFCView::OnSize( UINT nType, int cx, int cy )
{
    CView::OnSize(nType, cx, cy);

    if( ( cx <= 0 ) || ( cy <= 0 ) )
    {
        return;
    }

    if ( m_enableOpenGL)
    {
        BindGL( );
        ::glViewport( 0, 0, cx, cy );
        UnbindGL( );
    }
}

BOOL CFlyCap2MFCView::OnEraseBkgnd( CDC* /*pDC*/ )
{
    return 0;
}

bool CFlyCap2MFCView::InitializeOpenGL()
{
    if ( m_openGLInitialized )
    {
        // Nothing to be done here
        return true;
    }

    // Get a DC for the Client Area
    m_pDC = new CClientDC( this );
    if ( m_pDC == NULL )
    {
        MessageBox("Error Obtaining DC");
        return false;
    }

    // Set Pixel Format
    if ( !SetupPixelFormat() )
    {
        return false;
    }

    // Create Rendering Context
    m_hRC = ::wglCreateContext( m_pDC->GetSafeHdc() );
    if( m_hRC == NULL )
    {
        MessageBox("Error Creating RC.");;
        return false;
    }

    // initialize OGL texture
    BindGL();

#define GL_CLAMP_TO_EDGE 0x812F

    glGenTextures( sk_maxNumTextures, m_imageTextures );

    bool useClampToEdge = true;
    if ( atof( (const char*)glGetString(GL_VERSION)) < 1.15)
    {
        useClampToEdge = false;
    }

    for ( int i = 0; i < sk_maxNumTextures; i++)
    {
        glBindTexture( GL_TEXTURE_2D, m_imageTextures[ i] );

        if ( useClampToEdge)
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        }
        else
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    }

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glShadeModel( GL_FLAT );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    //
    // initialize matrices
    //
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0, 1, 0, 1 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    UnbindGL();

    return true; 
}

bool CFlyCap2MFCView::SetupPixelFormat()
{
    // Fill in the Pixel Format Descriptor
    PIXELFORMATDESCRIPTOR pfd;
    memset( &pfd, 0x0, sizeof( PIXELFORMATDESCRIPTOR ) );

    pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
    pfd.nVersion = 1;
    pfd.dwFlags =	
        PFD_DOUBLEBUFFER |
        PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cAlphaBits = 0;
    pfd.cDepthBits = 0;

    int nPixelFormat = ::ChoosePixelFormat( m_pDC->m_hDC, &pfd );
    if( nPixelFormat == 0 )
    {
        ASSERT( FALSE );
        return false;
    }

    if( !::SetPixelFormat( m_pDC->m_hDC, nPixelFormat, &pfd ) )
    {
        ASSERT( FALSE );
        return false;
    }

    return true;
}

void CFlyCap2MFCView::BindGL()
{
    if( !::wglMakeCurrent( m_pDC->m_hDC, m_hRC ) )
    {
        MessageBox("Error binding OpenGL.");
    }
}

void CFlyCap2MFCView::UnbindGL()
{
    if( !::wglMakeCurrent( m_pDC->m_hDC, NULL ) )
    {
        MessageBox("Error unbinding OpenGL.");
    }
}
