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
// $Id: CamSelection.cpp,v 1.71 2010/04/13 21:35:37 hirokim Exp $
//=============================================================================

#include "Precompiled.h"
#include "FlyCapture2GUI.h"
#include "CamSelection.h"
#include "GladeFileUtil.h"

namespace FlyCapture2
{
    const char* CamSelection::sk_camSelectionWindow = "CamSelectionWin";

    const char* CamSelection::sk_buttonOk = "buttonOk";
    const char* CamSelection::sk_buttonCancel = "buttonCancel";
    const char* CamSelection::sk_buttonConfigure = "buttonConfigure";
    const char* CamSelection::sk_buttonRefresh = "buttonRefresh";

    const char* CamSelection::sk_treeViewCamList = "treeviewCamList";	

    const char* CamSelection::sk_labelNumCameras = "lblNumCameras";

    const char* CamSelection::sk_labelSerial = "lblCamSlnSerialNum";
    const char* CamSelection::sk_labelModel = "lblCamSlnModel";
    const char* CamSelection::sk_labelVendor = "lblCamSlnVendor";
    const char* CamSelection::sk_labelSensor = "lblCamSlnSensor";
    const char* CamSelection::sk_labelResolution = "lblCamSlnResolution";
    const char* CamSelection::sk_labelInterface = "lblCamSlnInterface";
    const char* CamSelection::sk_labelBusSpeed = "lblCamSlnBusSpeed";
    const char* CamSelection::sk_labelDCAMVer = "lblCamSlnDCAMVer";
    const char* CamSelection::sk_labelFirmwareVer = "lblCamSlnFirmwareVersion";
    const char* CamSelection::sk_labelFirmwareBuildTime = "lblCamSlnFirmwareBuildTime";

    const char* CamSelection::sk_labelGigEVersion = "lblCamSlnGigEVersion";
    const char* CamSelection::sk_labelGigEUserDefinedName = "lblCamSlnGigEUserDefinedName";
    const char* CamSelection::sk_labelGigEXmlUrl1 = "lblCamSlnGigEXmlUrl1";
    const char* CamSelection::sk_labelGigEXmlUrl2 = "lblCamSlnGigEXmlUrl2";
    const char* CamSelection::sk_labelGigEMacAddress = "lblCamSlnGigEMacAddress";
    const char* CamSelection::sk_labelGigEIpAddress = "lblCamSlnGigEIpAddress";
    const char* CamSelection::sk_labelGigESubnetMask = "lblCamSlnGigESubnetMask";
    const char* CamSelection::sk_labelGigEDefaultGateway = "lblCamSlnGigEDefaultGateway";
    const char* CamSelection::sk_checkGigEIpLLA = "checkCamSlnIpLLA";
    const char* CamSelection::sk_checkGigEIpDHCP = "checkCamSlnIpDHCP";
    const char* CamSelection::sk_checkGigEIpPersistentIp = "checkCamSlnIpPersistentIp";

    const char* CamSelection::sk_expanderGigE = "expanderGigEInfo";
    
    CamSelection::CamSelection() :
        m_refXml(0),
        m_pDialog(0),
        m_pButtonOk(0),
        m_pButtonCancel(0),
        m_pButtonConfigure(0),
        m_pTreeViewCamList(0),
        m_pLabelNumCameras(0),
        m_pLabelSerial(0),
        m_pLabelModel(0),
        m_pLabelVendor(0),
        m_pLabelSensor(0),
        m_pLabelResolution(0),
        m_pLabelInterface(0),
        m_pLabelBusSpeed(0),
        m_pLabelDCAMVer(0),
        m_pLabelFirmwareVer(0),
        m_pLabelFirmwareBuildTime(0),
        m_guidArray(0),
        m_psize(0),
        m_response(Gtk::RESPONSE_NONE)
    {             
        m_pKit = Gtk::Main::instance();
        if ( m_pKit == NULL )
        {
            m_pKit = new Gtk::Main( 0, NULL );
        }

        // Initialize the thread system
        if( !Glib::thread_supported() ) 
        {
            Glib::thread_init();
        }                


        m_pBusEventDispatcher = NULL;

        // Create the list store
        m_refListStoreCamList = Gtk::ListStore::create( m_camListColumns );
    }

    CamSelection::~CamSelection()
    {                
        if ( m_pBusEventDispatcher != NULL )
        {
            delete m_pBusEventDispatcher;
            m_pBusEventDispatcher = NULL;
        }

        if ( m_pDialog != NULL )
        {
            delete m_pDialog;
            m_pDialog = NULL;
        }
    }    

    bool CamSelection::Initialize()
    {
        Glib::ustring gladePath = GladeFileUtil::GetGladeFilePath();

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

        m_refXml->get_widget( sk_camSelectionWindow, m_pDialog );
        if ( m_pDialog == NULL )
        {
            return false;
        }			              

        // Get the widgets
        GetWidgets();      

        // Disable the IP configuration checkboxes since they are read only
        m_pCheckGigEIpLLA->set_sensitive( false );
        m_pCheckGigEIpDHCP->set_sensitive( false );
        m_pCheckGigEIpPersistentIp->set_sensitive( false );

        // Attach the signals
        AttachSignals();        

        // Set the tree view model
        m_pTreeViewCamList->set_model( m_refListStoreCamList );        

        // Create column headers
        PrepareTreeView();                

        // Load the PGR icon
        LoadPGRIcon();

        // Append the current version to the title
        FC2Version version;
        Utilities::GetLibraryVersion( &version );

        char titleStr[128];
        sprintf( 
            titleStr,
            "FlyCapture2 Camera Selection %u.%u.%u.%u",
            version.major,
            version.minor,
            version.type,
            version.build );

        m_pDialog->set_title( titleStr );

        return true;
    }

    void CamSelection::ShowModal( bool* okSelected, PGRGuid* guidArray, unsigned int* size )
    {
        // Populate the tree view
        PopulateTreeView();		

        m_guidArray = guidArray;
        m_psize = size;

        Error error;
        CallbackHandle callbackHandleBusReset;

        // Register callback for bus resets                
        error = m_busMgr.RegisterCallback(&CamSelection::OnBusReset, BUS_RESET, this, &callbackHandleBusReset );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error registering callback\n", error );
        }
        
        // Create a dispatcher if there is none
        if ( m_pBusEventDispatcher == NULL )
        {
            m_pBusEventDispatcher = new Glib::Dispatcher();
        }  

        m_pBusEventDispatcher->connect( sigc::mem_fun(*this, &CamSelection::PopulateTreeView ) );
        m_pBusEventDispatcher->connect( sigc::mem_fun(*this, &CamSelection::CloseAllDialogs ) );

        m_pKit->run( *m_pDialog );

        // Unregister callback for bus resets
        error = m_busMgr.UnregisterCallback( callbackHandleBusReset );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error unregistering callback\n", error );
        }

        // When we get here, either Ok or Cancel was clicked
        switch (m_response)
        {
        case Gtk::RESPONSE_OK:
            *okSelected = true;
            break;
        case Gtk::RESPONSE_CANCEL:
        case Gtk::RESPONSE_NONE:
            *okSelected = false;
            break;		
        default:
            *okSelected = false;
            break;
        }

        m_guidArray = 0; // letting go of weak pointer
    }

    void CamSelection::GetWidgets()
    {		
        m_refXml->get_widget( sk_buttonOk, m_pButtonOk );
        m_refXml->get_widget( sk_buttonCancel, m_pButtonCancel );
        m_refXml->get_widget( sk_buttonConfigure, m_pButtonConfigure );
        m_refXml->get_widget( sk_buttonRefresh, m_pButtonRefresh );

        m_refXml->get_widget( sk_treeViewCamList, m_pTreeViewCamList );

        m_refXml->get_widget( sk_labelNumCameras, m_pLabelNumCameras );

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

        m_refXml->get_widget( sk_expanderGigE, m_pExpanderGigE );
    }

    void CamSelection::AttachSignals()
    {
        m_pButtonOk->signal_clicked().connect( sigc::mem_fun(*this, &CamSelection::OnOk) );
        m_pButtonCancel->signal_clicked().connect( sigc::mem_fun(*this, &CamSelection::OnCancel) );
        m_pButtonConfigure->signal_clicked().connect( sigc::mem_fun(*this, &CamSelection::OnConfigure) );
        m_pButtonRefresh->signal_clicked().connect( sigc::mem_fun(*this, &CamSelection::OnRefresh) );

        m_pTreeViewCamList->signal_row_activated().connect( sigc::mem_fun(*this, &CamSelection::OnTreeViewSelect) );

        Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_pTreeViewCamList->get_selection();
        refTreeSelection->signal_changed().connect( sigc::mem_fun(*this, &CamSelection::OnTreeViewSelectChanged) );

        m_pExpanderGigE->property_expanded().signal_changed().connect( sigc::mem_fun(*this, &CamSelection::OnExpanderGigEClicked) );
    }

    void CamSelection::OnOk()
    {              
        *m_psize = 0;

        Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_pTreeViewCamList->get_selection();        

        refTreeSelection->selected_foreach_iter(
            sigc::mem_fun(*this, &CamSelection::OnOkCallback));

        m_response = Gtk::RESPONSE_OK;
        m_pDialog->hide();

        CloseAllDialogs();

    }

    void CamSelection::OnCancel()
    {
        *m_psize = 0;

        m_response = Gtk::RESPONSE_CANCEL;
        m_pDialog->hide();

        CloseAllDialogs();
    }

    void CamSelection::OnConfigure()
    {			
        Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_pTreeViewCamList->get_selection();

        refTreeSelection->selected_foreach_iter(
            sigc::mem_fun(*this, &CamSelection::OnConfigureCallback));        
    }

    void CamSelection::OnRefresh()
    {
        Error error = m_busMgr.RescanBus();
        if (error != PGRERROR_OK)
        {
            ShowErrorMessageDialog( "Error rescanning bus\n", error );
            return;
        }

        PopulateTreeView();
    }

    void CamSelection::OnTreeViewSelect( 
        const Gtk::TreeModel::Path& /*path*/, 
        Gtk::TreeViewColumn* /*column*/ )
    {
        // This function is called when a double click (or enter) is received 
        OnOk();
    }

    void CamSelection::OnTreeViewSelectChanged()
    {
        // This function is called when the selection changes in the tree view		
        Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_pTreeViewCamList->get_selection();

        refTreeSelection->selected_foreach_iter(
            sigc::mem_fun(*this, &CamSelection::TreeViewSelectChangedCallback));               
    }

    void CamSelection::OnOkCallback( const Gtk::TreeModel::iterator& iter )
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring serialNum = row[m_camListColumns.m_colSerialNum];
        unsigned int serial = strtoul( serialNum.c_str(), NULL, 10 );

        PGRGuid guid;
        Error error = m_busMgr.GetCameraFromSerialNumber( serial, &guid );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error getting camera\n", error );
            return;
        }

        m_guidArray[*m_psize] = guid;
        (*m_psize)++;            
    }

    void CamSelection::OnConfigureCallback( const Gtk::TreeModel::iterator& iter )
    {              
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring serialNum = row[m_camListColumns.m_colSerialNum];
        unsigned int serial = strtoul( serialNum.c_str(), NULL, 10 );

        // Check if the camera control dialog already exists
        std::map<unsigned int, SelectionStruct>::iterator iterSelection;
        iterSelection = m_mapOnConfigureStruct.find(serial);
        if ( iterSelection == m_mapOnConfigureStruct.end() )
        {
            // Camera currently does not have a camera control dialog connected to it

            Error error;
            PGRGuid guid;
            error = m_busMgr.GetCameraFromSerialNumber( serial, &guid );
            if ( error != PGRERROR_OK )
            {
                ShowErrorMessageDialog( "Error getting camera\n", error );
                return;
            }        

            InterfaceType interfaceType;
            error = m_busMgr.GetInterfaceTypeFromGuid( &guid, &interfaceType );
            if ( error != PGRERROR_OK )
            {
                ShowErrorMessageDialog( "Error getting interface type\n", error );
                return;
            }  

            CameraBase* pCamera = NULL;
            if ( interfaceType == INTERFACE_GIGE )
            {
                pCamera = new GigECamera;
            }
            else
            {
                pCamera = new Camera;
            }

            error = pCamera->Connect( &guid );
            if ( error != PGRERROR_OK )
            {
                ShowErrorMessageDialog( "Error connecting to camera\n", error );
                delete pCamera;
                return;
            }

            CameraControlDlg* pCamCtl = new CameraControlDlg;
            pCamCtl->Connect( pCamera );
            pCamCtl->Show();

            SelectionStruct selectStruct;
            selectStruct.pCamera = pCamera;
            selectStruct.pCamCtlDlg = pCamCtl;        

            m_mapOnConfigureStruct.insert( 
                std::pair<unsigned int, SelectionStruct>( serial, selectStruct) );
        }
        else
        {
            CameraControlDlg* pCamCtlDlg = iterSelection->second.pCamCtlDlg;
            pCamCtlDlg->Show();
        }        
    }

    void CamSelection::TreeViewSelectChangedCallback( const Gtk::TreeModel::iterator& iter )
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring serialNum = row[m_camListColumns.m_colSerialNum];
        unsigned int serial = strtoul( serialNum.c_str(), NULL, 10 );

        PGRGuid guid;
        Error error = m_busMgr.GetCameraFromSerialNumber( serial, &guid );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error getting camera\n", error );
            return;
        }

        SetCameraInformation( guid );	
    }

    void CamSelection::PrepareTreeView()
    {
        m_pTreeViewCamList->set_headers_clickable( true );

        m_pTreeViewCamList->append_column( "Serial #", m_camListColumns.m_colSerialNum );
        m_pTreeViewCamList->append_column( "Model", m_camListColumns.m_colModel );
        m_pTreeViewCamList->append_column( "Interface", m_camListColumns.m_colInterface ); 
        m_pTreeViewCamList->append_column( "IP Address", m_camListColumns.m_colIpAddress );

        Gtk::TreeView::Column* serialCol = m_pTreeViewCamList->get_column(0);
        serialCol->set_sort_column(m_camListColumns.m_colSerialNum);
        serialCol->set_expand( false );

        Gtk::TreeView::Column* modelCol = m_pTreeViewCamList->get_column(1);
        modelCol->set_sort_column(m_camListColumns.m_colModel);
        modelCol->set_expand( true );

        Gtk::TreeView::Column* interfaceCol = m_pTreeViewCamList->get_column(2);
        interfaceCol->set_sort_column(m_camListColumns.m_colInterface);
        interfaceCol->set_expand( false );

        Gtk::TreeView::Column* ipAddressCol = m_pTreeViewCamList->get_column(3);
        ipAddressCol->set_sort_column(m_camListColumns.m_colIpAddress);
        ipAddressCol->set_expand( false );
    }

    void CamSelection::PopulateTreeView()
    {        
        // Clear the camera list
        m_refListStoreCamList->clear();

        Error error;
        unsigned int numCameras = 0;
        error = m_busMgr.GetNumOfCameras( &numCameras );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error getting number of cameras\n", error );
            return;
        }
                
        char numCamerasString[64];
        sprintf( numCamerasString, "<b>(%u cameras detected)</b>", numCameras );			
        m_pLabelNumCameras->set_markup( numCamerasString );

        if ( numCameras == 0 )
        {            
            ClearCameraInformation();
            return;
        }

        for ( unsigned int i = 0; i < numCameras; i++ )
        {
            PGRGuid guid;			
            error = m_busMgr.GetCameraFromIndex( i, &guid );
            if ( error != PGRERROR_OK )
            {
                ShowErrorMessageDialog( "Error getting camera\n", error );
                continue;
            }

            InterfaceType currInterface;
            error = m_busMgr.GetInterfaceTypeFromGuid( &guid, &currInterface );
            if ( error != PGRERROR_OK )
            {
                ShowErrorMessageDialog( "Error getting camera interface\n", error );
                continue;
            }

            CameraBase* pCamera;
            if ( currInterface == INTERFACE_GIGE )
            {
                pCamera = new GigECamera();
            }
            else
            {
                pCamera = new Camera();
            }
		
            error = pCamera->Connect( &guid );
            if ( error != PGRERROR_OK )
            {
                ShowErrorMessageDialog( "Error connecting to camera\n", error );
                delete pCamera;
                continue;
            }

            CameraInfo camInfo;
            error = pCamera->GetCameraInfo( &camInfo );		
            if ( error != PGRERROR_OK )
            {
                ShowErrorMessageDialog( "Error getting camera information\n", error );
                delete pCamera;
                continue;
            }

            // Append the camera to the list
            Gtk::TreeModel::Row row = *(m_refListStoreCamList->append());

            char serialString[16];
            sprintf( serialString, "%u", camInfo.serialNumber );

            const char* interfaceString;
            interfaceString = GetInterfaceString( camInfo.interfaceType );

            char ipAddressString[32] = {0};

            if ( camInfo.ipAddress.octets[0] == 0 &&
                camInfo.ipAddress.octets[1] == 0 && 
                camInfo.ipAddress.octets[2] == 0 &&
                camInfo.ipAddress.octets[3] == 0 )
            {
                sprintf( ipAddressString, "N/A" );
            }     
            else
            {
                sprintf( 
                    ipAddressString, 
                    "%u.%u.%u.%u", 
                    camInfo.ipAddress.octets[0],
                    camInfo.ipAddress.octets[1],
                    camInfo.ipAddress.octets[2],
                    camInfo.ipAddress.octets[3]);
            }

            row[m_camListColumns.m_colSerialNum] = serialString;
            row[m_camListColumns.m_colModel] = camInfo.modelName;
            row[m_camListColumns.m_colInterface] = interfaceString;
            row[m_camListColumns.m_colIpAddress] = ipAddressString;

            // Set the camera info to the first camera
            if ( i == 0 )
            {
                SetCameraInformation( guid );                
                m_pTreeViewCamList->get_selection()->select( row );
            }		

            delete pCamera;
        }	

        m_pTreeViewCamList->get_selection()->set_mode( Gtk::SELECTION_MULTIPLE );        
    }	

    void CamSelection::SetCameraInformation( PGRGuid guid )
    {
        Error error;

        InterfaceType interfaceType;
        error = m_busMgr.GetInterfaceTypeFromGuid( &guid, &interfaceType );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error getting camera interface\n", error );
            return;
        }

        CameraBase* pCamera;
        if ( interfaceType == INTERFACE_GIGE )
        {
            pCamera = new GigECamera();
        }
        else
        {
            pCamera = new Camera();
        }
		
        error = pCamera->Connect( &guid );
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error connecting to camera\n", error );
            delete pCamera;
            return;
        }

        CameraInfo camInfo;
        error = pCamera->GetCameraInfo( &camInfo );		
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error getting camera information\n", error );
            delete pCamera;
            return;
        }                

        // Set the camera info labels
        char serial[16];
        sprintf( serial, "%u", camInfo.serialNumber );

        const char* interfaceString;
        interfaceString = GetInterfaceString( camInfo.interfaceType );

        const char* busSpeedString;
        busSpeedString = GetBusSpeedString( camInfo.maximumBusSpeed );

        char dcamVer[16];
        sprintf( dcamVer, "%1.2f", camInfo.iidcVer / 100.0f );

        m_pLabelSerial->set_text( serial );
        m_pLabelModel->set_text( camInfo.modelName );
        m_pLabelVendor->set_text( camInfo.vendorName );
        m_pLabelSensor->set_text( camInfo.sensorInfo );
        m_pLabelResolution->set_text( camInfo.sensorResolution );        
        m_pLabelInterface->set_text( interfaceString );
        m_pLabelBusSpeed->set_text( busSpeedString );
        m_pLabelDCAMVer->set_text( dcamVer );        
        m_pLabelFirmwareVer->set_text( camInfo.firmwareVersion );
        m_pLabelFirmwareBuildTime->set_text( camInfo.firmwareBuildTime );     

        if ( camInfo.interfaceType == INTERFACE_GIGE )
        {
            m_pExpanderGigE->set_expanded(true);

            char gigEVersion[16];
            sprintf( gigEVersion, "%u.%u", camInfo.gigEMajorVersion, camInfo.gigEMinorVersion );
            m_pLabelGigEVersion->set_text( gigEVersion );

            m_pLabelGigEUserDefinedName->set_text( camInfo.userDefinedName );
            m_pLabelGigEXmlUrl1->set_text( camInfo.xmlURL1 );
            m_pLabelGigEXmlUrl2->set_text( camInfo.xmlURL2 );

            char macAddress[64];
            sprintf( 
                macAddress, 
                "%02X:%02X:%02X:%02X:%02X:%02X", 
                camInfo.macAddress.octets[0],
                camInfo.macAddress.octets[1],
                camInfo.macAddress.octets[2],
                camInfo.macAddress.octets[3],
                camInfo.macAddress.octets[4],
                camInfo.macAddress.octets[5]);
            m_pLabelGigEMacAddress->set_text( macAddress );

            char ipAddress[32];
            sprintf( 
                ipAddress, 
                "%u.%u.%u.%u", 
                camInfo.ipAddress.octets[0],
                camInfo.ipAddress.octets[1],
                camInfo.ipAddress.octets[2],
                camInfo.ipAddress.octets[3]);
            m_pLabelGigEIpAddress->set_text( ipAddress );

            char subnetMask[32];
            sprintf( 
                subnetMask, 
                "%u.%u.%u.%u", 
                camInfo.subnetMask.octets[0],
                camInfo.subnetMask.octets[1],
                camInfo.subnetMask.octets[2],
                camInfo.subnetMask.octets[3]);
            m_pLabelGigESubnetMask->set_text( subnetMask );

            char defaultGateway[32];
            sprintf( 
                defaultGateway, 
                "%u.%u.%u.%u", 
                camInfo.defaultGateway.octets[0],
                camInfo.defaultGateway.octets[1],
                camInfo.defaultGateway.octets[2],
                camInfo.defaultGateway.octets[3]);
            m_pLabelGigEDefaultGateway->set_text( defaultGateway );

            GigECamera* pGigECamera = dynamic_cast<GigECamera*>(pCamera);
            if ( pCamera != NULL )
            {
                unsigned int ipConfigurationVal = 0;
                Error error = pGigECamera->ReadGVCPRegister( 0x0010, &ipConfigurationVal );
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
            m_pExpanderGigE->set_expanded(false);

            m_pLabelGigEVersion->set_text( "N/A" );
            m_pLabelGigEUserDefinedName->set_text( "N/A" );
            m_pLabelGigEXmlUrl1->set_text( "N/A" );
            m_pLabelGigEXmlUrl2->set_text( "N/A" );
            m_pLabelGigEMacAddress->set_text( "N/A" );
            m_pLabelGigEIpAddress->set_text( "N/A" );
            m_pLabelGigESubnetMask->set_text( "N/A" );
            m_pLabelGigEDefaultGateway->set_text( "N/A" );

            m_pCheckGigEIpLLA->set_active( false );
            m_pCheckGigEIpDHCP->set_active( false );
            m_pCheckGigEIpPersistentIp->set_active( false );
        }           

        error = pCamera->Disconnect();
        if ( error != PGRERROR_OK )
        {
            ShowErrorMessageDialog( "Error getting camera information\n", error );
            delete pCamera;
            return;
        }

        delete pCamera;
    }

    void CamSelection::ClearCameraInformation()
    {
        const char* blank = "N/A";

        m_pLabelSerial->set_text( blank );
        m_pLabelModel->set_text( blank );
        m_pLabelVendor->set_text( blank );
        m_pLabelSensor->set_text( blank );
        m_pLabelResolution->set_text( blank );        
        m_pLabelInterface->set_text( blank );
        m_pLabelBusSpeed->set_text( blank );
        m_pLabelDCAMVer->set_text( blank );
        m_pLabelFirmwareVer->set_text( blank );
        m_pLabelFirmwareBuildTime->set_text( blank );
    }

    void CamSelection::OnBusReset( void* pParameter, unsigned int serialNumber )
    {        
        static_cast<CamSelection*>(pParameter)->m_pBusEventDispatcher->emit();
    }  

    void CamSelection::LoadPGRIcon()
    {
        m_iconPixBuf = Gdk::Pixbuf::create_from_inline( sizeof(PGRIcon), PGRIcon, false );
        m_pDialog->set_default_icon( m_iconPixBuf );
    }

    const char* CamSelection::GetInterfaceString( InterfaceType type )
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

    const char* CamSelection::GetBusSpeedString( BusSpeed speed )
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

    int CamSelection::ShowErrorMessageDialog( 
        Glib::ustring mainTxt, 
        Glib::ustring secondaryTxt )
    {
        Gtk::MessageDialog dialog( mainTxt, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK );       
        dialog.set_secondary_text( secondaryTxt );

        return dialog.run();	
    }

    int CamSelection::ShowErrorMessageDialog( 
        Glib::ustring mainTxt, 
        Error error )
    {
        Gtk::MessageDialog dialog( mainTxt, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK );       
        dialog.set_secondary_text( error.GetDescription() );

        return dialog.run();	
    }

    void CamSelection::CloseAllDialogs()
    {
        std::map<unsigned int, SelectionStruct>::iterator iterSelection;
        for ( iterSelection = m_mapOnConfigureStruct.begin(); iterSelection != m_mapOnConfigureStruct.end(); iterSelection++ )
        {
            SelectionStruct tempStruct = iterSelection->second;
            CameraBase* pCamera = tempStruct.pCamera;
            CameraControlDlg* pCamCtlDlg = tempStruct.pCamCtlDlg;

            pCamCtlDlg->Hide();
            pCamCtlDlg->Disconnect();
            delete pCamCtlDlg;

            pCamera->Disconnect();
            delete pCamera;
        }

        m_mapOnConfigureStruct.clear();
    }

    void CamSelection::OnExpanderGigEClicked()
    {
        if ( m_pExpanderGigE->get_expanded() != true )
        {
            //m_pDialog->resize(1,1);
            //m_pDialog->queue_resize();
        } 
    }
}
