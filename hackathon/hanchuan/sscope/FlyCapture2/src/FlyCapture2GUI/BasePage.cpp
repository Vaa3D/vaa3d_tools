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
// $Id: BasePage.cpp,v 1.27 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "BasePage.h"

namespace FlyCapture2
{
	BasePage::BasePage() :
		m_refXml(0)
	{
		m_pCamera = 0;
		m_connected = false;
        m_pWindow = 0;

        InitColors();
	}
		
	BasePage::BasePage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) :
		m_connected(false)
	{		
		SetCamera(pCamera);
		m_refXml = refXml;
		m_connected = false;
        m_pWindow = 0;

        m_refXml->get_widget( "windowCamCtl", m_pWindow );

        InitColors();  
	}
		
	BasePage::~BasePage()
	{
		if (!m_timerSignalConnection.empty()
			&& m_timerSignalConnection.connected())
		{
			m_timerSignalConnection.disconnect();
		}
	}
		
	void BasePage::SetCamera( CameraBase* pCam )
	{
		if ( m_pCamera != NULL )
		{
			//printf( "WARNING: Assigning m_pCamera that is not NULL!\n" );
		}
		
		m_pCamera = pCam;
	}

    void BasePage::SetCameraInfo( CameraInfo camInfo )
    {
        m_camInfo = camInfo;
    }
		
	void BasePage::SetConnected( bool connected )
	{
		m_connected = connected;
	}
		
	void BasePage::SetRefXml( Glib::RefPtr<Gnome::Glade::Xml> refXml )
	{
		if ( m_refXml != NULL )
		{
			//printf( "WARNING: Assigning m_pWidget that is not NULL!\n" );
		}
		
		m_refXml->get_widget( "windowCamCtl", m_pWindow );
	}
		
	void BasePage::Initialize()
	{
		GetWidgets();
		AttachSignals();
	}	

    Gdk::Color BasePage::GetColorAccentRed() const
    {
        return m_color_accent_red;
    }

    Gdk::Color BasePage::GetColorAccentGreen() const
    {
        return m_color_accent_green;
    }

    bool BasePage::IsIconified() const
    {
        if ( m_pWindow == NULL )
        {
            return true;
        }

        if ( m_pWindow->is_realized() != true )
        {
            return true;
        }

        Glib::RefPtr<Gdk::Window> refGdkWindow = m_pWindow->get_window();
        Gdk::WindowState windowState = refGdkWindow->get_state();
        if ( windowState & Gdk::WINDOW_STATE_ICONIFIED )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool BasePage::IsConnected() const
    {
        return m_connected;
    }
		
	int BasePage::ShowMessageDialog( 
        Glib::ustring mainTxt, 
        Glib::ustring subTxt, 
        Gtk::MessageType messageType, 
        Gtk::ButtonsType buttonsType )
	{
		Gtk::MessageDialog dialog( mainTxt, false, messageType, buttonsType );
  		dialog.set_secondary_text( subTxt );

  		return dialog.run();	
	}

    int BasePage::ShowErrorMessageDialog( Glib::ustring mainTxt, Glib::ustring secondaryTxt )
    {
        /*
        Gtk::MessageDialog dialog( mainTxt, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK );       
        dialog.set_secondary_text( secondaryTxt );

        return dialog.run();	
        */

        return 0;
    }

    int BasePage::ShowErrorMessageDialog( Glib::ustring mainTxt, Error error )
    {
        /*
        ShowErrorMessageDialog( mainTxt, error.GetDescription() );
        */

        return 0;
    }

    double BasePage::Convert32bitIEEEToFloat( unsigned int value )
    {
        const double k_DBL_MAX = 99999.0;

        int sign, exp;
        long frac;
        double result;

        // grab the sign, exponent, and mantissa/fraction
        sign = (value & 0x80000000UL) >> 31;
        exp  = (value & 0x7f800000UL) >> 23;
        frac = (value & 0x007FFFFFUL);

        if ( exp == 255 )
        {
            // frac != 0 : invalid number (return max)
            // frac == 0 : negative or positive infinity (return -/+ max)
            result = k_DBL_MAX;

            if ( frac == 0 && sign == 1 ) 
            {
                result = -result;
            }
        } 
        else if ( exp == 0 ) 
        {
            if ( frac == 0 ) 
            {
                // exp = 0 and frac = 0 -> result = 0
                result = 0;
            } 
            else 
            {
                // denormal number
                if ( sign == 1 ) 
                {
                    frac = -frac;
                }

                result = ldexp( static_cast<double>(frac), -126 - 23 );
            }
        } 
        else if ( exp > 0 && exp < 255 ) 
        {
            // proper number.  Convert.
            frac += 0x00800000UL;
            if (sign == 1) 
            {
                frac = -frac;
            }

            result = ldexp( static_cast<double>(frac), exp - 127 - 23 );
        } 
        else 
        {
            //should never happen, but for completion
            result = k_DBL_MAX;  
        } 

        return result;
    }

    unsigned int BasePage::ConvertFloatTo32bitIEEE( float value )
    {
        int sign, exp;
        double frac;
        unsigned long result;

        if ( value == 0 ) 
        {
            result = 0;  
        } 
        else 
        {
            // if negative, set the sign bit and change the value to +'ve.
            // This is so frexp returns the same result as when positive.
            if ( value < 0 ) 
            {
                sign = 1;
                value = -value;
            } 
            else 
            {
                sign = 0;
            }

            // Note, frac is supposed to be mantissa x 2^23, but that 
            // assumes mantissa is normalized between 1 and 2 (leading 1).
            // However, frexp returns mantissa between 0.5 and 1 by definition
            // therefore, we multiply result by 2 (use 2^24) and reduce the
            // exponent by 1.
            // For example, frexp(15.2, &exp) return 0.95 with exponent of 3.
            // To convert to IEEE Floating Point representation, we need
            // 1.90 x 2^2 instead of 0.95 x 2^3, although they are both the same.
            frac = ldexp( frexp( value, &exp ), 24 );
            exp -= 1;

            // remove the leading 1 from the mantissa.
            frac -= 0x00800000;

            // Create the IEEE Floating Point respresentation
            result = ((sign << 31) | ((exp + 127) << 23) | static_cast<unsigned int>(frac));
        }

        return (result);
    }

    const char* BasePage::GetRegisterString( unsigned int registerVal )
    {
        return Camera::GetRegisterString( registerVal );
    }    

    void BasePage::InitColors()
    {
        m_color_accent_red.set_rgb( 223*257, 66*257, 30*257 );
        m_color_accent_green.set_rgb( 70*257, 160*257, 70*257 );    
    }    

	 void BasePage::SetTimerConnection(sigc::connection timerSignalConnection)
	 {
		 m_timerSignalConnection = timerSignalConnection;
	 }

     bool BasePage::IsIIDC()
     {       
        return (dynamic_cast<Camera*>(m_pCamera) != NULL);
     }

     bool BasePage::IsGEV()
     {
        return (dynamic_cast<GigECamera*>(m_pCamera) != NULL);
     }
}
