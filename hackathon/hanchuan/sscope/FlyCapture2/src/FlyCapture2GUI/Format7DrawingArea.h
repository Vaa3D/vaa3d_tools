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
// $Id: Format7DrawingArea.h,v 1.14 2010/03/29 19:18:47 soowei Exp $
//=============================================================================

#include "FlyCapture2.h"

namespace FlyCapture2
{   
    class Format7DrawingArea : public Gtk::DrawingArea
    {
    public:
        Format7DrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
        virtual ~Format7DrawingArea();

        void UpdateSettings( Format7Info* pFmt7Info, Format7ImageSettings* pImageSettings );
        void UpdateSettings( GigEImageSettingsInfo* pImageInfo, GigEImageSettings* pImageSettings );

        bool on_button_press_event(GdkEventButton* event);
        bool on_button_release_event( GdkEventButton* event );
        bool on_motion_notify_event( GdkEventMotion* event );

        sigc::signal<void, unsigned int, unsigned int, unsigned int, unsigned int> signal_image_size_changed();

    protected:
        virtual bool on_expose_event(GdkEventExpose* event);

        sigc::signal<void, unsigned int, unsigned int, unsigned int, unsigned int> m_signal_image_size_changed;

    private:    
        // Booleans to store whether any of the mouse buttons are being held
        bool m_leftMBHeld;
        bool m_rightMBHeld;        
        
        // Image dimensions
        unsigned int m_left;
        unsigned int m_top;
        unsigned int m_width;
        unsigned int m_height;

        // Image dimensions of last fired event, to prevent repeated firings
        unsigned int m_lastFiredLeft;
        unsigned int m_lastFiredTop;
        unsigned int m_lastFiredWidth;
        unsigned int m_lastFiredHeight;
        
        // Maximum resolution
        unsigned int m_maxWidth;
        unsigned int m_maxHeight;

        // Step sizes
        unsigned int m_offsetHStepSize;
        unsigned int m_offsetVStepSize;
        unsigned int m_imageHStepSize;
        unsigned int m_imageVStepSize;

        // Start position for drags
        unsigned int m_startX;
        unsigned int m_startY;

        // Offset calculation for right click drags
        // This offset is the difference between the start position and
        // the left/top of the image
        int m_offsetX;
        int m_offsetY;

        // Current position of the cursor
        unsigned int m_currX;
        unsigned int m_currY;       

        // Scaling factor
        float m_previewScale;

        // Did the image size change since the last redraw?
        bool m_imageSizeChanged;
        
        // Fleur cursor (for moving)
        Gdk::Cursor m_moveCursor;

        // Make sure that the image values are valid in Format7
        static unsigned int ClampValue( unsigned int inputVal, unsigned int stepSize );
        void ClampAllValues();

        // Make sure that the x,y coordinates do not wander outside the boundaries
        void ClipPoint( gdouble* x, gdouble* y );

        float ToScaled( int original );
        int ToOriginal( float scaled );

        void FillBackground(
            Cairo::RefPtr<Cairo::Context> refCairo, 
            unsigned int left, 
            unsigned int top, 
            unsigned int width, 
            unsigned int height );

        void DrawRectangle( 
            Cairo::RefPtr<Cairo::Context> refCairo, 
            unsigned int left, 
            unsigned int top, 
            unsigned int width, 
            unsigned int height );

        void DrawDashedLines(
            Cairo::RefPtr<Cairo::Context> refCairo, 
            unsigned int left, 
            unsigned int top, 
            unsigned int width, 
            unsigned int height );

        void DrawImageDimensionsText(
            Cairo::RefPtr<Cairo::Context> refCairo, 
            unsigned int left, 
            unsigned int top, 
            unsigned int width, 
            unsigned int height );

        void DrawCurrentCursorPositionText(
            Cairo::RefPtr<Cairo::Context> refCairo, 
            unsigned int currX,
            unsigned int currY ); 

    };
}
