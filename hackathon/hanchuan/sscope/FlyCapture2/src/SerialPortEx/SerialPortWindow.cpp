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
// $Id: SerialPortWindow.cpp,v 1.12 2009/08/20 22:47:14 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "SerialPortWindow.h"
#include <string.h>

SerialPortWindow::SerialPortWindow()
{    
    m_run = false;
}

SerialPortWindow::~SerialPortWindow()
{
}

bool
SerialPortWindow::Initialize()
{
    // Load Glade file

    const char* k_flycap2Glade = "SerialPortEx.glade";

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
        m_refXml = Gnome::Glade::Xml::create(k_flycap2Glade);
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
    m_refXml = Gnome::Glade::Xml::create(k_flycap2Glade, "", "", error);
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
   
    m_refXml->get_widget( "window", m_pWindow );
    if ( m_pWindow == NULL )
    {
        return false;
    }

    GetWidgets();
    AttachSignals();

    LoadIcon();    

    m_pWindow->set_default_icon( m_iconPixBuf );
    
    return true;
}

void 
SerialPortWindow::GetWidgets()
{
    m_refXml->get_widget( "menu_quit", m_pMenuQuit );
    m_refXml->get_widget( "menu_about", m_pMenuAbout );

    m_refXml->get_widget( "labelSerialNumber", m_pLabelSerialNumber );
    m_refXml->get_widget( "labelModel", m_pLabelModel );
    m_refXml->get_widget( "labelSensor", m_pLabelSensor );
    m_refXml->get_widget( "labelInterface", m_pLabelInterface );
    m_refXml->get_widget( "labelBaudRate", m_pLabelBaudRate );
    m_refXml->get_widget( "labelParity", m_pLabelParity );
    m_refXml->get_widget( "labelCharacterLength", m_pLabelCharacterLength );
    m_refXml->get_widget( "labelStopBits", m_pLabelStopBits );
    m_refXml->get_widget( "labelBufferSize", m_pLabelBufferSize );

    m_refXml->get_widget( "entryTransmit", m_pEntryTransmit );
    m_refXml->get_widget( "buttonTransmit", m_pButtonTransmit );

    m_refXml->get_widget( "textviewReceiveData", m_pTextViewReceiveData );
    m_pTextViewReceiveData->modify_font( Pango::FontDescription("monospace") );

    m_refXml->get_widget( "buttonQuit", m_pButtonQuit );

    m_refTextBuffer = Gtk::TextBuffer::create();    
}

void 
SerialPortWindow::AttachSignals()
{
    m_pMenuQuit->signal_activate().connect(sigc::mem_fun( *this, &SerialPortWindow::OnMenuQuit ));
    m_pMenuAbout->signal_activate().connect(sigc::mem_fun( *this, &SerialPortWindow::OnMenuAbout ));

    m_pWindow->signal_delete_event().connect(
        sigc::mem_fun( *this, &SerialPortWindow::OnDestroy ));

    m_pButtonTransmit->signal_clicked().connect(
        sigc::mem_fun( *this, &SerialPortWindow::OnTransmit ));

    m_pButtonQuit->signal_clicked().connect(
        sigc::mem_fun( *this, &SerialPortWindow::OnQuit ));

    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &SerialPortWindow::OnTimer), 
        sk_timeout );    
}

bool SerialPortWindow::Run( PGRGuid guid )
{
    Error error;
    error = m_camera.Connect( &guid );
    if ( error != PGRERROR_OK )
    {
        // Error
        return false;
    }

    PopulateCameraInfo();
    PopulateConnectionParameters();

    LaunchReceiveLoop();

    Gtk::Main* pKit = NULL;
    pKit = Gtk::Main::instance();
    pKit->run( *m_pWindow );

    return true;
}

bool
SerialPortWindow::Cleanup()
{   
    return true;
}

void SerialPortWindow::LoadIcon()
{
    m_iconPixBuf = Gdk::Pixbuf::create_from_inline( sizeof(PGRIcon), PGRIcon, false );    
}

void SerialPortWindow::PopulateCameraInfo()
{
    Error error;
    CameraInfo camInfo;
    error = m_camera.GetCameraInfo( &camInfo );
    if ( error != PGRERROR_OK )
    {
        // Error
        return;
    }

    char serial[32];
    sprintf( serial, "%u", camInfo.serialNumber );
    m_pLabelSerialNumber->set_text( serial );

    m_pLabelModel->set_text( camInfo.modelName );

    m_pLabelSensor->set_text( camInfo.sensorInfo );

    switch (camInfo.interfaceType)
    {
    case INTERFACE_IEEE1394:
        m_pLabelInterface->set_text( "IEEE-1394" );
        break;

    case INTERFACE_USB2:
        m_pLabelInterface->set_text( "USB 2.0" );
        break;

	case INTERFACE_GIGE:
        m_pLabelInterface->set_text( "GigE" );
        break;

    default:
        m_pLabelInterface->set_text( "Unknown" );
        break;
    }
}

void SerialPortWindow::PopulateConnectionParameters()
{    
    Error error;
    const unsigned int k_serialModeReg = 0x2000;
    unsigned int serialModeRegVal = 0;
    error = m_camera.ReadRegister( k_serialModeReg, &serialModeRegVal );
    if ( error != PGRERROR_OK )
    {
        // Error
        return;
    }

    char caption[64];
    
    sprintf( caption, "%ubps", GetBaudRate( serialModeRegVal ) );
    m_pLabelBaudRate->set_text( caption );

    sprintf( caption, "%u", GetCharacterLength( serialModeRegVal ) );
    m_pLabelCharacterLength->set_text( caption );

    switch ( GetParity( serialModeRegVal ) )
    {
    case 0: strcpy( caption, "None" ); break;
    case 1: strcpy( caption, "Odd" ); break;
    case 2: strcpy( caption, "Even" ); break;
    default: strcpy( caption, "Unknown" ); break;
    }
    m_pLabelParity->set_text( caption );

    sprintf( caption, "%.1f", GetStopBit( serialModeRegVal ) );
    m_pLabelStopBits->set_text( caption );

    sprintf( caption, "%u", GetBufferSize( serialModeRegVal ) );
    m_pLabelBufferSize->set_text( caption );
}

void SerialPortWindow::LaunchReceiveLoop()
{
    m_run = true;       

    m_pReceiveLoop = Glib::Thread::create( 
        sigc::bind<SerialPortWindow*>(
        sigc::mem_fun(*this, &SerialPortWindow::ReceiveLoop), this ),
        true );
}

void SerialPortWindow::ReceiveLoop( SerialPortWindow* pWindow )
{
    if ( pWindow == NULL )
    {
        return;
    }

    PushMessage( "Entering receive loop\n" );

    while ( m_run )
    {   
        Glib::usleep( 100000 );

        //PushMessage( "Test\n" );

        Error error;
        std::bitset<32> serialCtlBS;

        // Verify that receive is enabled
        serialCtlBS = GetSerialControlBS();
        if ( serialCtlBS[31] != true )
        {
            serialCtlBS[31] = true;

            if ( SetSerialControlBS( serialCtlBS ) != true )
            {
                PushMessage( "Error enabling receive\n" );
                continue;
            }

            PushMessage( "Receive is enabled\n" );
        }        

        // Check for errors
        if ( serialCtlBS[19] == true )
        {
            // Buffer overrun error
            PushMessage( "Buffer overrun error\n" );
        }

        if ( serialCtlBS[18] == true )
        {
            // Data framing error
            PushMessage( "Data framing error\n" );
        }

        if ( serialCtlBS[17] == true )
        {
            // Data parity error
            PushMessage( "Data parity error\n" );
        }

        // Clear the error bits
        serialCtlBS[17] = false;
        serialCtlBS[18] = false;
        serialCtlBS[19] = false;
        if ( SetSerialControlBS( serialCtlBS ) != true )
        {
            PushMessage( "Error clearing error bits\n" );
            continue;
        }

        // Verify that the receive buffer is ready to be read
        serialCtlBS = GetSerialControlBS();        
        if ( serialCtlBS[23] != true )
        {
            // Buffer not ready to be read
            continue;
        }
        
        // Determine amount of data to be read
        const unsigned int k_receiveBufferStatus = 0x2008;
        unsigned int receiveBufferStatusVal = 0;
        error = m_camera.ReadRegister( k_receiveBufferStatus, &receiveBufferStatusVal );
        if ( error != PGRERROR_OK )
        {
            PushMessage( "Error reading receive buffer status\n" );
            continue;
        }

        unsigned int bytesAvailable = receiveBufferStatusVal >> 24;

        if ( bytesAvailable == 0 )
        {
            continue;
        }       

        char receiveStr[256];
        sprintf( receiveStr, "%u bytes available to read\n", bytesAvailable );
        //PushMessage( receiveStr );

        // Send characters to register
        error = m_camera.WriteRegister( 
            k_receiveBufferStatus, 
            bytesAvailable << 16 );
        if ( error != PGRERROR_OK )
        {
            PushMessage( "Error writing receive buffer register\n" );
            continue;
        }

        // Make sure the value written to the camera matches what is
        // currently in the camera
        unsigned int newReceiveBufferStatusVal = 0;
        error = m_camera.ReadRegister( k_receiveBufferStatus, &newReceiveBufferStatusVal );
        if ( error != PGRERROR_OK )
        {
            PushMessage( "Error reading receive buffer status\n" );
            continue;
        }
        
        unsigned int originalVal = (receiveBufferStatusVal >> 8) & 0x00FF0000;
        unsigned int newVal = newReceiveBufferStatusVal & 0x00FF0000;

        if ( originalVal != newVal )
        {
            continue;
        }

        // Read the data
        unsigned int bytesToRead = receiveBufferStatusVal >> 24;
        if ( bytesToRead <= 4 )
        {
            // Under 1 quadlet, perform regular register read
            
            const unsigned int k_sioDataReg = 0x2100;
            unsigned int sioDataVal = 0;
            error = m_camera.ReadRegister( k_sioDataReg, &sioDataVal );
            if ( error != PGRERROR_OK )
            {
                PushMessage( "Error reading SIO data\n" );
                continue;
            }

            char szBuffer[64];
            sprintf(
                szBuffer,
                "Data (%u byte%s): %c%c%c%c\n",
                bytesToRead,
                bytesToRead == 1 ? "" : "s",
                static_cast<char>((sioDataVal >> 24) & 0xFF),
                bytesToRead > 1 ? static_cast<char>((sioDataVal >> 16) & 0xFF) : 32,
                bytesToRead > 2 ? static_cast<char>((sioDataVal >> 8) & 0xFF) : 32,
                bytesToRead > 3 ? static_cast<char>((sioDataVal >> 0) & 0xFF) : 32 );
      
            PushMessage( szBuffer );
        }
        else
        {
            // Above 1 quadlet, perform block read

            const unsigned int k_sioDataBlockReg = 0xF0F02104;
            char szBlock[1024];
            error = m_camera.ReadRegisterBlock(
                0xFFFF,
                k_sioDataBlockReg,                
                (unsigned int*)(&szBlock[0]),
                static_cast<unsigned int>(ceil(bytesToRead/4.0)) );
            if ( error != PGRERROR_OK )
            {
                PushMessage( "Error block reading SIO data\n" );
                continue;
            }

            // Put an string terminator in case some extra garbage was
            // read as a result of reading in multiples of 4.
            szBlock[bytesToRead]='\0';
    
            char szBuffer[2048];
            sprintf( szBuffer, "Data block received: %s\n", szBlock );

            PushMessage( szBuffer );
        }        
    }
}

void SerialPortWindow::OnTransmit()
{
    Error error;

    // Make sure serial transmission is enabled
    std::bitset<32> serialCtlBS = GetSerialControlBS();
    serialCtlBS[30] = true;
    SetSerialControlBS( serialCtlBS );

    // Verify that transmit buffer is ready
    const int numChecks = 3;
    int checkCount = 0;
    do 
    {
        if ( checkCount > 0 )
        {
            Glib::usleep( 100000 );            
        }

        serialCtlBS = GetSerialControlBS();
        checkCount++;

    } while ( (serialCtlBS[23] != true) && (checkCount < numChecks) );

    if ( serialCtlBS[23] != true )
    {
        ShowErrorMessageDialog( 
            "Transmit buffer not ready", 
            "The transmit buffer is not ready to accept data." );
        return;
    }

    // Write the data
    Glib::ustring transmitStr = m_pEntryTransmit->get_text();
    unsigned int bytesToWrite = static_cast<unsigned int>(transmitStr.length());

    if ( transmitStr.length() <= 4 )
    {
        // Under 1 quadlet, perform regular register write

        const unsigned int k_sioDataReg = 0x2100;
        unsigned int sioDataVal[2];
        char* sioDataBuf = (char*)(&sioDataVal[0]);
        Glib::ustring reversed = ReverseStrEndianness( transmitStr );
        sprintf( sioDataBuf, "%s", reversed.c_str() );

        error = m_camera.WriteRegister(
            k_sioDataReg,
            sioDataVal[0] );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error writing SIO data register", error );
            return;
        }
    }
    else
    {
        // Above 1 quadlet, perform block write

        const unsigned int k_sioDataBlockReg = 0xF0F02104;
        unsigned int sioDataVal[1024] = {0};
        char* sioDataBuf = (char*)(&sioDataVal[0]);
        
        Glib::ustring reversed = ReverseStrEndianness( transmitStr );
        sprintf( sioDataBuf, "%s", reversed.c_str() );
        
        unsigned int quadletsToWrite = static_cast<unsigned int>(ceil(bytesToWrite / 4.0));

        error = m_camera.WriteRegisterBlock(
            0xFFFF,
            k_sioDataBlockReg,                
            (unsigned int*)(&sioDataVal[0]),
            quadletsToWrite );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error writing SIO data register", error );
            return;
        }
    }
    
    const unsigned int k_transmitBufferStatus = 0x200C;
    unsigned int transmitBufferStatusVal = 0;
    error = m_camera.ReadRegister( k_transmitBufferStatus, &transmitBufferStatusVal );
    if ( error != PGRERROR_OK )
    {
        ShowErrorMessageDialog( "Error reading transmit buffer status register", error );
        return;
    }

    // Verify that the buffer is storing the data
    unsigned int numBytesStored = (transmitBufferStatusVal >> 16) & 0x00FF;
    if ( numBytesStored < bytesToWrite )
    {
        // Buffer isn't storing enough data
    }

    // Send the data out
    error = m_camera.WriteRegister( k_transmitBufferStatus, transmitBufferStatusVal );
    if ( error != PGRERROR_OK )
    {
        ShowErrorMessageDialog( "Error writing transmit buffer status register", error );
        return;
    }
}

void SerialPortWindow::OnQuit()
{
    m_run = false;

    m_pReceiveLoop->join();

    m_pWindow->hide();    
}

bool SerialPortWindow::OnDestroy( GdkEventAny* /*event*/ )
{
    OnQuit();

    return true;
}


void SerialPortWindow::OnMenuQuit()
{
    OnQuit();
}

void SerialPortWindow::OnMenuAbout()
{
    Gtk::AboutDialog aboutDlg;

    char timeStamp[512];
    sprintf( timeStamp, "%s %s", __DATE__, __TIME__ );

    Glib::ustring comments( "Serial port example for FlyCapture 2.\nBuilt: " );
    comments += timeStamp;

    aboutDlg.set_program_name( "SerialPortEx" );
    aboutDlg.set_comments( comments );
    aboutDlg.set_copyright( "Copyright 2009 Point Grey Research, Inc. All Rights Reserved." );

    FC2Version fc2Version;
    Utilities::GetLibraryVersion( &fc2Version );
    char version[128];
    sprintf( version, "%d.%d.%d.%d", fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build );

    aboutDlg.set_version( version );

    Glib::ustring ustrLicense;
    ustrLicense.append( 
        "The FlyCapture Software Development Kit (the \"Software\") is owned and copyrighted by Point Grey Research, Inc.  All rights are reserved.\n"
        "The Original Purchaser is granted a license to use the Software subject to the following restrictions and limitations.\n"
        "1.	The license is to the Original Purchaser only, and is nontransferable unless you have received written permission of Point Grey Research, Inc.\n" 
        "2.	The Original Purchaser may use the Software only with Point Grey Research, Inc. cameras owned by the Original Purchaser, including but not limited to, Flea, Flea2, Firefly2, Firefly MV, Dragonfly, Dragonfly2, Dragonfly Express or Scorpion Camera Modules.\n"
        "3.	The Original Purchaser may make back-up copies of the Software for his or her own use only, subject to the use limitations of this license.\n"
        "4.	Subject to s.5 below, the Original Purchaser may not engage in, nor permit third parties to engage in, any of the following:\n"
        "a)	Providing or disclosing the Software to third parties.\n"
        "b)	Making alterations or copies of any kind of the Software (except as specifically permitted in s.3 above).\n"
        "c)	Attempting to un-assemble, de-compile or reverse engineer the Software in any way.\n"
        "Granting sublicenses, leases or other rights in the Software to others.\n"
        "5.	Original Purchasers who are Original Equipment Manufacturers may make Derivative Products with the Software. Derivative Products are new software products developed, in whole or in part, using the Software and other Point Grey Research, Inc. products.\n"
        "Point Grey Research, Inc. hereby grants a license to Original Equipment Manufacturers to incorporate and distribute the libraries found in the Software with the Derivative Products.\n"
        "The components of any Derivative Product that contain the Software libraries may only be used with Point Grey Research, Inc. products, or images derived from such products.\n"
        "5.1	By the distribution of the Software libraries with Derivative Products, Original Purchasers agree to:\n"
        "a)	not permit further redistribution of the Software libraries by end-user customers;\n"
        "b)	include a valid copyright notice on any Derivative Product; and\n"
        "c)	indemnify, hold harmless, and defend Point Grey Research, Inc. from and against any claims or lawsuits, including attorney's fees, that arise or result from the use or distribution of any Derivative Product.\n"
        "6.	Point Grey Research, Inc. reserves the right to terminate this license if there are any violations of its terms or if there is a default committed by the Original Purchaser.\n"
        "Upon termination, for any reason, all copies of the Software must be immediately returned to Point Grey Research, Inc. and the Original Purchaser shall be liable to Point Grey Research, Inc. for any and all damages suffered as a result of the violation or default.");

    aboutDlg.set_wrap_license( true );
    aboutDlg.set_license( ustrLicense );
    aboutDlg.set_logo( m_iconPixBuf );

    aboutDlg.run();
}

bool SerialPortWindow::OnTimer()
{
    if ( m_run )
    {
        PopulateConnectionParameters();

        Glib::ustring nextMessage = PopMessage();

        if ( nextMessage == "" )
        {
            return true;
        }

        Glib::ustring currText = m_refTextBuffer->get_text();
        currText += nextMessage;

        m_refTextBuffer->set_text( currText );

        m_pTextViewReceiveData->set_buffer( m_refTextBuffer );

        // Scroll to end of buffer
        Gtk::TextIter iter = m_refTextBuffer->end();
        m_pTextViewReceiveData->scroll_to(iter);

        return true;
    }

    return false;
}

std::bitset<32> SerialPortWindow::GetSerialControlBS()
{
    Error error;
    const unsigned int k_serialCtl = 0x2004;
    unsigned int serialCtlVal = 0;
    error = m_camera.ReadRegister( k_serialCtl, &serialCtlVal );
    if ( error != PGRERROR_OK )
    {
        std::bitset<32> serialCtlBS( 0 );
        return serialCtlBS;
    }

    std::bitset<32> serialCtlBS( serialCtlVal );
    return serialCtlBS;
}

bool SerialPortWindow::SetSerialControlBS( std::bitset<32> serialControlBS )
{
    Error error;
    const unsigned int k_serialCtl = 0x2004;
    error = m_camera.WriteRegister( k_serialCtl, serialControlBS.to_ulong() );
    if ( error != PGRERROR_OK )
    {
        return false;
    }

    return true;
}

void SerialPortWindow::PushMessage( Glib::ustring message )
{
    time_t rawtime;
    struct tm* timeinfo;
    time( &rawtime );
    timeinfo = localtime( &rawtime );

    char currTimeStr[128];
    strftime( currTimeStr, 128, "%X", timeinfo );

    char timeMsg[1024];
    sprintf( 
        timeMsg, 
        "[%s]: %s",
        currTimeStr,
        message.c_str() );    

    Glib::Mutex::Lock messageLock( m_messageMutex );     

    m_messageQueue.push( timeMsg );
}

Glib::ustring SerialPortWindow::PopMessage()
{
    Glib::Mutex::Lock messageLock( m_messageMutex );

    if ( m_messageQueue.size() == 0 )
    {
        return "";
    }

    Glib::ustring message = m_messageQueue.front();
    m_messageQueue.pop();

    return message;
}

Glib::ustring SerialPortWindow::ReverseStrEndianness( Glib::ustring original )
{
    // Determine the number of quadlets to convert
    unsigned int iNumQuadlets = static_cast<unsigned int>(ceil(original.length()/ 4.0));

    Glib::ustring buffer;

    for (unsigned int i=0; i < iNumQuadlets; i++)
    {
        Glib::ustring current = original.substr( i*4, 4 );
        Glib::ustring reversed;

        unsigned int currentLen = static_cast<unsigned int>(current.length());
        for ( unsigned int j=0; j < currentLen; j++ )
        {
            reversed += current.substr( currentLen-1-j, 1 );
        }

        buffer += reversed;
    }

    return buffer;
}

unsigned int SerialPortWindow::GetBaudRate( unsigned int serialRegVal )
{
    switch ( serialRegVal >> 24 )
    {
    case 0: return 300;
    case 1: return 600;
    case 2: return 1200;
    case 3: return 2400;
    case 4: return 4800;
    case 5: return 9600;
    case 6: return 19200;
    case 7: return 38400;
    case 8: return 57600;
    case 9: return 115200;
    case 10: return 230400;
    default: return 0;
    }
}

unsigned int SerialPortWindow::GetCharacterLength( unsigned int serialRegVal )
{
    switch( (serialRegVal>>16) & 0xFF )
    {
    case 7: return 7;
    case 8: return 8;
    default: return 0;
    }
}

unsigned int SerialPortWindow::GetParity( unsigned int serialRegVal )
{
    switch( (serialRegVal>>14) & 0x3 )
    {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    default: return 0;
    }
}

float SerialPortWindow::GetStopBit( unsigned int serialRegVal )
{
    switch( (serialRegVal>>12) & 0x3 )
    {
    case 0: return 1;
    case 1: return 1.5;
    case 2: return 2;
    default: return 0;
    }
}

unsigned int SerialPortWindow::GetBufferSize( unsigned int serialRegVal )
{
    return serialRegVal & 0xFF;
}

int SerialPortWindow::ShowErrorMessageDialog( Glib::ustring mainTxt, Error error )
{
    Gtk::MessageDialog dialog( mainTxt, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK );       
    dialog.set_secondary_text( error.GetDescription() );

    return dialog.run();	
}

int SerialPortWindow::ShowErrorMessageDialog( Glib::ustring mainTxt, Glib::ustring secondaryTxt )
{
    Gtk::MessageDialog dialog( mainTxt, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK );       
    dialog.set_secondary_text( secondaryTxt );

    return dialog.run();	
}
