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
// $Id: Format7DrawingArea.cpp,v 1.27 2010/08/05 22:16:18 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "Format7DrawingArea.h"
#include <vector>

namespace FlyCapture2
{    
    Format7DrawingArea::Format7DrawingArea( BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /*refGlade*/ )
        : Gtk::DrawingArea(cobject),
        m_moveCursor( Gdk::FLEUR )
    {
        Gdk::EventMask mask = 
            Gdk::BUTTON_PRESS_MASK |
            Gdk::BUTTON_RELEASE_MASK |
            Gdk::BUTTON1_MOTION_MASK |
            Gdk::POINTER_MOTION_MASK;
        set_events( mask );

        m_leftMBHeld = false;
        m_rightMBHeld = false;        

        m_left = 0;
        m_top = 0;
        m_width = 0;
        m_height = 0;

        m_lastFiredLeft = 0;
        m_lastFiredTop = 0;
        m_lastFiredWidth = 0;
        m_lastFiredHeight = 0;
        
        m_maxWidth = 0;
        m_maxHeight = 0;

        m_offsetHStepSize = 0;
        m_offsetVStepSize = 0;
        m_imageHStepSize = 0;
        m_imageVStepSize = 0;

        m_startX = 0;
        m_startY = 0;

        m_offsetX = 0;
        m_offsetY = 0;

        m_currX = 0;
        m_currY = 0;

        m_imageSizeChanged = false;
    }

    Format7DrawingArea::~Format7DrawingArea()
    {
    }

    sigc::signal<void, unsigned int, unsigned int, unsigned int, unsigned int> Format7DrawingArea::signal_image_size_changed()
    {
        return m_signal_image_size_changed;
    }

    bool Format7DrawingArea::on_expose_event( GdkEventExpose* event )
    {
        Glib::RefPtr<Gdk::Window> window = get_window();
        if( window == NULL)
        {
            return true;
        }

        Cairo::RefPtr<Cairo::Context> refCairo = window->create_cairo_context();

        // clip to the area indicated by the expose event so that we only redraw
        // the portion of the window that needs to be redrawn
        refCairo->rectangle(
            event->area.x, 
            event->area.y,
            event->area.width, 
            event->area.height);
        refCairo->clip();               

        // Get width / height of widget
        int width;
        int height;
        window->get_size( width, height );        

        // Figure out which scale to use (horizontal or vertical)
        const float horzScale = m_maxWidth / (float)width;
        const float vertScale = m_maxHeight / (float)height;
                
        m_previewScale = (horzScale < vertScale) ? vertScale : horzScale;

        Gtk::AspectFrame* pFrame = (Gtk::AspectFrame*)get_parent();
        float fRatio = m_maxWidth / (float)m_maxHeight;       
        if ( fRatio != pFrame->property_ratio())
        {
            pFrame->set( 0.0, 0.0, fRatio, false );
        } 

        unsigned int scaledLeft = static_cast<unsigned int>(ToScaled( m_left ));
        unsigned int scaledTop = static_cast<unsigned int>(ToScaled( m_top ));
        unsigned int scaledWidth = static_cast<unsigned int>(ToScaled( m_width ));
        unsigned int scaledHeight = static_cast<unsigned int>(ToScaled( m_height ));        

        // Fill the background with the PGR color
        FillBackground( 
            refCairo, 
            event->area.x, 
            event->area.y,
            event->area.width, 
            event->area.height);

        // Draw the data on top of the filled background
        DrawRectangle( refCairo, scaledLeft, scaledTop, scaledWidth, scaledHeight );
        DrawDashedLines( refCairo, scaledLeft, scaledTop, scaledWidth, scaledHeight );        
        DrawImageDimensionsText( refCairo, m_left, m_top, m_width, m_height );
        DrawCurrentCursorPositionText( refCairo, m_currX, m_currY );       

        if ( m_imageSizeChanged == true )
        {
            if ( m_left != m_lastFiredLeft ||
                m_top != m_lastFiredTop ||
                m_width != m_lastFiredWidth ||
                m_height != m_lastFiredHeight )
            {
                m_signal_image_size_changed( m_left, m_top, m_width, m_height );     

                m_lastFiredLeft = m_left;
                m_lastFiredTop = m_top;
                m_lastFiredWidth = m_width;
                m_lastFiredHeight = m_height;
            }
            
            m_imageSizeChanged = false;
        }

        return true;
    }

    void Format7DrawingArea::FillBackground( 
        Cairo::RefPtr<Cairo::Context> refCairo, 
        unsigned int left, unsigned int top, unsigned int width, unsigned int height )
    {
        refCairo->save();

        refCairo->rectangle(
            left,
            top,
            width,
            height );

        const double divisor = 255.0;
        const double red = 157.0 / divisor;
        const double green = 184.0 / divisor;
        const double blue = 210.0 / divisor;

        refCairo->set_source_rgb( red, green, blue );

        refCairo->fill();

        refCairo->restore();
    }

    void Format7DrawingArea::DrawRectangle( 
        Cairo::RefPtr<Cairo::Context> refCairo,
        unsigned int left, unsigned int top, unsigned int width, unsigned int height )
    {
        refCairo->save();

        // Set draw color
        refCairo->set_source_rgb(1.0, 0.0, 0.0);  

        // Set line width
        refCairo->set_line_width(1.0);

        // Draw the image rectangle
        refCairo->rectangle(
            left,
            top,
            width,
            height );

        refCairo->stroke();

        refCairo->restore();
    }

    void Format7DrawingArea::DrawDashedLines( 
        Cairo::RefPtr<Cairo::Context> refCairo,
        unsigned int left, unsigned int top, unsigned int width, unsigned int height )
    {
        refCairo->save();

        // Get width / height of widget
        int windowWidth;
        int windowHeight;
        get_window()->get_size( windowWidth, windowHeight );

        // Set the dash parameters
        std::vector<double> dash_vector(2);
        dash_vector[0] = 4.0;
        dash_vector[1] = 2.0;
        refCairo->set_dash( dash_vector, 0.0 );

        // Set draw color
        refCairo->set_source_rgb(0.0, 0.0, 0.0);

        // Set line width
        refCairo->set_line_width(1.0);

        refCairo->move_to( left, top );
        refCairo->line_to( left, 0 );

        refCairo->move_to( left, top );
        refCairo->line_to( 0, top );

        refCairo->move_to( left + width, top + height );
        refCairo->line_to( left + width, windowHeight );

        refCairo->move_to( left + width, top + height );
        refCairo->line_to( windowWidth, top + height );

        refCairo->stroke();  

        refCairo->restore();
    }

    void Format7DrawingArea::DrawImageDimensionsText( 
        Cairo::RefPtr<Cairo::Context> refCairo,
        unsigned int left, unsigned int top, unsigned int width, unsigned int height )
    {        
        refCairo->save();

        // Set the font parameters
        refCairo->select_font_face( 
            "monospace",
            Cairo::FONT_SLANT_NORMAL, 
            Cairo::FONT_WEIGHT_BOLD );
        refCairo->set_font_size( 10 );

        // Set draw color to black
        refCairo->set_source_rgb(0.0, 0.0, 0.0);       

        // Get width / height of widget
        int widgetWidth = 0;
        int widgetHeight = 0;
        get_window()->get_size( widgetWidth, widgetHeight );

        // Create text for image offset
        char imageOffsets[128];
        sprintf(
            imageOffsets,
            "Start: (%d,%d) End: (%d,%d)",
            left,
            top,
            left + width,
            top + height );
        Cairo::TextExtents offsetExtents;
        refCairo->get_text_extents(imageOffsets, offsetExtents);

        // Draw the offset text
        refCairo->move_to( 
            (widgetWidth/2) - (offsetExtents.width/2),
            (widgetHeight/2) - offsetExtents.height - (offsetExtents.height/2));
        refCairo->show_text( imageOffsets );

        // Create text for image dimensions
        char imageDimensions[128];
        sprintf(
            imageDimensions,
            "Dimensions: %d x %d",
            width,
            height);
        Cairo::TextExtents dimensionsExtents;
        refCairo->get_text_extents(imageDimensions, dimensionsExtents);

        // Draw the dimensions text
        refCairo->move_to( 
            (widgetWidth/2) - (dimensionsExtents.width/2),
            (widgetHeight/2) + dimensionsExtents.height + (dimensionsExtents.height/2));
        refCairo->show_text( imageDimensions );

        refCairo->restore();
    }

    void Format7DrawingArea::DrawCurrentCursorPositionText( 
        Cairo::RefPtr<Cairo::Context> refCairo,
        unsigned int currX, unsigned int currY )
    {
        refCairo->save();

        // Set the font parameters
        refCairo->select_font_face( 
            "monospace",
            Cairo::FONT_SLANT_NORMAL, 
            Cairo::FONT_WEIGHT_BOLD );
        refCairo->set_font_size( 10 );

        // Set draw color to black
        refCairo->set_source_rgb(0.0, 0.0, 0.0);      

        // Print current cursor position
        char cursorPosition[128];
        sprintf( 
            cursorPosition,
            "Cursor: (%d, %d)",
            currX,
            currY );

        // Get width / height of widget
        int width;
        int height;
        get_window()->get_size( width, height );

        Cairo::TextExtents textExtents;
        refCairo->get_text_extents( cursorPosition, textExtents );
        refCairo->move_to( (width / 2) - (textExtents.width / 2), (height * 0.9) - (textExtents.height / 2) );
        refCairo->show_text( cursorPosition );

        refCairo->restore();
    }

    bool Format7DrawingArea::on_button_press_event( GdkEventButton* event )
    {
        gdouble startX = event->x;
        gdouble startY = event->y;

        ClipPoint( &startX, &startY );

        unsigned int startXOrig = ToOriginal( (float)startX );
        unsigned int startYOrig = ToOriginal( (float)startY );

        switch ( event->button )
        {
        case 1:
            // Store the start point
            m_startX = startXOrig;
            m_startY = startYOrig;

            m_leftMBHeld = true;
            break;

        case 3:
            get_window()->set_cursor( m_moveCursor );

            // Store the difference between the start point and the left/top
            m_offsetX = m_left - startXOrig;
            m_offsetY = m_top - startYOrig;

            m_rightMBHeld = true;
            break;
        }

        return true;
    }

    bool Format7DrawingArea::on_button_release_event( GdkEventButton* event )
    {
        gdouble endX = event->x;
        gdouble endY = event->y;

        ClipPoint( &endX, &endY );

        const unsigned int endXOrig = ToOriginal( (float)endX );
        const unsigned int endYOrig = ToOriginal( (float)endY );

        switch ( event->button )
        {
        case 1:
            m_left = std::min(m_startX, endXOrig);
            m_top = std::min(m_startY, endYOrig);
            m_width = std::max(m_startX, endXOrig) - m_left;
            m_height = std::max(m_startY, endYOrig) - m_top;

            ClampAllValues();

            m_imageSizeChanged = true;

            m_leftMBHeld = false;          
            break;

        case 3:
            get_window()->set_cursor();

            m_rightMBHeld = false;
            break;
        }       

        queue_draw();

        return true;
    }

    bool Format7DrawingArea::on_motion_notify_event( GdkEventMotion* event )
    {
        gdouble currX = event->x;
        gdouble currY = event->y;        

        // This may actually be negative if the mouse is dragged to the 
        // left or top of the image, so ints should be used instead of
        // unsigned ints to prevent overflow
        int currXOrig = 0;
        int currYOrig = 0; 

        if ( m_leftMBHeld == true )
        {     
            ClipPoint( &currX, &currY );

            currXOrig = ToOriginal( static_cast<float>(currX) );
            currYOrig = ToOriginal( static_cast<float>(currY) );  

            if ( currXOrig <= static_cast<int>(m_startX) )
            {                
                m_left = currXOrig;
                m_width = m_startX - m_left;
            }
            else
            {
                m_left = m_startX;
                m_width = currXOrig - m_left;
            }

            if ( currYOrig <= static_cast<int>(m_startY) )
            {                
                m_top = currYOrig;
                m_height = m_startY - m_top;
            }
            else
            {
                m_top = m_startY;
                m_height = currYOrig - m_top;
            }

            ClampAllValues();   

            m_imageSizeChanged = true;
        }   
        else if ( m_rightMBHeld == true )
        {                            
            currXOrig = ToOriginal( static_cast<float>(currX) );
            currYOrig = ToOriginal( static_cast<float>(currY) );  

            // Perform some initial calculations to make sure that the
            // left and top values are not overflowing
            if ( currXOrig + m_offsetX < 0 )
            {
                m_left = 0;
            }
            else
            {
                m_left = currXOrig + m_offsetX;
            }

            if ( currYOrig + m_offsetY < 0 )
            {
                m_top = 0;
            }
            else
            {
                m_top = currYOrig + m_offsetY;         
            }

            ClampAllValues();               

            // Ensure we are still within the boundaries
            if ( m_left + m_width > m_maxWidth )
            {
                m_left = m_maxWidth - m_width;
            }
            else if ( m_left + m_width < m_width )
            {
                m_left = 0;
            }

            if ( m_top + m_height > m_maxHeight )
            {
                m_top = m_maxHeight - m_height;
            }
            else if ( m_top + m_height < m_height )
            {
                m_top = 0;
            }                  

            m_imageSizeChanged = true;
        }
        else
        {
            ClipPoint( &currX, &currY );

            currXOrig = ToOriginal( static_cast<float>(currX) );
            currYOrig = ToOriginal( static_cast<float>(currY) );  
        }

        // Store the current cursor coordinates
        m_currX = currXOrig;
        m_currY = currYOrig;       

        queue_draw();

        return true;
    }    

    void Format7DrawingArea::UpdateSettings( Format7Info* pFmt7Info, Format7ImageSettings* pImageSettings )
    {
        // Maximum resolution
        m_maxWidth = pFmt7Info->maxWidth;
        m_maxHeight = pFmt7Info->maxHeight;

        // Step sizes
        m_offsetHStepSize = pFmt7Info->offsetHStepSize;
        m_offsetVStepSize = pFmt7Info->offsetVStepSize;
        m_imageHStepSize = pFmt7Info->imageHStepSize;
        m_imageVStepSize = pFmt7Info->imageVStepSize;

        m_left = pImageSettings->offsetX;
        m_top = pImageSettings->offsetY;
        m_width = pImageSettings->width;
        m_height = pImageSettings->height;

        queue_draw();        
    }

    void Format7DrawingArea::UpdateSettings( GigEImageSettingsInfo* pImageInfo, GigEImageSettings* pImageSettings )
    {
        // Maximum resolution
        m_maxWidth = pImageInfo->maxWidth;
        m_maxHeight = pImageInfo->maxHeight;

        // Step sizes
        m_offsetHStepSize = pImageInfo->offsetHStepSize;
        m_offsetVStepSize = pImageInfo->offsetVStepSize;
        m_imageHStepSize = pImageInfo->imageHStepSize;
        m_imageVStepSize = pImageInfo->imageVStepSize;

        m_left = pImageSettings->offsetX;
        m_top = pImageSettings->offsetY;
        m_width = pImageSettings->width;
        m_height = pImageSettings->height;

        queue_draw();        
    }

    unsigned int Format7DrawingArea::ClampValue( unsigned int inputVal, unsigned int stepSize )
    {
        if ( inputVal % stepSize != 0 )
        {
            return inputVal - (inputVal % stepSize);
        }
        else
        {
            return inputVal;
        }
    }

    void Format7DrawingArea::ClampAllValues()
    {
        m_left = ClampValue( m_left, m_offsetHStepSize );
        m_top = ClampValue( m_top, m_offsetVStepSize );
        m_width = ClampValue( m_width, m_imageHStepSize );
        m_height = ClampValue( m_height, m_imageVStepSize );
    }    

    void Format7DrawingArea::ClipPoint( gdouble* x, gdouble* y )
    {
        // Get window size
        int width;
        int height;
        get_window()->get_size( width, height );

        if ( *x < 0 )
        {
            *x = 0;
        }
        else if ( *x > width )
        {
            *x = width;
        }

        if ( *y < 0 )
        {
            *y = 0;
        }
        else if ( *y > height )
        {
            *y = height;
        }
    }

    float Format7DrawingArea::ToScaled( int original )
    {
        return original / m_previewScale;
    }

    int Format7DrawingArea::ToOriginal( float scaled )
    {
        return static_cast<int>(scaled * m_previewScale);
    }            
}
