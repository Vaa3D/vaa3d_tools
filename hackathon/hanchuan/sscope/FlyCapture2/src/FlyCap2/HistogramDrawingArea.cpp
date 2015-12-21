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
// $Id: HistogramDrawingArea.cpp,v 1.23 2010/02/09 01:16:19 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "HistogramDrawingArea.h"

const float HistogramDrawingArea::sk_horzScale = 0.9f;
const float HistogramDrawingArea::sk_vertScale = 0.8f;
   
HistogramDrawingArea::HistogramDrawingArea( BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /*refGlade*/ )
: Gtk::DrawingArea(cobject)
{        
}

HistogramDrawingArea::~HistogramDrawingArea()
{
}    

bool HistogramDrawingArea::on_expose_event( GdkEventExpose* event )
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

    // Clear the background
    refCairo->set_source_rgb( 255, 255, 255 );
    refCairo->paint();

    Glib::Mutex::Lock pixBufLock(statsMutex, Glib::NOT_LOCK );
    if ( pixBufLock.try_acquire() != true )
    {
        return true;
    }

    if ( m_drawMode == Histogram::MODE_HISTOGRAM )
    {
        for ( int i=0; i < ImageStatistics::NUM_STATISTICS_CHANNELS; i++ )
        {            
            DrawSingleHistogramLine( 
                refCairo, 
                static_cast<ImageStatistics::StatisticsChannel>(i) );
        }
        
        DrawHistogramGridLabels( refCairo );
    }
    else if ( m_drawMode == Histogram::MODE_ROWCOL )
    {
        double red, green, blue;

        if ( m_dispOptions.showGrey == true )
        {            
            GetLineColor( ImageStatistics::GREY, red, green, blue );

            DrawSingleRowColChannel( 
                refCairo, 
                m_rowColStats.grey, 
                m_rowColStats.numPixelValues,
                red, 
                green, 
                blue );
        }

        if ( m_dispOptions.showRed == true )
        {
            GetLineColor( ImageStatistics::RED, red, green, blue );

            DrawSingleRowColChannel( 
                refCairo, 
                m_rowColStats.red, 
                m_rowColStats.numPixelValues,
                red, 
                green, 
                blue );
        }

        if ( m_dispOptions.showGreen == true )
        {
            GetLineColor( ImageStatistics::GREEN, red, green, blue );

            DrawSingleRowColChannel( 
                refCairo, 
                m_rowColStats.green, 
                m_rowColStats.numPixelValues,
                red, 
                green, 
                blue );
        }

        if ( m_dispOptions.showBlue == true )
        {
            GetLineColor( ImageStatistics::BLUE, red, green, blue );

            DrawSingleRowColChannel( 
                refCairo, 
                m_rowColStats.blue, 
                m_rowColStats.numPixelValues,
                red, 
                green, 
                blue );
        }

        DrawRowColGridLabels( 
            refCairo, 
            m_rowColStats.numPixelValues, 
            m_rowColStats.imageDimension );
    }    

    DrawBackgroundGrid( refCairo );

    return true;
}

void HistogramDrawingArea::Update( 
    ImageStatistics* pStatistics, 
    unsigned int maxVal )
{
    Glib::Mutex::Lock lock(statsMutex);

    m_histogramStats = *pStatistics;
    m_maxVal = maxVal;

    queue_draw();
}

void HistogramDrawingArea::Update( 
    RowColStats* pRowColStats, 
    RowColDisplayOptions dispOptions, 
    RowColFunctionOptions funcOptions )
{
    Glib::Mutex::Lock lock(statsMutex);

    m_rowColStats = *pRowColStats;
    m_dispOptions = dispOptions;
    m_funcOptions = funcOptions;

    queue_draw();
}

void HistogramDrawingArea::DrawSingleHistogramLine( 
    Cairo::RefPtr<Cairo::Context> refCairo,
    ImageStatistics::StatisticsChannel channel )
{       
    unsigned int rangeMin, rangeMax, pixelValueMin, pixelValueMax;
    unsigned int numPixelValues;
    float pixelValueMean;
    int* pHistogram;

    m_histogramStats.GetStatistics( 
        channel,
        &rangeMin,
        &rangeMax,
        &pixelValueMin,
        &pixelValueMax,
        &numPixelValues,
        &pixelValueMean,
        &pHistogram );    
           
    std::vector<float> percentages = GetPercentages( numPixelValues, pHistogram );

    refCairo->save();

    // Reserve the outside 10%
    float paddedLeft, paddedTop, paddedWidth, paddedHeight;
    GetPaddedSize( paddedLeft, paddedTop, paddedWidth, paddedHeight ); 

    const float xScale = static_cast<float>(paddedWidth / numPixelValues);
    const float yScale = paddedHeight / static_cast<float>(m_maxVal);

    // Set draw color
    double red, green, blue;
    GetLineColor( channel, red, green, blue );
    refCairo->set_source_rgb(red, green, blue);

    // Set line width
    refCairo->set_line_width(1.0);

    if ( numPixelValues > 256 )
    {        
        const int k_pixelsToAvg = static_cast<int>(numPixelValues / paddedWidth);

        for ( unsigned int i=0; i < numPixelValues; i += k_pixelsToAvg )
        {
            float maxVal = 0.0;
            for ( int j=0; j < k_pixelsToAvg; j++ )
            {
                if ( i+j >= numPixelValues )
                {
                    break;
                }

                if ( percentages[i+j] > maxVal )
                {
                    maxVal = percentages[i+j];
                }
            }

            // Calculate the scaled height of the current percentage
            float yScaledHeight = yScale * maxVal;

            if ( i == 0 )
            {
                refCairo->move_to( paddedLeft, (paddedTop + paddedHeight - yScaledHeight) );
            }

            refCairo->line_to( paddedLeft + (xScale * i), paddedTop + paddedHeight - yScaledHeight );    
        }
    }
    else
    {
        for ( unsigned int i=0; i < numPixelValues; i++ )
        {
            // Calculate the scaled height of the current percentage
            float yScaledHeight = yScale * percentages[i];

            if ( i == 0 )
            {
                refCairo->move_to( paddedLeft, (paddedTop + paddedHeight - yScaledHeight) );
            }

            refCairo->line_to( paddedLeft + (xScale * i), paddedTop + paddedHeight - yScaledHeight );    
        }
    }    
   
    refCairo->stroke();

    // Overwrite anything that appears on the paddedTop section
    refCairo->rectangle(
        paddedLeft, 
        0,
        paddedWidth, 
        paddedTop );

    refCairo->set_source_rgb( 255, 255, 255 );

    refCairo->set_operator( Cairo::OPERATOR_OVER );

    refCairo->fill();    

    refCairo->restore();
}

void HistogramDrawingArea::DrawBackgroundGrid( Cairo::RefPtr<Cairo::Context> refCairo )
{
    // Reserve the outside 10%
    float paddedLeft, paddedTop, paddedWidth, paddedHeight;
    GetPaddedSize( paddedLeft, paddedTop, paddedWidth, paddedHeight ); 

    refCairo->save();

    // Set draw color
    refCairo->set_source_rgb(0, 0, 0);

    // Set line width
    refCairo->set_line_width(1.0);

    // Draw a horizontal line across the bottom
    refCairo->move_to( paddedLeft, paddedTop + paddedHeight );
    refCairo->line_to( paddedLeft + paddedWidth, paddedTop + paddedHeight );

    refCairo->stroke();      

    /*
    std::vector<double> dash_vector(2);  
    dash_vector[0] = 1.0;
    dash_vector[1] = 2.0;
    refCairo->set_dash( dash_vector, 0.0 );
    */

    // Set line width
    refCairo->set_line_width(0.25);

    const int k_numPartitions = 8;      
    for ( int i=1; i < k_numPartitions; i++ )
    {
        const float fraction = i / static_cast<float>(k_numPartitions);

        // Draw vertical lines
        float xOffset = paddedLeft + (paddedWidth * fraction);
        refCairo->move_to( xOffset, paddedTop );
        refCairo->line_to( xOffset, paddedTop + paddedHeight );

        float yOffset = 0;

        if ( m_drawMode == Histogram::MODE_HISTOGRAM )
        {
            if ( i % 2 != 0 )
            {
                continue;
            }
            
            float scaleRatio = 100 / static_cast<float>(m_maxVal);       
            yOffset = paddedTop + paddedHeight - (paddedHeight * fraction * scaleRatio);
        }
        else if ( m_drawMode == Histogram::MODE_ROWCOL )
        {
            yOffset = paddedTop + paddedHeight - (paddedHeight * fraction);
        }

        if ( yOffset > paddedTop )
        {
            refCairo->move_to( paddedLeft, yOffset );
            refCairo->line_to( paddedLeft + paddedWidth, yOffset );
        }          
    }

    refCairo->stroke();

    refCairo->restore();
}

void HistogramDrawingArea::DrawHistogramGridLabels( Cairo::RefPtr<Cairo::Context> refCairo )
{
    // Reserve the outside 10%
    float paddedLeft, paddedTop, paddedWidth, paddedHeight;
    GetPaddedSize( paddedLeft, paddedTop, paddedWidth, paddedHeight );   

    refCairo->save();

    // Set draw color
    refCairo->set_source_rgb(0, 0, 0);

    // Set the font parameters
    refCairo->select_font_face( 
        "monospace",
        Cairo::FONT_SLANT_NORMAL, 
        Cairo::FONT_WEIGHT_BOLD );
    refCairo->set_font_size( 10 );
    
    const int k_numPartitions = 8;      

    // Render the x-axis labels
    for ( int i=0; i <= k_numPartitions; i++ )
    {
        const float fraction = i / static_cast<float>(k_numPartitions);

        unsigned int maxNumPixelValues = 0;

        for ( int j=0; j < ImageStatistics::NUM_STATISTICS_CHANNELS; j++ )
        {
            unsigned int numPixelValues = 0;

            m_histogramStats.GetNumPixelValues( 
                static_cast<ImageStatistics::StatisticsChannel>(j),
                &numPixelValues );

            maxNumPixelValues = std::max( numPixelValues, maxNumPixelValues );
        }

        char caption[32];
        float pixelValue;
        if ( maxNumPixelValues == 0 )
        {
            pixelValue = 256 * fraction;
        }
        else
        {
            pixelValue = maxNumPixelValues * fraction;                      
        }
        
        sprintf(
            caption,
            "%.0f",
            pixelValue );

        const float xOffset = paddedLeft + (paddedWidth * fraction);
        const float yOffset = paddedTop + paddedHeight + (paddedTop/2); //height - paddedTop;

        Cairo::TextExtents textExtents;
        refCairo->get_text_extents( caption, textExtents );
        refCairo->move_to( 
            xOffset - (textExtents.width / 2), 
            yOffset + (textExtents.height / 2) );
        refCairo->show_text( caption );
    }

    // Render the y-axis labels
    for ( int i=1; i < k_numPartitions; i++ )
    {
        if ( i % 2 != 0 )
        {
            continue;
        }

        const float fraction = i / static_cast<float>(k_numPartitions);       
        float scaleRatio = 100 / static_cast<float>(m_maxVal);       
        float yOffset = paddedTop + paddedHeight - (paddedHeight * fraction * scaleRatio);

        char caption[32];
        sprintf(
            caption,
            "%u%%",
            (i*100)/k_numPartitions );

        if ( yOffset > paddedTop )
        {
            Cairo::TextExtents textExtents;
            refCairo->get_text_extents( caption, textExtents );

            refCairo->move_to( 
                (paddedLeft / 2) - (textExtents.width / 2), 
                yOffset + (textExtents.height / 2) );
            refCairo->show_text( caption );
        }          
    }

    refCairo->restore();
}

void HistogramDrawingArea::GetPaddedSize( 
    float& paddedLeft, float& paddedTop, float& paddedWidth, float& paddedHeight )
{
    // Get width / height of widget
    int width;
    int height;
    get_window()->get_size( width, height ); 

    // Reserve the outside 10%
    paddedLeft = width * ((1-sk_horzScale)/2);
    paddedTop = height * ((1-sk_vertScale)/2);
    paddedWidth = width * sk_horzScale;
    paddedHeight = height * sk_vertScale;    
}

int HistogramDrawingArea::GetNumPixels( 
    unsigned int numPixelValues, 
    const int* pHistogram )
{
    int numPixels = 0;

    for ( unsigned int i=0; i < numPixelValues; i++ )
    {
        numPixels += pHistogram[i];     
    }

    return numPixels;
}

std::vector<float> HistogramDrawingArea::GetPercentages( 
    unsigned int numPixelValues, 
    const int* pHistogram )
{
    int numPixels = GetNumPixels( numPixelValues, pHistogram );

    std::vector<float> percentages;
    percentages.assign( 65536, 0 );

    if ( numPixels == 0 )
    {
        return percentages;
    }

    for ( unsigned int i=0; i < numPixelValues; i++ )
    {
        float currPercentage = (pHistogram[i] * 100)/ static_cast<float>(numPixels);
        percentages[i] = currPercentage;  
    }

    return percentages;
}

void HistogramDrawingArea::GetLineColor( 
    ImageStatistics::StatisticsChannel channel, 
    double& red, 
    double& green, 
    double& blue )
{
    switch (channel)
    {
    case ImageStatistics::GREY:
        red = 128;
        green = 128;
        blue = 128;
        break;

    case ImageStatistics::RED:
        red = 255;
        green = 0;
        blue = 0;
        break;

    case ImageStatistics::GREEN:
        red = 0;
        green = 255;
        blue = 0;
        break;

    case ImageStatistics::BLUE:
        red = 0;
        green = 0;
        blue = 255;
        break;

    case ImageStatistics::HUE:
        red = 255;
        green = 150;
        blue = 0;
        break;

    case ImageStatistics::SATURATION:
        red = 0;
        green = 255;
        blue = 255;
        break;

    case ImageStatistics::LIGHTNESS:
        red = 255;
        green = 0;
        blue = 255;
        break;

    default:
        red = 0;
        green = 0;
        blue = 0;
    }

    double divisor = 255.0;
    red /= divisor;
    green /= divisor;
    blue /= divisor;
}

void HistogramDrawingArea::SetDrawMode( Histogram::DrawMode drawMode )
{
    m_drawMode = drawMode;
}

void HistogramDrawingArea::SetRowColMode( Histogram::RowColMode rowColMode )
{
    m_rowColMode = rowColMode;
}

void HistogramDrawingArea::DrawSingleRowColChannel( 
    Cairo::RefPtr<Cairo::Context> refCairo, 
    RowColChannelStats stats,
    unsigned int numValues,
    double red,
    double green,
    double blue )
{
    if ( m_funcOptions.showMax == true )
    {
        DrawSingleRowColLine( refCairo, stats.vecMax, numValues, red, green, blue );
    }

    if ( m_funcOptions.showMin == true )
    {
         DrawSingleRowColLine( refCairo, stats.vecMin, numValues, red, green, blue );
    }

    if ( m_funcOptions.showMean == true )
    {
         DrawSingleRowColLine( refCairo, stats.vecMean, numValues, red, green, blue );
    }    
}

void HistogramDrawingArea::DrawSingleRowColLine( 
    Cairo::RefPtr<Cairo::Context> refCairo, 
    std::vector<unsigned int> vecData,
    unsigned int numValues,
    double red,
    double green,
    double blue )
{
    refCairo->save();

    // Reserve the outside 10%
    float paddedLeft, paddedTop, paddedWidth, paddedHeight;
    GetPaddedSize( paddedLeft, paddedTop, paddedWidth, paddedHeight ); 

    const float xScale = paddedWidth / static_cast<float>(vecData.size());
    const float yScale = paddedHeight / static_cast<float>(numValues);

    // Set draw color
    refCairo->set_source_rgb(red, green, blue);

    // Set line width
    refCairo->set_line_width(1.0);
      
    int k_pixelsToAvg = static_cast<int>( vecData.size() / paddedWidth);
    if ( k_pixelsToAvg == 0 )
    {
        k_pixelsToAvg = 1;
    }

    const unsigned int dataSize = static_cast<unsigned int>(vecData.size());

    for ( unsigned int i=0; i < dataSize; i++ )
    {
        unsigned int maxVal = 0;
        for ( int j=0; j < k_pixelsToAvg; j++ )
        {
            if ( i+j >= dataSize )
            {
                break;
            }

            if ( vecData[i+j] > maxVal )
            {
                maxVal = vecData[i+j];
            }
        }

        // Calculate the scaled height of the current percentage
        float yScaledHeight = yScale * maxVal;

        if ( i == 0 )
        {
            refCairo->move_to( 
                paddedLeft, 
                (paddedTop + paddedHeight - yScaledHeight) );
        }

        refCairo->line_to( 
            paddedLeft + (xScale * i), 
            paddedTop + paddedHeight - yScaledHeight );    
    }
    

    refCairo->stroke();
    
    refCairo->restore();
}

void HistogramDrawingArea::DrawRowColGridLabels( 
    Cairo::RefPtr<Cairo::Context> refCairo, 
    unsigned int numPixelValues, 
    unsigned int numValues )
{
    // Reserve the outside 10%
    float paddedLeft, paddedTop, paddedWidth, paddedHeight;
    GetPaddedSize( paddedLeft, paddedTop, paddedWidth, paddedHeight );   

    refCairo->save();

    // Set draw color
    refCairo->set_source_rgb(0, 0, 0);

    // Set the font parameters
    refCairo->select_font_face( 
        "monospace",
        Cairo::FONT_SLANT_NORMAL, 
        Cairo::FONT_WEIGHT_BOLD );
    refCairo->set_font_size( 10 );

    const int k_numPartitions = 8;      
    for ( int i=0; i <= k_numPartitions; i++ )
    {
        const float fraction = i / static_cast<float>(k_numPartitions);

        char caption[32];
        float dimensionValue = numValues * fraction;

        sprintf(
            caption,
            "%.0f",
            dimensionValue );

        const float xOffset = paddedLeft + (paddedWidth * fraction);
        const float yOffset = paddedTop + paddedHeight + (paddedTop/2);

        Cairo::TextExtents textExtents;
        refCairo->get_text_extents( caption, textExtents );
        refCairo->move_to( 
            xOffset - (textExtents.width / 2), 
            yOffset + (textExtents.height / 2) );
        refCairo->show_text( caption );
    }

    for ( int i=1; i <= k_numPartitions; i++ )
    {
        const float fraction = i / static_cast<float>(k_numPartitions);

        char caption[32];
        float pixelValue = numPixelValues * fraction;

        sprintf(
            caption,
            "%.0f",
            pixelValue );

        float yOffset = paddedTop + paddedHeight - (paddedHeight * fraction);

        Cairo::TextExtents textExtents;
        refCairo->get_text_extents( caption, textExtents );
        refCairo->move_to( 
            (paddedLeft/2) - (textExtents.width / 2), 
            yOffset + (textExtents.height / 2) );
        refCairo->show_text( caption );                   
    }

    refCairo->restore();
}
