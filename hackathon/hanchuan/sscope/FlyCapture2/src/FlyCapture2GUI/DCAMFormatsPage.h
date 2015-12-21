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
// $Id: DCAMFormatsPage.h,v 1.12 2009/04/24 16:19:24 soowei Exp $
//=============================================================================

#ifndef _DCAMFORMATSPAGE_H_
#define _DCAMFORMATSPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
	class DCAMFormatsPage: public BasePage 
	{
	public:
        /** 
        * Constructor. Initialize the object with the specified parameters. 
        *
        * @param pCamera Pointer to a valid Camera object.
        * @param refXml RefPtr to a Glade XML object.
        */
		DCAMFormatsPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );

        /** Destructor. */
		~DCAMFormatsPage();
			
        /** Update widgets on page. */
        void UpdateWidgets();
			
		void OnVideoModeChanged( Gtk::RadioButton* pButton );
		void OnFrameRateChanged( Gtk::RadioButton* pButton );

        void OnFormat7Applied();

        sigc::signal<void> signal_dcam_format_changed();

	protected:
        /** Load widgets. */
        void GetWidgets();

        /** Attach signals to widgets. */
        void AttachSignals();

        sigc::signal<void> m_signal_dcam_format_changed;

	private:		
        static const char* sk_radioFormat7;

		static const char* sk_radio160x120YUV444;
			
		static const char* sk_radio320x240YUV422;
		static const char* sk_radio640x480YUV422;
		static const char* sk_radio800x600YUV422;
		static const char* sk_radio1024x768YUV422;
        static const char* sk_radio1280x960YUV422;
		static const char* sk_radio1600x1200YUV422;	
			
		static const char* sk_radio640x480YUV411;
			
		static const char* sk_radio640x480RGB;
		static const char* sk_radio800x600RGB;
		static const char* sk_radio1024x768RGB;
		static const char* sk_radio1280x960RGB;
		static const char* sk_radio1600x1200RGB;
			
		static const char* sk_radio640x480Y8;
		static const char* sk_radio800x600Y8;
		static const char* sk_radio1024x768Y8;
		static const char* sk_radio1280x960Y8;
		static const char* sk_radio1600x1200Y8;	
			
		static const char* sk_radio640x480Y16;
		static const char* sk_radio800x600Y16;
		static const char* sk_radio1024x768Y16;
		static const char* sk_radio1280x960Y16;
		static const char* sk_radio1600x1200Y16;
			
		static const char* sk_radioFrmRate_1_875;
		static const char* sk_radioFrmRate_3_75;
		static const char* sk_radioFrmRate_7_5;
		static const char* sk_radioFrmRate_15;
		static const char* sk_radioFrmRate_30;
		static const char* sk_radioFrmRate_60;
		static const char* sk_radioFrmRate_120;
		static const char* sk_radioFrmRate_240;

        Gtk::RadioButton* m_pRadioFormat7;
			
		Gtk::RadioButton* m_pRadio160x120YUV444;
			
		Gtk::RadioButton* m_pRadio320x240YUV422;
		Gtk::RadioButton* m_pRadio640x480YUV422;
		Gtk::RadioButton* m_pRadio800x600YUV422;
		Gtk::RadioButton* m_pRadio1024x768YUV422;
        Gtk::RadioButton* m_pRadio1280x960YUV422;
		Gtk::RadioButton* m_pRadio1600x1200YUV422;	
			
		Gtk::RadioButton* m_pRadio640x480YUV411;
			
		Gtk::RadioButton* m_pRadio640x480RGB;
		Gtk::RadioButton* m_pRadio800x600RGB;
		Gtk::RadioButton* m_pRadio1024x768RGB;
		Gtk::RadioButton* m_pRadio1280x960RGB;
		Gtk::RadioButton* m_pRadio1600x1200RGB;
			
		Gtk::RadioButton* m_pRadio640x480Y8;
		Gtk::RadioButton* m_pRadio800x600Y8;
		Gtk::RadioButton* m_pRadio1024x768Y8;
		Gtk::RadioButton* m_pRadio1280x960Y8;
		Gtk::RadioButton* m_pRadio1600x1200Y8;	
			
		Gtk::RadioButton* m_pRadio640x480Y16;
		Gtk::RadioButton* m_pRadio800x600Y16;
		Gtk::RadioButton* m_pRadio1024x768Y16;
		Gtk::RadioButton* m_pRadio1280x960Y16;
		Gtk::RadioButton* m_pRadio1600x1200Y16;
			
		Gtk::RadioButton* m_pRadioFrmRate_1_875;
		Gtk::RadioButton* m_pRadioFrmRate_3_75;
		Gtk::RadioButton* m_pRadioFrmRate_7_5;
		Gtk::RadioButton* m_pRadioFrmRate_15;
		Gtk::RadioButton* m_pRadioFrmRate_30;
		Gtk::RadioButton* m_pRadioFrmRate_60;
		Gtk::RadioButton* m_pRadioFrmRate_120;
		Gtk::RadioButton* m_pRadioFrmRate_240;
			
		std::map<Gtk::RadioButton*, VideoMode> m_mapVideoMode;
		std::map<Gtk::RadioButton*, FrameRate> m_mapFrameRate;

        DCAMFormatsPage();
        DCAMFormatsPage( const DCAMFormatsPage& );
        DCAMFormatsPage& operator=( const DCAMFormatsPage& );
		
		void AttachVideoModeRadioButton( Gtk::RadioButton* pButton);
		void AttachFrameRateRadioButton( Gtk::RadioButton* pButton );
			
		VideoMode GetVideoModeFromButton( Gtk::RadioButton* pButton );
		FrameRate GetFrameRateFromButton( Gtk::RadioButton* pButton );
			
		void UpdateFrameRatesForVideoMode( 
            VideoMode currVideoMode, 
            FrameRate currFrameRate = NUM_FRAMERATES );
			
		FrameRate GetFastestFrameRateForVideoMode( VideoMode videoMode );

        void EnableWidgets();
        void DisableWidgets();

        void EnableFrameRateButtons();
        void DisableFrameRateButtons();        
	};
}

#endif // _DCAMFORMATSPAGE_H_
