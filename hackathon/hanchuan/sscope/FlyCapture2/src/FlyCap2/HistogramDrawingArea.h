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
// $Id: HistogramDrawingArea.h,v 1.9 2009/04/23 17:17:41 soowei Exp $
//=============================================================================
 
#ifndef HISTOGRAMDRAWINGAREA_H_INCLUDED
#define HISTOGRAMDRAWINGAREA_H_INCLUDED

#include "FlyCapture2.h"

using namespace FlyCapture2;

/**
 * This class takes image statistics data such as histograms or row/column
 * calculations and draws them onto a Gtk::DrawingArea.
 */ 
class HistogramDrawingArea : public Gtk::DrawingArea
{
public:
    struct RowColChannelStats
    {
        std::vector<unsigned int> vecMax;
        std::vector<unsigned int> vecMean;
        std::vector<unsigned int> vecMin;
    };

    struct RowColStats
    {
        int numPixelValues;
        int imageDimension;

        RowColChannelStats grey;
        RowColChannelStats red;
        RowColChannelStats green;
        RowColChannelStats blue;
    };   

    struct RowColDisplayOptions
    {
        bool showGrey;
        bool showRed;
        bool showGreen;
        bool showBlue;
    };

    struct RowColFunctionOptions
    {
        bool showMax;
        bool showMean;
        bool showMin;
    };

    HistogramDrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
    virtual ~HistogramDrawingArea();

    /**
     * Update the drawing area for histogram mode.
     */ 
    void Update( 
        ImageStatistics* pStatistics, 
        unsigned int maxVal );

    /**
     * Update the drawing area for row column mode.
     */ 
    void Update( 
        RowColStats* pRowColStats, 
        RowColDisplayOptions dispOptions, 
        RowColFunctionOptions funcOptions );

    /**
     * Set the overall draw mode.
     */ 
    void SetDrawMode( Histogram::DrawMode drawMode );
    
    /**
     * Set the row column plot mode.
     */ 
    void SetRowColMode( Histogram::RowColMode rowColMode );

protected:      
    /**
     * Implementation of on_expose_event() to draw the histogram
     * to the drawing area.
     *
     * @param event The expose event.
     */
    virtual bool on_expose_event(GdkEventExpose* event);

private:    
    static const float sk_horzScale;
    static const float sk_vertScale;

    Glib::Mutex statsMutex;

    ImageStatistics m_histogramStats;
    unsigned int m_maxVal;

    RowColStats m_rowColStats;
    RowColDisplayOptions m_dispOptions;
    RowColFunctionOptions m_funcOptions;

    /** The draw mode, i.e. histogram or row/column. */
    Histogram::DrawMode m_drawMode;

    /** The row column mode, i.e. row or column. */
    Histogram::RowColMode m_rowColMode;

    /** @name Row column drawing functions.
     *  These functions deal with drawing for the row column mode.
     */
    //@{

    void DrawSingleRowColChannel(
        Cairo::RefPtr<Cairo::Context> refCairo,
        RowColChannelStats stats,
        unsigned int numValues,
        double red,
        double green,
        double blue );
    
    void DrawSingleRowColLine(
        Cairo::RefPtr<Cairo::Context> refCairo,
        std::vector<unsigned int> vecData,
        unsigned int numValues,
        double red,
        double green,
        double blue );

    void DrawRowColGridLabels( 
        Cairo::RefPtr<Cairo::Context> refCairo,
        unsigned int numPixelValues,
        unsigned int numValues );

    //@}

    /** @name Histogram drawing functions.
     *  These functions deal with drawing for the histogram mode.
     */
    //@{

    void DrawSingleHistogramLine( 
        Cairo::RefPtr<Cairo::Context> refCairo,
        ImageStatistics::StatisticsChannel channel );

    void DrawHistogramGridLabels( Cairo::RefPtr<Cairo::Context> refCairo );

    //@}

    /** @name Common drawing functions.
     *  These functions deal with drawing for both modes.
     */
    //@{

    void DrawBackgroundGrid( Cairo::RefPtr<Cairo::Context> refCairo );    

    //@}

    void GetPaddedSize( 
        float& paddedLeft, 
        float& paddedTop, 
        float& paddedWidth, 
        float& paddedHeight );  

    static int GetNumPixels( 
        unsigned int numPixelValues, 
        const int* pHistogram );

    static std::vector<float> GetPercentages( 
        unsigned int numPixelValues, 
        const int* pHistogram );

    static void GetLineColor( 
        ImageStatistics::StatisticsChannel channel, 
        double& red, 
        double& green, 
        double& blue );
};

#endif
