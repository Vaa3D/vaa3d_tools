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
// $Id: HDRPage.cpp,v 1.14 2010/05/14 21:38:45 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "HDRPage.h"
#include "string.h"

namespace FlyCapture2
{
    const char* HDRPage::sk_checkbuttonEnableHDR = "checkbuttonEnableHDR";
    const char* HDRPage::sk_hScaleHDR1Gain = "hscaleHDR1Gain";
    const char* HDRPage::sk_hScaleHDR1Shutter = "hscaleHDR1Shutter";
    const char* HDRPage::sk_hScaleHDR2Gain = "hscaleHDR2Gain";
    const char* HDRPage::sk_hScaleHDR2Shutter = "hscaleHDR2Shutter";
    const char* HDRPage::sk_hScaleHDR3Gain = "hscaleHDR3Gain";
    const char* HDRPage::sk_hScaleHDR3Shutter = "hscaleHDR3Shutter";
    const char* HDRPage::sk_hScaleHDR4Gain = "hscaleHDR4Gain";
    const char* HDRPage::sk_hScaleHDR4Shutter = "hscaleHDR4Shutter";

    HDRPage::HDRPage()
    {
    }

    HDRPage::HDRPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
    {
    }

    HDRPage::~HDRPage()
    {
        for ( int i=0; i < 4; i++ )
        {
            delete m_hdrImageArray[i].pAdjustmentGain;
            delete m_hdrImageArray[i].pAdjustmentShutter;
        }
    }

    void HDRPage::UpdateWidgets()
    {    
        if ( m_pCamera == NULL || IsConnected() != true )
        {
            DisableHDRWidgets();
            return;	
        }

        m_hdrRegOffset = GetHDRRegOffset();

        if ( IsHDRSupported() == false )
        {
            m_pCheckbuttonEnableHDR->set_sensitive( false );
            DisableHDRWidgets();

            return;
        }

        // HDR is available
        m_pCheckbuttonEnableHDR->set_sensitive( true );

        UpdateHDRWidgets();        

        if ( IsHDREnabled() == false )
        {
            m_pCheckbuttonEnableHDR->set_active( false );
            DisableHDRWidgets();

            return;
        }
        
        m_pCheckbuttonEnableHDR->set_active( true );
        EnableHDRWidgets();                    
    }

    void HDRPage::GetWidgets()
    {
        m_refXml->get_widget( sk_checkbuttonEnableHDR, m_pCheckbuttonEnableHDR );

        m_refXml->get_widget( sk_hScaleHDR1Gain, m_hdrImageArray[0].pHscaleGain );
        m_refXml->get_widget( sk_hScaleHDR1Shutter, m_hdrImageArray[0].pHscaleShutter );
        m_refXml->get_widget( sk_hScaleHDR2Gain, m_hdrImageArray[1].pHscaleGain );
        m_refXml->get_widget( sk_hScaleHDR2Shutter, m_hdrImageArray[1].pHscaleShutter );
        m_refXml->get_widget( sk_hScaleHDR3Gain, m_hdrImageArray[2].pHscaleGain );
        m_refXml->get_widget( sk_hScaleHDR3Shutter, m_hdrImageArray[2].pHscaleShutter );
        m_refXml->get_widget( sk_hScaleHDR4Gain, m_hdrImageArray[3].pHscaleGain );
        m_refXml->get_widget( sk_hScaleHDR4Shutter, m_hdrImageArray[3].pHscaleShutter );

        for ( int i=0; i < 4; i++ )
        {
            m_hdrImageArray[i].pAdjustmentGain = new Gtk::Adjustment(0, 0, 100);
            m_hdrImageArray[i].pAdjustmentShutter = new Gtk::Adjustment(0, 0, 100);

            m_hdrImageArray[i].pHscaleGain->set_adjustment( *m_hdrImageArray[i].pAdjustmentGain );
            m_hdrImageArray[i].pHscaleShutter->set_adjustment( *m_hdrImageArray[i].pAdjustmentShutter );
        }
    }

    void HDRPage::AttachSignals()
    {
        m_pCheckbuttonEnableHDR->signal_toggled().connect( sigc::mem_fun(*this, &HDRPage::OnEnableHDRToggled));

        for ( int i=0; i < 4; i++ )
        {
            m_hdrImageArray[i].pAdjustmentGain->signal_value_changed().connect(
                sigc::bind<int>(
                    sigc::mem_fun(*this, &HDRPage::OnGainScroll), i ) );

            m_hdrImageArray[i].pAdjustmentShutter->signal_value_changed().connect(
                sigc::bind<int>(
                    sigc::mem_fun(*this, &HDRPage::OnShutterScroll), i ) );
        }
    }

    void HDRPage::OnEnableHDRToggled()
    {
        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( m_hdrRegOffset, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading HDR offset register", error );
        }

        if ( m_pCheckbuttonEnableHDR->get_active() == true )
        {
            value |= (0x1 << 25);
            UpdateHDRWidgets();
            EnableHDRWidgets();
        }
        else
        {
            value &= ~(0x1 << 25);
            DisableHDRWidgets();
        }

        error = m_pCamera->WriteRegister( m_hdrRegOffset, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing HDR offset register", error );
        }
    }

    void HDRPage::OnGainScroll( int image )
    {
        Gtk::Adjustment* pAdjustment = m_hdrImageArray[image].pAdjustmentGain;

        double value = pAdjustment->get_value();

        SetGain( image, static_cast<unsigned int>(value) );
    }

    void HDRPage::OnShutterScroll( int image )
    {
        Gtk::Adjustment* pAdjustment = m_hdrImageArray[image].pAdjustmentShutter;
        
        double value = pAdjustment->get_value();

        SetShutter( image, static_cast<unsigned int>(value) );
    }

    bool HDRPage::IsHDRSupported()
    {
        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( m_hdrRegOffset, &value );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error reading HDR offset register", error );
            return false;
        }

        return (value >> 31) != 0;        
    }

    bool HDRPage::IsHDREnabled()
    {
        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( m_hdrRegOffset, &value );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error reading HDR offset register", error );
            return false;
        }

        return (value & (0x1 << 25)) != 0;        
    }

    unsigned int HDRPage::GetHDRRegOffset()
    {
        unsigned int hdrReg;
        if ( strstr( m_camInfo.modelName, "Bumblebee" ) != NULL &&
            strstr( m_camInfo.modelName, "Bumblebee2" ) == NULL )
        {
            // The camera is a Bumblebee, but not a Bumblebee2 or XB3
            hdrReg = sk_bumblebeeHDRReg;
        }
        else
        {
            hdrReg = sk_hdrReg;
        }        

        return hdrReg;
    }

    unsigned int HDRPage::GetGain( int image )
    {
        unsigned int regOffset = GetRegisterOffset( m_hdrRegOffset, image ) + 0x4;

        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( regOffset, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading HDR gain register", error );
        }

        return (value & 0x00000FFF);
    }

    void HDRPage::SetGain( int image, unsigned int gainValue )
    {
        unsigned int regOffset = GetRegisterOffset( m_hdrRegOffset, image ) + 0x4;

        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( regOffset, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading HDR gain register", error );
        }

        value &= ~0xFFF;
        value += gainValue;

        error = m_pCamera->WriteRegister( regOffset, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing HDR gain register", error );
        }
    }

    unsigned int HDRPage::GetShutter( int image )
    {
        unsigned int regOffset = GetRegisterOffset( m_hdrRegOffset, image );

        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( regOffset, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading HDR shutter register", error );
        }

        return (value & 0x00000FFF);
    }

    void HDRPage::SetShutter( int image, unsigned int shutterValue )
    {
        unsigned int regOffset = GetRegisterOffset( m_hdrRegOffset, image );

        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( regOffset, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading HDR shutter register", error );
        }

        value &= ~0xFFF;
        value += shutterValue;

        error = m_pCamera->WriteRegister( regOffset, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing HDR shutter register", error );
        }
    }  

    void HDRPage::SetGainRange( int image, unsigned int min, unsigned int max )
    {   
        m_hdrImageArray[image].pAdjustmentGain->set_lower( min );
        m_hdrImageArray[image].pAdjustmentGain->set_upper( max );
    }

    void HDRPage::SetShutterRange( int image, unsigned int min, unsigned int max )
    {
        m_hdrImageArray[image].pAdjustmentShutter->set_lower( min );
        m_hdrImageArray[image].pAdjustmentShutter->set_upper( max );
    }

    unsigned int HDRPage::GetRegisterOffset( unsigned int regBase, int image )
    {
        unsigned int regOffset = 0;

        switch( image )
        {
        case 0:
            regOffset = regBase + 0x20;
        	break;
        case 1:
            regOffset = regBase + 0x40;
        	break;
        case 2:
            regOffset = regBase + 0x60;
            break;
        case 3:
            regOffset = regBase + 0x80;
            break;
        default:
            break;
        }

        return regOffset;
    }

    void HDRPage::EnableHDRWidgets()
    {      
        for ( int i=0; i < 4; i++ )
        {
            m_hdrImageArray[i].pHscaleGain->set_sensitive( true );
            m_hdrImageArray[i].pHscaleShutter->set_sensitive( true );
        }
    }

    void HDRPage::DisableHDRWidgets()
    {
        for ( int i=0; i < 4; i++ )
        {
            m_hdrImageArray[i].pHscaleGain->set_sensitive( false );
            m_hdrImageArray[i].pHscaleShutter->set_sensitive( false );
        }
    }            

    void HDRPage::UpdateHDRWidgets()
    {
        Error error;
        PropertyInfo propGain;
        propGain.type = GAIN;
        error = m_pCamera->GetPropertyInfo( &propGain );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading gain property", error );
        }

        PropertyInfo propShutter;
        propShutter.type = SHUTTER;
        error = m_pCamera->GetPropertyInfo( &propShutter );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading gain property information", error );
        }

        for ( int i=0; i < 4; i++ )
        {
            SetGainRange( i, propGain.min, propGain.max );
            SetShutterRange( i, propShutter.min, propShutter.max );

            unsigned int gain = GetGain( i );
            unsigned int shutter = GetShutter( i );

            m_hdrImageArray[i].pAdjustmentGain->set_value( gain );
            m_hdrImageArray[i].pAdjustmentShutter->set_value( shutter );
        }                                
    }
    
}
