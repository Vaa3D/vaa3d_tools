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
// $Id: LUTPage.cpp,v 1.28 2010/05/14 21:38:45 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "LUTPage.h"
#include <vector>

namespace FlyCapture2
{
    const char* LUTPage::sk_curveLUT = "curveLut";
    const char* LUTPage::sk_inputMin = "inputMinValueLut";
    const char* LUTPage::sk_inputMid = "inputMidValueLut";
    const char* LUTPage::sk_inputMax = "inputMaxValueLut";
    const char* LUTPage::sk_outputMin = "outputMinValueLut";
    const char* LUTPage::sk_outputMid = "outputMidValueLut";
    const char* LUTPage::sk_outputMax = "outputMaxValueLut";
    const char* LUTPage::sk_resetButton = "buttonResetLut";
    const char* LUTPage::sk_channelCombo = "comboboxChannelLut";
    const char* LUTPage::sk_loadFile = "buttonLoadFileLut";
    const char* LUTPage::sk_saveFile = "buttonSaveFileLut";
    const char* LUTPage::sk_loadCamera = "buttonLoadCameraLut";
    const char* LUTPage::sk_setCamera = "buttonSetCameraLut";
    const char* LUTPage::sk_enable = "checkButtonEnableLut";
    const char* LUTPage::sk_linearRadio = "radioButtonLinearLut";
    const char* LUTPage::sk_splineRadio = "radioButtonSplineLut";
    const char* LUTPage::sk_freeRadio = "radioButtonFreeLut";

    LUTPage::LUTPage()
    {
    }

    LUTPage::LUTPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
    {
    }

    LUTPage::~LUTPage()
    {
    }

    void LUTPage::UpdateWidgets()
    {       
        if ( m_pCamera == NULL || IsConnected() != true )
        {
            DisableWidgets();
            return;	
        }

        Error error;
        LUTData data;		
        error = m_pCamera->GetLUTInfo( &data );
        if( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error querying LUT support", error );
            DisableWidgets();
            return;
        }
        else
        {
            /*
            printf( "Supported: %u\nenabled: %u\nbitDepth: %u\nnumEntries: %u\nnumChannels: %u\n",
                data.supported,
                data.enabled,
                data.bitDepth,
                data.numEntries,
                data.numChannels );
            */
        }

        if( !data.supported )
        {
            DisableWidgets();
            return;
        }
        else
        {
            EnableWidgets();
        }

        PopulateChannelComboBox( data.numChannels );

        const unsigned int inputMin = 0;
        const unsigned int inputMax = data.numEntries > 512 ? (data.numEntries/4) - 1 : data.numEntries-1;
        const unsigned int inputMid = inputMax / 2;
        const unsigned int outputMin = 0;
        const double bitDepth = static_cast<double>(data.outputBitDepth);
        const unsigned int outputMax = static_cast<unsigned int>(pow(2,bitDepth)-1);
        const unsigned int outputMid = outputMax / 2;

        m_pCurve->set_range( 
            static_cast<float>(inputMin), 
            static_cast<float>(inputMax), 
            static_cast<float>(outputMin), 
            static_cast<float>(outputMax) );

        char tempStr[ 256 ];

        m_pInputMin->set_text( "0" );

        sprintf( tempStr, "%u", inputMid );
        m_pInputMid->set_text( tempStr );

        sprintf( tempStr, "%u", inputMax );
        m_pInputMax->set_text( tempStr);

        m_pOutputMin->set_text( "0" );

        sprintf( tempStr, "%u", outputMid );
        m_pOutputMid->set_text( tempStr );

        sprintf( tempStr, "%u", outputMax );
        m_pOutputMax->set_text( tempStr );

        m_pEnable->set_active( data.enabled );

        OnLoadFromCamera();
    }

    void LUTPage::GetWidgets()
    {
        m_refXml->get_widget( sk_curveLUT, m_pCurve );

        m_pCurve->modify_fg( Gtk::STATE_NORMAL, Gdk::Color( "Red" ) );
        m_pCurve->modify_bg( Gtk::STATE_NORMAL, Gdk::Color( "White" ) );

        m_refXml->get_widget( sk_inputMin, m_pInputMin );
        m_refXml->get_widget( sk_inputMid, m_pInputMid );
        m_refXml->get_widget( sk_inputMax, m_pInputMax );

        m_refXml->get_widget( sk_outputMin, m_pOutputMin );
        m_refXml->get_widget( sk_outputMid, m_pOutputMid );
        m_refXml->get_widget( sk_outputMax, m_pOutputMax );

        m_refXml->get_widget( sk_resetButton, m_pReset );
        m_refXml->get_widget( sk_channelCombo, m_pChannel );
        m_refXml->get_widget( sk_enable, m_pEnable );

        m_refXml->get_widget( sk_loadFile, m_pLoadFile );
        m_refXml->get_widget( sk_saveFile, m_pSaveFile );
        m_refXml->get_widget( sk_loadCamera, m_pLoadCamera );
        m_refXml->get_widget( sk_setCamera, m_pSetCamera );

        m_refXml->get_widget( sk_linearRadio, m_pLinear );
        m_refXml->get_widget( sk_splineRadio, m_pSpline );
        m_refXml->get_widget( sk_freeRadio, m_pFree );	 

		m_refListStoreChannels = Gtk::ListStore::create( m_channelColumns );
		m_pChannel->set_model( m_refListStoreChannels );	
    }

    void 
        LUTPage::AttachSignals()
    {
        m_pLoadFile->signal_clicked().connect(
            sigc::mem_fun( *this, &LUTPage::OnLoadFromFile ) );
        m_pSaveFile->signal_clicked().connect(
            sigc::mem_fun( *this, &LUTPage::OnSaveToFile ) );
        m_pLoadCamera->signal_clicked().connect(
            sigc::mem_fun( *this, &LUTPage::OnLoadFromCamera ) );
        m_pSetCamera->signal_clicked().connect(
            sigc::mem_fun( *this, &LUTPage::OnSetToCamera ) );
        m_pReset->signal_clicked().connect(
            sigc::mem_fun( *this, &LUTPage::OnReset ) );
        m_pChannel->signal_changed().connect(
            sigc::mem_fun( *this, &LUTPage::OnChannelChange ) );
        m_pLinear->signal_toggled().connect(
            sigc::mem_fun( *this, &LUTPage::OnStyleChange ) );
        m_pSpline->signal_toggled().connect(
            sigc::mem_fun( *this, &LUTPage::OnStyleChange ) );
        m_pFree->signal_toggled().connect(
            sigc::mem_fun( *this, &LUTPage::OnStyleChange ) );
        m_pEnable->signal_clicked().connect(
            sigc::mem_fun( *this, &LUTPage::OnToggleEnable ) );
        m_pCurve->signal_curve_type_changed().connect(
            sigc::mem_fun( *this, &LUTPage::OnCurveStyleChanged ) );
    }

    void
        LUTPage::OnReset()
    {
        m_pCurve->reset();
    }

    void
        LUTPage::OnLoadFromFile()
    {
        // Make LUT is available
        Error error;
        LUTData data;		
        error = m_pCamera->GetLUTInfo( &data );
        if( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error querying LUT support", error );
            return;
        }

        Gtk::FileChooserDialog loadDialog( 
            "Please select a LUT file", 
            Gtk::FILE_CHOOSER_ACTION_OPEN );

        // Add response buttons the the dialog:
        loadDialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
        loadDialog.add_button( Gtk::Stock::OPEN, Gtk::RESPONSE_OK );       

        // Only allow single selection of files
        loadDialog.set_select_multiple( false );

        AddFileFilters( loadDialog );

        int result = loadDialog.run();            
        if ( result != Gtk::RESPONSE_OK )
        {
            return;
        }

        std::string specifiedFilename = loadDialog.get_filename();

        // Open the file
        Glib::RefPtr<Gio::File> loadFile;
        loadFile = Gio::File::create_for_path( specifiedFilename );

        // Open output stream
        Glib::RefPtr<Gio::DataInputStream> loadInputStream;
        loadInputStream = Gio::DataInputStream::create( loadFile->read() );        

        std::vector<float> entries;
        for( unsigned int i = 0; i < data.numEntries; i++ )
        {
            std::string currLine;
            loadInputStream->read_line( currLine );

            // Scan the line to find the values
            unsigned int currInput = 0;
            unsigned int currResult = 0;
            int iNumRead = sscanf( 
                currLine.c_str(),
                "%u,%u\n",
                &currInput,
                &currResult );
            if ( iNumRead == 2 )
            {
                if ( currInput < 0 || 
                    currInput >= data.numEntries ||
                    currInput != i )
                {
                    ShowErrorMessageDialog( 
                        "Invalid LUT data detected", 
                        "LUT data appears to be invalid. Aborting file load." );
                    loadInputStream->close();
                    return;
                }
            }
            else
            {
                ShowErrorMessageDialog( 
                    "Error reading LUT data from file", 
                    "Required format not found. Aborting file load." );
                loadInputStream->close();
                return;
            }

            // Data passed validation check, add it to the list of entries
            entries.push_back( (float)currResult );
        }        

        // Close output stream
        loadInputStream->close();       

        std::vector<float> vecToUse;
        if ( data.numEntries > 512 )
        {
            vecToUse.resize( entries.size()/4, 0.0 );
            for( unsigned int i = 0; i < vecToUse.size(); i++ )
            {
                float avgValue = 0.0f;
                for ( unsigned int j=0; j < 4; j++ )
                {
                    avgValue += static_cast<float>(entries[(i*4)+j]);
                }

                avgValue /= 4.0f;

                vecToUse[i] = avgValue;
            }            
        }     
        else
        {
            vecToUse = entries;
        }

        m_pCurve->set_vector( vecToUse );

        OnSetToCamera();
    }

    void
        LUTPage::OnSaveToFile()
    {
        // Make LUT is available
        Error error;
        LUTData data;		
        error = m_pCamera->GetLUTInfo( &data );
        if( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error querying LUT support", error );
            return;
        }

        Gtk::FileChooserDialog saveDialog( 
            "Please enter a LUT filename", 
            Gtk::FILE_CHOOSER_ACTION_SAVE );
        
        // Add response buttons the the dialog:
        saveDialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
        saveDialog.add_button( Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK );       

        // Set the default filename that appears
        char defaultFilename[128];
        sprintf(
            defaultFilename, 
            "%u.lut",
            m_camInfo.serialNumber );
        saveDialog.set_current_name( defaultFilename );

        // Prompt to overwrite an existing file
        saveDialog.set_do_overwrite_confirmation( true );

        // Only allow single selection of files
        saveDialog.set_select_multiple( false );

        AddFileFilters( saveDialog );
        
        int result = saveDialog.run();            
        if ( result != Gtk::RESPONSE_OK )
        {
            return;
        }
            
        std::string specifiedFilename = saveDialog.get_filename();

        // Open the file
        Glib::RefPtr<Gio::File> saveFile;
        saveFile = Gio::File::create_for_path( specifiedFilename );

        // Open output stream
        Glib::RefPtr<Gio::FileOutputStream> saveOutputStream;
        saveOutputStream = saveFile->replace();        

        std::vector<float> entries = m_pCurve->get_vector( data.numEntries );
        std::string finalStr;
        
        for( unsigned int i = 0; i < data.numEntries; i++ )
        {
            char singleEntry[32];
            sprintf(
                singleEntry,
                "%u,%u\n",
                i,
                static_cast<unsigned int>(entries[i]) );

            finalStr.append( singleEntry );
        }

        int retVal = saveOutputStream->write( finalStr );
        if ( retVal == -1 )
        {
            // Error
            ShowErrorMessageDialog( 
                "Error writing LUT data to file", 
                "There was an error writing the LUT data. Aborting file write." );
        }   

        // Close output stream
        saveOutputStream->close();                     
    }

    void
        LUTPage::OnLoadFromCamera()
    {
        Error error;
        LUTData data;		
        error = m_pCamera->GetLUTInfo( &data );
        if( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error querying LUT support", error );
            return;
        }

        int channel = GetActiveChannelComboBox();
        if( channel == -1 )
        {
            // If we're on the "All" channels setting, just get the zeroth channel.
            channel = 0;
        }

        unsigned int* pEntries = new unsigned int[ data.numEntries ];
        memset( pEntries, 0x0, data.numEntries );
        error = m_pCamera->GetLUTChannel( 0, channel, data.numEntries, pEntries );
        if( error != PGRERROR_OK )
        {
            delete [] pEntries;
            //ShowErrorMessageDialog( "Error getting LUT channel", error );
            return;
        }

        std::vector<float> lutEntries;
        if ( data.numEntries > 512 )
        {
            lutEntries.resize( data.numEntries/4, 0.0 );
            for( unsigned int i = 0; i < lutEntries.size(); i++ )
            {                
                float avgValue = 0.0f;
                for ( unsigned int j=0; j < 4; j++ )
                {
                    avgValue += static_cast<float>(pEntries[(i*4)+j]);
                }

                avgValue /= 4.0f;

                lutEntries[i] = avgValue;
            }
        }
        else
        {
            lutEntries.resize( data.numEntries, 0.0 );
            for( unsigned int i = 0; i < lutEntries.size(); i++ )
            {
                lutEntries[i] = static_cast<float>(pEntries[i]);                  
            }
        }               

        m_pCurve->set_vector( lutEntries );         

        delete [] pEntries;               
    }

    void
        LUTPage::OnSetToCamera()
    {
        Error error;
        LUTData data;		
        error = m_pCamera->GetLUTInfo( &data );
        if( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error querying LUT support", error );
            return;
        }
        
        std::vector<float> entries = m_pCurve->get_vector( data.numEntries );
        unsigned int* pEntries = new unsigned int[ data.numEntries ];

        for( unsigned int i = 0; i < data.numEntries; i++ )
        {
            pEntries[i] = static_cast<unsigned int>(entries[i]);            
        }

        int channel = GetActiveChannelComboBox();
        if( channel == -1 )
        {
            // Perform it for all channels.
            for( int i = 0; i < 3; i++ )
            {
                error = m_pCamera->SetLUTChannel( 0, i, data.numEntries, pEntries );
                if( error != PGRERROR_OK )
                {
                    delete [] pEntries;
                    ShowErrorMessageDialog( "Error setting LUT channel", error );
                    return;
                }
            }
        }
        else
        {
            // Perform it for a single channel.
            error = m_pCamera->SetLUTChannel( 0, channel, data.numEntries, pEntries );
            if( error != PGRERROR_OK )
            {
                delete [] pEntries;
                ShowErrorMessageDialog( "Error setting LUT channel", error );
                return;
            }
        }
        delete [] pEntries;
    }

    void 
        LUTPage::OnToggleEnable()
    {
        Error error;
        error = m_pCamera->EnableLUT( m_pEnable->get_active() );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error toggling LUT", error );
        }
    }

    void
        LUTPage::OnStyleChange()
    {
        if( m_pLinear->get_active() )
        {
            m_pCurve->set_curve_type( Gtk::CURVE_TYPE_LINEAR );
        }
        else if( m_pSpline->get_active() )
        {
            m_pCurve->set_curve_type( Gtk::CURVE_TYPE_SPLINE );
        }
        else
        {
            m_pCurve->set_curve_type( Gtk::CURVE_TYPE_FREE );
        }
    }

    void
        LUTPage::OnChannelChange()
    {
        Gtk::TreeModel::iterator iter = m_pChannel->get_active();
        if ( iter == 0 )
        {
            return;
        }	

        OnLoadFromCamera();
    }

    void
        LUTPage::OnCurveStyleChanged()
    {
        switch( m_pCurve->property_curve_type() )
        {
        case Gtk::CURVE_TYPE_LINEAR:
            m_pLinear->set_active( true );
            break;
        case Gtk::CURVE_TYPE_SPLINE:
            m_pSpline->set_active( true );
            break;
        case Gtk::CURVE_TYPE_FREE:
            m_pFree->set_active( true );
            break;
        default:
            printf( "Unrecognized curve type.\n" );
        }
    }

    void
        LUTPage::DisableWidgets()
    {
        m_pCurve->set_sensitive( false );
        m_pSaveFile->set_sensitive( false );
        m_pLoadFile->set_sensitive( false );
        m_pSetCamera->set_sensitive( false );
        m_pLoadCamera->set_sensitive( false );
        m_pSpline->set_sensitive( false );
        m_pLinear->set_sensitive( false );
        m_pFree->set_sensitive( false );
        m_pChannel->set_sensitive( false );
        m_pEnable->set_sensitive( false );
        m_pReset->set_sensitive( false );        
    }

    void
        LUTPage::EnableWidgets()
    {
        m_pCurve->set_sensitive( true );
        m_pSaveFile->set_sensitive( true );
        m_pLoadFile->set_sensitive( true );
        m_pSetCamera->set_sensitive( true );
        m_pLoadCamera->set_sensitive( true );
        m_pSpline->set_sensitive( true );
        m_pLinear->set_sensitive( true );
        m_pFree->set_sensitive( true );
        m_pChannel->set_sensitive( true );
        m_pEnable->set_sensitive( true );
        m_pReset->set_sensitive( true );
    }

	void 
        LUTPage::AddToChannelComboBox( Glib::ustring description, int channel )
	{
		Gtk::TreeModel::Row row = *(m_refListStoreChannels->append());
		row[m_channelColumns.m_colChannelStr] = description;
		row[m_channelColumns.m_colChannel] = channel;
	}

    int
        LUTPage::GetActiveChannelComboBox()
	{
		// Get the active row
		Gtk::TreeModel::iterator iter = m_pChannel->get_active();
		if ( iter == 0 )
		{
			return 0;
		}		
			
		Gtk::TreeModel::Row row = *iter;		
		int channel = row[m_channelColumns.m_colChannel];
		
		return channel;
	}

    void
        LUTPage::PopulateChannelComboBox( unsigned int numChannels )
    {
        m_refListStoreChannels->clear();	
		
        if( numChannels == 1 )
        {
            AddToChannelComboBox( "All", 0 );
        }
        else if( numChannels == 3 )
        {
            AddToChannelComboBox( "All", -1 );
            AddToChannelComboBox( "Red", 0 );
            AddToChannelComboBox( "Green", 1 );
            AddToChannelComboBox( "Blue", 2 );
        }

		m_pChannel->clear();
		m_pChannel->pack_start( m_channelColumns.m_colChannelStr );
        m_pChannel->set_active(0);
    }

    void LUTPage::AddFileFilters( Gtk::FileChooserDialog& dialog )
    {
        // Add file filters
        Gtk::FileFilter lutFilter;
        lutFilter.set_name( "LUT data files (*.lut)" );
        lutFilter.add_pattern("*.lut");

        Gtk::FileFilter allFilter;
        allFilter.set_name( "All files (*.*)" );
        allFilter.add_pattern("*.*");

        dialog.add_filter( lutFilter );
        dialog.add_filter( allFilter );
    }

    void LUTPage::OnFormatChanged()
    {
        UpdateWidgets();
    }
}
