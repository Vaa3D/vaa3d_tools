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
// $Id: BusTopologyPage.h,v 1.11 2010/06/26 00:49:33 warrenm Exp $
//=============================================================================

#ifndef _BUSTOPOLOGYPAGE_H_
#define _BUSTOPOLOGYPAGE_H_

#include "BasePage.h"
#include "BusTopologyDrawingArea.h"
#include "PhyRegisterDefs.h"

namespace FlyCapture2
{
	class BusTopologyPage : public BasePage 
	{
	public:
		BusTopologyPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );
		~BusTopologyPage();
			
		void UpdateWidgets();

	protected:
        static const char* sk_drawingAreaTopology;
        static const char* sk_buttonRefreshTopology;

        Gtk::Frame* m_pFramePhyRegisters;

        Gtk::TreeView* m_pNodeList;
        Gtk::TreeView* m_pRegisterList;

        Gtk::ScrolledWindow* m_pPhyScrolledWindowNode;
		Gtk::ScrolledWindow* m_pPhyScrolledWindowRegister;

        Glib::RefPtr<Gtk::TreeStore> m_refNodeModel;
        Glib::RefPtr<Gtk::ListStore> m_refRegisterModel;
        Glib::RefPtr<Gtk::TreeSelection> m_refTreeSelection;

        Gtk::ToggleButton* m_pTogglePhyRegisters;

        // node column consists of string for node name
        Gtk::TreeModel::ColumnRecord m_nodeColumnRecord;        
        Gtk::TreeModelColumn<Glib::ustring> m_nodeColumn;

        // register columns consists of string for register name and int for value
        Gtk::TreeModel::ColumnRecord m_registerColumnRecord;
        Gtk::TreeModelColumn<Glib::ustring> m_registerColumn;
        Gtk::TreeModelColumn<unsigned int> m_valueColumn;

        static const char* sk_phyScrolledWindowRegister;
	    static const char* sk_phyScrolledWindowNode;
   	
        static const char* sk_nodeList;
        static const char* sk_registerList;

        static const Glib::ustring sk_nodeColumnName;
        static const Glib::ustring sk_registerColumnName;
        static const Glib::ustring sk_valueColumnName;

        static const Glib::ustring sk_framePhyRegisters;
        static const Glib::ustring sk_toggleButtonName;

        NodeDesc m_nodeDesc;


      
        static const char* sk_labelBusTopologyNodeName;
        static const char* sk_labelBusTopologyNodePID;
        static const char* sk_labelBusTopologyNodeVoltages;
		static const char* sk_labelBusTopologyNodeCurrents;

        static const char* sk_eventBoxLegendPC;
        static const char* sk_eventBoxLegendBus;
        static const char* sk_eventBoxLegendNode;
        static const char* sk_eventBoxLegendCamera;

        Gtk::Label* m_pLabelBusTopologyNodeName;
        Gtk::Label* m_pLabelBusTopologyNodePID;
        Gtk::Label* m_pLabelBusTopologyNodeVoltages;
		Gtk::Label* m_pLabelBusTopologyNodeCurrents;

        Gtk::EventBox* m_pEventBoxLegendPC;
        Gtk::EventBox* m_pEventBoxLegendBus;
        Gtk::EventBox* m_pEventBoxLegendNode;
        Gtk::EventBox* m_pEventBoxLegendCamera;

        Gdk::Color m_colorPC;
        Gdk::Color m_colorBus;
        Gdk::Color m_colorNode;
        Gdk::Color m_colorCamera;

        Gtk::Button* m_pButtonRefreshTopology;     

        BusManager m_busMgr;

        /** The first node in the topology (i.e. the PC). */
        TopologyNode m_baseNode;

        /** The bus topology drawing area where the topology is rendered. */
        BusTopologyDrawingArea* m_pDrawingArea;     

        BusTopologyPage();
        BusTopologyPage( const BusTopologyPage& );
        
        BusTopologyPage& operator=( const BusTopologyPage& );

		void GetWidgets();
		void AttachSignals();	

        void OnRefreshTopology();
        void OnPhyNodeChanged();
        void resetRegisterPane();

        /**
         * Event handler when the selected node changes. This event is only
         * fired when a valid node is clicked on (i.e. not whitespace).
         *
         * @param guid PGRGuid of the newly selected node.
         */
        void OnNodeSelectionChanged( PGRGuid guid );

        /**
         * Given a guid, update the phy register pane for that
         * device
         *
         * @param guid The PGRGUid of the device to use,
         *
         */
        void updatePhyRegisters(PGRGuid guid);

        /**
         * Given a guid, returns the vendor name and vendorID for the device
         *
         * @param guid The PGRGUid of the device to use,
         * @param vendorID VendorD and name are returned in this parameter
         *
         */
        bool getVendorID(PGRGuid guid, Field* vendorID);

        /**
         * Given a guid, returns the number of phy ports for the device
         *
         * @param guid The PGRGUid of the device to use,
         * @param numPorts The number of ports is returned in this parameter
         *
         */
        void getNumberOfPorts(PGRGuid guid, unsigned int* numPorts);

        /**
         * Returns field value for a given register
         *
         * @param Register The Register that the field and value correspond to 
         * @param regVal The value read from the register
         * @param fieldVal The value of the extracted field from regVal
         */
        void getField(const Register* field, unsigned int* regVal, unsigned int* fieldVal);
        
        /**
         * Returns guid and port of the selected entry in the treeview node list
         *
         * @param strText The path of teh selected entry
         * @param guid The device guid is returned in this parameter
         * @param port The selected node port is returned in this parameter
         */
        bool getNodeFromPath(Glib::ustring* strText, PGRGuid* guid, unsigned int* port);
        
        /**
         * Populates the phy register page 
         *
         * @param guid The guid of the slected device
         * @param port The selected port to populate
         */
        void populateRegisterPage(PGRGuid* guid, unsigned int port);
        
        /**
         * Event handler for phy register toggle button
         *
         */
        void OnTogglePhyRegisters();

        /**
         * Determines whether any interfaces on the system are 
         * using the firepro driver
         *
         */
        bool isFireProPresent();

        /**
         * Given a guid, generate a node name string.
         *
         * @param guid The PGRGUid to use,
         *
         * @return A string containing the node name.
         */
        Glib::ustring GenerateNodeName( PGRGuid guid );

        /**
         * Given a guid, generate a product id string.
         *
         * @param guid The PGRGUid to use,
         *
         * @return A string containing the product id.
         */
        Glib::ustring GenerateProductId( PGRGuid guid );

        /**
         * Given a guid, generate a voltage string.
         *
         * @param guid The PGRGUid to use,
         * 
         * @return A string containing the voltages.
         */
        Glib::ustring GenerateVoltageStr( PGRGuid guid );

        /**
         * Given a guid, generate a current string.
         *
         * @param guid The PGRGUid to use,
         * 
         * @return A string containing the currents.
         */
        Glib::ustring GenerateCurrentStr( PGRGuid guid );

        /** Set the colors to be used in the legend. */
        void SetColors();

	private:
	};
}

#endif // _BUSTOPOLOGYPAGE_H_
