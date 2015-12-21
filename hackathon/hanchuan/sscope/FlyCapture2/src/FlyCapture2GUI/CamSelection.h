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
// $Id: CamSelection.h,v 1.33 2010/03/18 21:46:12 mgara Exp $
//=============================================================================

#ifndef CAMSELECTION_H_INCLUDED
#define CAMSELECTION_H_INCLUDED

#include <map>

namespace FlyCapture2
{      	  
    class CameraControlDlg;

    /** Model for camera list. */
	class CamListColumns : public Gtk::TreeModelColumnRecord
	{
	public:
        /** Serial number of camera. */
	  	Gtk::TreeModelColumn<Glib::ustring> m_colSerialNum;

        /** Model name of camera. */
	  	Gtk::TreeModelColumn<Glib::ustring> m_colModel;

        /** Interface type of camera. */
		Gtk::TreeModelColumn<Glib::ustring> m_colInterface;

        /** IP address of camera (if applicable). */
        Gtk::TreeModelColumn<Glib::ustring> m_colIpAddress;
			
        /** Constructor. */
		CamListColumns()
		{ 
			add( m_colSerialNum ); 
			add( m_colModel ); 
			add( m_colInterface );
            add( m_colIpAddress );
		}							 
	};
	
	class CamSelection
	{		
	public:
        /** Constructor. */
		CamSelection();

        /** Destructor. */
		virtual ~CamSelection();				      

        /**
         * Bus reset event handler.
         *
         * @param pParameter Parameter passed in RegisterCallback().
         */
        static void OnBusReset( void* pParameter, unsigned int serialNumber );

		/**
		 * Initialize the dialog by getting widgets, attaching widgets ec.
		 *
		 * @return bool Whether the initialization was successful.
		 */
		bool Initialize();

		/**
		 * Show the dialog. This function call blocks until a response is
         * received from the dialog.
		 *
		 * @param okSelected Whether Ok was selected.
		 * @param guidArray Array of PGRGuids containing selected cameras.
		 * @param size Size of array.
		 */
		void ShowModal( bool* okSelected, PGRGuid* guidArray, unsigned int* size );				

	private:
        /** Structure to hold camera with their matching camera control dialogs. */ 
        struct SelectionStruct
        {        
            CameraBase* pCamera;
            CameraControlDlg* pCamCtlDlg;
        };

        static const char* sk_camSelectionWindow;

        static const char* sk_buttonOk;
        static const char* sk_buttonCancel;
        static const char* sk_buttonConfigure;
        static const char* sk_buttonRefresh;

        static const char* sk_treeViewCamList;

        static const char* sk_labelNumCameras;

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

        static const char* sk_expanderGigE;

        Gtk::Main* m_pKit;

		Glib::RefPtr<Gnome::Glade::Xml> m_refXml;
		Gtk::Dialog* m_pDialog;
			
		Gtk::Button* m_pButtonOk;
		Gtk::Button* m_pButtonCancel;
		Gtk::Button* m_pButtonConfigure;
        Gtk::Button* m_pButtonRefresh;
		
		Gtk::TreeView* m_pTreeViewCamList;
			
		Gtk::Label* m_pLabelNumCameras;
			
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

        Gtk::Expander* m_pExpanderGigE;
			
		Glib::RefPtr<Gtk::ListStore> m_refListStoreCamList;

        Glib::Dispatcher* m_pBusEventDispatcher;
			
		CamListColumns m_camListColumns;

        PGRGuid* m_guidArray;
        unsigned int* m_psize;
			
		Gtk::ResponseType m_response;     

        Glib::RefPtr<Gdk::Pixbuf> m_iconPixBuf;

        std::map<unsigned int, SelectionStruct> m_mapOnConfigureStruct;

        BusManager m_busMgr;

        void OnOk();        
        void OnCancel();
        void OnConfigure();
        void OnRefresh();
        void OnTreeViewSelect( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
        void OnTreeViewSelectChanged();
        void OnExpanderGigEClicked();

        // Various tree view callbacks
        void OnOkCallback( const Gtk::TreeModel::iterator& iter );
        void OnConfigureCallback( const Gtk::TreeModel::iterator& iter );
        void TreeViewSelectChangedCallback( const Gtk::TreeModel::iterator& iter );
		
        /** Load widgets. */
        void GetWidgets();

        /** Attach signals to widgets. */
        void AttachSignals();
		
        /** Create column headers for the camera list tree view. */
		void PrepareTreeView();

        /** Populate camera list with detected cameras. */
		void PopulateTreeView();
		
		/**
		 * Set the camera information labels on dialog to the specified
         * camera information.
		 *
		 * @param pCamInfo Camera info to populate dialog with.
		 */
		void SetCameraInformation( PGRGuid guid );

        /** Remove camera information from dialog. */
        void ClearCameraInformation();

        /** Load PGR icon. */
        void LoadPGRIcon();

        /**
         * Closes all camera control dialogs that have been opened by
         * this camera selection dialog.
         */
        void CloseAllDialogs();

        /**
         * Get a string representation of the interface type.
         *
         * @param type Interface type.
         *
         * @return String representation of the interface type.
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

        static int ShowErrorMessageDialog( 
            Glib::ustring mainTxt, 
            Glib::ustring secondaryTxt );

        static int ShowErrorMessageDialog( 
            Glib::ustring mainTxt, 
            Error error );
	};	
}

#endif // CAMSELECTION_H_INCLUDED
