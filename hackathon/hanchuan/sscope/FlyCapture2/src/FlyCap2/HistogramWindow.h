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
// $Id: HistogramWindow.h,v 1.8 2010/07/23 21:21:29 soowei Exp $
//=============================================================================

#ifndef HISTOGRAMWINDOW_H_INCLUDED
#define HISTOGRAMWINDOW_H_INCLUDED

#include <list>
#include "FlyCapture2.h"
#include "HistogramDrawingArea.h"

using namespace FlyCapture2;

class HistogramDrawingArea;

class HistogramWindow : public Gtk::Window
{
public:    
    HistogramWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
    virtual ~HistogramWindow(void);
    
    void Update();

    void Reset();

    void SetImageForStatistics( Image image );

protected:

private:   
    struct HistogramStruct
    {
        ImageStatistics::StatisticsChannel channel;
        Gtk::CheckButton* pCheckbutton;
        Gtk::Label* pRange;
        Gtk::Label* pMean;
        Gtk::Label* pMin;
        Gtk::Label* pMax;
    };

    Glib::RefPtr<Gnome::Glade::Xml> m_refXml;

    // Histogram notebook
    Gtk::Notebook* m_pNotebook;

    // Histogram tab widgets
    HistogramStruct m_arHistogramStruct[ImageStatistics::NUM_STATISTICS_CHANNELS];   

    Gtk::RadioButton* m_pRadioHistogramBinning8;
    Gtk::RadioButton* m_pRadioHistogramBinning10;
    Gtk::RadioButton* m_pRadioHistogramBinning12;

    Gtk::SpinButton* m_pSpinHistogramMax;
    Gtk::Adjustment* m_pAdjHistogramMax;
    Gtk::CheckButton* m_pCheckHistogramAverage;    

    // Row column plot widgets
    Gtk::RadioButton* m_pRadioRowColModeColumn;
    Gtk::RadioButton* m_pRadioRowColModeRow;

    Gtk::CheckButton* m_pCheckbuttonRowColDisplayGrey;
    Gtk::CheckButton* m_pCheckbuttonRowColDisplayRed;
    Gtk::CheckButton* m_pCheckbuttonRowColDisplayGreen;
    Gtk::CheckButton* m_pCheckbuttonRowColDisplayBlue;

    Gtk::CheckButton* m_pCheckbuttonRowColFunctionMax;
    Gtk::CheckButton* m_pCheckbuttonRowColFunctionMean;
    Gtk::CheckButton* m_pCheckbuttonRowColFunctionMin;

    Glib::RefPtr<Gdk::Pixbuf> m_iconPixBuf;	

    HistogramDrawingArea* m_pHistogramDrawingArea;

    Glib::Mutex statsMutex;

    Histogram::DrawMode m_drawMode;

    Histogram::RowColMode m_rowColMode;

    Image m_image;

    // Blank histogram states
    ImageStatistics m_blankStats;

    // Histogram stats
    ImageStatistics m_histogramStats;

    // Row col stats
    HistogramDrawingArea::RowColStats m_rowColStats;

    // Row col display options
    HistogramDrawingArea::RowColDisplayOptions m_dispOptions;

    // Row col function options
    HistogramDrawingArea::RowColFunctionOptions m_funcOptions;

    unsigned int m_histogramMax;

    static const char* GetChannelString( ImageStatistics::StatisticsChannel channel );

    bool OnDestroy( GdkEventAny* event );

    void OnSwitchPage( 
        GtkNotebookPage* page, 
        guint page_num );

    void LoadIcon();

    bool Initialize();		

    void GetWidgets();
    void AttachSignals();

    void UpdateSingleChannel( 
        ImageStatistics::StatisticsChannel channel );

    void EnableChannel( ImageStatistics::StatisticsChannel channel );
    void DisableChannel( ImageStatistics::StatisticsChannel channel );

    void EnableForPixelFormat( PixelFormat pixelFormat );   

    static void ResizeRowColChannelStats( 
        HistogramDrawingArea::RowColChannelStats* pStats, 
        unsigned int newSize );

    static void ClearRowColChannelStats(
        HistogramDrawingArea::RowColChannelStats* pStats );

    void CalculateRowColStats( 
        Image* pImage, 
        HistogramDrawingArea::RowColStats* pStats );

    void CalculateRowColStatsMono( 
        Image* pImage, 
        HistogramDrawingArea::RowColStats* pStats );

    void CalculateRowColStatsMonoColumn( 
        Image* pImage, 
        HistogramDrawingArea::RowColStats* pStats );

    void CalculateRowColStatsMonoRow( 
        Image* pImage, 
        HistogramDrawingArea::RowColStats* pStats );

    void CalculateRowColStatsColor( 
        Image* pImage, 
        HistogramDrawingArea::RowColStats* pStats );

    void CalculateRowColStatsColorColumn( 
        Image* pImage, 
        HistogramDrawingArea::RowColStats* pStats );

    void CalculateRowColStatsColorRow( 
        Image* pImage, 
        HistogramDrawingArea::RowColStats* pStats );

    // Histogram handlers
    void OnHistogramChannelChecked( ImageStatistics::StatisticsChannel channel );
    void OnHistogramMaxChanged();

    // Row/column plot handlers
    void OnRowColModeChanged( Gtk::RadioButton* pButton );
    void OnRowColDisplayChecked( Gtk::CheckButton* pButton );
    void OnRowColFunctionChecked( Gtk::CheckButton* pButton );
};

#endif // HISTOGRAMWINDOW_H_INCLUDED
