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
// $Id: CamInfoPage.h,v 1.20 2010/03/25 00:26:26 soowei Exp $
//=============================================================================

#ifndef _CAMINFOPAGE_H_
#define _CAMINFOPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{		
	class CamInfoPage : public BasePage 
	{
	public:
		CamInfoPage();
		CamInfoPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );
		~CamInfoPage();
			
		void UpdateWidgets();

	protected:
		void GetWidgets();
		void AttachSignals();		

	private:
        static const int sk_timeout = 1000;

		static const char* sk_labelSerial;
		static const char* sk_labelModel;
		static const char* sk_labelVendor;
		static const char* sk_labelSensor;
		static const char* sk_labelResolution;
		static const char* sk_labelInterface;
		static const char* sk_labelBusSpeed;
		static const char* sk_labelDCAMVer;
		static const char* sk_labelFirmwareVer;
		static const char* sk_labelFirmwareBuildTime;

        static const char* sk_vboxGigECameraInfo;
        static const char* sk_labelGigEVersion;
        static const char* sk_labelGigEUserDefinedName;
        static const char* sk_labelGigEXmlUrl1;
        static const char* sk_labelGigEXmlUrl2;
        static const char* sk_labelGigEMacAddress;
        static const char* sk_labelGigEIpAddress;
        static const char* sk_labelGigESubnetMask;
        static const char* sk_labelGigEDefaultGateway;
        static const char* sk_checkGigEIpLLA;
        static const char* sk_checkGigEIpDHCP;
        static const char* sk_checkGigEIpPersistentIp;

        static const char* sk_labelTimeSinceInitialization;
        static const char* sk_labelTimeSinceBusReset;
        static const char* sk_labelPixelClockFreq;
        static const char* sk_labelHorizontalLineFreq;
        static const char* sk_labelTransmitFailures;
        static const char* sk_btnResetTransmitFailures;        
        static const char* sk_btnCopyCameraInfo;

        static const char* sk_vboxConsoleLog;
        static const char* sk_textviewConsoleLog;
        static const char* sk_btnCopyConsoleLog;
        static const char* sk_btnRefreshConsoleLog;

        static const char* sk_notebook;
			
		Gtk::Label* m_pLabelSerial;
		Gtk::Label* m_pLabelModel;
		Gtk::Label* m_pLabelVendor;
		Gtk::Label* m_pLabelSensor;
		Gtk::Label* m_pLabelResolution;
		Gtk::Label* m_pLabelInterface;
		Gtk::Label* m_pLabelBusSpeed;
		Gtk::Label* m_pLabelDCAMVer;
		Gtk::Label* m_pLabelFirmwareVer;
		Gtk::Label* m_pLabelFirmwareBuildTime;

        Gtk::VBox* m_pVboxGigECameraInfo;
        Gtk::Label* m_pLabelGigEVersion;
        Gtk::Label* m_pLabelGigEUserDefinedName;
        Gtk::Label* m_pLabelGigEXmlUrl1;
        Gtk::Label* m_pLabelGigEXmlUrl2;
        Gtk::Label* m_pLabelGigEMacAddress;
        Gtk::Label* m_pLabelGigEIpAddress;
        Gtk::Label* m_pLabelGigESubnetMask;
        Gtk::Label* m_pLabelGigEDefaultGateway;
        Gtk::CheckButton* m_pCheckGigEIpLLA;
        Gtk::CheckButton* m_pCheckGigEIpDHCP;
        Gtk::CheckButton* m_pCheckGigEIpPersistentIp;

        Gtk::Label* m_pLabelTimeSinceInitialization;
        Gtk::Label* m_pLabelTimeSinceBusReset;
        Gtk::Label* m_pLabelPixelClockFreq;
        Gtk::Label* m_pLabelHorizontalLineFreq;
        Gtk::Label* m_pLabelTransmitFailures;
        Gtk::Button* m_pBtnResetTransmitFailures;                
        Gtk::Button* m_pBtnCopyCameraInfo;

        Gtk::VBox* m_vboxConsoleLog;
        Gtk::TextView* m_pTextViewConsoleLog;
        Gtk::Button* m_pBtnCopyConsoleLog;
        Gtk::Button* m_pBtnRefreshConsoleLog;

        Gtk::Notebook* m_pNotebook;

        Glib::RefPtr<Gtk::TextBuffer> m_refConsoleLogBuffer;

        virtual bool OnTimer();
			
		/**
		 * Set the specified camera information to the page.
		 *
		 * @param pCamInfo The CameraInfo to set.
		 */
		void SetCameraInformation( CameraInfo* pCamInfo );

        /** Set blank information to the page. */
        void SetBlankInformation();        

        /**
         * Read the initialize time from the camera and update it on the page.
         */
        void UpdateInitializeTime();

        /**
         * Read the bus reset time from the camera and update it on the page.
         */
        void UpdateBusResetTime();

        /**
         * Read the pixel clock frequency from the camera and update it
         * on the page.
         */
        void UpdatePixelClockFreq();

        /**
         * Read the horizontal line frequency from the camera and update it
         * on the page.
         */
        void UpdateHorizontalLineFreq();

        /** 
         * Read the transmit failure register from the camera and update it
         * on the page.
         */
        void UpdateTransmitFailures();
        
        /**
         * Get a string representation of the specified interface.
         *
         * @param type The type of interface to get the representation for.
         *
         * @return A string containing the representation of the interface.
         */
        static const char* GetInterfaceString( InterfaceType type );

        /**
         * Get a string representation of the bus speed.
         *
         * @param speed Bus speed.
         *
         * @return String representation of the interface type.
         */
        static const char* GetBusSpeedString( BusSpeed speed );

        /**
         * Parse the time register in hours, minutes and seconds.
         *
         * @param timeRegVal Value of the time register.
         * @param hours Parsed hours.
         * @param mins Parsed minutes.
         * @param seconds Parsed seconds.
         */
        static void ParseTimeRegister( 
            unsigned int timeRegVal, 
            unsigned int& hours, 
            unsigned int& mins, 
            unsigned int& seconds );

        /** Reset the transmit failure register. */
        void OnResetTransmitFailures();
              
        /** Copy the camera information to the clipboard. */
        void OnCopyCameraInfo();

        /** Copy the console log to the clipboard. */
        void OnCopyConsoleLog();

        /** Refresh the console log. */
        void OnRefreshConsoleLog();

        /** Handler for key presses. */
        bool OnKeyPressEvent( GdkEventKey* event );
    };
}

#endif // _CAMINFOPAGE_H_
