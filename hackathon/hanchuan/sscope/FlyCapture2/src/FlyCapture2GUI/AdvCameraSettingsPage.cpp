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
// $Id: AdvCameraSettingsPage.cpp,v 1.32 2010/06/25 17:47:17 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "AdvCameraSettingsPage.h"

namespace FlyCapture2
{
    const char* AdvCameraSettingsPage::sk_checkbuttonRawBayerOutput = "checkbuttonRawBayerOutput";
    const char* AdvCameraSettingsPage::sk_checkbuttonMirrorImage = "checkbuttonMirrorImage";
    const char* AdvCameraSettingsPage::sk_radiobuttonY16BigEndian = "radiobuttonY16BigEndian";
    const char* AdvCameraSettingsPage::sk_radiobuttonY16LittleEndian = "radiobuttonY16LittleEndian";
    const char* AdvCameraSettingsPage::sk_radiobuttonTestPattern1 = "radiobuttonTestPattern1";
    const char* AdvCameraSettingsPage::sk_radiobuttonTestPattern2 = "radiobuttonTestPattern2";
    const char* AdvCameraSettingsPage::sk_radiobuttonTestPatternNone = "radiobuttonTestPatternNone";
    const char* AdvCameraSettingsPage::sk_comboboxMemChannels = "comboboxMemChannels";
    const char* AdvCameraSettingsPage::sk_btnSaveToMemChannel = "btnSaveToMemChannel";
    const char* AdvCameraSettingsPage::sk_btnRestoreFromMemChannel = "btnRestoreFromMemChannel";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedTimestamp = "checkbuttonEmbeddedTimestamp";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedGain = "checkbuttonEmbeddedGain";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedShutter = "checkbuttonEmbeddedShutter";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedBrightness = "checkbuttonEmbeddedBrightness";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedExposure = "checkbuttonEmbeddedExposure";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedWhiteBalance = "checkbuttonEmbeddedWhiteBalance";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedFrameCounter = "checkbuttonEmbeddedFrameCounter";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedStrobe = "checkbuttonEmbeddedStrobe";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedGPIO = "checkbuttonEmbeddedGPIO";
    const char* AdvCameraSettingsPage::sk_checkbuttonEmbeddedROIPosition = "checkbuttonEmbeddedRoiPosition";
    const char* AdvCameraSettingsPage::sk_btnEmbeddedSelectAll = "btnEmbeddedSelectAll";
    const char* AdvCameraSettingsPage::sk_btnEmbeddedUnselectAll = "btnEmbeddedUnselectAll";
    const char* AdvCameraSettingsPage::sk_comboBoxAutoRangeControl = "comboboxAutoRangeControl";
    const char* AdvCameraSettingsPage::sk_entryAutoRangeMin = "entryAutoRangeMin";
    const char* AdvCameraSettingsPage::sk_entryAutoRangeMax = "entryAutoRangeMax";
    const char* AdvCameraSettingsPage::sk_btnGetAutoRange = "btnGetAutoRange";
    const char* AdvCameraSettingsPage::sk_btnSetAutoRange = "btnSetAutoRange";

    AdvCameraSettingsPage::AdvCameraSettingsPage()
	{
	}
	
	AdvCameraSettingsPage::AdvCameraSettingsPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
	{
	}
	
	AdvCameraSettingsPage::~AdvCameraSettingsPage()
	{
	}
			
	void AdvCameraSettingsPage::UpdateWidgets()
	{
        if ( m_pCamera == NULL || IsConnected() != true )
        {
            return;	
        }

        UpdateMemoryChannels();
        UpdateBayerOutput();
        UpdateTestPattern();
        UpdateImageDataFormat();

        OnAutoRangeControlComboBoxChanged();
	}
	
	void AdvCameraSettingsPage::GetWidgets()
	{
        m_refXml->get_widget( sk_checkbuttonRawBayerOutput, m_pCheckbuttonRawBayerOutput );
        m_refXml->get_widget( sk_checkbuttonMirrorImage, m_pCheckbuttonMirrorImage );

        m_refXml->get_widget( sk_radiobuttonY16BigEndian, m_pRadiobuttonY16BigEndian );
        m_refXml->get_widget( sk_radiobuttonY16LittleEndian, m_pRadiobuttonY16LittleEndian );
        m_refXml->get_widget( sk_radiobuttonTestPattern1, m_pRadiobuttonY16TestPattern1 );
        m_refXml->get_widget( sk_radiobuttonTestPattern2, m_pRadiobuttonY16TestPattern2 );
        m_refXml->get_widget( sk_radiobuttonTestPatternNone, m_pRadiobuttonY16TestPatternNone );

        m_refXml->get_widget( sk_comboboxMemChannels, m_pComboboxMemChannels );
        m_refXml->get_widget( sk_btnSaveToMemChannel, m_pBtnSaveToMemChannel );
        m_refXml->get_widget( sk_btnRestoreFromMemChannel, m_pBtnRestoreFromMemChannel ); 

        m_refXml->get_widget( sk_checkbuttonEmbeddedTimestamp, m_arCheckButton[EMBEDDED_TIMESTAMP] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedGain, m_arCheckButton[EMBEDDED_GAIN] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedShutter, m_arCheckButton[EMBEDDED_SHUTTER] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedBrightness, m_arCheckButton[EMBEDDED_BRIGHTNESS] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedExposure, m_arCheckButton[EMBEDDED_EXPOSURE] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedWhiteBalance, m_arCheckButton[EMBEDDED_WHITE_BALANCE] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedFrameCounter, m_arCheckButton[EMBEDDED_FRAME_COUNTER] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedStrobe, m_arCheckButton[EMBEDDED_STROBE_PATTERN] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedGPIO, m_arCheckButton[EMBEDDED_GPIO_PIN_STATE] );
        m_refXml->get_widget( sk_checkbuttonEmbeddedROIPosition, m_arCheckButton[EMBEDDED_ROI_POSITION] );

        m_refXml->get_widget( sk_btnEmbeddedSelectAll, m_pBtnEmbeddedSelectAll );
        m_refXml->get_widget( sk_btnEmbeddedUnselectAll, m_pBtnEmbeddedUnselectAll );

        m_refXml->get_widget( sk_comboBoxAutoRangeControl, m_pComboBoxAutoRangeControl );
        m_refXml->get_widget( sk_entryAutoRangeMin, m_pEntryAutoRangeMin );
        m_refXml->get_widget( sk_entryAutoRangeMax, m_pEntryAutoRangeMax );
        m_refXml->get_widget( sk_btnGetAutoRange, m_pBtnGetAutoRange );
        m_refXml->get_widget( sk_btnSetAutoRange, m_pBtnSetAutoRange );

        // Memory channel setup
        m_refListStoreMemChannels = Gtk::ListStore::create( m_memChannelColumns );
        m_pComboboxMemChannels->set_model( m_refListStoreMemChannels );	

        // Auto range control setup
        m_refListStoreAutoRangeControl = Gtk::ListStore::create( m_autoRangeControlColumns );
        m_pComboBoxAutoRangeControl->set_model( m_refListStoreAutoRangeControl );
        m_refListStoreAutoRangeControl->clear();

        // Add the auto range control combo box members
        Gtk::TreeModel::Row row;
        
        row = *(m_refListStoreAutoRangeControl->append());
        row[m_autoRangeControlColumns.m_colAutoRangeControlStr] = "Exposure";
        row[m_autoRangeControlColumns.m_colProperty] = AUTO_EXPOSURE;
        row[m_autoRangeControlColumns.m_colPropertyRegister] = 0x1088;

        row = *(m_refListStoreAutoRangeControl->append());
        row[m_autoRangeControlColumns.m_colAutoRangeControlStr] = "Shutter";
        row[m_autoRangeControlColumns.m_colProperty] = SHUTTER;
        row[m_autoRangeControlColumns.m_colPropertyRegister] = 0x1098;

        row = *(m_refListStoreAutoRangeControl->append());
        row[m_autoRangeControlColumns.m_colAutoRangeControlStr] = "Gain";
        row[m_autoRangeControlColumns.m_colProperty] = GAIN;
        row[m_autoRangeControlColumns.m_colPropertyRegister] = 0x10A0;

        m_pComboBoxAutoRangeControl->clear();
        m_pComboBoxAutoRangeControl->pack_start( m_autoRangeControlColumns.m_colAutoRangeControlStr );
        m_pComboBoxAutoRangeControl->set_active(0);
	}
	
	void AdvCameraSettingsPage::AttachSignals()
	{
        m_pCheckbuttonRawBayerOutput->signal_toggled().connect( 
            sigc::mem_fun( *this, &AdvCameraSettingsPage::OnRawBayerOutputToggled ) );

        m_pCheckbuttonMirrorImage->signal_toggled().connect( 
            sigc::mem_fun( *this, &AdvCameraSettingsPage::OnMirrorImageToggled ) );

        m_pRadiobuttonY16BigEndian->signal_toggled().connect( 
            sigc::bind<bool>(
                sigc::mem_fun(*this, &AdvCameraSettingsPage::OnY16EndiannessToggled), true ) );

        m_pRadiobuttonY16LittleEndian->signal_toggled().connect( 
            sigc::bind<bool>(
                sigc::mem_fun(*this, &AdvCameraSettingsPage::OnY16EndiannessToggled), false ) );

        m_pRadiobuttonY16TestPattern1->signal_toggled().connect( 
            sigc::bind<int>(
                sigc::mem_fun(*this, &AdvCameraSettingsPage::OnTestPatternChanged), 1 ) );

        m_pRadiobuttonY16TestPattern2->signal_toggled().connect( 
            sigc::bind<int>(
                sigc::mem_fun(*this, &AdvCameraSettingsPage::OnTestPatternChanged), 2 ) );

        m_pRadiobuttonY16TestPatternNone->signal_toggled().connect( 
            sigc::bind<int>(
                sigc::mem_fun(*this, &AdvCameraSettingsPage::OnTestPatternChanged), 0 ) );

        m_pBtnSaveToMemChannel->signal_clicked().connect(
            sigc::mem_fun(*this, &AdvCameraSettingsPage::OnSaveToMemoryChannel) );

        m_pBtnRestoreFromMemChannel->signal_clicked().connect(
            sigc::mem_fun(*this, &AdvCameraSettingsPage::OnRestoreFromMemoryChannel) );

        for (int i=0; i < EMBEDDED_NUM_INFORMATION; i++)
        {
            m_arCheckButton[i]->signal_toggled().connect(
                sigc::bind<EmbeddedInfoType>(
                sigc::mem_fun(*this, &AdvCameraSettingsPage::OnEmbeddedCheck),
                (EmbeddedInfoType)i ) );
        }

        m_pBtnEmbeddedSelectAll->signal_clicked().connect(
            sigc::mem_fun(*this, &AdvCameraSettingsPage::OnEmbeddedSelectAll) );

        m_pBtnEmbeddedUnselectAll->signal_clicked().connect(
            sigc::mem_fun(*this, &AdvCameraSettingsPage::OnEmbeddedUnselectAll) );

        m_pComboBoxAutoRangeControl->signal_changed().connect(
            sigc::mem_fun(*this, &AdvCameraSettingsPage::OnAutoRangeControlComboBoxChanged));

        m_pBtnGetAutoRange->signal_clicked().connect(
            sigc::mem_fun(*this, &AdvCameraSettingsPage::OnGetAutoRangeControl));

        m_pBtnSetAutoRange->signal_clicked().connect(
            sigc::mem_fun(*this, &AdvCameraSettingsPage::OnSetAutoRangeControl));

		  SetTimerConnection(
			  Glib::signal_timeout().connect(
			  sigc::mem_fun(*this, &AdvCameraSettingsPage::OnTimer), 
			  sk_timeout ));
	}

    void AdvCameraSettingsPage::OnRawBayerOutputToggled()
    {
        Error error;
        unsigned int value = 0;
		unsigned int rawBayerCtrlRegister = sk_imageDataFmtReg;
		unsigned int rawBayerMask = 0x1 << 7;

		if( m_camInfo.iidcVer >= sk_IIDC132Version )
		{
			rawBayerCtrlRegister = sk_bayerMonoCtrlReg;
			rawBayerMask = 0x1;
		}

        error = m_pCamera->ReadRegister( rawBayerCtrlRegister, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading image data format register", error );
        }

        if ( m_pCheckbuttonRawBayerOutput->get_active() == true )
        {
            value |= rawBayerMask;
        }
        else
        {
            value &= ~rawBayerMask;
        }

        error = m_pCamera->WriteRegister( rawBayerCtrlRegister, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing image data format register", error );
        }
    }

    void AdvCameraSettingsPage::OnMirrorImageToggled()
    {
        Error error;
        unsigned int value = 0;
		unsigned int mirrorCtrlRegister = sk_imageDataFmtReg;
		unsigned int mirrorMask = 0x1 << 8;

		if( m_camInfo.iidcVer >= sk_IIDC132Version )
		{
			mirrorCtrlRegister = sk_mirrorImageCtrlReg;
			mirrorMask = 0x1;
		}

        error = m_pCamera->ReadRegister( mirrorCtrlRegister, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading image data format register", error );
        }

        if ( m_pCheckbuttonMirrorImage->get_active() == true )
        {
            value |= mirrorMask;
        }
        else
        {
            value &= ~mirrorMask;
        }

        error = m_pCamera->WriteRegister( mirrorCtrlRegister, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing image data format register", error );
        }
    }

    void AdvCameraSettingsPage::OnY16EndiannessToggled( bool isBigEndian )
    {
        Error error;
        unsigned int value = 0;
		unsigned int endianCtrlRegister = sk_imageDataFmtReg;
		unsigned int endianMask = 0x1;

		if( m_camInfo.iidcVer >= sk_IIDC132Version )
		{
			endianCtrlRegister = sk_dataDepthCtrlReg;
			endianMask = 0x1 << 23;
		}

        error = m_pCamera->ReadRegister( endianCtrlRegister, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading image data format register", error );
        }

        if ( isBigEndian == true )
        {
			if( m_camInfo.iidcVer >= sk_IIDC132Version )
			{
				value &= ~endianMask;
			}
			else
			{
				value |= endianMask;
			}
        }
        else
        {
			if (m_camInfo.iidcVer >= sk_IIDC132Version )
			{
				value |= endianMask;
			}
			else
			{
				value &= ~endianMask;
			}
        }

        error = m_pCamera->WriteRegister( endianCtrlRegister, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing image data format register", error );
        }
    }   

    void AdvCameraSettingsPage::OnTestPatternChanged( int testPattern )
    {
        // 0 - No pattern
        // 1 - Test pattern 1
        // 2 - Test pattern 2

        Error error;
        unsigned int value = 0;
        error = m_pCamera->ReadRegister( sk_testPatternReg, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading test pattern register", error );
        }

        value &= 0x00;

        switch( testPattern )
        {
        case 0:
            // Nothing to do here
        	break;

        case 1:
            value |= (0x1 << 0);
            break;

        case 2:
            value |= (0x1 << 1);
            break;

        default:
            break;
        }

        error = m_pCamera->WriteRegister( sk_testPatternReg, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing test pattern register", error );
        }
    }

    void AdvCameraSettingsPage::OnSaveToMemoryChannel()
    {
        unsigned int memChannel = GetActiveMemChannelComboBox();

        if ( memChannel == 0 )
        {
            // Make sure user really wants to save to default channel
            int responseType = ShowMessageDialog( 
                "Confirm restore to default settings",
                "Writing to the default memory channel will restore the camera to factory defaults!.\n"
                "Do you wish to proceed?",
                Gtk::MESSAGE_QUESTION,
                Gtk::BUTTONS_OK_CANCEL );

            if ( responseType != Gtk::RESPONSE_OK )
            {
                return;
            }
        }

        Error error;
        error = m_pCamera->SaveToMemoryChannel( memChannel );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error saving to memory channel", error );
        }
    }

    void AdvCameraSettingsPage::OnRestoreFromMemoryChannel()
    {
        bool wasStreaming = false;
        Error error = m_pCamera->StopCapture();
        if ( error != PGRERROR_ISOCH_NOT_STARTED && error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( 
                "Error stopping camera in preparation for restoring from memory channel", 
                error );
            return;
        }

        // The camera was streaming, need to restart it after restoring
        // the memory channel.
        if ( error == PGRERROR_OK )
        {
            wasStreaming = true;
        }

        unsigned int memChannel = GetActiveMemChannelComboBox();

        error = m_pCamera->RestoreFromMemoryChannel( memChannel );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( 
                "Error restoring from memory channel", 
                error );
        }

        // Restart the camera if needed
        if ( wasStreaming == true )
        {
            error = m_pCamera->StartCapture();
            if ( error != PGRERROR_OK )
            {
                ShowErrorMessageDialog( 
                    "Error restarting camera after restoring from memory channel", 
                    error );
            }
        }

        m_signal_restore_from_memory_channel();
    }

    void AdvCameraSettingsPage::UpdateMemChannelComboBox()
    {
        m_refListStoreMemChannels->clear();

        Error error;
        unsigned int numMemChannels;
        error = m_pCamera->GetMemoryChannelInfo( &numMemChannels );
        if ( error != PGRERROR_OK || numMemChannels == 0 )
        {
            m_pComboboxMemChannels->set_sensitive( false );
            m_pBtnSaveToMemChannel->set_sensitive( false );
            m_pBtnRestoreFromMemChannel->set_sensitive( false );

            return;
        }

        // Iterate to numMemChannels+1, since 0th channel is default
        for ( unsigned int i=0; i < numMemChannels+1; i++ )
        {
            Gtk::TreeModel::Row row = *(m_refListStoreMemChannels->append());

            char memChannelEntry[64];
            if ( i == 0 )
            {
                sprintf( memChannelEntry, "%s", "Default" );
            }
            else
            {
                sprintf( memChannelEntry, "%d", i );
            }

            row[m_memChannelColumns.m_colMemChannelStr] = memChannelEntry;
            row[m_memChannelColumns.m_colMemChannel] = i;
        }

        m_pComboboxMemChannels->clear();
        m_pComboboxMemChannels->pack_start( m_memChannelColumns.m_colMemChannelStr );
        m_pComboboxMemChannels->set_sensitive( true ); 
        m_pBtnSaveToMemChannel->set_sensitive( true );
        m_pBtnRestoreFromMemChannel->set_sensitive( true );

    }

    void AdvCameraSettingsPage::SetActiveMemChannelComboBox( unsigned int channel )
    {
        if ( m_refListStoreMemChannels->children().size() == 0 )
        {
            return;
        }

        // Find the selected mode in the list store and set it		
        Gtk::TreeModel::iterator iter;
        Gtk::TreeModel::Row row;
        bool found = false;

        for ( iter = m_refListStoreMemChannels->children().begin(); 
            iter != m_refListStoreMemChannels->children().end(); 
            iter++ )
        {
            row = *iter;
            if ( row[m_memChannelColumns.m_colMemChannel] == channel )
            {
                found = true;
                break;
            }
        }

        if ( found == true )
        {
            m_pComboboxMemChannels->set_active(iter);				
        }	
        else
        {
            m_pComboboxMemChannels->set_active(0);				
        }
    }

    unsigned int AdvCameraSettingsPage::GetActiveMemChannelComboBox()
    {
        // Get the selected mode
        Gtk::TreeModel::iterator iter = m_pComboboxMemChannels->get_active();

        if (iter)
        {
            Gtk::TreeModel::Row row = *iter;

            int memChannel = row[m_memChannelColumns.m_colMemChannel];       

            return memChannel;
        }

        return 0;
    }

    void AdvCameraSettingsPage::OnEmbeddedSelectAll()
    {
		unsigned int regVal = 0;
		Error error = m_pCamera->ReadRegister(sk_frameInfoReg, &regVal);
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error reading embedded image info", error );
            return;
        }

		for (unsigned int i=0, presence = 0x10000; i < EMBEDDED_NUM_INFORMATION; i++, presence = presence << 1)
        {
			if(( m_camInfo.iidcVer < sk_IIDC132Version) || (presence & regVal) != 0)
			{
				m_arCheckButton[i]->set_active(true);        
			}
        }
    }

    void AdvCameraSettingsPage::OnEmbeddedUnselectAll()
    {
        for (int i=0; i < EMBEDDED_NUM_INFORMATION; i++)
        {
            m_arCheckButton[i]->set_active(false);
        }
    }

    void AdvCameraSettingsPage::OnEmbeddedCheck( EmbeddedInfoType type )
    {
        Error error;
        unsigned int frameInfoRegVal;
        error = m_pCamera->ReadRegister( sk_frameInfoReg, &frameInfoRegVal );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error reading frame info register", error );
            return;
        }

        EmbeddedImageInfo embeddedInfo;
        error = m_pCamera->GetEmbeddedImageInfo( &embeddedInfo );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error reading embedded image info", error );
            return;
        }

        bool checkState = m_arCheckButton[type]->get_active();

        switch (type)
        {
        case EMBEDDED_TIMESTAMP: embeddedInfo.timestamp.onOff = checkState; break;
        case EMBEDDED_GAIN: embeddedInfo.gain.onOff = checkState; break;
        case EMBEDDED_SHUTTER: embeddedInfo.shutter.onOff = checkState; break;
        case EMBEDDED_BRIGHTNESS: embeddedInfo.brightness.onOff = checkState; break;
        case EMBEDDED_EXPOSURE: embeddedInfo.exposure.onOff = checkState; break;
        case EMBEDDED_WHITE_BALANCE: embeddedInfo.whiteBalance.onOff = checkState; break;
        case EMBEDDED_FRAME_COUNTER: embeddedInfo.frameCounter.onOff = checkState; break;
        case EMBEDDED_STROBE_PATTERN: embeddedInfo.strobePattern.onOff = checkState; break;
        case EMBEDDED_GPIO_PIN_STATE: embeddedInfo.GPIOPinState.onOff = checkState; break;
        case EMBEDDED_ROI_POSITION: embeddedInfo.ROIPosition.onOff = checkState; break;
        default:
            break;
        }

        error = m_pCamera->SetEmbeddedImageInfo( &embeddedInfo );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error writing embedded image info", error );
            return;
        }
    }

    bool AdvCameraSettingsPage::OnTimer()
    {
        if (m_pCamera == NULL || IsIconified())
        {
            return true;
        }

        Gtk::Notebook* pNotebook;
        m_refXml->get_widget( "notebookCamCtl", pNotebook );
        if ( pNotebook->get_current_page() == 6 )
        {
            UpdateImageDataFormat();
            UpdateTestPattern();

            Error error;
            unsigned int frameInfoRegVal = 0;
            error = m_pCamera->ReadRegister( sk_frameInfoReg, &frameInfoRegVal );
            if ( error != PGRERROR_OK || (frameInfoRegVal >> 31) == 0 )
            {
                DisableEmbedded();
                return true;
            }
            else
            {
                // TODO: Maybe figure out what is supported, since some cameras
                // don't support specific functions
                EnableEmbedded();
            }    

            for (int i=0; i < EMBEDDED_NUM_INFORMATION; i++)
            {
                if ( (frameInfoRegVal >> i & 0x1) != 0 )
                {
                    //printf( "%d is enabled\n", i);
                    m_arCheckButton[i]->set_active(true);
                }
                else
                {
                    //printf( "%d is not enabled\n", i);
                    m_arCheckButton[i]->set_active(false);
                }
            }
        }        

        return true;
    }

    void AdvCameraSettingsPage::EnableEmbedded()
    {	
        unsigned int regVal = 0;
		Error error = m_pCamera->ReadRegister(sk_frameInfoReg, &regVal);
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading FRAME_INFO register", error );
        }
		
		for (unsigned int i=0, presence = 0x10000; i < EMBEDDED_NUM_INFORMATION; i++, presence = presence << 1)
        {
			if(( m_camInfo.iidcVer < sk_IIDC132Version) || (presence & regVal) != 0)
			{
				m_arCheckButton[i]->set_sensitive(true);        
			}
        }

		if (m_camInfo.iidcVer < sk_IIDC132Version || ((regVal & 0x03FF0000) != 0))
		{
			m_pBtnEmbeddedSelectAll->set_sensitive(true);
			m_pBtnEmbeddedUnselectAll->set_sensitive(true);
		}
    }

    void AdvCameraSettingsPage::DisableEmbedded()
    {
        for (int i=0; i < EMBEDDED_NUM_INFORMATION; i++)
        {
            m_arCheckButton[i]->set_sensitive(false);        
        }

        m_pBtnEmbeddedSelectAll->set_sensitive(false);
        m_pBtnEmbeddedUnselectAll->set_sensitive(false);
    }    

    void AdvCameraSettingsPage::OnAutoRangeControlComboBoxChanged()
    {        
        // Get the selected mode
        Gtk::TreeModel::iterator iter = m_pComboBoxAutoRangeControl->get_active();

        if (iter == 0)
        {
            return;            
        }

        Gtk::TreeModel::Row row = *iter;

        PropertyType propType;
        unsigned int propRegister;
        propType = row[m_autoRangeControlColumns.m_colProperty];   
        propRegister = row[m_autoRangeControlColumns.m_colPropertyRegister];

        Error error;
        unsigned int propRegVal = 0;
        error = m_pCamera->ReadRegister( propRegister, &propRegVal );
        if ( error != PGRERROR_OK )
        {
            // Error
            return;
        }

        if ( propRegVal >> 31 )
        {
            char minValStr[64];
            sprintf( minValStr, "%03u", (propRegVal & 0x00FFF000) >> 12 );
            m_pEntryAutoRangeMin->set_text( minValStr );

            char maxValStr[64];
            sprintf( maxValStr, "%03u", propRegVal & 0x00000FFF );
            m_pEntryAutoRangeMax->set_text( maxValStr );   

            m_pEntryAutoRangeMin->set_sensitive( true );
            m_pEntryAutoRangeMax->set_sensitive( true );
            m_pBtnGetAutoRange->set_sensitive( true );
            m_pBtnSetAutoRange->set_sensitive( true );
        }
        else
        {
            const char* unknownStr = "N/A";
            m_pEntryAutoRangeMin->set_text( unknownStr );
            m_pEntryAutoRangeMax->set_text( unknownStr );    

            m_pEntryAutoRangeMin->set_sensitive( false );
            m_pEntryAutoRangeMax->set_sensitive( false );
            m_pBtnGetAutoRange->set_sensitive( false );
            m_pBtnSetAutoRange->set_sensitive( false );
        }        
    }

    void AdvCameraSettingsPage::OnGetAutoRangeControl()
    {
        OnAutoRangeControlComboBoxChanged();
    }

    void AdvCameraSettingsPage::OnSetAutoRangeControl()
    {
        // Get the selected mode
        Gtk::TreeModel::iterator iter = m_pComboBoxAutoRangeControl->get_active();

        if (iter == 0)
        {
            return;            
        }

        Gtk::TreeModel::Row row = *iter;

        PropertyType propType;
        unsigned int propRegister;
        propType = row[m_autoRangeControlColumns.m_colProperty];   
        propRegister = row[m_autoRangeControlColumns.m_colPropertyRegister];

        Glib::ustring minValStr = m_pEntryAutoRangeMin->get_text();
        unsigned int minVal = strtoul( minValStr.c_str(), NULL, 10 );

        Glib::ustring maxValStr = m_pEntryAutoRangeMax->get_text();
        unsigned int maxVal = strtoul( maxValStr.c_str(), NULL, 10 );

        unsigned int propRegVal = 0;
        propRegVal += minVal << 12;
        propRegVal += maxVal;

        Error error;
        error = m_pCamera->WriteRegister( propRegister, propRegVal );
        if ( error != PGRERROR_OK )
        {
            // Error
            return;
        }
    }

    void AdvCameraSettingsPage::UpdateMemoryChannels()
    {
        UpdateMemChannelComboBox();

        Error error;
        unsigned int currMemChannel = 0;
        error = m_pCamera->GetMemoryChannel( &currMemChannel );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting current memory channel", error );
        }  

        SetActiveMemChannelComboBox( currMemChannel );
    }

    void AdvCameraSettingsPage::UpdateImageDataFormat()
    {
        // Read image data format register
        unsigned int dataDepthVal = 0, rawBayerOutputVal = 0, mirrorImageVal = 0, testPatternVal = 0;       
		unsigned int imageDataFmtVal = 0;    

        Error error;
		if(m_camInfo.iidcVer >= sk_IIDC132Version)
		{
			error = m_pCamera->ReadRegister(sk_dataDepthCtrlReg, &dataDepthVal);
			if ( error != PGRERROR_OK )
			{
				// Error
				ShowErrorMessageDialog( "Error reading data depth register", error );
			}

			error = m_pCamera->ReadRegister(sk_mirrorImageCtrlReg, &mirrorImageVal);
			if ( error != PGRERROR_OK )
			{
				// Error
				ShowErrorMessageDialog( "Error reading mirror image control register", error );
			}		
			error = m_pCamera->ReadRegister(sk_bayerMonoCtrlReg, &rawBayerOutputVal);
			if ( error != PGRERROR_OK )
			{
				// Error
				ShowErrorMessageDialog( "Error reading bayer mono control register", error );
			}
			m_pRadiobuttonY16BigEndian->set_sensitive( true );
			m_pRadiobuttonY16LittleEndian->set_sensitive( true );
			m_pCheckbuttonMirrorImage->set_sensitive( (mirrorImageVal & 0x80000000) != 0);
			m_pCheckbuttonRawBayerOutput->set_sensitive( (rawBayerOutputVal & 0x80000000) != 0 );

			m_pCheckbuttonRawBayerOutput->set_active( (rawBayerOutputVal & 0x1) != 0);
			m_pCheckbuttonMirrorImage->set_active( (mirrorImageVal & 0x1) != 0 );
			m_pRadiobuttonY16LittleEndian->set_active( (dataDepthVal  & 0x00800000) != 0 );
			m_pRadiobuttonY16BigEndian->set_active( (dataDepthVal  & 0x00800000) == 0 );
		}
		else
		{
			error = m_pCamera->ReadRegister( sk_imageDataFmtReg, &imageDataFmtVal );
			if ( error != PGRERROR_OK )
			{
				// Error
				ShowErrorMessageDialog( "Error reading image data format register", error );
			}       

			m_pCheckbuttonRawBayerOutput->set_sensitive( (imageDataFmtVal >> 31) != 0);
			m_pCheckbuttonMirrorImage->set_sensitive( (imageDataFmtVal >> 31) != 0);
			m_pRadiobuttonY16BigEndian->set_sensitive( (imageDataFmtVal >> 31) != 0);
			m_pRadiobuttonY16LittleEndian->set_sensitive( (imageDataFmtVal >> 31) != 0);

			// Update Bayer output
			bool bayerEnabled = (imageDataFmtVal & (0x1 << 7)) != 0 ? true : false;
			m_pCheckbuttonRawBayerOutput->set_active( bayerEnabled );

			// Update mirror output
			bool mirror = (imageDataFmtVal & (0x1 << 8)) != 0  ? true : false;
			m_pCheckbuttonMirrorImage->set_active( mirror );

			// Update Y16 output
			bool bY16Endianness = (imageDataFmtVal  & (0x1 << 0)) != 0 ? true : false;
			if ( bY16Endianness == true )
			{
				m_pRadiobuttonY16BigEndian->set_active( true );
			}
			else
			{
				m_pRadiobuttonY16LittleEndian->set_active( true );
			}
		}

		error = m_pCamera->ReadRegister(sk_testPatternReg, &testPatternVal);
		if ( error != PGRERROR_OK )
		{
			// Error
			ShowErrorMessageDialog( "Error reading bayer mono control register", error );
		}
		m_pRadiobuttonY16TestPattern1->set_sensitive((testPatternVal & 0x80000000) != 0);
		m_pRadiobuttonY16TestPattern2->set_sensitive((testPatternVal & 0x80000000) != 0);
		m_pRadiobuttonY16TestPatternNone->set_sensitive((testPatternVal & 0x80000000) != 0);
    }

    void AdvCameraSettingsPage::UpdateTestPattern()
    {
        unsigned int testPatternVal = 0;
        Error error = m_pCamera->ReadRegister( sk_testPatternReg, &testPatternVal );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading test pattern register", error );
        }

        if ( testPatternVal >> 31 )
        {
            m_pRadiobuttonY16TestPattern1->set_sensitive( true );
            m_pRadiobuttonY16TestPattern2->set_sensitive( true );
            m_pRadiobuttonY16TestPatternNone->set_sensitive( true );
        }
        else
        {
            m_pRadiobuttonY16TestPattern1->set_sensitive( false );
            m_pRadiobuttonY16TestPattern2->set_sensitive( false );
            m_pRadiobuttonY16TestPatternNone->set_sensitive( false );
        }

        // Update test pattern
        bool testPattern1 = (testPatternVal & (0x1 << 0)) != 0 ? true : false;
        bool testPattern2 = (testPatternVal & (0x1 << 1)) != 0 ? true : false;

        if ( testPattern1 && !testPattern2 )
        {
            m_pRadiobuttonY16TestPattern1->set_active( true );
        }
        else if ( !testPattern1 && testPattern2 )
        {
            m_pRadiobuttonY16TestPattern2->set_active( true );
        }
        else if ( !testPattern1 && !testPattern2 )
        {
            m_pRadiobuttonY16TestPatternNone->set_active( true );
        }
        else
        {
            // Should never get here
        }
    }    

    void AdvCameraSettingsPage::UpdateBayerOutput()
    {
        // If the camera is not color, disable bayer output checkbox
        if ( m_camInfo.isColorCamera != true )
        {
            m_pCheckbuttonRawBayerOutput->set_sensitive( false );
        }
    }

    sigc::signal<void> AdvCameraSettingsPage::signal_restore_from_memory_channel()
    {
        return m_signal_restore_from_memory_channel;
    }
}
