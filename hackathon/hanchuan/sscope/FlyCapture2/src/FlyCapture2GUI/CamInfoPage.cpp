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
// $Id: CamInfoPage.cpp,v 1.36 2010/06/22 21:23:44 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "CamInfoPage.h"
#include <ctype.h>

namespace FlyCapture2
{
	const char* CamInfoPage::sk_labelSerial = "lblSerialNum";
	const char* CamInfoPage::sk_labelModel = "lblModel";
	const char* CamInfoPage::sk_labelVendor = "lblVendor";
	const char* CamInfoPage::sk_labelSensor = "lblSensor";
	const char* CamInfoPage::sk_labelResolution = "lblResolution";
	const char* CamInfoPage::sk_labelInterface = "lblInterface";
	const char* CamInfoPage::sk_labelBusSpeed = "lblBusSpeed";
	const char* CamInfoPage::sk_labelDCAMVer = "lblDCAMVer";
	const char* CamInfoPage::sk_labelFirmwareVer = "lblFirmwareVersion";
	const char* CamInfoPage::sk_labelFirmwareBuildTime = "lblFirmwareBuildTime";

    const char* CamInfoPage::sk_vboxGigECameraInfo = "vboxGigECameraInfo";
    const char* CamInfoPage::sk_labelGigEVersion = "lblGigEVersion";
    const char* CamInfoPage::sk_labelGigEUserDefinedName = "lblGigEUserDefinedName";
    const char* CamInfoPage::sk_labelGigEXmlUrl1 = "lblGigEXmlUrl1";
    const char* CamInfoPage::sk_labelGigEXmlUrl2 = "lblGigEXmlUrl2";
    const char* CamInfoPage::sk_labelGigEMacAddress = "lblGigEMacAddress";
    const char* CamInfoPage::sk_labelGigEIpAddress = "lblGigEIpAddress";
    const char* CamInfoPage::sk_labelGigESubnetMask = "lblGigESubnetMask";
    const char* CamInfoPage::sk_labelGigEDefaultGateway = "lblGigEDefaultGateway";
    const char* CamInfoPage::sk_checkGigEIpLLA = "checkIpLLA";
    const char* CamInfoPage::sk_checkGigEIpDHCP = "checkIpDHCP";
    const char* CamInfoPage::sk_checkGigEIpPersistentIp = "checkIpPersistentIp";

    const char* CamInfoPage::sk_labelTimeSinceInitialization = "lblTimeSinceInitialization";
    const char* CamInfoPage::sk_labelTimeSinceBusReset = "lblTimeSinceBusReset";
    const char* CamInfoPage::sk_labelPixelClockFreq = "lblPixelClockFreq";
    const char* CamInfoPage::sk_labelHorizontalLineFreq = "lblHorizontalLineFreq";	
    const char* CamInfoPage::sk_labelTransmitFailures = "lblTransmitFailures";
    const char* CamInfoPage::sk_btnResetTransmitFailures = "btnResetTransmitFailures";    
    const char* CamInfoPage::sk_btnCopyCameraInfo = "btnCopyCameraInfo";

    const char* CamInfoPage::sk_vboxConsoleLog = "vboxConsoleLog";
    const char* CamInfoPage::sk_textviewConsoleLog = "textviewConsoleLog";
    const char* CamInfoPage::sk_btnCopyConsoleLog = "btnCopyConsoleLog";
    const char* CamInfoPage::sk_btnRefreshConsoleLog = "btnRefreshConsoleLog";

    const char* CamInfoPage::sk_notebook = "notebookCamCtl";
	
	CamInfoPage::CamInfoPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
	{		
	}
	
	CamInfoPage::~CamInfoPage()
	{
	}
	
	void CamInfoPage::GetWidgets()
	{
		m_refXml->get_widget( sk_labelSerial, m_pLabelSerial );
		m_refXml->get_widget( sk_labelModel, m_pLabelModel );
		m_refXml->get_widget( sk_labelVendor, m_pLabelVendor );
		m_refXml->get_widget( sk_labelSensor, m_pLabelSensor );		
		m_refXml->get_widget( sk_labelResolution, m_pLabelResolution );
		m_refXml->get_widget( sk_labelInterface, m_pLabelInterface );
		m_refXml->get_widget( sk_labelBusSpeed, m_pLabelBusSpeed );
		m_refXml->get_widget( sk_labelDCAMVer, m_pLabelDCAMVer );
		m_refXml->get_widget( sk_labelFirmwareVer, m_pLabelFirmwareVer );
		m_refXml->get_widget( sk_labelFirmwareBuildTime, m_pLabelFirmwareBuildTime );	

        m_refXml->get_widget( sk_vboxGigECameraInfo, m_pVboxGigECameraInfo );        
        m_refXml->get_widget( sk_labelGigEVersion, m_pLabelGigEVersion );       
        m_refXml->get_widget( sk_labelGigEUserDefinedName, m_pLabelGigEUserDefinedName );       
        m_refXml->get_widget( sk_labelGigEXmlUrl1, m_pLabelGigEXmlUrl1 );       
        m_refXml->get_widget( sk_labelGigEXmlUrl2, m_pLabelGigEXmlUrl2 );       
        m_refXml->get_widget( sk_labelGigEMacAddress, m_pLabelGigEMacAddress );       
        m_refXml->get_widget( sk_labelGigEIpAddress, m_pLabelGigEIpAddress );       
        m_refXml->get_widget( sk_labelGigESubnetMask, m_pLabelGigESubnetMask );       
        m_refXml->get_widget( sk_labelGigEDefaultGateway, m_pLabelGigEDefaultGateway );       
        m_refXml->get_widget( sk_checkGigEIpLLA, m_pCheckGigEIpLLA );       
        m_refXml->get_widget( sk_checkGigEIpDHCP, m_pCheckGigEIpDHCP );   
        m_refXml->get_widget( sk_checkGigEIpPersistentIp, m_pCheckGigEIpPersistentIp );    

        m_refXml->get_widget( sk_labelTimeSinceInitialization, m_pLabelTimeSinceInitialization );		
        m_refXml->get_widget( sk_labelTimeSinceBusReset, m_pLabelTimeSinceBusReset );		
        m_refXml->get_widget( sk_labelPixelClockFreq, m_pLabelPixelClockFreq );		
        m_refXml->get_widget( sk_labelHorizontalLineFreq, m_pLabelHorizontalLineFreq );	
        m_refXml->get_widget( sk_labelTransmitFailures, m_pLabelTransmitFailures );
        m_refXml->get_widget( sk_btnResetTransmitFailures, m_pBtnResetTransmitFailures );        
        m_refXml->get_widget( sk_btnCopyCameraInfo, m_pBtnCopyCameraInfo );	

        m_refXml->get_widget( sk_vboxConsoleLog, m_vboxConsoleLog );
        m_refXml->get_widget( sk_textviewConsoleLog, m_pTextViewConsoleLog );
        m_refXml->get_widget( sk_btnCopyConsoleLog, m_pBtnCopyConsoleLog );
        m_refXml->get_widget( sk_btnRefreshConsoleLog, m_pBtnRefreshConsoleLog );

        m_refXml->get_widget( sk_notebook, m_pNotebook );

        m_refConsoleLogBuffer = Gtk::TextBuffer::create();

        // Set the text view to monospace
        m_pTextViewConsoleLog->modify_font( Pango::FontDescription("monospace") );

        // Hide console log
        m_vboxConsoleLog->hide();
	}
	
	void CamInfoPage::AttachSignals()
	{
        SetTimerConnection(
			  Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &CamInfoPage::OnTimer), 
            sk_timeout ));

        m_pBtnResetTransmitFailures->signal_clicked().connect(
            sigc::mem_fun(*this, &CamInfoPage::OnResetTransmitFailures) );
            
        m_pBtnCopyCameraInfo->signal_clicked().connect( 
            sigc::mem_fun(*this, &CamInfoPage::OnCopyCameraInfo) );

        m_pBtnCopyConsoleLog->signal_clicked().connect( 
            sigc::mem_fun(*this, &CamInfoPage::OnCopyConsoleLog) );

        m_pBtnRefreshConsoleLog->signal_clicked().connect( 
            sigc::mem_fun(*this, &CamInfoPage::OnRefreshConsoleLog) );

        m_pNotebook->signal_key_press_event().connect(
            sigc::mem_fun(*this, &CamInfoPage::OnKeyPressEvent) );
	}
	
	void CamInfoPage::UpdateWidgets()
	{
        if ( m_pCamera == NULL || IsConnected() != true )
        {
            SetBlankInformation();
            return;	
        }
		
		SetCameraInformation( &m_camInfo );	

        OnRefreshConsoleLog();
	}
	
	void CamInfoPage::SetCameraInformation( CameraInfo* pCamInfo )
	{		
		// Set the camera info labels
		char serial[16];
		sprintf( serial, "%u", pCamInfo->serialNumber );

        char dcamVer[16];
        sprintf( dcamVer, "%1.2f", pCamInfo->iidcVer / 100.0f );

		m_pLabelSerial->set_text( serial );
		m_pLabelModel->set_text( pCamInfo->modelName );		
		m_pLabelVendor->set_text( pCamInfo->vendorName );
        m_pLabelSensor->set_text( pCamInfo->sensorInfo );
        m_pLabelResolution->set_text( pCamInfo->sensorResolution );
        m_pLabelInterface->set_text( GetInterfaceString( pCamInfo->interfaceType ) );
        m_pLabelBusSpeed->set_text( GetBusSpeedString( pCamInfo->maximumBusSpeed ) );
        m_pLabelDCAMVer->set_text( dcamVer );		
        m_pLabelFirmwareVer->set_text( pCamInfo->firmwareVersion );
        m_pLabelFirmwareBuildTime->set_text( pCamInfo->firmwareBuildTime );	

        if ( pCamInfo->interfaceType == INTERFACE_GIGE )
        {
            m_pVboxGigECameraInfo->show();

            char gigEVersion[16];
            sprintf( gigEVersion, "%u.%u", pCamInfo->gigEMajorVersion, pCamInfo->gigEMinorVersion );
            m_pLabelGigEVersion->set_text( gigEVersion );

            m_pLabelGigEUserDefinedName->set_text( pCamInfo->userDefinedName );
            m_pLabelGigEXmlUrl1->set_text( pCamInfo->xmlURL1 );
            m_pLabelGigEXmlUrl2->set_text( pCamInfo->xmlURL2 );

            char macAddress[64];
            sprintf( 
                macAddress, 
                "%02X:%02X:%02X:%02X:%02X:%02X", 
                pCamInfo->macAddress.octets[0],
                pCamInfo->macAddress.octets[1],
                pCamInfo->macAddress.octets[2],
                pCamInfo->macAddress.octets[3],
                pCamInfo->macAddress.octets[4],
                pCamInfo->macAddress.octets[5]);
            m_pLabelGigEMacAddress->set_text( macAddress );

            char ipAddress[32];
            sprintf( 
                ipAddress, 
                "%u.%u.%u.%u", 
                pCamInfo->ipAddress.octets[0],
                pCamInfo->ipAddress.octets[1],
                pCamInfo->ipAddress.octets[2],
                pCamInfo->ipAddress.octets[3]);
            m_pLabelGigEIpAddress->set_text( ipAddress );

            char subnetMask[32];
            sprintf( 
                subnetMask, 
                "%u.%u.%u.%u", 
                pCamInfo->subnetMask.octets[0],
                pCamInfo->subnetMask.octets[1],
                pCamInfo->subnetMask.octets[2],
                pCamInfo->subnetMask.octets[3]);
            m_pLabelGigESubnetMask->set_text( subnetMask );

            char defaultGateway[32];
            sprintf( 
                defaultGateway, 
                "%u.%u.%u.%u", 
                pCamInfo->defaultGateway.octets[0],
                pCamInfo->defaultGateway.octets[1],
                pCamInfo->defaultGateway.octets[2],
                pCamInfo->defaultGateway.octets[3]);
            m_pLabelGigEDefaultGateway->set_text( defaultGateway );

            GigECamera* pCamera = dynamic_cast<GigECamera*>(m_pCamera);
            if ( pCamera != NULL )
            {
                unsigned int ipConfigurationVal = 0;
                Error error = pCamera->ReadGVCPRegister( 0x0010, &ipConfigurationVal );
                if ( error != PGRERROR_OK )
                {
                    return;
                }   

                m_pCheckGigEIpLLA->set_active( (ipConfigurationVal & 0x1) != 0 );
                m_pCheckGigEIpDHCP->set_active( (ipConfigurationVal & 0x2) != 0 );
                m_pCheckGigEIpPersistentIp->set_active( (ipConfigurationVal & 0x4) != 0 );
            }
            else
            {
                m_pCheckGigEIpLLA->set_active( false );
                m_pCheckGigEIpDHCP->set_active( false );
                m_pCheckGigEIpPersistentIp->set_active( false );
            }       

            m_pCheckGigEIpLLA->set_sensitive( false );
            m_pCheckGigEIpDHCP->set_sensitive( false );
            m_pCheckGigEIpPersistentIp->set_sensitive( false );
        }
        else
        {
            m_pVboxGigECameraInfo->hide();
        }
	}

    void CamInfoPage::SetBlankInformation()
    {
        const char* unknown = "Unknown";
        m_pLabelModel->set_text( unknown );		
        m_pLabelVendor->set_text( unknown );
        m_pLabelSensor->set_text( unknown );
        m_pLabelResolution->set_text( unknown );
        m_pLabelInterface->set_text( unknown );
        m_pLabelBusSpeed->set_text( unknown );
        m_pLabelDCAMVer->set_text( unknown );
        m_pLabelFirmwareVer->set_text( unknown );
        m_pLabelFirmwareBuildTime->set_text( unknown );
    }

    const char* CamInfoPage::GetInterfaceString( InterfaceType type )
    {
        switch (type)
        {
        case INTERFACE_IEEE1394:
            return "IEEE-1394";
        case INTERFACE_USB2:
            return "USB 2.0";
		case INTERFACE_GIGE:
            return "GigE";
        default:
            return "Unknown interface";
        }
    }

    const char* CamInfoPage::GetBusSpeedString( BusSpeed speed )
    {
        switch (speed)
        {
        case BUSSPEED_S100:
            return "S100";
        case BUSSPEED_S200:
            return "S200";
        case BUSSPEED_S400:
            return "S400";
        case BUSSPEED_S480:
            return "S480";
        case BUSSPEED_S800:
            return "S800";        
        case BUSSPEED_S1600:
            return "S1600";
        case BUSSPEED_S3200:
            return "S3200";
        default:
            return "Unknown bus speed";
        }
    }

    bool CamInfoPage::OnTimer()
    {
        if (m_pCamera == NULL || IsIconified())
        {
            return true;
        }

        Gtk::Notebook* pNotebook;
        m_refXml->get_widget( "notebookCamCtl", pNotebook );
        if ( pNotebook->get_current_page() == 3 )
        {
            UpdateInitializeTime();
            UpdateBusResetTime();
            UpdatePixelClockFreq();
            UpdateHorizontalLineFreq();
            UpdateTransmitFailures();
        }       

        return true;
    }

    void CamInfoPage::ParseTimeRegister( 
        unsigned int timeRegVal, 
        unsigned int& hours, 
        unsigned int& mins, 
        unsigned int& seconds )
    {
        hours = timeRegVal / (60 * 60);
        mins = (timeRegVal - (hours * 60 * 60)) / 60;
        seconds = timeRegVal - (hours * 60 * 60) - (mins * 60);
    }

    void CamInfoPage::UpdateInitializeTime()
    {
        const unsigned int k_initializeTimeReg = 0x12E0;   
        unsigned int initializeTimeRegVal = 0;
        Error error = m_pCamera->ReadRegister( k_initializeTimeReg, &initializeTimeRegVal );
        if ( error != PGRERROR_OK )
        {
            m_pLabelTimeSinceInitialization->set_text( "N/A" );
        }
        else
        {
            unsigned int numHours = 0;
            unsigned int numMins = 0;
            unsigned int numSeconds = 0;

            ParseTimeRegister( initializeTimeRegVal, numHours, numMins, numSeconds );

            char timeStr[512];
            sprintf( 
                timeStr,
                "%uh %um %us",
                numHours,
                numMins,
                numSeconds );

            m_pLabelTimeSinceInitialization->set_text( timeStr );
        }
    }

    void CamInfoPage::UpdateBusResetTime()
    {
        const unsigned int k_busResetTimeReg = 0x12E4;  
        unsigned int busResetTimeRegVal = 0;
        Error error = m_pCamera->ReadRegister( k_busResetTimeReg, &busResetTimeRegVal );
        if ( error != PGRERROR_OK )
        {
            m_pLabelTimeSinceBusReset->set_text( "N/A" );
        }
        else
        {
            unsigned int numHours = 0;
            unsigned int numMins = 0;
            unsigned int numSeconds = 0;

            ParseTimeRegister( busResetTimeRegVal, numHours, numMins, numSeconds );

            char timeStr[512];
            sprintf( 
                timeStr,
                "%uh %um %us",
                numHours,
                numMins,
                numSeconds );

            m_pLabelTimeSinceBusReset->set_text( timeStr );
        }      
    }

    void CamInfoPage::UpdatePixelClockFreq()
    {
        Error error;
        const unsigned int k_pixelClockFreqReg = 0x1AF0;
        unsigned int pixelClockFreqRegVal = 0;

        error = m_pCamera->ReadRegister( k_pixelClockFreqReg, &pixelClockFreqRegVal );
        if ( error != PGRERROR_OK )
        {
            m_pLabelPixelClockFreq->set_text( "N/A" );
            return;
        }

        double pixelClockFreq = Convert32bitIEEEToFloat( pixelClockFreqRegVal );
        pixelClockFreq /= 1000000.0;

        char pixelClockStr[128];
        sprintf( 
            pixelClockStr,
            "%3.5f MHz",
            pixelClockFreq );
        m_pLabelPixelClockFreq->set_text( pixelClockStr );
    }

    void CamInfoPage::UpdateHorizontalLineFreq()
    {
        Error error;
        const unsigned int k_horizontalLineFreqReg = 0x1AF4;
        unsigned int horizontalLineFreqRegVal = 0;

        error = m_pCamera->ReadRegister( k_horizontalLineFreqReg, &horizontalLineFreqRegVal );
        if ( error != PGRERROR_OK )
        {
            m_pLabelHorizontalLineFreq->set_text( "N/A" );
            return;
        }

        double horizontalLineFreq = Convert32bitIEEEToFloat( horizontalLineFreqRegVal );
        //horizontalLineFreq /= 1000000.0;

        char horizontalLineStr[128];
        sprintf( 
            horizontalLineStr,
            "%.1f Hz",
            horizontalLineFreq );
        m_pLabelHorizontalLineFreq->set_text( horizontalLineStr );
    }

    void CamInfoPage::UpdateTransmitFailures()
    {
        Error error;
        const unsigned int k_transmitFailureReg = 0x12FC;
        unsigned int transmitFailureRegVal = 0;

		CameraInfo camInfo;
		m_pCamera->GetCameraInfo(&camInfo);

        error = m_pCamera->ReadRegister( k_transmitFailureReg, &transmitFailureRegVal );
		if ( error != PGRERROR_OK  || ( camInfo.iidcVer >= 132 && (transmitFailureRegVal & 0x80000000) == 0 ))
        {
            m_pLabelTransmitFailures->set_text( "N/A" );
            return;
        }

        char transmitFailureStr[32];
        sprintf(
            transmitFailureStr,
            "%u",
            transmitFailureRegVal & 0x7FFFFFFF );
        m_pLabelTransmitFailures->set_text( transmitFailureStr );
    }

    void CamInfoPage::OnResetTransmitFailures()
    {
        const unsigned int k_transmitFailureReg = 0x12FC;
        Error error = m_pCamera->WriteRegister( k_transmitFailureReg, 0 );
        if ( error != PGRERROR_OK )
        {
            return;
        }
    }
    
    void CamInfoPage::OnCopyCameraInfo()
    {        
        char tempStr[2048];
        
        sprintf(
            tempStr,
            "Serial Number: %u\n"
            "Model: %s\n"
            "Vendor: %s\n"
            "Sensor: %s\n"
            "Resolution: %s\n"
            "Interface: %s\n"
            "Bus Speed: %s\n"
            "IIDC Version: %1.2f\n"
            "Firmware Version: %s\n"
            "Firmware Build Time: %s\n",
            m_camInfo.serialNumber,
            m_camInfo.modelName,
            m_camInfo.vendorName,
            m_camInfo.sensorInfo,
            m_camInfo.sensorResolution,
            GetInterfaceString( m_camInfo.interfaceType ),
            GetBusSpeedString( m_camInfo.maximumBusSpeed ),
            m_camInfo.iidcVer / 100.0f,
            m_camInfo.firmwareVersion,
            m_camInfo.firmwareBuildTime );                   
        
        Glib::ustring clipboardStr = "*** Camera Information ***\n";         
        clipboardStr += tempStr;
        
        Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
        refClipboard->set_text( clipboardStr );
    }

    void CamInfoPage::OnCopyConsoleLog()
    {        
        Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
        refClipboard->set_text( m_refConsoleLogBuffer->get_text() );
    }

    void CamInfoPage::OnRefreshConsoleLog()
    {
        const unsigned int consoleStartLoc = 0x1D00;
        const unsigned int consoleEndLoc = 0x1E00;
        const unsigned int consoleLength = consoleEndLoc - consoleStartLoc;

        unsigned int consoleData[consoleLength];

        Error error = m_pCamera->ReadRegisterBlock(
            0xFFFF,
            0xF0F00000 + consoleStartLoc,
            consoleData,
            consoleLength / 4 );
        if ( error != PGRERROR_OK )
        {
            m_refConsoleLogBuffer->set_text( "Error reading console log" );
            m_pTextViewConsoleLog->set_buffer( m_refConsoleLogBuffer );
            return;
        }

        Glib::ustring consoleLogString;
        const char* pSrc = (char*)(&consoleData[0]);

        // Data is stored backwards, reverse it for display
        for (unsigned int i=0; i < consoleLength; i++)
        {
            if( pSrc[consoleLength-1-i] == '\r' )
            {
                consoleLogString += "\r\n";
            }
            else if( isascii(pSrc[consoleLength-1-i]) )
            {
                consoleLogString += pSrc[consoleLength-1-i];
            }
            else
            {
                consoleLogString += " " ;
            }
        }

        m_refConsoleLogBuffer->set_text( consoleLogString );
        m_pTextViewConsoleLog->set_buffer( m_refConsoleLogBuffer );
    }

    bool CamInfoPage::OnKeyPressEvent( GdkEventKey* event )
    {
        if ( event->type == GDK_KEY_PRESS )
        {     
            const GdkModifierType maskToMatch = (GdkModifierType)(GDK_SHIFT_MASK | GDK_CONTROL_MASK);
            const guint keyToMatch = 0x43; // Match the character 'c'
            if ( event->state == (guint)maskToMatch && event->keyval == keyToMatch )
            {                
                if ( m_vboxConsoleLog->is_visible() )
                {
                    m_vboxConsoleLog->hide();
                }
                else
                {
                    m_vboxConsoleLog->show();
                }   

                return true;
            }
        }        

        return false;
    }
}
