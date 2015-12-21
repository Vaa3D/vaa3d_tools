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
// $Id: ImageDrawingArea.cpp,v 1.47 2010/09/23 18:06:57 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "ImageDrawingArea.h"
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>

// in order to get function prototypes from glext.h, define GL_GLEXT_PROTOTYPES before including glext.h
#define GL_GLEXT_PROTOTYPES
#include "glext.h"

// function pointers for PBO Extension
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.
#ifdef _WIN32
PFNGLGENBUFFERSARBPROC pglGenBuffersARB = 0;                     // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC pglBindBufferARB = 0;                     // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC pglBufferDataARB = 0;                     // VBO Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB = 0;               // VBO Deletion Procedure
#define glGenBuffersARB           pglGenBuffersARB
#define glBindBufferARB           pglBindBufferARB
#define glBufferDataARB           pglBufferDataARB
#define glDeleteBuffersARB        pglDeleteBuffersARB
#endif

/** 
 * This function is required due to a bug in GTKmm that causes the on_realize()
 * event to never be called. Creating the widget this way allows the event
 * to be properly called.
 */
extern "C" G_MODULE_EXPORT
GtkWidget* CreateDisplay(char*, char*, char*, int, int)
{
    Gtk::Widget *const widget = new ImageDrawingArea();
    widget->show();
    return Gtk::manage(widget)->gobj();
}

int GetMinimumPowerOfTwo(int in)
{
    int i = 1;
    while ( i < in)
    {
        i *= 2;
    }

    return i;
}

ImageDrawingArea::ImageDrawingArea(BaseObjectType* /*cobject*/, const Glib::RefPtr<Gnome::Glade::Xml>& /*refGlade*/)
: Gtk::GL::DrawingArea(),
m_displayedFrameRate(60),
m_moveCursor( Gdk::FLEUR )
{
    CommonInit();
}

ImageDrawingArea::ImageDrawingArea()
{
    CommonInit();
}

ImageDrawingArea::~ImageDrawingArea()
{
#ifdef _WIN32
    if ( m_PBOSupported)
    {
        glDeleteBuffersARB( 1, &m_PBO);
    }
#endif

    DestroyImageTexture();
}

sigc::signal<void, double, double> ImageDrawingArea::signal_offset_changed()
{
    return m_signal_offset_changed;
}

bool ImageDrawingArea::on_button_press_event(GdkEventButton* event)
{
    switch ( event->button )
    {
    case 1:
        m_leftMBHeld = true;
        get_window()->set_cursor( m_moveCursor );
        m_startX = event->x;
        m_startY = event->y;

    default:
        break;
    }

    return true;
}

bool ImageDrawingArea::on_button_release_event( GdkEventButton* event )
{
    switch ( event->button )
    {
    case 1:
        m_leftMBHeld = false;
        get_window()->set_cursor();
        break;
    default:
        break;
    }
    return true;
}

bool ImageDrawingArea::on_motion_notify_event( GdkEventMotion* event )
{
    const unsigned int absCurrXPos = static_cast<unsigned int>(event->x);
    const unsigned int absCurrYPos = static_cast<unsigned int>(event->y);

    // Get the drawing area size
    int screenWidth;
    int screenHeight;
    get_window()->get_size( screenWidth, screenHeight );

    if ( m_leftMBHeld && !m_stretchToFit)
    {
        // handle mouse-dragging
        m_shiftX += (double)( event->x - m_startX) / screenWidth;
        m_shiftY -= (double)( event->y - m_startY) / screenHeight;

        m_startX = event->x;
        m_startY = event->y;

        LimitShift();

        double sx, sy;
        GetImageShift( sx, sy);
        m_signal_offset_changed( sx, sy); 
    }


    // obtain image coordinate from cursor position
    double shiftX = ( m_scaleX - 1.0) / 2.0 - m_shiftX;
    double shiftY = ( m_scaleY - 1.0) / 2.0 + m_shiftY;
    m_xPos = static_cast<int>( ( static_cast<float>( absCurrXPos) / screenWidth + shiftX) * m_imageWidth / m_scaleX);
    m_yPos = static_cast<int>( ( static_cast<float>( absCurrYPos) / screenHeight + shiftY) * m_imageHeight / m_scaleY);

    if ( m_xPos < 0 || m_xPos >= static_cast<int>(m_imageWidth) || m_yPos < 0 || m_yPos >= static_cast<int>(m_imageHeight))
    {
        m_xPos = -1;
        m_yPos = -1;
    }

    return true;
}

void ImageDrawingArea::SetImageSize( unsigned int width, unsigned int height )
{
    m_imageWidth = width;
    m_imageHeight = height;

    on_configure_event( NULL );
}

void ImageDrawingArea::SetStretchToFit( bool stretch )
{
    m_stretchToFit = stretch;

    m_shiftX = 0.0;
    m_shiftY = 0.0;
}

void ImageDrawingArea::SetShowCrosshair( bool show )
{
    m_showCrosshair = show;
}

void ImageDrawingArea::SetCrosshairColor( Gdk::Color color )
{
    m_colorCrosshair = color;
}

Gdk::Color ImageDrawingArea::GetCrosshairColor()
{
    return m_colorCrosshair;
}

void ImageDrawingArea::GetMouseCoordinates( int* pXPos, int* pYPos )
{
    *pXPos = m_xPos;
    *pYPos = m_yPos;
}

bool ImageDrawingArea::on_expose_event( GdkEventExpose* /*event*/ )
{
    if ( m_pixBuf == NULL )
    {
        return true;
    }      

    Glib::Mutex::Lock lock(m_pixBufMutex);
    
    const int width = m_pixBuf->get_width();
    const int height = m_pixBuf->get_height();     
    double validTextureWidth = 1.0;
    double validTextureHeight = 1.0;
    bool useTiledTextures = false;
    GLenum errorno;

    BeginGL();

    SetImageSize( width, height );

    int screenWidth = 0;
    int screenHeight = 0;
    get_window()->get_size( screenWidth, screenHeight );

    // This makes sure that sampling for rendering doesn't occur on the border of pixels.
    const double halfPixelAdjustW = 0.25 / (double)screenWidth;
    const double halfPixelAdjustH = 0.25 / (double)screenHeight;

    glEnable( GL_TEXTURE_2D );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // center the image
    glTranslated( 
        -(m_scaleX - 1.0)/2.0 + halfPixelAdjustW, 
        -(m_scaleY - 1.0)/2.0 + halfPixelAdjustH, 0.0);

    // apply mouse-drag shift
    glTranslated(m_shiftX, m_shiftY, 0.0);

    // scale the image
    glScaled(m_scaleX, m_scaleY, 0.0);

    // draw the image
    glBindTexture(GL_TEXTURE_2D, m_imageTextures[0]);

#ifdef _WIN32
    if (m_PBOSupported)
    {
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_PBO);
        errorno = glGetError();

        if (errorno != GL_NO_ERROR)
        {
            m_PBOSupported = false;
        }
        else
        {
            glBufferDataARB( 
               GL_PIXEL_UNPACK_BUFFER_ARB, 
               width * height * 4, 
               m_pixBuf->get_pixels(),
               GL_STREAM_DRAW_ARB);
            errorno = glGetError();

            if (errorno != GL_NO_ERROR)
            {
                m_PBOSupported = false;
            }
        }
    }
#endif

    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA, 
        width, 
        height, 
        0, 
        GL_BGRA_EXT, 
        GL_UNSIGNED_BYTE,
        m_PBOSupported ? NULL : m_pixBuf->get_pixels() );
    errorno = glGetError();

    if (errorno != GL_NO_ERROR)
    {
        // Attempt to fall back and use a power-of-two sized texture.
        // This is for older cards that don't support more arbitrary
        // texture sizes.

#ifdef _WIN32
        if (m_PBOSupported)
        {
            // unbind PBO to use normal texture transfer
            glBindBufferARB( GL_PIXEL_UNPACK_BUFFER_ARB, 0);
        }
#endif

        const int textureWidth = GetMinimumPowerOfTwo(width);
        const int textureHeight = GetMinimumPowerOfTwo(height);
        validTextureWidth = (double)width / textureWidth;
        validTextureHeight = (double)height / textureHeight;

        glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GL_RGBA, 
            textureWidth, 
            textureHeight, 
            0, 
            GL_BGRA_EXT, 
            GL_UNSIGNED_BYTE,
            NULL );
        errorno = glGetError();
        if (errorno != GL_NO_ERROR)
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
                GL_BGRA_EXT, 
                GL_UNSIGNED_BYTE, 
                m_pixBuf->get_pixels() );
            errorno = glGetError();
            if ( errorno != GL_NO_ERROR)
            {
                // Error
            }
        }
    }

    if (useTiledTextures)
    {
        //
        // The image is split into multiple textures.
        //
        const int bytesPerPixel = 4;
        const int tileSize = 1024;
        int horizResidual = width % tileSize;
        int vertResidual = height % tileSize;
        int numHorizTextures = width / tileSize + (horizResidual > 0);
        int numVertTextures = height / tileSize + (vertResidual > 0);

        unsigned char *tileBuffer = new unsigned char[tileSize * tileSize * bytesPerPixel];

        for (int tileY = 0; tileY < numVertTextures ; tileY++)
        {
            for (int tileX = 0; tileX < numHorizTextures; tileX++)
            {
                int subTexHeight = tileSize;
                if ( tileY == numVertTextures - 1 && vertResidual > 0)
                {
                    subTexHeight = vertResidual;
                }

                int subTexWidth = tileSize;
                if (tileX == numHorizTextures - 1 && horizResidual > 0)
                {
                    subTexWidth = horizResidual;
                }

                // copy image buffer to the tile
                for (int line = 0; line < subTexHeight; line++)
                {
                    memcpy( 
                        tileBuffer + line * tileSize * bytesPerPixel, 
                        m_pixBuf->get_pixels() + ((line + tileSize * tileY) * width + tileSize * tileX) * bytesPerPixel, 
                        subTexWidth * bytesPerPixel);
                }

                const unsigned int texId = tileY * numHorizTextures + tileX;
                if (texId >= sk_maxNumTextures)
                {
                    continue;
                }

                glBindTexture( GL_TEXTURE_2D, m_imageTextures[ texId] );
                glTexImage2D(
                    GL_TEXTURE_2D, 
                    0, 
                    GL_RGBA, 
                    tileSize, 
                    tileSize, 
                    0, 
                    GL_BGRA_EXT, 
                    GL_UNSIGNED_BYTE,
                    tileBuffer );

                const double validTileWidth = (double)subTexWidth/tileSize;
                const double validTileHeight = (double)subTexHeight/tileSize;

                const double x_begin = (double)tileSize / width * tileX;
                const double x_end = (double)tileSize / width * ( tileX + validTileWidth);
                const double y_begin = 1.0 - (double)tileSize / height * ( tileY + validTileHeight);
                const double y_end = 1.0 - (double)tileSize / height * tileY;

                glBegin( GL_QUADS );

                glTexCoord2d(0.0, validTileHeight);
                glVertex2d(x_begin, y_begin);

                glTexCoord2d(validTileWidth, validTileHeight);
                glVertex2d(x_end, y_begin);

                glTexCoord2d(validTileWidth, 0.0);
                glVertex2d(x_end, y_end);

                glTexCoord2d(0.0, 0.0);
                glVertex2d(x_begin, y_end);

                glEnd();
            }
        }

        delete [] tileBuffer;
    }
    else
    {
        // Just one texture
        glBegin( GL_QUADS );

        glTexCoord2d(0.0, validTextureHeight);
        glVertex2d(0.0, 0.0);

        glTexCoord2d(validTextureWidth, validTextureHeight);
        glVertex2d(1.0, 0.0);

        glTexCoord2d(validTextureWidth, 0.0);
        glVertex2d(1.0, 1.0);

        glTexCoord2d(0.0, 0.0);
        glVertex2d(0.0, 1.0);

        glEnd();
    }

    if ( m_showCrosshair )
    {
        const double aspectRatio = (double)width/height;
        glTranslated(0.5, 0.5, 0.0);
        glScaled(1.0, aspectRatio, 0.0);       
        ShowCrosshair();
    }   

    if (get_gl_window()->is_double_buffered())
    {
        get_gl_window()->swap_buffers();
    }
    else
    {
        glFlush();
    }   

    EndGL();

    lock.release();     

    m_displayedFrameRate.NewFrame();

    return true;
}

void ImageDrawingArea::SetPixBuf( Glib::RefPtr<Gdk::Pixbuf> refPixBuf )
{
    Glib::Mutex::Lock lock(m_pixBufMutex);

    m_pixBuf = refPixBuf;
}

void ImageDrawingArea::ShowCrosshair()
{         
    glDisable( GL_TEXTURE_2D );

    glColor3d( 
        m_colorCrosshair.get_red_p(), 
        m_colorCrosshair.get_green_p(), 
        m_colorCrosshair.get_blue_p() );

    const float length = 0.02f;
    const float center = 0.0f;

    glBegin( GL_LINES );

    glVertex2f( center - length, center );
    glVertex2f( center + length, center );

    glVertex2f( center, center - length );
    glVertex2f( center, center + length );

    glEnd();
}

double ImageDrawingArea::GetDisplayedFrameRate()
{
    return m_displayedFrameRate.GetFrameRate();
}

void ImageDrawingArea::GetDisplaySizeAndMagnificationRate( unsigned int &cols, unsigned int &rows, double &magRate)
{
    get_window()->get_size( (int&)cols, (int&)rows );

    magRate = m_scaleX * cols / m_imageWidth;
}

void ImageDrawingArea::GetImageShift( double &shiftX, double &shiftY)
{
    double maxShiftX = m_scaleX / 2.0 - 0.5;
    double maxShiftY = m_scaleY / 2.0 - 0.5;

    // Return the value only when the image is movable to that direction.
    // Otherwise, it returns 0.5 to indicate it's in the middle.
    if ( maxShiftX > 0.0)
        shiftX = m_shiftX / ( maxShiftX * 2.0) + 0.5;
    else
        shiftX = 0.5;

    if ( maxShiftY > 0.0)
        shiftY = m_shiftY / ( maxShiftY * 2.0) + 0.5;
    else
        shiftY = 0.5;
}

void ImageDrawingArea::SetImageShift( double shiftX, double shiftY)
{
    if ( shiftX < 0.0 || shiftX > 1.0 || shiftY < 0.0 || shiftY > 1.0) return;

    double maxShiftX = m_scaleX / 2.0 - 0.5;
    double maxShiftY = m_scaleY / 2.0 - 0.5;

    // Set the value only when the image is movable to that direction
    if ( maxShiftX >= 0.0)
        m_shiftX = ( shiftX * 2.0 - 1.0) * maxShiftX;
    if ( maxShiftY >= 0.0)
        m_shiftY = ( shiftY * 2.0 - 1.0) * maxShiftY;
}

void ImageDrawingArea::LimitShift( void)
{
    double maxShiftX = m_scaleX / 2.0 - 0.5;
    double maxShiftY = m_scaleY / 2.0 - 0.5;
    if ( maxShiftX < 0.0)
        maxShiftX = 0.0;
    if ( maxShiftY < 0.0)
        maxShiftY = 0.0;

    if ( m_shiftX < -maxShiftX)
        m_shiftX = -maxShiftX;
    if ( m_shiftX > maxShiftX)
        m_shiftX = maxShiftX;
    if ( m_shiftY < -maxShiftY)
        m_shiftY = -maxShiftY;
    if ( m_shiftY > maxShiftY)
        m_shiftY = maxShiftY;
}

void ImageDrawingArea::ResetFrameRate()
{
    m_displayedFrameRate.Reset();
}

bool ImageDrawingArea::BeginGL()
{
    if ( !get_gl_window()->gl_begin( get_gl_context() ) )
    {        
        return false;
    }

    return true;
}

void ImageDrawingArea::EndGL()
{
    get_gl_window()->gl_end();
}

bool ImageDrawingArea::on_configure_event( GdkEventConfigure* /*event*/ )
{
    BeginGL();       

    int screenWidth = 0;
    int screenHeight = 0;
    get_window()->get_size( screenWidth, screenHeight );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if ( m_stretchToFit)
    {
        double heightStretch = (double) m_imageHeight * screenWidth / ( m_imageWidth * screenHeight);
        if ( heightStretch > 1.0)
        {
            m_scaleX = 1.0 / heightStretch;
            m_scaleY = 1.0;
        }else{
            m_scaleX = 1.0;
            m_scaleY = heightStretch;
        }
    }
    else
    {
        m_scaleX = (double)m_imageWidth / screenWidth;
        m_scaleY = (double)m_imageHeight / screenHeight;
    }

    LimitShift();

    double sx, sy;
    GetImageShift( sx, sy);
    m_signal_offset_changed( sx, sy); 

    glViewport( 
        0,
        0,
        screenWidth,
        screenHeight);

    EndGL();

    return true;
}

void ImageDrawingArea::InitializeOpenGL()
{
    BeginGL();

    // check PBO is supported
    const char *extensions = (const char*)glGetString(GL_EXTENSIONS);
    if ( strstr( extensions, "GL_ARB_pixel_buffer_object") != NULL)
    {

#ifdef _WIN32
        // get pointers to GL functions
        glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
        glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");

        if ( glGenBuffersARB == 0 || glBindBufferARB == 0 || glBufferDataARB == 0 || glDeleteBuffersARB == 0 )
        {
            // failed to get function pointer
        }
        else
        {
            m_PBOSupported = true;
            glGenBuffersARB( 1, &m_PBO);
        }
#else
        //
        // We can't figure out why extensions don't work on linux, so disabling.
        //
#endif
    }

    InitializeImageTexture();

    glShadeModel( GL_FLAT );

    // Initialize matrices
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0, 1, 0, 1 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    EndGL();
}

void ImageDrawingArea::InitializeImageTexture()
{
    glGenTextures( sk_maxNumTextures, m_imageTextures );

    bool useClampToEdge = true;
    if ( atof( (const char*)glGetString(GL_VERSION)) < 1.15)
    {
        useClampToEdge = false;
    }

    for ( unsigned int i = 0; i < sk_maxNumTextures; i++)
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
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    }
}

void ImageDrawingArea::DestroyImageTexture()
{
    glDeleteTextures( sk_maxNumTextures, m_imageTextures );
}

void ImageDrawingArea::on_realize()
{
    Gtk::GL::DrawingArea::on_realize();

    InitializeOpenGL();
}

void ImageDrawingArea::CommonInit()
{
    Gtk::GL::init(0, NULL);

    m_imageWidth = 0;
    m_imageHeight = 0;

    m_scaleX = 1.0;
    m_scaleY = 1.0;

    m_leftMBHeld = false;

    m_xPos = -1;
    m_yPos = -1;

    m_shiftX = 0.0;
    m_shiftY = 0.0;

    m_colorCrosshair.set_rgb_p( 1, 0, 0 );

    m_showCrosshair = false;
    m_stretchToFit = false;

    Gdk::EventMask mask = 
        Gdk::EXPOSURE_MASK |
        Gdk::BUTTON_PRESS_MASK |
        Gdk::BUTTON_RELEASE_MASK |
        Gdk::BUTTON1_MOTION_MASK |
        Gdk::POINTER_MOTION_MASK;
    set_events( mask );

    Glib::RefPtr<Gdk::GL::Config> glConfig;
    glConfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB | Gdk::GL::MODE_DOUBLE);

    if( !glConfig )
    {        
        // Bad
    }

    if( !set_gl_capability(glConfig) || !is_gl_capable() )
    {        
        // Bad
    }

    m_PBOSupported = false;
}

double ImageDrawingArea::SnapValue( double coord )
{
    return floor(coord) + 0.5;
}
