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
// $Id: CamControl.cpp,v 1.50 2010/06/18 17:17:35 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "CamControl.h"

#include "Format7Page.h"
#include "DCAMFormatsPage.h"
#include "CamSettingsPage.h"
#include "CamInfoPage.h"

#include "RegisterPage.h"
#include "GPIOPage.h"
#include "AdvCameraSettingsPage.h"
#include "HDRPage.h"
#include "LUTPage.h"
#include "FrameBufferPage.h"
#include "DataFlashPage.h"

#include "SystemInfoPage.h"
#include "BusTopologyPage.h"
#include "HelpSupportPage.h"
#include "GladeFileUtil.h"

namespace FlyCapture2
{
	const char* CamControl::sk_camControlWindow = "windowCamCtl";
	
	CamControl::CamControl() :	
		m_pWindow(0),
		m_connected(false)
	{		
        m_pKit = Gtk::Main::instance();
        if ( m_pKit == NULL )
        {
            m_pKit = new Gtk::Main( 0, NULL );
        }

		m_pCamera = 0;
		m_run = 0;

        m_xPos = 0;
        m_yPos = 0;

        bFirst = true;

        m_pFormat7Page = NULL;
        m_pDcamFormatsPage = NULL;
        m_pCamSettingsPage = NULL;
        m_pCamInfoPage = NULL;	
        m_pRegisterPage = NULL;
        m_pGpioPage = NULL;
        m_pAdvCamSettingsPage = NULL;
        m_pHdrPage = NULL;
        m_pLutPage = NULL;
        m_pFrameBufferPage = NULL;
        m_pDataFlashPage = NULL;
        m_pSystemInfoPage = NULL;
		m_pBusTopologyPage = NULL;
        m_pHelpPage = NULL;

        // Initialize the thread system
        if( !Glib::thread_supported() ) 
        {
            Glib::thread_init();
        }      	
	}
	
	CamControl::~CamControl()
	{      
		DeleteAllPages();      

        if ( m_pWindow != NULL )
        {
            delete m_pWindow;
            m_pWindow = NULL;
        }

	}
	
	bool CamControl::Initialize()
	{
        Glib::ustring gladePath = GladeFileUtil::GetGladeFilePath();

		//m_refXml = Gnome::Glade::Xml::create(gladePath);

#ifdef GLIBMM_EXCEPTIONS_ENABLED
        try
        {
            m_refXml = Gnome::Glade::Xml::create(gladePath);
        }
        catch(const Gnome::Glade::XmlError& ex)
        {           
            char szSecondary[512];
            sprintf( 
                szSecondary,
                "Error: %s. Make sure that the file is present.",
                ex.what().c_str() );

            Gtk::MessageDialog dialog( "Error loading Glade file", false, Gtk::MESSAGE_ERROR );
            dialog.set_secondary_text( szSecondary );
            dialog.run();

            return false;
        }
#else
        std::auto_ptr<Gnome::Glade::XmlError> error;
        m_refXml = Gnome::Glade::Xml::create(gladePath, "", "", error);
        if(error.get())
        {
            char szSecondary[512];
            sprintf( 
                szSecondary,
                "Error: %s. Make sure that the file is present.",
                ex.what().c_str() );

            Gtk::MessageDialog dialog( "Error loading Glade file", false, Gtk::MESSAGE_ERROR );
            dialog.set_secondary_text( szSecondary );
            dialog.run();

            return false;
        }
#endif

		m_refXml->get_widget( sk_camControlWindow, m_pWindow );
		if ( m_pWindow == NULL )
		{
			throw std::runtime_error("Error!");
		}

        m_pWindow->signal_delete_event().connect(sigc::mem_fun( *this, &CamControl::OnDestroy ));
        m_pWindow->signal_expose_event().connect(sigc::mem_fun( *this, &CamControl::OnExposeEvent ));
		
		// Create all the sub classes
		m_pFormat7Page = new Format7Page( m_pCamera, m_refXml );
		m_pDcamFormatsPage = new DCAMFormatsPage( m_pCamera, m_refXml );
		m_pCamSettingsPage = new CamSettingsPage( m_pCamera, m_refXml );
		m_pCamInfoPage = new CamInfoPage( m_pCamera, m_refXml );	
		m_pRegisterPage = new RegisterPage( m_pCamera, m_refXml );
		m_pGpioPage = new GPIOPage( m_pCamera, m_refXml );
		m_pAdvCamSettingsPage = new AdvCameraSettingsPage( m_pCamera, m_refXml );
        m_pHdrPage = new HDRPage( m_pCamera, m_refXml );
		m_pLutPage = new LUTPage( m_pCamera, m_refXml );
        m_pFrameBufferPage = new FrameBufferPage( m_pCamera, m_refXml );
        m_pDataFlashPage = new DataFlashPage( m_pCamera, m_refXml );
		m_pSystemInfoPage = new SystemInfoPage( m_pCamera, m_refXml );
		m_pBusTopologyPage = new BusTopologyPage( m_pCamera, m_refXml );
		m_pHelpPage = new HelpSupportPage( m_pCamera, m_refXml );
		
		// Add it to the list
		m_listPages.push_back( m_pFormat7Page );
		m_listPages.push_back( m_pDcamFormatsPage );
		m_listPages.push_back( m_pCamSettingsPage );
		m_listPages.push_back( m_pCamInfoPage );
		m_listPages.push_back( m_pRegisterPage );		
		m_listPages.push_back( m_pGpioPage );
		m_listPages.push_back( m_pAdvCamSettingsPage );
        m_listPages.push_back( m_pHdrPage );
		m_listPages.push_back( m_pLutPage );
        m_listPages.push_back( m_pFrameBufferPage );
        m_listPages.push_back( m_pDataFlashPage );
		m_listPages.push_back( m_pSystemInfoPage );
		m_listPages.push_back( m_pBusTopologyPage );
		m_listPages.push_back( m_pHelpPage );		

        m_pFormat7Page->signal_format7_applied().connect(
            sigc::mem_fun(*this, &CamControl::OnFormat7Applied));

        m_pFormat7Page->signal_format7_applied().connect(
            sigc::mem_fun(*this, &CamControl::OnFormatChanged));

        m_pDcamFormatsPage->signal_dcam_format_changed().connect(
            sigc::mem_fun(*this, &CamControl::OnFormatChanged));

        m_pAdvCamSettingsPage->signal_restore_from_memory_channel().connect(
            sigc::mem_fun(*this, &CamControl::OnRestoreFromMemoryChannel));
		
		// Prepare for display
		InitializeAllPages();

        // Load the PGR icon
        LoadPGRIcon();

        return true;
	}
		
	void CamControl::Connect( CameraBase* pCamera )
	{				
        if ( pCamera == NULL )
        {
            // Bad!
            return;
        }

        m_pCamera = pCamera;
				
		m_connected = true;

        Error error;
        CameraInfo camInfo;
        error = m_pCamera->GetCameraInfo( &camInfo );
        if ( error != PGRERROR_OK )
        {
            // Error
        }

        FC2Version version;
        Utilities::GetLibraryVersion( &version );

        char title[512];
        sprintf(
            title,
            "FlyCapture2 Camera Control %u.%u.%u.%u - %s %s (%u)",
            version.major,
            version.minor,
            version.type,
            version.build,
            camInfo.vendorName,
            camInfo.modelName,
            camInfo.serialNumber );

        m_pWindow->set_title( title );
		
        SetCameraAllPages( pCamera );
        SetCameraInfoAllPages( camInfo );
		SetConnectedAllPages( m_connected );
		
		UpdateAllPages();
	}
	
	void CamControl::Disconnect()
	{
        m_pCamera = NULL;
		
		m_connected = false;
		
		SetConnectedAllPages( m_connected );
	}
		
	void CamControl::Show()
	{
        if ( m_pCamera == NULL )
        {
            return;
        }     

        if ( bFirst )
        {
            m_pWindow->set_position( Gtk::WIN_POS_CENTER );
            m_pWindow->get_position( m_xPos, m_yPos );
            bFirst = false;
        }

        m_pWindow->move( m_xPos, m_yPos );
			        
        Gtk::Main* kit = NULL;
        kit = Gtk::Main::instance();   

        // If the level is 0, then it means there is no GTK loop currently
        // running. This is often seen in applications that are not
        // written in GTKmm (e.g. MFC). 
        if ( kit->level() == 0 )
        {
            // Start a loop since none are present.
            kit->run( *m_pWindow );
        }
        else
        {
            // There already is a loop, just show the window
            m_pWindow->show();
        }        
	}
	
	void CamControl::Hide()
	{
        m_pWindow->get_position( m_xPos, m_yPos );

        m_pWindow->hide();		  		  
	}
	
	bool CamControl::IsVisible()
	{		
        if ( m_pCamera == NULL )
        {
            return false;
        }

		return m_pWindow->is_visible();
	}	
		
	void CamControl::DeleteAllPages()
	{
		std::list<BasePage*>::iterator iter;
		for ( iter = m_listPages.begin() ; iter != m_listPages.end(); iter++ )
		{
			delete ((BasePage*)*iter);
		}
		
		m_listPages.clear();
	}
		
	void CamControl::InitializeAllPages()
	{
		std::list<BasePage*>::iterator iter;
		for ( iter = m_listPages.begin() ; iter != m_listPages.end(); iter++ )
		{
			((BasePage*)*iter)->Initialize();
		}
	}
		
	void CamControl::UpdateAllPages()
	{
		std::list<BasePage*>::iterator iter;
		for ( iter = m_listPages.begin() ; iter != m_listPages.end(); iter++ )
		{
			((BasePage*)*iter)->UpdateWidgets();
		}
	}

    void CamControl::SetCameraAllPages( CameraBase* pCamera )
    {
        std::list<BasePage*>::iterator iter;
        for ( iter = m_listPages.begin() ; iter != m_listPages.end(); iter++ )
        {
            ((BasePage*)*iter)->SetCamera( pCamera );
        }
    }

    void CamControl::SetCameraInfoAllPages( CameraInfo camInfo )
    {
        std::list<BasePage*>::iterator iter;
        for ( iter = m_listPages.begin() ; iter != m_listPages.end(); iter++ )
        {
            ((BasePage*)*iter)->SetCameraInfo( camInfo );
        }
    }
		
	void CamControl::SetConnectedAllPages( bool connected )
	{
		std::list<BasePage*>::iterator iter;
		for ( iter = m_listPages.begin() ; iter != m_listPages.end(); iter++ )
		{
			((BasePage*)*iter)->SetConnected( connected );
		}
	}

    bool CamControl::OnDestroy( GdkEventAny* /*event*/ )
    {
        m_pWindow->hide();

        return true;
    }

    bool CamControl::OnExposeEvent( GdkEventExpose* /*event*/ )
    {
        m_pWindow->get_position( m_xPos, m_yPos );

        return true;
    }

    void CamControl::LoadPGRIcon()
    {
        m_iconPixBuf = Gdk::Pixbuf::create_from_inline( sizeof(PGRIcon), PGRIcon, false );
        m_pWindow->set_default_icon( m_iconPixBuf );
    }

    void CamControl::OnFormat7Applied()
    {
        if ( m_pDcamFormatsPage != NULL )
        {
            m_pDcamFormatsPage->OnFormat7Applied();
        }
    }

    void CamControl::OnFormatChanged()
    {
        if ( m_pLutPage != NULL )
        {
            m_pLutPage->OnFormatChanged();
        }
    }

    void CamControl::OnRestoreFromMemoryChannel()
    {
        if ( m_pFormat7Page != NULL )
        {
            m_pFormat7Page->UpdateWidgets();
        }
    }
}
