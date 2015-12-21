//=============================================================================
// Copyright © 2009 Point Grey Research, Inc. All Rights Reserved.
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
// $Id: HistogramWindow.cpp,v 1.21 2010/07/23 21:21:29 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "HistogramWindow.h"

HistogramWindow::HistogramWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject)
{		
    m_refXml = refGlade;	
    m_pHistogramDrawingArea = NULL;
    m_histogramMax = 100;
    m_drawMode = Histogram::MODE_HISTOGRAM;
    m_rowColMode = Histogram::ROWCOL_COLUMN;

    m_dispOptions.showGrey = true;
    m_dispOptions.showRed = false;
    m_dispOptions.showGreen = false;
    m_dispOptions.showBlue = false;

    m_funcOptions.showMax = true;
    m_funcOptions.showMean = true;
    m_funcOptions.showMin = true;

    Initialize();
}

HistogramWindow::~HistogramWindow()
{           
    if ( m_pAdjHistogramMax != NULL )
    {
        delete m_pAdjHistogramMax;
        m_pAdjHistogramMax = NULL;
    }

    if ( m_pHistogramDrawingArea != NULL )
    {
        delete m_pHistogramDrawingArea;
        m_pHistogramDrawingArea = NULL;   
    }
}

bool HistogramWindow::Initialize()
{    
    GetWidgets();
    AttachSignals();

    // Load the PGR icon
    LoadIcon();
    set_default_icon( m_iconPixBuf );

    set_default_size( 400, 400 );

    // Set up initial state for row col plot
    m_pRadioRowColModeColumn->set_active( true );

    m_pCheckbuttonRowColDisplayGrey->set_active( true );
    m_pCheckbuttonRowColDisplayRed->set_active( false );
    m_pCheckbuttonRowColDisplayGreen->set_active( false );
    m_pCheckbuttonRowColDisplayBlue->set_active( false );

    m_pCheckbuttonRowColFunctionMax->set_active( true );
    m_pCheckbuttonRowColFunctionMean->set_active( true );
    m_pCheckbuttonRowColFunctionMin->set_active( true );

    return true;
}	

void HistogramWindow::LoadIcon()
{
    m_iconPixBuf = Gdk::Pixbuf::create_from_inline( sizeof(PGRIcon), PGRIcon, false );        
}

void HistogramWindow::GetWidgets()
{
    m_refXml->get_widget( "notebookHistogram", m_pNotebook );

    //
    // Get the histogram widgets
    //

    for (int i=0; i < ImageStatistics::NUM_STATISTICS_CHANNELS; i++)
    {
        ImageStatistics::StatisticsChannel channel;
        channel = static_cast<ImageStatistics::StatisticsChannel>(i);
        
        const char* channelName = GetChannelString( channel );

        char checkChannel[64];
        sprintf( checkChannel, "checkHistogram%s", channelName );
        m_refXml->get_widget( checkChannel, m_arHistogramStruct[i].pCheckbutton );

        char labelRange[64];
        sprintf( labelRange, "labelHistogramRange%s", channelName );
        m_refXml->get_widget( labelRange, m_arHistogramStruct[i].pRange );

        char labelMean[64];
        sprintf( labelMean, "labelHistogramMean%s", channelName );
        m_refXml->get_widget( labelMean, m_arHistogramStruct[i].pMean );

        char labelMin[64];
        sprintf( labelMin, "labelHistogramMin%s", channelName );
        m_refXml->get_widget( labelMin, m_arHistogramStruct[i].pMin );

        char labelMax[64];
        sprintf( labelMax, "labelHistogramMax%s", channelName );
        m_refXml->get_widget( labelMax, m_arHistogramStruct[i].pMax );
    }        

    m_refXml->get_widget( "radioHistogramBinning8", m_pRadioHistogramBinning8 );
    m_refXml->get_widget( "radioHistogramBinning10", m_pRadioHistogramBinning10 );
    m_refXml->get_widget( "radioHistogramBinning12", m_pRadioHistogramBinning12 );

    m_refXml->get_widget( "spinHistogramMax", m_pSpinHistogramMax );
    m_pAdjHistogramMax = new Gtk::Adjustment( 100.0, 1.0, 100.0 );
    m_pSpinHistogramMax->set_adjustment( *m_pAdjHistogramMax );

    m_refXml->get_widget( "checkHistogramAverage", m_pCheckHistogramAverage );

    m_refXml->get_widget_derived( "drawingareaHistogram", m_pHistogramDrawingArea );

    // Disable unimplemented widgets
    m_pRadioHistogramBinning8->set_sensitive( false );
    m_pRadioHistogramBinning10->set_sensitive( false );
    m_pRadioHistogramBinning12->set_sensitive( false );    
    m_pCheckHistogramAverage->set_sensitive( false );

    //
    // Get row / column plot widgets
    //
    m_refXml->get_widget( "radioRowColModeColumn", m_pRadioRowColModeColumn );
    m_refXml->get_widget( "radioRowColModeRow", m_pRadioRowColModeRow );

    m_refXml->get_widget( "checkbuttonRowColDisplayGrey", m_pCheckbuttonRowColDisplayGrey );
    m_refXml->get_widget( "checkbuttonRowColDisplayRed", m_pCheckbuttonRowColDisplayRed );
    m_refXml->get_widget( "checkbuttonRowColDisplayGreen", m_pCheckbuttonRowColDisplayGreen );
    m_refXml->get_widget( "checkbuttonRowColDisplayBlue", m_pCheckbuttonRowColDisplayBlue );

    m_refXml->get_widget( "checkbuttonRowColFunctionMax", m_pCheckbuttonRowColFunctionMax );
    m_refXml->get_widget( "checkbuttonRowColFunctionMean", m_pCheckbuttonRowColFunctionMean );
    m_refXml->get_widget( "checkbuttonRowColFunctionMin", m_pCheckbuttonRowColFunctionMin );
}

void HistogramWindow::AttachSignals()
{
    m_pNotebook->signal_switch_page().connect(
        sigc::mem_fun( *this, &HistogramWindow::OnSwitchPage ) );

    for (int i=0; i < ImageStatistics::NUM_STATISTICS_CHANNELS; i++)
    {
        m_arHistogramStruct[i].pCheckbutton->signal_toggled().connect(
            sigc::bind<ImageStatistics::StatisticsChannel>(
            sigc::mem_fun(
                *this, &HistogramWindow::OnHistogramChannelChecked), 
                static_cast<ImageStatistics::StatisticsChannel>(i) ) );
    }    

    m_pAdjHistogramMax->signal_value_changed().connect(
        sigc::mem_fun(*this, &HistogramWindow::OnHistogramMaxChanged) );

    m_pRadioRowColModeColumn->signal_toggled().connect(
        sigc::bind<Gtk::RadioButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColModeChanged ), 
        m_pRadioRowColModeColumn ) );

    m_pRadioRowColModeRow->signal_toggled().connect(
        sigc::bind<Gtk::RadioButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColModeChanged ), 
        m_pRadioRowColModeRow ) );

    m_pCheckbuttonRowColDisplayGrey->signal_toggled().connect(
        sigc::bind<Gtk::CheckButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColDisplayChecked ), 
        m_pCheckbuttonRowColDisplayGrey ) );

    m_pCheckbuttonRowColDisplayRed->signal_toggled().connect(
        sigc::bind<Gtk::CheckButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColDisplayChecked ), 
        m_pCheckbuttonRowColDisplayRed ) );

    m_pCheckbuttonRowColDisplayGreen->signal_toggled().connect(
        sigc::bind<Gtk::CheckButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColDisplayChecked ), 
        m_pCheckbuttonRowColDisplayGreen ) );

    m_pCheckbuttonRowColDisplayBlue->signal_toggled().connect(
        sigc::bind<Gtk::CheckButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColDisplayChecked ), 
        m_pCheckbuttonRowColDisplayBlue ) );

    m_pCheckbuttonRowColFunctionMax->signal_toggled().connect(
        sigc::bind<Gtk::CheckButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColFunctionChecked ), 
        m_pCheckbuttonRowColFunctionMax ) );

    m_pCheckbuttonRowColFunctionMean->signal_toggled().connect(
        sigc::bind<Gtk::CheckButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColFunctionChecked ), 
        m_pCheckbuttonRowColFunctionMean ) );

    m_pCheckbuttonRowColFunctionMin->signal_toggled().connect(
        sigc::bind<Gtk::CheckButton*>(
        sigc::mem_fun( this, &HistogramWindow::OnRowColFunctionChecked ), 
        m_pCheckbuttonRowColFunctionMin ) );
}

void HistogramWindow::OnSwitchPage( GtkNotebookPage* /*page*/, guint page_num )
{
    if ( page_num == 0 )
    {
        m_drawMode = Histogram::MODE_HISTOGRAM;
    }
    else if ( page_num == 1 )
    {
        m_drawMode = Histogram::MODE_ROWCOL;
    }
}

void HistogramWindow::Update()
{
    Glib::Mutex::Lock lock(statsMutex);

    EnableForPixelFormat( m_image.GetPixelFormat() );

    if ( m_drawMode == Histogram::MODE_HISTOGRAM )
    {
        m_pHistogramDrawingArea->SetDrawMode( Histogram::MODE_HISTOGRAM );        

        UpdateSingleChannel( ImageStatistics::GREY );
        UpdateSingleChannel( ImageStatistics::RED );
        UpdateSingleChannel( ImageStatistics::GREEN );
        UpdateSingleChannel( ImageStatistics::BLUE );
        UpdateSingleChannel( ImageStatistics::HUE );
        UpdateSingleChannel( ImageStatistics::SATURATION );
        UpdateSingleChannel( ImageStatistics::LIGHTNESS );

        // Update the histogram drawing area
        m_pHistogramDrawingArea->Update( &m_histogramStats, m_histogramMax );
    }
    else
    {
        m_pHistogramDrawingArea->SetDrawMode( Histogram::MODE_ROWCOL );

        m_pHistogramDrawingArea->Update(
            &m_rowColStats,
            m_dispOptions,
            m_funcOptions );
    }    
}

void HistogramWindow::Reset()
{
    Glib::Mutex::Lock lock(statsMutex);

    m_drawMode = Histogram::MODE_HISTOGRAM;

    for (int i=0; i < ImageStatistics::NUM_STATISTICS_CHANNELS; i++)
    {
        m_arHistogramStruct[i].pCheckbutton->set_active(false);
    }    

    m_pHistogramDrawingArea->Update(&m_blankStats, m_histogramMax);
}

void HistogramWindow::UpdateSingleChannel( ImageStatistics::StatisticsChannel channel )
{
    unsigned int rangeMin, rangeMax, pixelValueMin, pixelValueMax;
    unsigned int numPixelValues;
    float pixelValueMean;

    m_histogramStats.GetStatistics( 
        channel,
        &rangeMin,
        &rangeMax,
        &pixelValueMin,
        &pixelValueMax,
        &numPixelValues,
        &pixelValueMean );    

    // If the channel is disabled, then there is no updating to be done
    if ( m_arHistogramStruct[channel].pCheckbutton->get_state() == Gtk::STATE_INSENSITIVE )
    {
        return;
    }

    bool enabled = false;
    m_histogramStats.GetChannelStatus( channel, &enabled );

    if ( enabled != true )
    {
        m_arHistogramStruct[channel].pCheckbutton->set_active( false );

        const char* rangeZero = "0 - 0";
        const char* meanZero = "0.00";        
        const char* minZero = "0";
        const char* maxZero = "0";

        m_arHistogramStruct[channel].pRange->set_text( rangeZero );
        m_arHistogramStruct[channel].pMean->set_text( meanZero );   
        m_arHistogramStruct[channel].pMin->set_text( minZero );
        m_arHistogramStruct[channel].pMax->set_text( maxZero );        
    }
    else
    {
        m_arHistogramStruct[channel].pCheckbutton->set_active( true );

        char rangeStr[32];
        sprintf( rangeStr, "%d - %d", rangeMin, rangeMax );
        m_arHistogramStruct[channel].pRange->set_text( rangeStr );

        char meanStr[32];
        sprintf( meanStr, "%3.2f", pixelValueMean );
        m_arHistogramStruct[channel].pMean->set_text( meanStr );

        char minStr[32];
        sprintf( minStr, "%d", pixelValueMin );
        m_arHistogramStruct[channel].pMin->set_text( minStr );

        char maxStr[32];
        sprintf( maxStr, "%d", pixelValueMax );
        m_arHistogramStruct[channel].pMax->set_text( maxStr );        
    }    
}

void HistogramWindow::OnHistogramChannelChecked( ImageStatistics::StatisticsChannel channel )
{
    Glib::Mutex::Lock lock(statsMutex);

    bool enabled = m_arHistogramStruct[channel].pCheckbutton->get_active();

    m_histogramStats.SetChannelStatus( channel, enabled );
}

void HistogramWindow::OnHistogramMaxChanged()
{
    m_histogramMax = static_cast<unsigned int>(m_pAdjHistogramMax->get_value());
}

const char* HistogramWindow::GetChannelString( ImageStatistics::StatisticsChannel channel )
{
    switch (channel)
    {
    case ImageStatistics::GREY:
        return "Grey";
    case ImageStatistics::RED:
        return "Red";
    case ImageStatistics::GREEN:
        return "Green";
    case ImageStatistics::BLUE:
        return "Blue";    
    case ImageStatistics::HUE:
        return "Hue";
    case ImageStatistics::SATURATION:
        return "Saturation";
    case ImageStatistics::LIGHTNESS:
        return "Lightness";
    default:
        return "Error";
    }
} 

void HistogramWindow::EnableChannel( ImageStatistics::StatisticsChannel channel )
{  
    m_arHistogramStruct[channel].pCheckbutton->set_sensitive( true );
    m_arHistogramStruct[channel].pRange->set_sensitive( true );
    m_arHistogramStruct[channel].pMean->set_sensitive( true );
    m_arHistogramStruct[channel].pMin->set_sensitive( true );
    m_arHistogramStruct[channel].pMax->set_sensitive( true );
}

void HistogramWindow::DisableChannel( ImageStatistics::StatisticsChannel channel )
{    
    const char* naStr = "N/A";

    m_arHistogramStruct[channel].pCheckbutton->set_sensitive( false );
    m_arHistogramStruct[channel].pRange->set_sensitive( false );
    m_arHistogramStruct[channel].pMean->set_sensitive( false );
    m_arHistogramStruct[channel].pMin->set_sensitive( false );
    m_arHistogramStruct[channel].pMax->set_sensitive( false );

    m_arHistogramStruct[channel].pRange->set_text( naStr );
    m_arHistogramStruct[channel].pMean->set_text( naStr );
    m_arHistogramStruct[channel].pMin->set_text( naStr );
    m_arHistogramStruct[channel].pMax->set_text( naStr );
}

void HistogramWindow::EnableForPixelFormat( PixelFormat pixelFormat )
{    
    if ( m_drawMode == Histogram::MODE_HISTOGRAM )
    {
        switch (pixelFormat)
        {
        case PIXEL_FORMAT_MONO8:
        case PIXEL_FORMAT_MONO16:
            EnableChannel( ImageStatistics::GREY );
            DisableChannel( ImageStatistics::RED );
            DisableChannel( ImageStatistics::GREEN );
            DisableChannel( ImageStatistics::BLUE );
            DisableChannel( ImageStatistics::HUE );
            DisableChannel( ImageStatistics::SATURATION );
            DisableChannel( ImageStatistics::LIGHTNESS );
            break;
        case PIXEL_FORMAT_BGR:
        case PIXEL_FORMAT_RGB:
        case PIXEL_FORMAT_BGRU:
        case PIXEL_FORMAT_RGBU:
            EnableChannel( ImageStatistics::GREY );
            EnableChannel( ImageStatistics::RED );
            EnableChannel( ImageStatistics::GREEN );
            EnableChannel( ImageStatistics::BLUE );
            EnableChannel( ImageStatistics::HUE );
            EnableChannel( ImageStatistics::SATURATION );
            EnableChannel( ImageStatistics::LIGHTNESS );
            break;
        default:
            DisableChannel( ImageStatistics::GREY );
            DisableChannel( ImageStatistics::RED );
            DisableChannel( ImageStatistics::GREEN );
            DisableChannel( ImageStatistics::BLUE );
            DisableChannel( ImageStatistics::HUE );
            DisableChannel( ImageStatistics::SATURATION );
            DisableChannel( ImageStatistics::LIGHTNESS );
            break;
        }
    }
    else if ( m_drawMode == Histogram::MODE_ROWCOL )
    {
        switch (pixelFormat)
        {
        case PIXEL_FORMAT_MONO8:
        case PIXEL_FORMAT_MONO16:
            m_pCheckbuttonRowColDisplayGrey->set_sensitive( true );
            m_pCheckbuttonRowColDisplayRed->set_sensitive( false );
            m_pCheckbuttonRowColDisplayGreen->set_sensitive( false );
            m_pCheckbuttonRowColDisplayBlue->set_sensitive( false );
            break;
        case PIXEL_FORMAT_BGR:
        case PIXEL_FORMAT_RGB:
        case PIXEL_FORMAT_BGRU:
        case PIXEL_FORMAT_RGBU:
            m_pCheckbuttonRowColDisplayGrey->set_sensitive( true );
            m_pCheckbuttonRowColDisplayRed->set_sensitive( true );
            m_pCheckbuttonRowColDisplayGreen->set_sensitive( true );
            m_pCheckbuttonRowColDisplayBlue->set_sensitive( true );
            break;
        default:
            break;
        }
    }    
}

void HistogramWindow::SetImageForStatistics( Image image )
{
    Glib::Mutex::Lock lock(statsMutex, Glib::NOT_LOCK);
    if ( lock.try_acquire() == true && is_visible() == true )
    {
        m_image.DeepCopy( &image );

        switch (m_drawMode)
        {
        case Histogram::MODE_HISTOGRAM:
            m_image.CalculateStatistics( &m_histogramStats );
            break;

        case Histogram::MODE_ROWCOL:
            CalculateRowColStats( &m_image, &m_rowColStats );
            break;
        }        
    }  
}

void HistogramWindow::CalculateRowColStats( 
    Image* pImage, 
    HistogramDrawingArea::RowColStats* pStats )
{
    if ( pImage == NULL || pStats == NULL )
    {
        return;
    }

    if ( m_pCheckbuttonRowColDisplayGrey->get_active() == false &&
        m_pCheckbuttonRowColDisplayRed->get_active() == false &&
        m_pCheckbuttonRowColDisplayGreen->get_active() == false &&
        m_pCheckbuttonRowColDisplayBlue->get_active() == false )
    {
        return;
    }

    PixelFormat pixelFormat = pImage->GetPixelFormat();
    
    unsigned int imageWidth, imageHeight;
    pImage->GetDimensions( &imageHeight, &imageWidth );

    if ( m_rowColMode == Histogram::ROWCOL_COLUMN )
    {
        pStats->imageDimension = imageWidth;
    }
    else if ( m_rowColMode == Histogram::ROWCOL_ROW )
    {
        pStats->imageDimension = imageHeight;
    }

    switch (pixelFormat)
    {
    case PIXEL_FORMAT_MONO8:
    case PIXEL_FORMAT_MONO16:
        CalculateRowColStatsMono( pImage, pStats );
        break;

    case PIXEL_FORMAT_BGR:
    case PIXEL_FORMAT_RGB:
    case PIXEL_FORMAT_BGRU:
    case PIXEL_FORMAT_RGBU:
        CalculateRowColStatsColor( pImage, pStats );
        break;

    default:
        break;
    }
}

void HistogramWindow::CalculateRowColStatsMono( 
    Image* pImage, 
    HistogramDrawingArea::RowColStats* pStats )
{    
    PixelFormat pixelFormat = pImage->GetPixelFormat();
    switch (pixelFormat)
    {
    case PIXEL_FORMAT_MONO8:
        pStats->numPixelValues = 256;
        break;
    case PIXEL_FORMAT_MONO16:
        pStats->numPixelValues = 65536;
        break;
    default:
        break;
    }

    if ( m_rowColMode == Histogram::ROWCOL_COLUMN )
    {
        CalculateRowColStatsMonoColumn( pImage, pStats );
    }
    else if ( m_rowColMode == Histogram::ROWCOL_ROW )
    {
        CalculateRowColStatsMonoRow( pImage, pStats );
    }    
}

void HistogramWindow::CalculateRowColStatsMonoColumn( 
    Image* pImage, 
    HistogramDrawingArea::RowColStats* pStats )
{
    unsigned int rows, cols, stride;
    pImage->GetDimensions( &rows, &cols, &stride );   

    PixelFormat pixelFormat = pImage->GetPixelFormat();

    ClearRowColChannelStats( &pStats->red );
    ClearRowColChannelStats( &pStats->green );
    ClearRowColChannelStats( &pStats->blue );

    unsigned char* pData = pImage->GetData();    

    ResizeRowColChannelStats( &pStats->grey, cols );
    std::vector<unsigned int> vecMeanTotal( cols );

    for ( unsigned int x=0; x < cols; x++ )
    {
        unsigned int minVal = 0;
        unsigned int maxVal = 0;
        unsigned int accumVal = 0;

        for ( unsigned int y=0; y < rows; y++ )
        {
            unsigned int currPixVal = 0;

            if ( pixelFormat == PIXEL_FORMAT_MONO8 )
            {
                currPixVal = pData[(y*stride) + x];
            }
            else if ( pixelFormat == PIXEL_FORMAT_MONO16 )
            {
                currPixVal = pData[ y*stride + 2*x + 1 ];
                currPixVal = currPixVal << 8;
                currPixVal |= pData[ y*stride + 2*x ];
            }                

            if ( y == 0 )
            {
                minVal = maxVal = currPixVal;
            }

            accumVal += currPixVal;

            if ( minVal > currPixVal )
            {
                minVal = currPixVal;
            }
            else if ( maxVal < currPixVal )
            {
                maxVal = currPixVal;
            }

            pStats->grey.vecMax[x] = maxVal;
            pStats->grey.vecMin[x] = minVal;
            pStats->grey.vecMean[x] = accumVal / rows;            
        }
    }
}

void HistogramWindow::CalculateRowColStatsMonoRow( 
    Image* pImage, 
    HistogramDrawingArea::RowColStats* pStats )
{
    unsigned int rows, cols, stride;
    pImage->GetDimensions( &rows, &cols, &stride );

    PixelFormat pixelFormat = pImage->GetPixelFormat();

    ClearRowColChannelStats( &pStats->red );
    ClearRowColChannelStats( &pStats->green );
    ClearRowColChannelStats( &pStats->blue );

    unsigned char* pData = pImage->GetData();    

    ResizeRowColChannelStats( &pStats->grey, rows );
    std::vector<unsigned int> vecMeanTotal( rows );

    for ( unsigned int y=0; y < rows; y++ )
    {
        unsigned int minVal = 0;
        unsigned int maxVal = 0;
        unsigned int accumVal = 0;

        for ( unsigned int x=0; x < cols; x++ )
        {
            unsigned int currPixVal = 0;

            if ( pixelFormat == PIXEL_FORMAT_MONO8 )
            {
                currPixVal = pData[(y*stride) + x];
            }
            else if ( pixelFormat == PIXEL_FORMAT_MONO16 )
            {
                currPixVal = pData[ y*stride + 2*x + 1 ];
                currPixVal = currPixVal << 8;
                currPixVal |= pData[ y*stride + 2*x ];
            }    

            if ( x == 0 )
            {
                minVal = maxVal = currPixVal;
            }

            accumVal += currPixVal;

            if ( minVal > currPixVal )
            {
                minVal = currPixVal;
            }
            else if ( maxVal < currPixVal )
            {
                maxVal = currPixVal;
            }

            pStats->grey.vecMax[y] = maxVal;
            pStats->grey.vecMin[y] = minVal;
            pStats->grey.vecMean[y] = accumVal / cols;         
        }
    }
}

void HistogramWindow::CalculateRowColStatsColor( 
    Image* pImage, 
    HistogramDrawingArea::RowColStats* pStats )
{
    pStats->numPixelValues = 256;

    if ( m_rowColMode == Histogram::ROWCOL_COLUMN )
    {
        CalculateRowColStatsColorColumn( pImage, pStats );
    }
    else if ( m_rowColMode == Histogram::ROWCOL_ROW )
    {
        CalculateRowColStatsColorRow( pImage, pStats );
    }    
}

void HistogramWindow::CalculateRowColStatsColorColumn( 
    Image* pImage, 
    HistogramDrawingArea::RowColStats* pStats )
{
    unsigned int rows, cols, stride;
    pImage->GetDimensions( &rows, &cols, &stride );

    unsigned int bytesPerPixel = pImage->GetBitsPerPixel() / 8;

    PixelFormat pixelFormat = pImage->GetPixelFormat();    

    ResizeRowColChannelStats( &pStats->grey, cols );
    ResizeRowColChannelStats( &pStats->red, cols );
    ResizeRowColChannelStats( &pStats->green, cols );
    ResizeRowColChannelStats( &pStats->blue, cols );    

    std::vector<unsigned int> vecGreyMeanTotal( cols );
    std::vector<unsigned int> vecRedMeanTotal( cols );
    std::vector<unsigned int> vecGreenMeanTotal( cols );
    std::vector<unsigned int> vecBlueMeanTotal( cols );

    unsigned char* pData = pImage->GetData();    

    // For the vectors below, assume the order they are in as:
    // 0 - Grey
    // 1 - Red
    // 2 - Green
    // 3 - Blue    
    for ( unsigned int x=0; x < cols; x++ )
    {        
        std::vector<unsigned int> minVal(4, 0);
        std::vector<unsigned int> maxVal(4, 0);
        std::vector<unsigned int> accumVal(4, 0);
        
        for ( unsigned int y=0; y < rows; y++ )
        {
            std::vector<unsigned int> currPixVal(4, 0);     

            unsigned char* pCurrent = pData + (y*stride) + (bytesPerPixel*x);

            switch( pixelFormat )           
            {
            case PIXEL_FORMAT_BGR:
            case PIXEL_FORMAT_BGRU:
                currPixVal[3] = *pCurrent++;
                currPixVal[2] = *pCurrent++;
                currPixVal[1] = *pCurrent;                
                break;

            case PIXEL_FORMAT_RGB:
            case PIXEL_FORMAT_RGBU:
                currPixVal[1] = *pCurrent++;
                currPixVal[2] = *pCurrent++;
                currPixVal[3] = *pCurrent;
                break;
            default:
                currPixVal[1] = currPixVal[2] = currPixVal[3] = 0;
            }

            // Calculate the grey value
            currPixVal[0] = ( currPixVal[3] + 6*currPixVal[2] + 3*currPixVal[1] ) / 10;

            if ( y == 0 )
            {                
                for ( unsigned int i=0; i < minVal.size(); i++ )
                {
                    minVal[i] = maxVal[i] = currPixVal[i];
                }
            }

            for ( unsigned int i=0; i < minVal.size(); i++ )
            {
                accumVal[i] += currPixVal[i];

                if ( minVal[i] > currPixVal[i] )
                {
                    minVal[i] = currPixVal[i];
                }
                else if ( maxVal[i] < currPixVal[i] )
                {
                    maxVal[i] = currPixVal[i];
                }
            }

            pStats->grey.vecMax[x] = maxVal[0];
            pStats->grey.vecMin[x] = minVal[0];
            pStats->grey.vecMean[x] = accumVal[0] / rows;         

            pStats->red.vecMax[x] = maxVal[1];
            pStats->red.vecMin[x] = minVal[1];
            pStats->red.vecMean[x] = accumVal[1] / rows;   

            pStats->green.vecMax[x] = maxVal[2];
            pStats->green.vecMin[x] = minVal[2];
            pStats->green.vecMean[x] = accumVal[2] / rows;   

            pStats->blue.vecMax[x] = maxVal[3];
            pStats->blue.vecMin[x] = minVal[3];
            pStats->blue.vecMean[x] = accumVal[3] / rows;   
        }
    }
}

void HistogramWindow::CalculateRowColStatsColorRow( 
    Image* pImage, 
    HistogramDrawingArea::RowColStats* pStats )
{
    unsigned int rows, cols, stride;
    pImage->GetDimensions( &rows, &cols, &stride );

    unsigned int bytesPerPixel = pImage->GetBitsPerPixel() / 8;

    PixelFormat pixelFormat = pImage->GetPixelFormat();

    pStats->numPixelValues = 256;

    ResizeRowColChannelStats( &pStats->grey, rows );
    ResizeRowColChannelStats( &pStats->red, rows );
    ResizeRowColChannelStats( &pStats->green, rows );
    ResizeRowColChannelStats( &pStats->blue, rows );    

    std::vector<unsigned int> vecGreyMeanTotal( rows );
    std::vector<unsigned int> vecRedMeanTotal( rows );
    std::vector<unsigned int> vecGreenMeanTotal( rows );
    std::vector<unsigned int> vecBlueMeanTotal( rows );

    unsigned char* pData = pImage->GetData();    

    // For the vectors below, assume the order they are in as:
    // 0 - Grey
    // 1 - Red
    // 2 - Green
    // 3 - Blue    
    for ( unsigned int y=0; y < rows; y++ )
    {        
        std::vector<unsigned int> minVal(4, 0);
        std::vector<unsigned int> maxVal(4, 0);
        std::vector<unsigned int> accumVal(4, 0);

        for ( unsigned int x=0; x < cols; x++ )
        {
            std::vector<unsigned int> currPixVal(4, 0);     

            unsigned char* pCurrent = pData + (y*stride) + (bytesPerPixel*x);

            switch( pixelFormat )           
            {
            case PIXEL_FORMAT_BGR:
            case PIXEL_FORMAT_BGRU:
                currPixVal[3] = *pCurrent++;
                currPixVal[2] = *pCurrent++;
                currPixVal[1] = *pCurrent;                
                break;

            case PIXEL_FORMAT_RGB:
            case PIXEL_FORMAT_RGBU:
                currPixVal[1] = *pCurrent++;
                currPixVal[2] = *pCurrent++;
                currPixVal[3] = *pCurrent;
                break;
            default:
                currPixVal[1] = currPixVal[2] = currPixVal[3] = 0;
            }

            // Calculate the grey value
            currPixVal[0] = ( currPixVal[3] + 6*currPixVal[2] + 3*currPixVal[1] ) / 10;

            if ( x == 0 )
            {                
                for ( unsigned int i=0; i < minVal.size(); i++ )
                {
                    minVal[i] = maxVal[i] = currPixVal[i];
                }
            }

            for ( unsigned int i=0; i < minVal.size(); i++ )
            {
                accumVal[i] += currPixVal[i];

                if ( minVal[i] > currPixVal[i] )
                {
                    minVal[i] = currPixVal[i];
                }
                else if ( maxVal[i] < currPixVal[i] )
                {
                    maxVal[i] = currPixVal[i];
                }
            }

            pStats->grey.vecMax[y] = maxVal[0];
            pStats->grey.vecMin[y] = minVal[0];
            pStats->grey.vecMean[y] = accumVal[0] / cols;         

            pStats->red.vecMax[y] = maxVal[1];
            pStats->red.vecMin[y] = minVal[1];
            pStats->red.vecMean[y] = accumVal[1] / cols;   

            pStats->green.vecMax[y] = maxVal[2];
            pStats->green.vecMin[y] = minVal[2];
            pStats->green.vecMean[y] = accumVal[2] / cols;   

            pStats->blue.vecMax[y] = maxVal[3];
            pStats->blue.vecMin[y] = minVal[3];
            pStats->blue.vecMean[y] = accumVal[3] / cols;   
        }
    }
}

void HistogramWindow::ResizeRowColChannelStats( 
    HistogramDrawingArea::RowColChannelStats* pStats, 
    unsigned int newSize )
{
    if ( pStats->vecMax.size() != newSize )
    {
        pStats->vecMax.resize( newSize, 0 );
    }

    if ( pStats->vecMean.size() != newSize )
    {
        pStats->vecMean.resize( newSize, 0 );
    }

    if ( pStats->vecMin.size() != newSize )
    {
        pStats->vecMin.resize( newSize, 0 );
    }
}

void HistogramWindow::ClearRowColChannelStats( 
    HistogramDrawingArea::RowColChannelStats* pStats )
{
    if ( pStats->vecMax.size() != 0 )
    {
        pStats->vecMax.clear();
    }

    if ( pStats->vecMean.size() != 0 )
    {
        pStats->vecMean.clear();
    }

    if ( pStats->vecMin.size() != 0 )
    {
        pStats->vecMin.clear();
    }
}

void HistogramWindow::OnRowColModeChanged( Gtk::RadioButton* pButton )
{
    if ( pButton->get_active() != true )
    {
        return;
    }

    if ( pButton == m_pRadioRowColModeColumn )
    {
        m_rowColMode = Histogram::ROWCOL_COLUMN;
    }
    else if ( pButton == m_pRadioRowColModeRow )
    {
        m_rowColMode = Histogram::ROWCOL_ROW;
    }
}

void HistogramWindow::OnRowColDisplayChecked( Gtk::CheckButton* pButton )
{
    bool checked = pButton->get_active();

    if ( pButton == m_pCheckbuttonRowColDisplayGrey )
    {
        m_dispOptions.showGrey = checked;
    }
    else if ( pButton == m_pCheckbuttonRowColDisplayRed )
    {
        m_dispOptions.showRed = checked;
    }
    else if ( pButton == m_pCheckbuttonRowColDisplayGreen )
    {
        m_dispOptions.showGreen = checked;
    }
    else if ( pButton == m_pCheckbuttonRowColDisplayBlue )
    {
        m_dispOptions.showBlue = checked;
    }
}

void HistogramWindow::OnRowColFunctionChecked( Gtk::CheckButton* pButton )
{
    bool checked = pButton->get_active();

    if ( pButton == m_pCheckbuttonRowColFunctionMax )
    {
        m_funcOptions.showMax = checked;
    }
    else if ( pButton == m_pCheckbuttonRowColFunctionMean )
    {
        m_funcOptions.showMean = checked;
    }
    else if ( pButton == m_pCheckbuttonRowColFunctionMin )
    {
        m_funcOptions.showMin = checked;
    }
}
