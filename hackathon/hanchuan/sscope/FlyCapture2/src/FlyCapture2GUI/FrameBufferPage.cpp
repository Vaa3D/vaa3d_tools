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
// $Id: FrameBufferPage.cpp,v 1.18 2010/04/28 23:03:54 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "FrameBufferPage.h"

namespace FlyCapture2
{
    const char* FrameBufferPage::sk_checkbuttonEnableImageHold = "checkbuttonEnableImageHold";
    const char* FrameBufferPage::sk_labelFrameBufferType = "labelFrameBufferType";
    const char* FrameBufferPage::sk_labelFrameBufferTotal = "labelFrameBufferTotal";
    const char* FrameBufferPage::sk_labelFrameBufferUsed = "labelFrameBufferUsed";
    const char* FrameBufferPage::sk_hscaleFrameBuffer = "hscaleFrameBuffer";
    const char* FrameBufferPage::sk_buttonTransmitSelectedImage = "buttonTransmitSelectedImage";
    const char* FrameBufferPage::sk_buttonRetransmitLastImage = "buttonRetransmitLastImage";

	const unsigned int FrameBufferPage::sk_frameBufferRegIIDC = 0x634;
	const unsigned int FrameBufferPage::sk_frameBufferReg = 0x12E8;

    FrameBufferPage::FrameBufferPage()
    {
        m_pAdjustmentFrameBuffer = NULL;
        m_quit = false;
        m_isIIDC = false;
    }

    FrameBufferPage::FrameBufferPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
    {
        m_pAdjustmentFrameBuffer = NULL;
        m_quit = false;
        m_isIIDC = false;
    }

    FrameBufferPage::~FrameBufferPage()
    {
        m_quit = true;

        if ( m_pAdjustmentFrameBuffer != NULL )
        {
            delete m_pAdjustmentFrameBuffer;
        }        
    }

    void FrameBufferPage::UpdateWidgets()
    {
        if ( m_pCamera == NULL || IsConnected() != true )
        {
            DisableWidgets();
            return;	
        }

        if ( IsFrameBufferSupported() != true )
        {
            m_pCheckbuttonEnableImageHold->set_sensitive( false );            
            DisableWidgets();

            return;
        }

        m_pCheckbuttonEnableImageHold->set_sensitive( true );
        
        char label[32];

        sprintf( label, "%s", m_isIIDC ? "IIDC 1.32" : "PGR" );
        m_pLabelFrameBufferType->set_text( label );

        sprintf( label, "%u", GetNumBuffers() );
        m_pLabelFrameBufferTotal->set_text( label );

        sprintf( label, "%u", GetNumUsedBuffers() );
        m_pLabelFrameBufferUsed->set_text( label );

        m_pAdjustmentFrameBuffer->set_value( m_pAdjustmentFrameBuffer->get_lower() );

        if ( IsFrameBufferEnabled() == false )
        {
            m_pCheckbuttonEnableImageHold->set_active( false );
            DisableWidgets();

            return;
        }        

        m_pCheckbuttonEnableImageHold->set_active( true );
        EnableWidgets();
    }

    void FrameBufferPage::GetWidgets()
    {
        m_refXml->get_widget( sk_checkbuttonEnableImageHold, m_pCheckbuttonEnableImageHold );
        m_refXml->get_widget( sk_labelFrameBufferType, m_pLabelFrameBufferType );
        m_refXml->get_widget( sk_labelFrameBufferTotal, m_pLabelFrameBufferTotal );
        m_refXml->get_widget( sk_labelFrameBufferUsed, m_pLabelFrameBufferUsed );
        m_refXml->get_widget( sk_hscaleFrameBuffer, m_pHscaleFrameBuffer );
        m_refXml->get_widget( sk_buttonTransmitSelectedImage, m_pButtonTransmitSelectedImage );
        m_refXml->get_widget( sk_buttonRetransmitLastImage, m_pButtonRetransmitLastImage );

        m_pAdjustmentFrameBuffer = new Gtk::Adjustment( 0, 0, 100 );

        m_pHscaleFrameBuffer->set_adjustment( *m_pAdjustmentFrameBuffer );
    }

    void FrameBufferPage::AttachSignals()
    {
        // Create a timer
        SetTimerConnection(
			  Glib::signal_timeout().connect( sigc::mem_fun(*this, &FrameBufferPage::OnTimer), sk_timeout ));

        m_pCheckbuttonEnableImageHold->signal_toggled().connect(
            sigc::mem_fun(*this, &FrameBufferPage::OnEnableFrameBufferToggled));

        m_pButtonTransmitSelectedImage->signal_clicked().connect(
            sigc::mem_fun(*this, &FrameBufferPage::OnTransmitSelectedImage));

        m_pButtonRetransmitLastImage->signal_clicked().connect(
            sigc::mem_fun(*this, &FrameBufferPage::OnRetransmitSelectedImage));
    }

    bool FrameBufferPage::IsFrameBufferSupported()
    {
        Error error;
        unsigned int value = 0;

        const unsigned int k_basicFuncInq = 0x400;
        error = m_pCamera->ReadRegister( k_basicFuncInq, &value );
        if ( error != PGRERROR_OK )
        {
            return false;
        }

        if ( (value & (0x1 << 10)) != 0 )
        {
            m_isIIDC = true;
            return true;
        }
        else
        {
            m_isIIDC = false;

            // Check if the PGR mode is supported as a fallback for the 
            // IIDC mode
            error = m_pCamera->ReadRegister( sk_frameBufferReg, &value );
            if ( error != PGRERROR_OK )
            {
                return false;
            }

            return (value >> 31) != 0;
        }   
    }

    bool FrameBufferPage::IsFrameBufferEnabled()
    {
        Error error;
        unsigned int value = 0;

        const unsigned int k_frameBufferReg = m_isIIDC ? sk_frameBufferRegIIDC : sk_frameBufferReg;
        error = m_pCamera->ReadRegister( k_frameBufferReg, &value );
        if ( error != PGRERROR_OK )
        {
            return false;
        }

        if ( m_isIIDC )
        {
            return (value >> 31) != 0;
        }
        else
        {
            return (value & (0x1 << 25)) != 0;
        }
    }

    unsigned int FrameBufferPage::GetNumBuffers()
    {
        Error error;
        unsigned int value = 0;

        const unsigned int k_frameBufferReg = m_isIIDC ? sk_frameBufferRegIIDC : sk_frameBufferReg;
        error = m_pCamera->ReadRegister( k_frameBufferReg, &value );
        if ( error != PGRERROR_OK )
        {
            return false;
        }

        if ( m_isIIDC )
        {
            return (value & 0x00FFF000) >> 12;
        }
        else
        {            
            return (value & 0x0000FF00) >> 8;
        }
        
    }

    unsigned int FrameBufferPage::GetNumUsedBuffers()
    {
        Error error;
        unsigned int value = 0;

        const unsigned int k_frameBufferReg = m_isIIDC ? sk_frameBufferRegIIDC : sk_frameBufferReg;
        error = m_pCamera->ReadRegister( k_frameBufferReg, &value );
        if ( error != PGRERROR_OK )
        {
            return false;
        }

        if ( m_isIIDC )
        {
            return value & 0x00000FFF;
        }
        else
        {
            return value & 0x000000FF;
        }        
    }

    void FrameBufferPage::EnableWidgets()
    {
        m_pHscaleFrameBuffer->set_sensitive( true );
        m_pButtonTransmitSelectedImage->set_sensitive( true );
        m_pButtonRetransmitLastImage->set_sensitive( true );
    }

    void FrameBufferPage::DisableWidgets()
    {
        m_pHscaleFrameBuffer->set_sensitive( false );
        m_pButtonTransmitSelectedImage->set_sensitive( false );
        m_pButtonRetransmitLastImage->set_sensitive( false );
    }

    bool FrameBufferPage::OnTimer()
    {       
        if ( m_quit )
        {
            return false;
        }

        if (m_pCamera == NULL || IsIconified())
        {
            return true;
        }

        Gtk::Notebook* pNotebook;
        m_refXml->get_widget( "notebookCamCtl", pNotebook );
        if ( pNotebook->get_current_page() == 9 )
        {
            if ( IsFrameBufferSupported() != true )
            {
                return true;
            }

            // Setting the count to 0 for IIDC does nothing, so set the minimum to 1.
            m_pAdjustmentFrameBuffer->set_lower( m_isIIDC ? 1 : 0 );
            m_pAdjustmentFrameBuffer->set_upper( GetNumBuffers() - 1 );

            char label[16];

            sprintf( label, "%u", GetNumBuffers() );
            m_pLabelFrameBufferTotal->set_text( label );

            sprintf( label, "%u", GetNumUsedBuffers() );
            m_pLabelFrameBufferUsed->set_text( label );
        }        

        return true;
    }

    void FrameBufferPage::OnEnableFrameBufferToggled()
    {
        Error error;
        unsigned int value = 0;

        const unsigned int k_frameBufferReg = m_isIIDC ? sk_frameBufferRegIIDC : sk_frameBufferReg;
        error = m_pCamera->ReadRegister( k_frameBufferReg, &value );
        if ( error != PGRERROR_OK )
        {
            DisableWidgets();
            return;
        }

        bool enableImageHold = m_pCheckbuttonEnableImageHold->get_active();

        enableImageHold ? EnableWidgets() : DisableWidgets();

        if ( m_isIIDC )
        {
            if ( enableImageHold == true )
            {
                value |= 0xC0000000;
                DisableIso();
            }
            else
            {
                value &= ~0xC0000000;
                EnableIso();
            }    
        }
        else
        {
            if ( enableImageHold == true )
            {
                value |= (0x1 << 25);
            }
            else
            {
                value &= ~(0x1 << 25);
            }            
        }

        error = m_pCamera->WriteRegister( k_frameBufferReg, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing frame buffer register", error );
        }
    }    

    void FrameBufferPage::OnTransmitSelectedImage()
    {        
        unsigned int position = 0;
        position = static_cast<unsigned int>(m_pAdjustmentFrameBuffer->get_value());

        Error error;
        unsigned int value = 0;
        const unsigned int k_oneShotReg = 0x61C;
        const unsigned int k_frameBufferReg = m_isIIDC ? k_oneShotReg : sk_frameBufferReg;
        error = m_pCamera->ReadRegister( k_frameBufferReg, &value );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error reading frame buffer register", error );
            return;
        }

        if ( m_isIIDC )
        {
            // Make sure one shot is disabled, and multishot is enabled
            value &= ~(0x1 << 31);
            value |= (0x1 << 30);

            value &= ~0xFFFF;
            value += position;            
        }
        else
        {            
            value &= ~0xFF;
            value += position;
        }    

        error = m_pCamera->WriteRegister( k_frameBufferReg, value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error writing frame buffer register", error );
            return;
        }
    }

    void FrameBufferPage::OnRetransmitSelectedImage()
    {        
        Error error;
        unsigned int value = 0;
        const unsigned int k_oneShotReg = 0x61C;
        const unsigned int k_frameBufferReg = m_isIIDC ? k_oneShotReg : sk_frameBufferReg;
        error = m_pCamera->ReadRegister( k_frameBufferReg, &value );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error reading frame buffer register", error );
            return;
        }

        if ( m_isIIDC )
        {
            // Make sure multishot is disabled, and one shot is enabled
            value &= ~(0x1 << 30);
            value |= (0x1 << 31);

            value &= ~0xFFFF;          
        }
        else
        {            
            value &= ~0xFF;
        }    

        error = m_pCamera->WriteRegister( k_frameBufferReg, value );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error writing frame buffer register", error );
            return;
        }
    }    

    void FrameBufferPage::EnableIso()
    {
        if ( m_isIIDC != true )
        {
            return;
        }

        const unsigned int k_isoReg = 0x614;
        Error error = m_pCamera->WriteRegister( k_isoReg, 0x80000000 );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error enabling isochronous transfer", error );
            return;
        }
    }

    void FrameBufferPage::DisableIso()
    {
        if ( m_isIIDC != true )
        {
            return;
        }

        const unsigned int k_isoReg = 0x614;
        Error error = m_pCamera->WriteRegister( k_isoReg, 0x00000000 );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error disabling isochronous transfer", error );
            return;
        }
    }
}
