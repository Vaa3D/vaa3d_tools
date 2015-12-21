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
// $Id: AdvCameraSettingsPage.h,v 1.16 2010/06/25 17:47:17 soowei Exp $
//=============================================================================

#ifndef _ADVCAMERASETTINGSPAGE_H_
#define _ADVCAMERASETTINGSPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
    class MemChannelsColumns : public Gtk::TreeModelColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> m_colMemChannelStr;
        Gtk::TreeModelColumn<unsigned int> m_colMemChannel;

        MemChannelsColumns()
        { 
            add( m_colMemChannelStr ); 
            add( m_colMemChannel ); 
        }							 
    };

    class AutoRangeControlColumns : public Gtk::TreeModelColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> m_colAutoRangeControlStr;
        Gtk::TreeModelColumn<PropertyType> m_colProperty;
        Gtk::TreeModelColumn<unsigned int> m_colPropertyRegister;

        AutoRangeControlColumns()
        { 
            add( m_colAutoRangeControlStr ); 
            add( m_colProperty ); 
            add( m_colPropertyRegister ); 
        }							 
    };

	class AdvCameraSettingsPage : public BasePage 
	{
	public:		
		AdvCameraSettingsPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );
		~AdvCameraSettingsPage();
			
		void UpdateWidgets();    

        /**
         * Get a signal that can be used to receive events when Restore
         * from memory channel is performed.
         *
         * @return Signal that can be connected to in order to receive
         *         restore events.
         */
        sigc::signal<void> signal_restore_from_memory_channel();
        
	protected:
		void GetWidgets();
		void AttachSignals();		

        sigc::signal<void> m_signal_restore_from_memory_channel;

	private:
        enum EmbeddedInfoType
        {
            EMBEDDED_TIMESTAMP,
            EMBEDDED_GAIN,
            EMBEDDED_SHUTTER,
            EMBEDDED_BRIGHTNESS,
            EMBEDDED_EXPOSURE,
            EMBEDDED_WHITE_BALANCE,
            EMBEDDED_FRAME_COUNTER,
            EMBEDDED_STROBE_PATTERN,
            EMBEDDED_GPIO_PIN_STATE,
            EMBEDDED_ROI_POSITION,
            EMBEDDED_NUM_INFORMATION, // Number of information entries
        };

        static const unsigned int sk_imageDataFmtReg = 0x1048;
        static const unsigned int sk_testPatternReg = 0x104C;
        static const unsigned int sk_frameInfoReg = 0x12F8;

		static const unsigned int sk_mirrorImageCtrlReg = 0x1054;  // IIDC v1.32 
		static const unsigned int sk_bayerMonoCtrlReg = 0x1050;    // IIDC v1.32 
		static const unsigned int sk_dataDepthCtrlReg = 0x0630;    // IIDC v1.32 

        static const int sk_timeout = 250;

        static const char* sk_checkbuttonRawBayerOutput;
        static const char* sk_checkbuttonMirrorImage;
        static const char* sk_radiobuttonY16BigEndian;
        static const char* sk_radiobuttonY16LittleEndian;
        static const char* sk_radiobuttonTestPattern1;
        static const char* sk_radiobuttonTestPattern2;
        static const char* sk_radiobuttonTestPatternNone;

        static const char* sk_comboboxMemChannels;
        static const char* sk_btnSaveToMemChannel;
        static const char* sk_btnRestoreFromMemChannel;

        static const char* sk_checkbuttonEmbeddedTimestamp;
        static const char* sk_checkbuttonEmbeddedGain;
        static const char* sk_checkbuttonEmbeddedShutter;
        static const char* sk_checkbuttonEmbeddedBrightness;
        static const char* sk_checkbuttonEmbeddedExposure;
        static const char* sk_checkbuttonEmbeddedWhiteBalance;
        static const char* sk_checkbuttonEmbeddedFrameCounter;
        static const char* sk_checkbuttonEmbeddedStrobe;
        static const char* sk_checkbuttonEmbeddedGPIO;
        static const char* sk_checkbuttonEmbeddedROIPosition;
        static const char* sk_btnEmbeddedSelectAll;
        static const char* sk_btnEmbeddedUnselectAll;

        static const char* sk_comboBoxAutoRangeControl;
        static const char* sk_entryAutoRangeMin;
        static const char* sk_entryAutoRangeMax;
        static const char* sk_btnGetAutoRange;
        static const char* sk_btnSetAutoRange;

        Gtk::CheckButton* m_pCheckbuttonRawBayerOutput;
        Gtk::CheckButton* m_pCheckbuttonMirrorImage;
        Gtk::RadioButton* m_pRadiobuttonY16BigEndian;
        Gtk::RadioButton* m_pRadiobuttonY16LittleEndian;
        Gtk::RadioButton* m_pRadiobuttonY16TestPattern1;
        Gtk::RadioButton* m_pRadiobuttonY16TestPattern2;
        Gtk::RadioButton* m_pRadiobuttonY16TestPatternNone;

        // Memory channels
        Gtk::ComboBox* m_pComboboxMemChannels;
        Gtk::Button* m_pBtnSaveToMemChannel;
        Gtk::Button* m_pBtnRestoreFromMemChannel;

        // Embedded image info
        Gtk::CheckButton* m_arCheckButton[EMBEDDED_NUM_INFORMATION];
        Gtk::Button* m_pBtnEmbeddedSelectAll;
        Gtk::Button* m_pBtnEmbeddedUnselectAll;

        // Auto range control
        Gtk::ComboBox* m_pComboBoxAutoRangeControl;
        Gtk::Entry* m_pEntryAutoRangeMin;
        Gtk::Entry* m_pEntryAutoRangeMax;
        Gtk::Button* m_pBtnGetAutoRange;
        Gtk::Button* m_pBtnSetAutoRange;

        // Memory channel model
        MemChannelsColumns m_memChannelColumns;
        Glib::RefPtr<Gtk::ListStore> m_refListStoreMemChannels;

        // Auto range control mode
        AutoRangeControlColumns m_autoRangeControlColumns;
        Glib::RefPtr<Gtk::ListStore> m_refListStoreAutoRangeControl;

        AdvCameraSettingsPage();
        AdvCameraSettingsPage( const AdvCameraSettingsPage& );
        AdvCameraSettingsPage& operator=( const AdvCameraSettingsPage& );

        void UpdateMemChannelComboBox();
        void SetActiveMemChannelComboBox( unsigned int channel );
        unsigned int GetActiveMemChannelComboBox();

        void OnRawBayerOutputToggled();
        void OnMirrorImageToggled();
        void OnY16EndiannessToggled( bool bigEndian );
        void OnTestPatternChanged( int testPattern );
        void OnSaveToMemoryChannel();
        void OnRestoreFromMemoryChannel();

        void OnEmbeddedSelectAll();
        void OnEmbeddedUnselectAll();    

        void OnEmbeddedCheck( EmbeddedInfoType type );

        void EnableEmbedded();
        void DisableEmbedded();

        void OnAutoRangeControlComboBoxChanged();
        void OnGetAutoRangeControl();
        void OnSetAutoRangeControl();

        void UpdateMemoryChannels();        
        void UpdateImageDataFormat();
        void UpdateTestPattern();
        void UpdateBayerOutput();

        virtual bool OnTimer();
	};
}

#endif // _ADVCAMERASETTINGSPAGE_H_
