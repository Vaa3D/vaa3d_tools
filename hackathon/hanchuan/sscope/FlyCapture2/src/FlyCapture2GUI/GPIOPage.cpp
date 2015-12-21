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
// $Id: GPIOPage.cpp,v 1.40 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "GPIOPage.h"

namespace FlyCapture2
{
    // Misc
    const char* GPIOPage::sk_checkbuttonToggleTrigger = "checkbuttonToggleTrigger";
    const char* GPIOPage::sk_comboBoxTriggerMode = "comboboxTriggerMode";
    const char* GPIOPage::sk_entryParameter = "entryParameter";    
    const char* GPIOPage::sk_btnFireSoftwareTrigger = "btnFireSoftwareTrigger";

    // Trigger polarity
    const char* GPIOPage::sk_radioTriggerPolarityLow = "radioTriggerPolarityLow";
    const char* GPIOPage::sk_radioTriggerPolarityHigh = "radioTriggerPolarityHigh";

    // Trigger delay
    const char* GPIOPage::sk_checkbuttonDelay = "checkbuttonDelay";
    const char* GPIOPage::sk_hscaleDelay = "hscaleDelay";
    const char* GPIOPage::sk_spinDelay = "spinDelay";
    const char* GPIOPage::sk_labelMinTriggerDelay = "labelMinTriggerDelay";
    const char* GPIOPage::sk_labelMaxTriggerDelay = "labelMaxTriggerDelay";

    // Trigger source
    const char* GPIOPage::sk_radioSourceGPIO0 = "radioGPIO0Source";		
    const char* GPIOPage::sk_radioSourceGPIO1 = "radioGPIO1Source";		
    const char* GPIOPage::sk_radioSourceGPIO2 = "radioGPIO2Source";	
    const char* GPIOPage::sk_radioSourceGPIO3 = "radioGPIO3Source";	
    const char* GPIOPage::sk_radioSourceNone = "radioNoneSource";		

    GPIOPage::GPIOPage()
    {
    }

    GPIOPage::GPIOPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
    {
    }

    GPIOPage::~GPIOPage()
    {
        for ( unsigned int i=0; i < sk_numPins; i++ )
        {
            delete m_widgetPinArray[i].pDelayAdjustment;
            delete m_widgetPinArray[i].pDurationAdjustment;
        }
        
        delete m_pAdjustmentDelay;
    }

    void GPIOPage::UpdateWidgets()
    {
        if ( m_pCamera == NULL || IsConnected() != true )
        {
            DisableTrigger();
            DisableAllStrobeControls();
            return;	
        }

        UpdateTrigger();
        UpdateStrobe(); 
        UpdatePinDirection();
    }

    void GPIOPage::GetWidgets()
    {
        // Misc
        m_refXml->get_widget( sk_checkbuttonToggleTrigger, m_pCheckbuttonToggleTrigger );
        m_refXml->get_widget( sk_comboBoxTriggerMode, m_pComboBoxMode );
        m_refXml->get_widget( sk_entryParameter, m_pEntryParameter );        
        m_refXml->get_widget( sk_btnFireSoftwareTrigger, m_pBtnFireSoftwareTrigger );

        // Trigger polarity
        m_refXml->get_widget( sk_radioTriggerPolarityLow, m_pTriggerPolarityLow );
        m_refXml->get_widget( sk_radioTriggerPolarityHigh, m_pTriggerPolarityHigh );

        // Trigger delay
        m_refXml->get_widget( sk_checkbuttonDelay, m_pCheckbuttonDelay );
        m_refXml->get_widget( sk_hscaleDelay, m_pHScaleDelay );
        m_refXml->get_widget( sk_spinDelay, m_pSpinDelay );
        m_refXml->get_widget( sk_labelMinTriggerDelay, m_pLabelMinTriggerDelay );
        m_refXml->get_widget( sk_labelMaxTriggerDelay, m_pLabelMaxTriggerDelay );

        // Create the trigger delay adjustment
        m_pAdjustmentDelay = new Gtk::Adjustment( 0.0f, 0.0f, 1.0f );
        m_pHScaleDelay->set_adjustment( *m_pAdjustmentDelay );
        m_pSpinDelay->set_adjustment( *m_pAdjustmentDelay );

        // Trigger source
        m_refXml->get_widget( sk_radioSourceGPIO0, m_triggerSourceArray[0] );
        m_refXml->get_widget( sk_radioSourceGPIO1, m_triggerSourceArray[1] );
        m_refXml->get_widget( sk_radioSourceGPIO2, m_triggerSourceArray[2] );
        m_refXml->get_widget( sk_radioSourceGPIO3, m_triggerSourceArray[3] );
        m_refXml->get_widget( sk_radioSourceNone, m_triggerSourceArray[4] );

        // GPIO pin direction
        for ( unsigned int i=0; i < sk_numPins; i++ )
        {
            char widgetName[64];

            sprintf( widgetName, "radioGPIO%dIn", i );
            m_refXml->get_widget( widgetName, m_widgetPinDirection[i].pIn );

            sprintf( widgetName, "radioGPIO%dOut", i );
            m_refXml->get_widget( widgetName, m_widgetPinDirection[i].pOut );
        }

        // GPIO widgets
        for ( unsigned int i=0; i < sk_numPins; i++ )
        {
            char widgetName[64];

            sprintf( widgetName, "checkGPIO%dStrobe", i );
            m_refXml->get_widget( widgetName, m_widgetPinArray[i].pOnOff );

            sprintf( widgetName, "radioGPIO%dHigh", i );
            m_refXml->get_widget( widgetName, m_widgetPinArray[i].pHigh );

            sprintf( widgetName, "radioGPIO%dLow", i );
            m_refXml->get_widget( widgetName, m_widgetPinArray[i].pLow );

            sprintf( widgetName, "spinGPIO%dDelay", i );
            m_refXml->get_widget( widgetName, m_widgetPinArray[i].pDelay );

            sprintf( widgetName, "spinGPIO%dDuration", i );
            m_refXml->get_widget( widgetName, m_widgetPinArray[i].pDuration );
        }        

        // Create the strobe delay and duration adjustments
        for ( unsigned int i=0; i < sk_numPins; i++ )
        {
            Gtk::Adjustment* pNewDelayAdjustment = new Gtk::Adjustment( 0.0f, 0.0f, 1.0f );
            m_widgetPinArray[i].pDelayAdjustment = pNewDelayAdjustment;
            m_widgetPinArray[i].pDelay->set_adjustment( *pNewDelayAdjustment );

            Gtk::Adjustment* pNewDurationAdjustment = new Gtk::Adjustment( 0.0f, 0.0f, 1.0f );
            m_widgetPinArray[i].pDurationAdjustment = pNewDurationAdjustment;
            m_widgetPinArray[i].pDuration->set_adjustment( *pNewDurationAdjustment );
        }

        // Attach the list store
        m_refListStoreTriggerModes = Gtk::ListStore::create( m_triggerModeColumns );
        m_pComboBoxMode->set_model( m_refListStoreTriggerModes );
    }

    void GPIOPage::AttachSignals()
    {
        m_pCheckbuttonToggleTrigger->signal_clicked().connect( sigc::mem_fun( *this, &GPIOPage::OnToggleTriggerClicked ) );
        m_pBtnFireSoftwareTrigger->signal_clicked().connect( sigc::mem_fun( *this, &GPIOPage::OnFireSoftwareTriggerClicked ) );
        m_pCheckbuttonDelay->signal_clicked().connect( sigc::mem_fun( *this, &GPIOPage::OnDelayChecked ) );
        m_pAdjustmentDelay->signal_value_changed().connect( sigc::mem_fun( *this, &GPIOPage::OnDelayScroll ) );
        m_pComboBoxMode->signal_changed().connect( sigc::mem_fun( *this, &GPIOPage::OnTriggerModeChanged ) );
        m_pEntryParameter->signal_focus_out_event().connect( sigc::mem_fun( *this, &GPIOPage::OnEntryParameterLostFocus ) );
        m_pTriggerPolarityLow->signal_toggled().connect( sigc::mem_fun( *this, &GPIOPage::OnTriggerPolarityChanged ) );
        m_pTriggerPolarityHigh->signal_toggled().connect( sigc::mem_fun( *this, &GPIOPage::OnTriggerPolarityChanged ) );

        for ( unsigned int i=0; i < sk_numPins + 1; i++ )
        {
            Gtk::RadioButton* pSource = m_triggerSourceArray[i];

            pSource->signal_toggled().connect( sigc::bind<unsigned int, Gtk::RadioButton*>( sigc::mem_fun( *this, &GPIOPage::OnRadioSourceChanged ), i, pSource ) );            
        }

        for ( unsigned int i=0; i < sk_numPins; i++ )
        {
            Gtk::CheckButton* pOnOff = m_widgetPinArray[i].pOnOff;
            Gtk::RadioButton* pHigh = m_widgetPinArray[i].pHigh;
            Gtk::RadioButton* pLow = m_widgetPinArray[i].pLow;
            Gtk::Adjustment* pDelayAdjustment = m_widgetPinArray[i].pDelayAdjustment;
            Gtk::Adjustment* pDurationAdjustment = m_widgetPinArray[i].pDurationAdjustment;

            pOnOff->signal_toggled().connect( 
                sigc::bind<int, Gtk::CheckButton*>( 
                    sigc::mem_fun( *this, &GPIOPage::OnCheckOnOffChanged ), i, pOnOff ) );  

            pHigh->signal_toggled().connect( 
                sigc::bind<int, Gtk::RadioButton*>( 
                    sigc::mem_fun( *this, &GPIOPage::OnRadioPolarityChanged ), i, pHigh ) );            

            pLow->signal_toggled().connect( 
                sigc::bind<int, Gtk::RadioButton*>( 
                    sigc::mem_fun( *this, &GPIOPage::OnRadioPolarityChanged ), i, pLow ) );	           

            pDelayAdjustment->signal_value_changed().connect( 
                sigc::bind<int>( 
                    sigc::mem_fun( *this, &GPIOPage::OnSpinDelayScroll ), i ) );	            

            pDurationAdjustment->signal_value_changed().connect( 
                sigc::bind<int>( 
                    sigc::mem_fun( *this, &GPIOPage::OnSpinDurationScroll ), i ) );	

            Gtk::RadioButton* pIn = m_widgetPinDirection[i].pIn;
            Gtk::RadioButton* pOut = m_widgetPinDirection[i].pOut;

            pIn->signal_toggled().connect(
                sigc::bind<int, Gtk::RadioButton*>(
                    sigc::mem_fun( *this, &GPIOPage::OnRadioPinDirectionChanged ), i, pIn ) );

            pOut->signal_toggled().connect(
                sigc::bind<int, Gtk::RadioButton*>(
                    sigc::mem_fun( *this, &GPIOPage::OnRadioPinDirectionChanged ), i, pOut ) );
        }        

		  SetTimerConnection(
			  Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &GPIOPage::OnTimer), 
            sk_timeout ));
    }

    // Misc event handlers
    void GPIOPage::OnToggleTriggerClicked()
    {
        Error error;
        TriggerMode triggerMode;
        error = m_pCamera->GetTriggerMode( &triggerMode );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger mode", error );
        }

        if ( triggerMode.onOff == m_pCheckbuttonToggleTrigger->get_active() )
        {
            return;
        }  

        triggerMode.onOff = m_pCheckbuttonToggleTrigger->get_active();

        error = m_pCamera->SetTriggerMode( &triggerMode );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting trigger mode", error );
        }        
    }

    void GPIOPage::OnFireSoftwareTriggerClicked()
    {
        Error error;
        error = m_pCamera->FireSoftwareTrigger();
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error firing software trigger", error );
        }        
    }

    void GPIOPage::OnDelayChecked()
    {
        if ( m_pCheckbuttonDelay->get_active() == true )
        {
            m_pHScaleDelay->set_sensitive( true );
            m_pSpinDelay->set_sensitive( true );

            // Update the trigger adjustment
            Error error;
            PropertyInfo propInfo;
            propInfo.type = TRIGGER_DELAY;
            error = m_pCamera->GetPropertyInfo( &propInfo );
            if ( error != PGRERROR_OK )
            {
                // Error
                ShowErrorMessageDialog( "Error getting trigger property information", error );
            }

            Property prop;
            prop.type = TRIGGER_DELAY;
            error = m_pCamera->GetProperty( &prop );
            if ( error != PGRERROR_OK )
            {
                // Error
                ShowErrorMessageDialog( "Error getting trigger information", error );
            }

            m_pAdjustmentDelay->set_lower( propInfo.absMin );
            m_pAdjustmentDelay->set_upper( propInfo.absMax );
            m_pAdjustmentDelay->set_value( prop.absValue );

            double stepIncrement = (propInfo.absMax - propInfo.absMin) / 100.0f;
            double pageIncrement = (propInfo.absMax - propInfo.absMin) / 10.0f;
            m_pAdjustmentDelay->set_step_increment( stepIncrement );
            m_pAdjustmentDelay->set_page_increment( pageIncrement );

            char value[32];

            sprintf( value, "%3.5fs", propInfo.absMin );
            m_pLabelMinTriggerDelay->set_text( value );

            sprintf( value, "%3.5fs", propInfo.absMax );
            m_pLabelMaxTriggerDelay->set_text( value );
        }
        else
        {
            m_pHScaleDelay->set_sensitive( false );
            m_pSpinDelay->set_sensitive( false );
        }
    }

    void GPIOPage::OnDelayScroll()
    {
        double delay = m_pAdjustmentDelay->get_value();

        Error error;
        TriggerDelay triggerDelay;
        triggerDelay.type = TRIGGER_DELAY;

        error = m_pCamera->GetTriggerDelay( &triggerDelay );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger delay", error );
        }

        triggerDelay.absControl = true;
        triggerDelay.absValue = static_cast<float>(delay);
        triggerDelay.onOff = true;

        error = m_pCamera->SetTriggerDelay( &triggerDelay );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting trigger delay", error );
        }

        error = m_pCamera->GetTriggerDelay( &triggerDelay );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger delay", error );
        }
    }

    void GPIOPage::OnTriggerModeChanged()
    {
        // Get the selected mode
        Gtk::TreeModel::iterator iter = m_pComboBoxMode->get_active();

        if (iter)
        {
            Gtk::TreeModel::Row row = *iter;

            int triggerMode = row[m_triggerModeColumns.m_colTriggerMode];

            Error error;
            TriggerMode triggerModeStruct;
            error = m_pCamera->GetTriggerMode( &triggerModeStruct );
            if ( error != PGRERROR_OK )
            {
                // Error
                ShowErrorMessageDialog( "Error getting trigger mode", error );
            }

            triggerModeStruct.mode = static_cast<unsigned int>(triggerMode);
            
            error = m_pCamera->SetTriggerMode( &triggerModeStruct );
            if ( error != PGRERROR_OK )
            {
                // Error
                ShowErrorMessageDialog( "Error setting trigger mode", error );
            }
        }
    }

    void GPIOPage::OnTriggerPolarityChanged()
    {
        Error error;
        TriggerMode triggerModeStruct;
        error = m_pCamera->GetTriggerMode( &triggerModeStruct );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger mode", error );
        }

        if ( m_pTriggerPolarityLow->get_active() == true && 
            triggerModeStruct.polarity != 0 )
        {
            triggerModeStruct.polarity = 0;
        }
        else if ( m_pTriggerPolarityHigh->get_active() == true &&
            triggerModeStruct.polarity != 1 )
        {
            triggerModeStruct.polarity = 1;
        }        

        error = m_pCamera->SetTriggerMode( &triggerModeStruct );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting trigger mode", error );
        }
    }

    bool GPIOPage::OnEntryParameterLostFocus( GdkEventFocus* /*event*/ )
    {
        Error error;
        TriggerMode triggerModeStruct;
        error = m_pCamera->GetTriggerMode( &triggerModeStruct );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger mode", error );
        }

        unsigned int parameter = strtoul( m_pEntryParameter->get_text().c_str(), NULL, 10 );
        triggerModeStruct.parameter = parameter;

        error = m_pCamera->SetTriggerMode( &triggerModeStruct );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting trigger mode", error );
        }

        return true;
    }

    void GPIOPage::OnRadioSourceChanged( unsigned int pin, Gtk::RadioButton* pButton )
    {       
        if ( pButton->get_active() != true )
        {
            return;
        }        

        Error error;
        TriggerMode triggerModeStruct;
        error = m_pCamera->GetTriggerMode( &triggerModeStruct );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger mode", error );
        }

        // The selected pin is the current source, nothing to be done here
        if ( triggerModeStruct.source == static_cast<unsigned int>(pin) )
        {
            return;            
        }            

        // Test the pin if it can be set as an input
        bool bOk = TestPinInputSuccess( pin );
        if ( bOk != true )
        {
            m_triggerSourceArray[triggerModeStruct.source]->set_active( true );
            return;
        }

        // The pin can be set as an input, continue with setting it to
        // the camera
        triggerModeStruct.source = pin;

        error = m_pCamera->SetTriggerMode( &triggerModeStruct );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting trigger mode", error );
        }

        UpdateTrigger();

        for ( unsigned int i=0; i < sk_numPins; i++ )
        {
            if ( pin == i )
            {
                DisableStrobeControls( i );
            }
            else
            {
                EnableStrobeControls( i );
            }
        }        
    }

    void GPIOPage::OnCheckOnOffChanged( int pin, Gtk::CheckButton* pButton )
    {        
        //printf( "OnCheckOnOffChanged: %d\n", iPin );

        Error error;
        StrobeControl strobeControl;
        strobeControl.source = pin;

        error = m_pCamera->GetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            DisableStrobeControls( pin );
            //ShowErrorMessageDialog( "Error getting strobe", error );
        }

        if ( strobeControl.onOff == pButton->get_active() )
        {
            // Nothing to do here, because the strobe is already in the
            // proper mode
            return;
        }

        strobeControl.onOff = pButton->get_active();

        error = m_pCamera->SetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            DisableStrobeControls( pin );
            //ShowErrorMessageDialog( "Error setting strobe", error );
        }

        UpdatePinDirection();
    }    

    void GPIOPage::OnRadioPolarityChanged( int pin, Gtk::RadioButton* pButton  )
    {
        if ( pButton->get_active() != true )
        {
            return;
        }

        //printf( "OnRadioPolarityChanged: %d\n", pin );

        Error error;
        StrobeControl strobeControl;
        strobeControl.source = pin;

        error = m_pCamera->GetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            DisableStrobeControls( pin );
            //ShowErrorMessageDialog( "Error getting strobe", error );
        }

        if ( m_widgetPinArray[pin].pHigh->get_active() == true )
        {
            strobeControl.polarity = 1;
        }
        else if ( m_widgetPinArray[pin].pLow->get_active() == true )
        {
            strobeControl.polarity = 0;
        }

        error = m_pCamera->SetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            DisableStrobeControls( pin );
            //ShowErrorMessageDialog( "Error setting strobe", error );
        }
    }

    void GPIOPage::OnSpinDelayScroll( int pin )
    {
        //printf( "OnSpinDelayScroll: %d\n", pin );

        Error error;
        StrobeControl strobeControl;
        strobeControl.source = pin;

        error = m_pCamera->GetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            DisableStrobeControls( pin );
            //ShowErrorMessageDialog( "Error getting strobe", error );
        }

        Gtk::Adjustment* pAdjustment = m_widgetPinArray[pin].pDelayAdjustment;
        strobeControl.delay = static_cast<float>(pAdjustment->get_value());

        error = m_pCamera->SetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            DisableStrobeControls( pin );
            //ShowErrorMessageDialog( "Error setting strobe", error );
        }
    }

    void GPIOPage::OnSpinDurationScroll( int pin )
    {
        //printf( "OnSpinDurationScroll: %d\n", pin );

        Error error;
        StrobeControl strobeControl;
        strobeControl.source = pin;

        error = m_pCamera->GetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting strobe", error );
        }

        Gtk::Adjustment* pAdjustment = m_widgetPinArray[pin].pDurationAdjustment;
        strobeControl.duration = static_cast<float>(pAdjustment->get_value());

        error = m_pCamera->SetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting strobe", error );
        }
    }

    void GPIOPage::OnRadioPinDirectionChanged( int pin, Gtk::RadioButton* pButton )
    {
        if ( pButton->get_active() != true )
        {
            return;
        }

        Error error;
        unsigned int direction;
        error = m_pCamera->GetGPIOPinDirection( pin, &direction );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting GPIO pin direction", error );
        }

        if ( m_widgetPinDirection[pin].pIn->get_active() == true &&
            direction == 1 )
        {
            // Direction is in and it needs to be set to the camera
            error = m_pCamera->SetGPIOPinDirection( pin, 0 );

            UpdateSingleStrobe( pin );
        }
        else if ( m_widgetPinDirection[pin].pOut->get_active() == true &&
            direction == 0 )
        {
            // Direction is out and it needs to be set to the camera
            error = m_pCamera->SetGPIOPinDirection( pin, 1 );

            UpdateSingleStrobe( pin );
        }
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting GPIO pin direction", error );
        }
    }

    bool GPIOPage::OnTimer()
    {
        if (m_pCamera == NULL || IsIconified())
        {
            return true;
        }

        Gtk::Notebook* pNotebook;
        m_refXml->get_widget( "notebookCamCtl", pNotebook );
        if ( pNotebook->get_current_page() == 5 )
        {
            UpdatePinDirection();
        }

        return true;
    }

    void GPIOPage::UpdateTrigger()
    {
        Error error;

        TriggerModeInfo triggerInfo;
        error = m_pCamera->GetTriggerModeInfo( &triggerInfo );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger mode information", error );
            DisableTrigger();
            return;
        }

        TriggerMode trigger;        
        error = m_pCamera->GetTriggerMode( &trigger );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger mode", error );
            DisableTrigger();
            return;
        }

        EnableTrigger();

        m_pComboBoxMode->set_sensitive( false );
        m_pEntryParameter->set_sensitive( false );
        m_pCheckbuttonDelay->set_sensitive( false );
        m_pHScaleDelay->set_sensitive( false );
        m_pSpinDelay->set_sensitive( false );
        //m_pBtnFireSoftwareTrigger->set_sensitive( false );        

        if ( triggerInfo.present == true && 
            triggerInfo.readOutSupported == true && 
            triggerInfo.onOffSupported == true )
        {           
            m_pCheckbuttonToggleTrigger->set_sensitive( true );
            m_pCheckbuttonToggleTrigger->set_active( trigger.onOff );            

            UpdateModeComboBox( &triggerInfo );
            UpdateActiveTriggerSource( &trigger );
            UpdateTriggerParameter( &triggerInfo, &trigger );
            UpdateTriggerPolarity( &trigger );
            UpdateTriggerDelay();

            SetActiveModeComboBox( trigger.mode );
        }
        else
        {
            m_pComboBoxMode->set_sensitive( false );
            m_pCheckbuttonToggleTrigger->set_sensitive( false );		
            m_pBtnFireSoftwareTrigger->set_sensitive( false );
            m_pEntryParameter->set_sensitive( false );
            m_pCheckbuttonDelay->set_sensitive( false );
            m_pHScaleDelay->set_sensitive( false );
            m_pSpinDelay->set_sensitive( false );
            m_pBtnFireSoftwareTrigger->set_sensitive( false );
        }
    }

    void GPIOPage::UpdateStrobe()
    {
        for ( unsigned int i = 0; i < sk_numPins; i++ )
        {
            UpdateSingleStrobe( i );
        }
    }

    void GPIOPage::UpdateSingleStrobe( unsigned int pin )
    {
        Error error;

        StrobeInfo strobeInfo;
        strobeInfo.source = pin;
        error = m_pCamera->GetStrobeInfo( &strobeInfo );
        if ( error != PGRERROR_OK )
        {
            // Error
            DisableStrobeControls( pin );
            //ShowErrorMessageDialog( "Error getting strobe information", error );
            return;
        }

        StrobeControl strobeControl;
        strobeControl.source = pin;
        error = m_pCamera->GetStrobe( &strobeControl );
        if ( error != PGRERROR_OK )
        {
            // Error
            DisableStrobeControls( pin );
            //ShowErrorMessageDialog( "Error getting strobe", error );
            return;
        }        

        m_widgetPinArray[pin].pOnOff->set_active( strobeControl.onOff );   

        if ( strobeControl.polarity == 0 )
        {
            m_widgetPinArray[pin].pLow->set_active( true );
        }
        else
        {
            m_widgetPinArray[pin].pHigh->set_active( false );
        }

        m_widgetPinArray[pin].pDelayAdjustment->set_lower( strobeInfo.minValue );
        m_widgetPinArray[pin].pDelayAdjustment->set_upper( strobeInfo.maxValue );
        m_widgetPinArray[pin].pDelayAdjustment->set_value( strobeControl.delay );

        m_widgetPinArray[pin].pDurationAdjustment->set_lower( strobeInfo.minValue );
        m_widgetPinArray[pin].pDurationAdjustment->set_upper( strobeInfo.maxValue );
        m_widgetPinArray[pin].pDurationAdjustment->set_value( strobeControl.duration );
    }

    void GPIOPage::UpdatePinDirection()
    {
        for (unsigned int i=0; i < sk_numPins; i++)
        {
            Error error;
            unsigned int direction = 0;
            error = m_pCamera->GetGPIOPinDirection( i, &direction );

            if ( direction == 0 )
            {
                m_widgetPinDirection[i].pIn->set_active( true );
            }
            else if ( direction == 1 )
            {
                m_widgetPinDirection[i].pOut->set_active( true );
            }
            else
            {
                // Error
            }
        }
    }

    void GPIOPage::UpdateModeComboBox( TriggerModeInfo* pInfo )
    {
        m_refListStoreTriggerModes->clear();

        if ( pInfo->present == true )
        {
            const int numTriggerModes = 16;

            for ( int i=0; i < numTriggerModes; i++ )
            {
                int reverse = numTriggerModes - i - 1;
                bool modeAvailable = ( (pInfo->modeMask >> reverse) & 0x1 ) == 1;
                if ( modeAvailable == true )
                {
                    char mode[16];
                    sprintf( mode, "%d", i );		

                    Gtk::TreeModel::Row row = *(m_refListStoreTriggerModes->append());
                    row[m_triggerModeColumns.m_colTriggerModeStr] = mode;
                    row[m_triggerModeColumns.m_colTriggerMode] = i;
                }
            }

            m_pComboBoxMode->clear();
            m_pComboBoxMode->pack_start( m_triggerModeColumns.m_colTriggerModeStr );
            m_pComboBoxMode->set_sensitive( true );
        }
        else
        {
            m_pComboBoxMode->set_sensitive( false );
        }      
    }  

    void GPIOPage::UpdateActiveTriggerSource( TriggerMode* pTrigger )
    {
        unsigned int pinToDisable = 4;

        if ( pTrigger->source < sk_numPins )
        {
            m_triggerSourceArray[pTrigger->source]->set_active( true );
            pinToDisable = pTrigger->source;
        }
        else
        {
            m_triggerSourceArray[4]->set_active( true );
        }
        
        for ( unsigned int i = 0; i < sk_numPins; i++ )
        {
            if ( i == pinToDisable )
            {
                DisableStrobeControls( i );
            }
            else
            {
                EnableStrobeControls( i );
            }
        }
    }

    void GPIOPage::UpdateTriggerParameter( TriggerModeInfo* /*pInfo*/, TriggerMode* pTrigger )
    {
        char parameter[32];
        sprintf( parameter, "%u", pTrigger->parameter );
        m_pEntryParameter->set_text( parameter );

        m_pEntryParameter->set_sensitive( true );
    }

    void GPIOPage::UpdateTriggerPolarity( TriggerMode* pTrigger )
    {
        if ( m_pTriggerPolarityLow->get_active() == true && pTrigger->polarity != 0 )
        {
            m_pTriggerPolarityLow->set_active();
        }
        else if ( m_pTriggerPolarityHigh->get_active() == true && pTrigger->polarity == 0 )
        {
            m_pTriggerPolarityHigh->set_active();
        }
    }

    void GPIOPage::UpdateTriggerDelay()
    {
        Error error;
        PropertyInfo propInfo;
        propInfo.type = TRIGGER_DELAY;
        error = m_pCamera->GetPropertyInfo( &propInfo );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger information", error );
        }

        Property prop;
        prop.type = TRIGGER_DELAY;
        error = m_pCamera->GetProperty( &prop );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting trigger", error );
        }

        if ( propInfo.onOffSupported == true )
        {            
            m_pCheckbuttonDelay->set_active( prop.onOff );
            m_pCheckbuttonDelay->set_sensitive( true );

            m_pSpinDelay->set_sensitive( prop.onOff );
            m_pHScaleDelay->set_sensitive( prop.onOff );

            m_pAdjustmentDelay->set_lower(propInfo.absMin);
            m_pAdjustmentDelay->set_upper(propInfo.absMax);
            m_pAdjustmentDelay->set_value(prop.absValue);    

            double stepIncrement = (propInfo.absMax - propInfo.absMin) / 100.0f;
            double pageIncrement = (propInfo.absMax - propInfo.absMin) / 10.0f;
            m_pAdjustmentDelay->set_step_increment( stepIncrement );
            m_pAdjustmentDelay->set_page_increment( pageIncrement );

            char value[32];

            sprintf( value, "%3.5fs", propInfo.absMin );
            m_pLabelMinTriggerDelay->set_text( value );

            sprintf( value, "%3.5fs", propInfo.absMax );
            m_pLabelMaxTriggerDelay->set_text( value );
        }
        else
        {
            m_pCheckbuttonDelay->set_sensitive( false );

            m_pSpinDelay->set_sensitive( false );
            m_pHScaleDelay->set_sensitive( false );
        }
    }    

    void GPIOPage::EnableTrigger()
    {
        m_pCheckbuttonToggleTrigger->set_sensitive( true );
        m_pComboBoxMode->set_sensitive( true );
        m_pEntryParameter->set_sensitive( true );
        m_pBtnFireSoftwareTrigger->set_sensitive( true );

        m_pTriggerPolarityLow->set_sensitive( true );
        m_pTriggerPolarityHigh->set_sensitive( true );

        m_pCheckbuttonDelay->set_sensitive( true );
        m_pHScaleDelay->set_sensitive( true );
        m_pSpinDelay->set_sensitive( true );

        for ( unsigned int i=0; i < sk_numPins + 1; i++ )
        {
            m_triggerSourceArray[i]->set_sensitive( true );
        }
    }

    void GPIOPage::DisableTrigger()
    {
        
        m_pCheckbuttonToggleTrigger->set_sensitive( false );
        m_pComboBoxMode->set_sensitive( false );
        m_pEntryParameter->set_sensitive( false );
        m_pBtnFireSoftwareTrigger->set_sensitive( false );

        m_pTriggerPolarityLow->set_sensitive( false );
        m_pTriggerPolarityHigh->set_sensitive( false );
        
        m_pCheckbuttonDelay->set_sensitive( false );
        m_pHScaleDelay->set_sensitive( false );
        m_pSpinDelay->set_sensitive( false );

        for ( unsigned int i=0; i < sk_numPins + 1; i++ )
        {
            m_triggerSourceArray[i]->set_sensitive( false );
        }
    }

    void GPIOPage::SetActiveModeComboBox( int mode )
    {
        if ( m_refListStoreTriggerModes->children().size() == 0 )
        {
            return;
        }

        // Find the selected mode in the list store and set it		
        Gtk::TreeModel::iterator iter;
        Gtk::TreeModel::Row row;
        bool found = false;

        for ( iter = m_refListStoreTriggerModes->children().begin(); 
            iter != m_refListStoreTriggerModes->children().end(); 
            iter++ )
        {
            row = *iter;
            if ( row[m_triggerModeColumns.m_colTriggerMode] == mode )
            {
                found = true;
                break;
            }
        }

        if ( found == true )
        {
            m_pComboBoxMode->set_active(iter);				
        }	
        else
        {
            m_pComboBoxMode->set_active(0);				
        }
    }

    void GPIOPage::EnableStrobeControls( unsigned int pin )
    {
        if ( pin < 0 || pin > sk_numPins  )
        {
            return;
        }

        m_widgetPinArray[pin].pOnOff->set_sensitive( true );
        m_widgetPinArray[pin].pHigh->set_sensitive( true );
        m_widgetPinArray[pin].pLow->set_sensitive( true );
        m_widgetPinArray[pin].pDelay->set_sensitive( true );
        m_widgetPinArray[pin].pDuration->set_sensitive( true );

        UpdateSingleStrobe( pin );
    }

    void GPIOPage::DisableStrobeControls( unsigned int pin )
    {
        if ( pin < 0 || pin > sk_numPins )
        {
            return;
        }

        m_widgetPinArray[pin].pOnOff->set_active( false );

        m_widgetPinArray[pin].pOnOff->set_sensitive( false );
        m_widgetPinArray[pin].pHigh->set_sensitive( false );
        m_widgetPinArray[pin].pLow->set_sensitive( false );
        m_widgetPinArray[pin].pDelay->set_sensitive( false );
        m_widgetPinArray[pin].pDuration->set_sensitive( false );
    }       

    void GPIOPage::DisableAllStrobeControls()
    {
        for ( unsigned int i=0; i < sk_numPins + 1; i++ )
        {
            DisableStrobeControls( i );
        }
    }

    bool GPIOPage::TestPinInputSuccess( int pin )
    {
        // Read the GPIO pin direction register
        Error error;
        const unsigned int k_pioDirection = 0x11f8;
        unsigned int pioDirectionVal = 0;
        error = m_pCamera->ReadRegister( k_pioDirection, &pioDirectionVal );
        if( error != PGRERROR_OK )
        {
            return false;
        }

        // Set the appropriate pin as an input
        pioDirectionVal &= ~( 0x80000000 >> pin );
        error = m_pCamera->WriteRegister( k_pioDirection, pioDirectionVal );
        if( error != PGRERROR_OK )
        {
            return false;
        }

        // Read the register back
        error = m_pCamera->ReadRegister( k_pioDirection, &pioDirectionVal );
        if( error != PGRERROR_OK )
        {
            return false;
        }

        // Check if the pin was successfully set as an input
        if ( (pioDirectionVal & (0x80000000 >> pin)) != 0 )
        {
            return false;
        }
        else
        {
            return true;
        }
    }            
}
