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
// $Id: GPIOPage.h,v 1.22 2010/02/26 17:33:13 soowei Exp $
//=============================================================================

#ifndef _GPIOPAGE_H_
#define _GPIOPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
    class TriggerModesColumns : public Gtk::TreeModelColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> m_colTriggerModeStr;
        Gtk::TreeModelColumn<int> m_colTriggerMode;	  	

        TriggerModesColumns()
        { 
            add( m_colTriggerModeStr ); 
            add( m_colTriggerMode ); 
        }							 
    };

    class GPIOPage : public BasePage 
    {
    public:
        /** 
        * Constructor. Initialize the object with the specified parameters. 
        *
        * @param pCamera Pointer to a valid Camera object.
        * @param refXml RefPtr to a Glade XML object.
        */
        GPIOPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );

        /** Destructor. */
        ~GPIOPage();

        /** Update widgets on page. */
        void UpdateWidgets();

    protected:
        /** Load widgets. */
        void GetWidgets();

        /** Attach signals to widgets. */
        void AttachSignals();

    private:
        // Internal struct to store the widgets associated with a single pin
        struct WidgetStrobe
        {
            Gtk::CheckButton* pOnOff;
            Gtk::RadioButton* pHigh;
            Gtk::RadioButton* pLow;
            Gtk::SpinButton* pDelay;
            Gtk::Adjustment* pDelayAdjustment;
            Gtk::SpinButton* pDuration;
            Gtk::Adjustment* pDurationAdjustment;			
        };

        struct WidgetPinDirection
        {
            Gtk::RadioButton* pIn;
            Gtk::RadioButton* pOut;
        };

        // Strobe
        static const unsigned int sk_numPins = 4;
        WidgetStrobe m_widgetPinArray[sk_numPins];

        // Misc
        Gtk::CheckButton* m_pCheckbuttonToggleTrigger;
        Gtk::ComboBox* m_pComboBoxMode;
        Gtk::Entry* m_pEntryParameter;
        Gtk::Button* m_pBtnFireSoftwareTrigger;

        // Trigger polarity
        Gtk::RadioButton* m_pTriggerPolarityLow;
        Gtk::RadioButton* m_pTriggerPolarityHigh;

        // Trigger delay
        Gtk::CheckButton* m_pCheckbuttonDelay;
        Gtk::HScale* m_pHScaleDelay;
        Gtk::Adjustment* m_pAdjustmentDelay;
        Gtk::SpinButton* m_pSpinDelay;
        Gtk::Label* m_pLabelMinTriggerDelay;
        Gtk::Label* m_pLabelMaxTriggerDelay;

        // Trigger source
        Gtk::RadioButton* m_triggerSourceArray[sk_numPins + 1];

        // GPIO pin direction
        WidgetPinDirection m_widgetPinDirection[sk_numPins];

        TriggerModesColumns m_triggerModeColumns;

        Glib::RefPtr<Gtk::ListStore> m_refListStoreTriggerModes;

        // Timeout
        static const unsigned int sk_timeout = 500;

        // Misc
        static const char* sk_checkbuttonToggleTrigger;
        static const char* sk_comboBoxTriggerMode;
        static const char* sk_entryParameter;        
        static const char* sk_btnFireSoftwareTrigger;

        // Trigger polarity
        static const char* sk_radioTriggerPolarityLow;
        static const char* sk_radioTriggerPolarityHigh;

        // Trigger delay
        static const char* sk_checkbuttonDelay;
        static const char* sk_hscaleDelay;
        static const char* sk_spinDelay;
        static const char* sk_labelMinTriggerDelay;
        static const char* sk_labelMaxTriggerDelay;

        // Trigger source
        static const char* sk_radioSourceGPIO0;		
        static const char* sk_radioSourceGPIO1;		
        static const char* sk_radioSourceGPIO2;	
        static const char* sk_radioSourceGPIO3;	
        static const char* sk_radioSourceNone;	        

        GPIOPage();
        GPIOPage( const GPIOPage& );
        GPIOPage& operator=( const GPIOPage& );

        void UpdateTrigger();
        void UpdateStrobe();
        void UpdateSingleStrobe( unsigned int pin );
        void UpdatePinDirection();

        void UpdateModeComboBox( TriggerModeInfo* pInfo );
        void UpdateActiveTriggerSource( TriggerMode* pTrigger );
        void UpdateTriggerParameter( TriggerModeInfo* pInfo, TriggerMode* pTrigger );
        void UpdateTriggerPolarity( TriggerMode* pTrigger );
        void UpdateTriggerDelay();

        void EnableTrigger();
        void DisableTrigger();

        void SetActiveModeComboBox( int mode );

        void EnableStrobeControls( unsigned int pin );
        void DisableStrobeControls( unsigned int pin );
        void DisableAllStrobeControls();

        bool TestPinInputSuccess( int pin );

        // Trigger handlers
        void OnToggleTriggerClicked();
        void OnFireSoftwareTriggerClicked();
        void OnDelayChecked();
        void OnDelayScroll();
        void OnTriggerModeChanged();
        void OnTriggerPolarityChanged();
        bool OnEntryParameterLostFocus( GdkEventFocus* event );
        void OnRadioSourceChanged( unsigned int iPin, Gtk::RadioButton* pButton );       

        // Strobe handlers
        void OnCheckOnOffChanged( int pin, Gtk::CheckButton* pButton );
        void OnRadioPolarityChanged( int pin, Gtk::RadioButton* pButton );
        void OnSpinDelayScroll( int pin );
        void OnSpinDurationScroll( int pin );

        // Pin direction handler
        void OnRadioPinDirectionChanged( int pin, Gtk::RadioButton* pButton );

        // Timer
        bool OnTimer();
    };
}

#endif // _GPIOPAGE_H_
