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
// $Id: ImageDrawingArea.h,v 1.25 2010/03/25 17:41:13 soowei Exp $
//=============================================================================

#ifndef PGR_FC2_IMAGEDRAWINGAREA_H
#define PGR_FC2_IMAGEDRAWINGAREA_H

#include "FrameRateCounter.h"

/**
 * This class inherits from Gtk::DrawingArea and provides the ability to
 * draw RGB/RGBU images to the screen.
 */ 
class ImageDrawingArea : public Gtk::GL::DrawingArea
{
public:
    /** Constructor. */
    ImageDrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

    /** Constructor. */
    ImageDrawingArea();

    /** Destructor. */
    virtual ~ImageDrawingArea();    
    
    /**
     * Set the pixbuf to draw on to the DrawingArea.
     *
     * @param refPixBuf The pixbuf containing the image data to be drawn.
     */
    void SetPixBuf(Glib::RefPtr<Gdk::Pixbuf> refPixBuf);

    /**
     * Set the stretch parameter of the DrawingArea.
     *
     * @param stretch Whether to stretch the image to fit the DrawingArea.
     */
    void SetStretchToFit( bool stretch );

    /**
     * Set the crosshair parameter of the DrawingArea.
     *
     * @param show Whether to show a crosshair.
     */
    void SetShowCrosshair( bool show );

    /**
     * Set the crosshair color.
     *
     * @param color The crosshair to set.
     */
    void SetCrosshairColor( Gdk::Color color );

    /**
     * Get the crosshair color.     
     *
     * @return The crosshair color.
     */
    Gdk::Color GetCrosshairColor();

    /**
     * Get the current position of the mouse cursor over the DrawingArea.
     * The value is given in image coordinates.
     *
     * @param pXPos X position of the cursor.
     * @param pYPos Y position of the cursor.
     */
    void GetMouseCoordinates( int* pXPos, int* pYPos );

    /**
     * Get the displayed frame rate.
     *
     * @return Displayed frame rate.
     */
    double GetDisplayedFrameRate(); 

    /**
     * Get the size of display area
     *
     * @param cols The width of the display area
     * @param rows The height of the display area
     * @param magRate The zoom rate of the image inside the display area
     */
    void GetDisplaySizeAndMagnificationRate( unsigned int &cols, unsigned int &rows, double &magRate);

    void GetImageShift( double &shiftX, double &shiftY);
    void SetImageShift( double shiftX, double shiftY);

    /** Reset the frame rate. */
    void ResetFrameRate();

    /** Signal to notify that the image offsets have changed. */
    sigc::signal<void, double, double> signal_offset_changed();

protected:    
    /**
     * Implementation of on_expose_event() to draw the image
     * to the drawing area.
     *
     * @param event The expose event.
     */
    virtual bool on_expose_event(GdkEventExpose* event);

    bool on_button_press_event(GdkEventButton* event);

    bool on_button_release_event( GdkEventButton* event );

    bool on_motion_notify_event (GdkEventMotion* event);  

    bool on_configure_event(GdkEventConfigure* event );

    void on_realize();

private:
    int m_xPos;
    int m_yPos;

    unsigned int m_imageWidth;
    unsigned int m_imageHeight;

    bool m_leftMBHeld;

    double m_scaleX;
    double m_scaleY;

    bool m_showCrosshair;
    bool m_stretchToFit;

    FrameRateCounter m_displayedFrameRate;

    sigc::signal<void, double, double> m_signal_offset_changed;

    Gdk::Color m_colorCrosshair;

    // Fleur cursor (for moving)
    Gdk::Cursor m_moveCursor;

    double m_startX;
    double m_startY;

    double m_shiftX;
    double m_shiftY;

    Glib::RefPtr<Gdk::Pixbuf> m_pixBuf;

    static const unsigned int sk_maxNumTextures = 16;
    GLuint m_imageTextures[ sk_maxNumTextures];

    Glib::Mutex m_pixBufMutex;

    bool   m_PBOSupported;
    GLuint m_PBO;

    static double SnapValue( double coord );
    
    void CommonInit();

    void InitializeOpenGL();

    void InitializeCrosshairTexture();
    void InitializeImageTexture();
    void DestroyImageTexture();
    bool BeginGL();
    void EndGL();    

    void SetImageSize( unsigned int width, unsigned int height );
    void LimitShift( void);
    
    void ShowCrosshair();
};

#endif // #ifndef PGR_FC2_IMAGEDRAWINGAREA_H
