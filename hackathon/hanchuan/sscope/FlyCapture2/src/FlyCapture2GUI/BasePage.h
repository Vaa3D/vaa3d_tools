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
// $Id: BasePage.h,v 1.27 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#ifndef _BASEPAGE_H_
#define _BASEPAGE_H_

namespace FlyCapture2
{
	class Camera;
    class Error;

    /**
     * This class is the base class for a page in the camera control dialog.
     */ 
	class BasePage
	{
	public:	   
        /** Default constructor. */
        BasePage();

        /**
         * Overloaded constructor. Initialize the object with the specified
         * parameters.
         *
         * @param pCamera Pointer to a valid Camera object.
         * @param refXml RefPtr to a Glade XML object.
         */ 
		BasePage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml  );

        /** Virtual destructor. */
		virtual ~BasePage() = 0;	
			
        /** Update widgets on page. */ 
		virtual void UpdateWidgets() = 0;
			
        /**
         * Set the camera to be used by the pages.
         *
         * * @param pCam Pointer to a valid Camera object.
         */ 
		void SetCamera( CameraBase* pCam );

        /**
         * Set the camera information to be used by the pages.
         *
         * * @param camInfo Camera information to be used
         */ 
		void SetCameraInfo( CameraInfo camInfo );

        /**
         * Set the connected state of the pages.
         *
         * @param connected Whether a camera should be considered as connected.
         */ 
		void SetConnected( bool connected );

        /**
         * Set the Glade XML object to use.
         *
         * @param refXml RefPtr to a Glade XML object.
         */ 
		void SetRefXml( Glib::RefPtr<Gnome::Glade::Xml> refXml );
			
        /**
         * Load widgets and attach signals.
         */ 
		void Initialize();

	protected:
        /** Pointer to a camera object. */
        CameraBase* m_pCamera;			

        /** Cached copy of camera information. */
        CameraInfo m_camInfo;

        /** RefPtr to a Glade XML object. */
		Glib::RefPtr<Gnome::Glade::Xml> m_refXml; 

		/** Sets the global timer signal, so that it can cleaned up */
		void SetTimerConnection(sigc::connection timerSignalConnection);
		
        /** Load widgets. */ 
		virtual void GetWidgets() = 0;

        /** Attach signals to widgets. */ 
		virtual void AttachSignals() = 0;		

        /** Checks if the CameraBase object is actually an IIDC camera. */
        bool IsIIDC();

        /** Checks if the CameraBase object is actually a GEV camera. */
        bool IsGEV();

        /**
         * Get the red color object.
         *
         * @return The red color object.
         */ 
        Gdk::Color GetColorAccentRed() const;

        /**
         * Get the green color object.
         *
         * @return The green color object.
         */ 
        Gdk::Color GetColorAccentGreen() const;        

        /**
         * Get the iconfication state of the dialog.
         *
         * @return Whether the dialog is iconified.
         */ 
        bool IsIconified() const;

        /**
         * Get the connection state of the dialog.
         *
         * @return Whether a camera is connected to the dialog.
         */ 
        bool IsConnected() const;
			
        /**
         * Display a message dialog using the specified arguments.
         *
         * @param mainTxt The main text to display.
         * @param subTxt The secondary text to display.
         * @param messageType The type of message dialog to display.
         * @param buttonsType The type of buttons to display.
         */ 
        static int ShowMessageDialog( 
            Glib::ustring mainTxt, 
            Glib::ustring subTxt, 
            Gtk::MessageType messageType = Gtk::MESSAGE_INFO, 
            Gtk::ButtonsType buttonsType = Gtk::BUTTONS_OK );

        /**
         * Display an error message dialog using the specified arguments.
         *
         * @param mainTxt The main text to display.
         * @param subTxt The secondary text to display.
         */ 
        static int ShowErrorMessageDialog( 
            Glib::ustring mainTxt, 
            Glib::ustring secondaryTxt );

        /**
         * Display an error message dialog using the specified arguments.
         *
         * @param mainTxt The main text to display.  
         * @param error A FlyCapture2 Error object.
         */ 
        static int ShowErrorMessageDialog( 
            Glib::ustring mainTxt, 
            Error error );

        /**
         * Convert a 32-bit unsigned int to a IEEE754 floating point value.
         *
         * @param value The unsigned int to be converted, e.g. from 
         *              Camera::ReadRegister().
         *
         * @return The IEEE754 floating point value.
         *
         * @see ConvertFloatTo32bitIEEE()
         */ 
        static double Convert32bitIEEEToFloat( unsigned int value );

        /**
         * Convert a IEEE754 floating point value to a 32-bit unsigned int.
         *
         * @param value The floating point value to be converted.
         *
         * @return The unsigned int value.
         *
         * @see Convert32bitIEEEToFloat()
         */ 
        static unsigned int ConvertFloatTo32bitIEEE( float value );

        /**
         * Get a string containing the text representation of the register.
         * 
         * @param registerVal The register to obtain the text for.
         *
         * @return A string containing the text representation.
         */ 
        static const char* GetRegisterString( unsigned int registerVal );

		static const unsigned int sk_IIDC132Version = 132;

	private:
        /** Whether a camera is connected. */
        bool m_connected;

        /** Pointer to the main dialog. This is needed for IsIconified(). */
        Gtk::Window* m_pWindow;

        /** Red color to signify errors and such. */
        Gdk::Color m_color_accent_red;

        /** Green color to signify acceptable values. */
        Gdk::Color m_color_accent_green;

		/** Timer Signal Connection to global timer */
		sigc::connection m_timerSignalConnection;
                
        /** Copy constructor. */
        BasePage( const BasePage& );

        /** Assignment operator. */
        BasePage& operator=( const BasePage& );

        /** Initialize colors to proper colors. */
        void InitColors();
	};
}

#endif // _BASEPAGE_H_
