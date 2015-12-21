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
// $Id: DCAMFormatsPage.cpp,v 1.17 2009/04/20 16:51:24 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "DCAMFormatsPage.h"

namespace FlyCapture2
{
    const char* DCAMFormatsPage::sk_radioFormat7 = "radioFormat7";

	const char* DCAMFormatsPage::sk_radio160x120YUV444 = "radio160x120YUV444";
	
	const char* DCAMFormatsPage::sk_radio320x240YUV422 = "radio320x240YUV422";
	const char* DCAMFormatsPage::sk_radio640x480YUV422 = "radio640x480YUV422";
	const char* DCAMFormatsPage::sk_radio800x600YUV422 = "radio800x600YUV422";
	const char* DCAMFormatsPage::sk_radio1024x768YUV422 = "radio1024x768YUV422";
    const char* DCAMFormatsPage::sk_radio1280x960YUV422 = "radio1280x960YUV422";    
	const char* DCAMFormatsPage::sk_radio1600x1200YUV422 = "radio1600x1200YUV422";	
	
	const char* DCAMFormatsPage::sk_radio640x480YUV411 = "radio640x480YUV411";
	
	const char* DCAMFormatsPage::sk_radio640x480RGB = "radio640x480RGB";
	const char* DCAMFormatsPage::sk_radio800x600RGB = "radio800x600RGB";
	const char* DCAMFormatsPage::sk_radio1024x768RGB = "radio1024x768RGB";
	const char* DCAMFormatsPage::sk_radio1280x960RGB = "radio1280x960RGB";
	const char* DCAMFormatsPage::sk_radio1600x1200RGB = "radio1600x1200RGB";
	
	const char* DCAMFormatsPage::sk_radio640x480Y8 = "radio640x480Y8";
	const char* DCAMFormatsPage::sk_radio800x600Y8 = "radio800x600Y8";
	const char* DCAMFormatsPage::sk_radio1024x768Y8 = "radio1024x768Y8";
	const char* DCAMFormatsPage::sk_radio1280x960Y8 = "radio1280x960Y8";
	const char* DCAMFormatsPage::sk_radio1600x1200Y8 = "radio1600x1200Y8";	
	
	const char* DCAMFormatsPage::sk_radio640x480Y16 = "radio640x480Y16";
	const char* DCAMFormatsPage::sk_radio800x600Y16 = "radio800x600Y16";
	const char* DCAMFormatsPage::sk_radio1024x768Y16 = "radio1024x768Y16";
	const char* DCAMFormatsPage::sk_radio1280x960Y16 = "radio1280x960Y16";
	const char* DCAMFormatsPage::sk_radio1600x1200Y16 = "radio1600x1200Y16";
	
	const char* DCAMFormatsPage::sk_radioFrmRate_1_875 = "radioFrmRate_1_875";
	const char* DCAMFormatsPage::sk_radioFrmRate_3_75 = "radioFrmRate_3_75";
	const char* DCAMFormatsPage::sk_radioFrmRate_7_5 = "radioFrmRate_7_5";
	const char* DCAMFormatsPage::sk_radioFrmRate_15 = "radioFrmRate_15";
	const char* DCAMFormatsPage::sk_radioFrmRate_30 = "radioFrmRate_30";
	const char* DCAMFormatsPage::sk_radioFrmRate_60 = "radioFrmRate_60";
	const char* DCAMFormatsPage::sk_radioFrmRate_120 = "radioFrmRate_120";
	const char* DCAMFormatsPage::sk_radioFrmRate_240 = "radioFrmRate_240";
	
	DCAMFormatsPage::DCAMFormatsPage()
	{
	}
	
	DCAMFormatsPage::DCAMFormatsPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
	{
	}
	
	DCAMFormatsPage::~DCAMFormatsPage()
	{
	}
			
	void DCAMFormatsPage::UpdateWidgets()
	{
        if ( m_pCamera == NULL || IsConnected() != true )
        {
            DisableWidgets();
            return;	
        }

        EnableWidgets();

		// Query the camera for the availability of video modes and
		// toggle visibility as needed
		
        Error error;
		std::map<Gtk::RadioButton*, VideoMode>::iterator iterVideoMode;
		std::map<Gtk::RadioButton*, FrameRate>::iterator iterFrameRate;

        if ( IsIIDC() )
        {
            Camera* pCamera = dynamic_cast<Camera*>(m_pCamera);

            // Update the video mode radio buttons
            // Loop through all the video modes
            // For each video mode, loop through all the frame rates
            // If the camera supports this video mode at any frame rate, then
            // the radio button should be shown
            for ( iterVideoMode = m_mapVideoMode.begin(); iterVideoMode != m_mapVideoMode.end(); iterVideoMode++ )
            {			
                iterVideoMode->first->hide();

                if ( iterVideoMode->second == VIDEOMODE_FORMAT7 )
                {
                    iterVideoMode->first->show();                   
                    continue;
                }

                for ( iterFrameRate = m_mapFrameRate.begin(); iterFrameRate != m_mapFrameRate.end(); iterFrameRate++ )
                {                
                    bool supported = false;

                    error = pCamera->GetVideoModeAndFrameRateInfo( iterVideoMode->second, iterFrameRate->second, &supported );
                    if ( error != PGRERROR_OK )
                    {
                        // Error	
                        ShowErrorMessageDialog( "Error querying video mode and frame rate", error );
                        break;
                    }

                    if ( supported == true )
                    {
                        // Enable this video mode button
                        iterVideoMode->first->show();

                        // Move on to next video mode
                        break;
                    }
                }
            }
        }
        else if ( IsGEV() )
        {
            for ( iterVideoMode = m_mapVideoMode.begin(); iterVideoMode != m_mapVideoMode.end(); iterVideoMode++ )
            {			
                iterVideoMode->first->hide();

                if ( iterVideoMode->second == VIDEOMODE_FORMAT7 )
                {
                    iterVideoMode->first->show();                   
                    continue;
                }                
            }
        }		
		
		VideoMode currVideoMode;
		FrameRate currFrameRate;
		
        if ( IsIIDC() )
        {
            Camera* pCamera = dynamic_cast<Camera*>(m_pCamera);

            error = pCamera->GetVideoModeAndFrameRate( &currVideoMode, &currFrameRate );
            if ( error != PGRERROR_OK )
            {
                // Error		
                ShowErrorMessageDialog( "Error getting current video mode and frame rate", error );
            }
        }
        else
        {
            currVideoMode = VIDEOMODE_FORMAT7;
            currFrameRate = FRAMERATE_FORMAT7;
        }
		
		// Set the right video mode button to be active
		for ( iterVideoMode = m_mapVideoMode.begin(); iterVideoMode != m_mapVideoMode.end(); iterVideoMode++ )
		{
            if ( currVideoMode == iterVideoMode->second )
			{
				iterVideoMode->first->set_active( true );
			}
		}
		
		// Update the available frame rates for the current video mode
		UpdateFrameRatesForVideoMode( currVideoMode, currFrameRate );
	}
	
	void DCAMFormatsPage::GetWidgets()
	{
        m_refXml->get_widget( sk_radioFormat7, m_pRadioFormat7 );

		m_refXml->get_widget( sk_radio160x120YUV444, m_pRadio160x120YUV444 );
			
		m_refXml->get_widget( sk_radio320x240YUV422, m_pRadio320x240YUV422 );
		m_refXml->get_widget( sk_radio640x480YUV422, m_pRadio640x480YUV422 );
		m_refXml->get_widget( sk_radio800x600YUV422, m_pRadio800x600YUV422 );
		m_refXml->get_widget( sk_radio1024x768YUV422, m_pRadio1024x768YUV422 );
        m_refXml->get_widget( sk_radio1280x960YUV422, m_pRadio1280x960YUV422 );
		m_refXml->get_widget( sk_radio1600x1200YUV422, m_pRadio1600x1200YUV422 );	
			
		m_refXml->get_widget( sk_radio640x480YUV411, m_pRadio640x480YUV411 );
			
		m_refXml->get_widget( sk_radio640x480RGB, m_pRadio640x480RGB );
		m_refXml->get_widget( sk_radio800x600RGB, m_pRadio800x600RGB );
		m_refXml->get_widget( sk_radio1024x768RGB, m_pRadio1024x768RGB );
		m_refXml->get_widget( sk_radio1280x960RGB, m_pRadio1280x960RGB );
		m_refXml->get_widget( sk_radio1600x1200RGB, m_pRadio1600x1200RGB );
			
		m_refXml->get_widget( sk_radio640x480Y8, m_pRadio640x480Y8 );
		m_refXml->get_widget( sk_radio800x600Y8, m_pRadio800x600Y8 );
		m_refXml->get_widget( sk_radio1024x768Y8, m_pRadio1024x768Y8 );
		m_refXml->get_widget( sk_radio1280x960Y8, m_pRadio1280x960Y8 );
		m_refXml->get_widget( sk_radio1600x1200Y8, m_pRadio1600x1200Y8 );	
			
		m_refXml->get_widget( sk_radio640x480Y16, m_pRadio640x480Y16 );
		m_refXml->get_widget( sk_radio800x600Y16, m_pRadio800x600Y16 );
		m_refXml->get_widget( sk_radio1024x768Y16, m_pRadio1024x768Y16 );
		m_refXml->get_widget( sk_radio1280x960Y16, m_pRadio1280x960Y16 );
		m_refXml->get_widget( sk_radio1600x1200Y16, m_pRadio1600x1200Y16 );
			
		m_refXml->get_widget( sk_radioFrmRate_1_875, m_pRadioFrmRate_1_875 );
		m_refXml->get_widget( sk_radioFrmRate_3_75, m_pRadioFrmRate_3_75 );
		m_refXml->get_widget( sk_radioFrmRate_7_5, m_pRadioFrmRate_7_5 );
		m_refXml->get_widget( sk_radioFrmRate_15, m_pRadioFrmRate_15 );
		m_refXml->get_widget( sk_radioFrmRate_30, m_pRadioFrmRate_30 );
		m_refXml->get_widget( sk_radioFrmRate_60, m_pRadioFrmRate_60 );
		m_refXml->get_widget( sk_radioFrmRate_120, m_pRadioFrmRate_120 );
		m_refXml->get_widget( sk_radioFrmRate_240, m_pRadioFrmRate_240 );

        m_mapVideoMode[m_pRadioFormat7] = VIDEOMODE_FORMAT7;

        m_mapVideoMode[m_pRadio160x120YUV444] = VIDEOMODE_160x120YUV444;

        m_mapVideoMode[m_pRadio320x240YUV422] = VIDEOMODE_320x240YUV422;
        m_mapVideoMode[m_pRadio640x480YUV422] = VIDEOMODE_640x480YUV422;
        m_mapVideoMode[m_pRadio800x600YUV422] = VIDEOMODE_800x600YUV422;
        m_mapVideoMode[m_pRadio1024x768YUV422] = VIDEOMODE_1024x768YUV422;
        m_mapVideoMode[m_pRadio1280x960YUV422] = VIDEOMODE_1280x960YUV422;
        m_mapVideoMode[m_pRadio1600x1200YUV422] = VIDEOMODE_1600x1200YUV422;

        m_mapVideoMode[m_pRadio640x480YUV411] = VIDEOMODE_640x480YUV411;

        m_mapVideoMode[m_pRadio640x480RGB] = VIDEOMODE_640x480RGB;
        m_mapVideoMode[m_pRadio800x600RGB] = VIDEOMODE_800x600RGB;
        m_mapVideoMode[m_pRadio1024x768RGB] = VIDEOMODE_1024x768RGB;
        m_mapVideoMode[m_pRadio1280x960RGB] = VIDEOMODE_1280x960RGB;
        m_mapVideoMode[m_pRadio1600x1200RGB] = VIDEOMODE_1600x1200RGB;

        m_mapVideoMode[m_pRadio640x480Y8] = VIDEOMODE_640x480Y8;
        m_mapVideoMode[m_pRadio800x600Y8] = VIDEOMODE_800x600Y8;
        m_mapVideoMode[m_pRadio1024x768Y8] = VIDEOMODE_1024x768Y8;
        m_mapVideoMode[m_pRadio1280x960Y8] = VIDEOMODE_1280x960Y8;
        m_mapVideoMode[m_pRadio1600x1200Y8] = VIDEOMODE_1600x1200Y8;

        m_mapVideoMode[m_pRadio640x480Y16] = VIDEOMODE_640x480Y16;
        m_mapVideoMode[m_pRadio800x600Y16] = VIDEOMODE_800x600Y16;
        m_mapVideoMode[m_pRadio1024x768Y16] = VIDEOMODE_1024x768Y16;
        m_mapVideoMode[m_pRadio1280x960Y16] = VIDEOMODE_1280x960Y16;
        m_mapVideoMode[m_pRadio1600x1200Y16] = VIDEOMODE_1600x1200Y16;
		
		m_mapFrameRate[m_pRadioFrmRate_1_875] = FRAMERATE_1_875;
		m_mapFrameRate[m_pRadioFrmRate_3_75] = FRAMERATE_3_75;
		m_mapFrameRate[m_pRadioFrmRate_7_5] = FRAMERATE_7_5;
		m_mapFrameRate[m_pRadioFrmRate_15] = FRAMERATE_15;
		m_mapFrameRate[m_pRadioFrmRate_30] = FRAMERATE_30;
		m_mapFrameRate[m_pRadioFrmRate_60] = FRAMERATE_60;
		m_mapFrameRate[m_pRadioFrmRate_120] = FRAMERATE_120;
		m_mapFrameRate[m_pRadioFrmRate_240] = FRAMERATE_240;
	}
	
	void DCAMFormatsPage::AttachSignals()
	{		
        AttachVideoModeRadioButton( m_pRadioFormat7 );

		AttachVideoModeRadioButton( m_pRadio160x120YUV444 );
			
		AttachVideoModeRadioButton( m_pRadio320x240YUV422 );
		AttachVideoModeRadioButton( m_pRadio640x480YUV422 );
		AttachVideoModeRadioButton( m_pRadio800x600YUV422 );
		AttachVideoModeRadioButton( m_pRadio1024x768YUV422 );
        AttachVideoModeRadioButton( m_pRadio1280x960YUV422 );
		AttachVideoModeRadioButton( m_pRadio1600x1200YUV422 );	
			
		AttachVideoModeRadioButton( m_pRadio640x480YUV411 );
			
		AttachVideoModeRadioButton( m_pRadio640x480RGB );
		AttachVideoModeRadioButton( m_pRadio800x600RGB );
		AttachVideoModeRadioButton( m_pRadio1024x768RGB );
		AttachVideoModeRadioButton( m_pRadio1280x960RGB );
		AttachVideoModeRadioButton( m_pRadio1600x1200RGB );
			
		AttachVideoModeRadioButton( m_pRadio640x480Y8 );
		AttachVideoModeRadioButton( m_pRadio800x600Y8 );
		AttachVideoModeRadioButton( m_pRadio1024x768Y8 );
		AttachVideoModeRadioButton( m_pRadio1280x960Y8 );
		AttachVideoModeRadioButton( m_pRadio1600x1200Y8 );	
			
		AttachVideoModeRadioButton( m_pRadio640x480Y16 );
		AttachVideoModeRadioButton( m_pRadio800x600Y16 );
		AttachVideoModeRadioButton( m_pRadio1024x768Y16 );
		AttachVideoModeRadioButton( m_pRadio1280x960Y16 );
		AttachVideoModeRadioButton( m_pRadio1600x1200Y16 );
		
		AttachFrameRateRadioButton( m_pRadioFrmRate_1_875 );
		AttachFrameRateRadioButton( m_pRadioFrmRate_3_75 );
		AttachFrameRateRadioButton( m_pRadioFrmRate_7_5 );
		AttachFrameRateRadioButton( m_pRadioFrmRate_15 );
		AttachFrameRateRadioButton( m_pRadioFrmRate_30 );
		AttachFrameRateRadioButton( m_pRadioFrmRate_60 );
		AttachFrameRateRadioButton( m_pRadioFrmRate_120 );
		AttachFrameRateRadioButton( m_pRadioFrmRate_240 );
	}
	
	void DCAMFormatsPage::OnVideoModeChanged( Gtk::RadioButton* pButton )
	{
		if ( pButton->get_active() != true )
		{
			// This function gets called twice, when the old button
			// is unchecked, and the new button is checked
			return;
		}	

        if ( IsIIDC() != true )
        {
            // Nothing to do here
            return;
        }

        Camera* pCamera = dynamic_cast<Camera*>(m_pCamera);
		
		VideoMode newVideoMode = GetVideoModeFromButton( pButton );
		
        Error error;
		VideoMode currVideoMode;
		FrameRate currFrameRate;
		error = pCamera->GetVideoModeAndFrameRate( &currVideoMode, &currFrameRate );		
        if ( error != PGRERROR_OK )
        {
            // Error			
            ShowErrorMessageDialog( "Error getting current video mode and frame rate", error );
        }
		
		// Set it to the camera if it is different from the current value
		if ( currVideoMode != newVideoMode )
		{
			// Figure out the fastest frame rate for this video mode
			FrameRate newFrameRate;
			newFrameRate = GetFastestFrameRateForVideoMode( newVideoMode );

            bool restartCamera = false;            

            error = m_pCamera->StopCapture();
            if ( error == PGRERROR_OK )
            {
                // The camera was streaming and thus it needs to be restarted
                restartCamera = true;
            }
			
			// Set the new combination to the camera
			error = pCamera->SetVideoModeAndFrameRate( newVideoMode, newFrameRate );
            if ( error != PGRERROR_OK )
            {
                // Error	
                ShowErrorMessageDialog( "Error setting video mode and frame rate", error );
            }

            if ( restartCamera == true )
            {                
                error = m_pCamera->StartCapture();
                if ( error != PGRERROR_OK )
                {
                    // Error	
                    ShowErrorMessageDialog( "Error restarting image streaming", error );
                }
            }
		}
		
		// Read it back from the camera
		error = pCamera->GetVideoModeAndFrameRate( &currVideoMode, &currFrameRate );		
        if ( error != PGRERROR_OK )
        {
            // Error	
            ShowErrorMessageDialog( "Error getting current video mode and frame rate", error );
        }
		
		// Update the frame rate list
		UpdateFrameRatesForVideoMode( currVideoMode, currFrameRate );

        m_signal_dcam_format_changed();
	}
	
	void DCAMFormatsPage::OnFrameRateChanged( Gtk::RadioButton* pButton )
	{
		if ( pButton->get_active() != true )
		{
			// This function gets called twice, when the old button
			// is unchecked, and the new button is checked
			return;
		}		

        if ( IsIIDC() != true )
        {
            return;
        }

        Camera* pCamera = dynamic_cast<Camera*>(m_pCamera);
		
		FrameRate newFrmRate = GetFrameRateFromButton( pButton );

        Error error;
        VideoMode currVideoMode;
        FrameRate currFrameRate;
        error = pCamera->GetVideoModeAndFrameRate( &currVideoMode, &currFrameRate );		
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting current video mode and frame rate", error );
        }
		
		// Set it to the camera if it is different from the current value
        if ( newFrmRate != currFrameRate )
        {
            bool restartCamera = false;
            error = m_pCamera->StopCapture();
            if ( error == PGRERROR_OK )
            {
                // This means the camera was started and therefore we
                // need to restart it once the new modes are set
                restartCamera = true;
            }  

            error = pCamera->SetVideoModeAndFrameRate( currVideoMode, newFrmRate );
            if ( error != PGRERROR_OK )
            {
                // Error	
                ShowErrorMessageDialog( "Error setting video mode and frame rate", error );
            }

            if ( restartCamera == true )
            {
                error = m_pCamera->StartCapture();
                if ( error != PGRERROR_OK )
                {
                    // Error	
                    ShowErrorMessageDialog( "Error restarting image streaming", error );
                }
            }
        }

        // Read it back from the camera
        error = pCamera->GetVideoModeAndFrameRate( &currVideoMode, &currFrameRate );	
        if ( error != PGRERROR_OK )
        {
            // Error	
            ShowErrorMessageDialog( "Error getting current video mode and frame rate", error );
        }

        // Update the frame rate list
        UpdateFrameRatesForVideoMode( currVideoMode, currFrameRate );

        m_signal_dcam_format_changed();
	}	
	
	// Helper function to attach the custom signal to the appropriate button
	void DCAMFormatsPage::AttachVideoModeRadioButton( Gtk::RadioButton* pButton)
	{
		pButton->signal_toggled().connect( 
			sigc::bind<Gtk::RadioButton*>( 
				sigc::mem_fun(*this, &DCAMFormatsPage::OnVideoModeChanged), 
				pButton ) );
	}
	
	// Helper function to attach the custom signal to the appropriate button
	void DCAMFormatsPage::AttachFrameRateRadioButton( Gtk::RadioButton* pButton )
	{
		pButton->signal_toggled().connect( 
			sigc::bind<Gtk::RadioButton*>( 
				sigc::mem_fun(*this, &DCAMFormatsPage::OnFrameRateChanged), 
				pButton ) );
	}
	
	VideoMode DCAMFormatsPage::GetVideoModeFromButton( Gtk::RadioButton* pButton )
	{
		if ( m_mapVideoMode.find( pButton ) == m_mapVideoMode.end() )
		{
			// Not found
			return NUM_VIDEOMODES;
		}
		
		return m_mapVideoMode.find( pButton )->second;
	}
	
	FrameRate DCAMFormatsPage::GetFrameRateFromButton( Gtk::RadioButton* pButton )
	{
		if ( m_mapFrameRate.find( pButton ) == m_mapFrameRate.end() )
		{
			// Not found
			return NUM_FRAMERATES;
		}
		
		return m_mapFrameRate.find( pButton )->second;
	}
	
	void DCAMFormatsPage::UpdateFrameRatesForVideoMode( VideoMode currVideoMode, FrameRate currFrameRate )
	{				
        if ( currVideoMode == VIDEOMODE_FORMAT7 )
        {
            m_pRadioFormat7->set_active( true );
            DisableFrameRateButtons();
            return;
        }

        if ( currFrameRate == NUM_FRAMERATES )
        {
            DisableFrameRateButtons();
            return;
        }
        
        EnableFrameRateButtons();  

        if ( IsIIDC() != true )
        {
            return;
        }

        Camera* pCamera = dynamic_cast<Camera*>(m_pCamera);

		// Populate the frame rates for the current video mode
		std::map<Gtk::RadioButton*, FrameRate>::iterator iterFrameRate;
		for ( iterFrameRate = m_mapFrameRate.begin(); iterFrameRate != m_mapFrameRate.end(); iterFrameRate++ )
		{
            Error error;
			bool supported = false;
			error = pCamera->GetVideoModeAndFrameRateInfo( currVideoMode, iterFrameRate->second, &supported );
            if ( error != PGRERROR_OK )
            {
                // Error			
            }

			if ( supported == true )
			{
				// Enable this video mode button
				iterFrameRate->first->show();
				
				if ( currFrameRate == iterFrameRate->second )
				{
					iterFrameRate->first->set_active( true );
				}
			}
			else
			{
				// Enable this video mode button
				iterFrameRate->first->hide();
			}
		}		
	}
	
	FrameRate DCAMFormatsPage::GetFastestFrameRateForVideoMode( VideoMode videoMode )
	{
        if ( IsGEV() == true )
        {
            return FRAMERATE_FORMAT7;
        }

        if ( IsIIDC() != true )
        {
            return NUM_FRAMERATES;
        }

        Camera* pCamera = dynamic_cast<Camera*>(m_pCamera);

		FrameRate fastestFrameRate = NUM_FRAMERATES;
		//std::map<Gtk::RadioButton*, FrameRate>::iterator iterFrameRate;
        std::map<Gtk::RadioButton*, FrameRate>::reverse_iterator iterFrameRate;
		
		for ( iterFrameRate = m_mapFrameRate.rbegin(); iterFrameRate != m_mapFrameRate.rend(); iterFrameRate++ )
		{
            Error error;
			bool supported = false;
			error = pCamera->GetVideoModeAndFrameRateInfo( videoMode, iterFrameRate->second, &supported );
            if ( error != PGRERROR_OK )
            {
                // Error			
            }

			if ( supported == true )
			{
				fastestFrameRate = iterFrameRate->second;
				break;
			}
		}

        return fastestFrameRate;
    }

    void DCAMFormatsPage::EnableWidgets()
    {
        std::map<Gtk::RadioButton*, VideoMode>::iterator iterVideoMode;       
        for ( iterVideoMode = m_mapVideoMode.begin(); iterVideoMode != m_mapVideoMode.end(); iterVideoMode++ )
        {			
            iterVideoMode->first->set_sensitive( true );            
        }

        std::map<Gtk::RadioButton*, FrameRate>::iterator iterFrameRate;
        for ( iterFrameRate = m_mapFrameRate.begin(); iterFrameRate != m_mapFrameRate.end(); iterFrameRate++ )
        {
            iterFrameRate->first->set_sensitive( true ); 
        }

        // Always set the Format7 radio button to false
        m_pRadioFormat7->set_sensitive(false);
    }

    void DCAMFormatsPage::DisableWidgets()
    {
        std::map<Gtk::RadioButton*, VideoMode>::iterator iterVideoMode;       
        for ( iterVideoMode = m_mapVideoMode.begin(); iterVideoMode != m_mapVideoMode.end(); iterVideoMode++ )
        {			
            iterVideoMode->first->set_sensitive( false );            
        }

        std::map<Gtk::RadioButton*, FrameRate>::iterator iterFrameRate;
        for ( iterFrameRate = m_mapFrameRate.begin(); iterFrameRate != m_mapFrameRate.end(); iterFrameRate++ )
        {
            iterFrameRate->first->set_sensitive( false ); 
        }
    }

    void DCAMFormatsPage::EnableFrameRateButtons()
    {
        std::map<Gtk::RadioButton*, FrameRate>::iterator iterFrameRate;
        for ( iterFrameRate = m_mapFrameRate.begin(); iterFrameRate != m_mapFrameRate.end(); iterFrameRate++ )
        {
            iterFrameRate->first->set_sensitive( true ); 
        }
    }

    void DCAMFormatsPage::DisableFrameRateButtons()
    {
        std::map<Gtk::RadioButton*, FrameRate>::iterator iterFrameRate;
        for ( iterFrameRate = m_mapFrameRate.begin(); iterFrameRate != m_mapFrameRate.end(); iterFrameRate++ )
        {
            iterFrameRate->first->hide();
            iterFrameRate->first->set_sensitive( false ); 
        }
    }    

    void DCAMFormatsPage::OnFormat7Applied()
    {
        UpdateFrameRatesForVideoMode( VIDEOMODE_FORMAT7 );
    }

    sigc::signal<void> DCAMFormatsPage::signal_dcam_format_changed()
    {
        return m_signal_dcam_format_changed;
    }
}
