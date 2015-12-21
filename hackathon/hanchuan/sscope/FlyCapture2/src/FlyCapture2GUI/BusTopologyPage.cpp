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
// $Id: BusTopologyPage.cpp,v 1.15 2010/06/28 16:38:55 warrenm Exp $
//=============================================================================

#include "Precompiled.h"
#include "BusTopologyPage.h"

namespace FlyCapture2
{
    const char* BusTopologyPage::sk_drawingAreaTopology = "drawingareaTopology";
    const char* BusTopologyPage::sk_buttonRefreshTopology = "buttonRefreshTopology";
    const char* BusTopologyPage::sk_labelBusTopologyNodeName = "labelBusTopologyNodeName";
    const char* BusTopologyPage::sk_labelBusTopologyNodePID = "labelBusTopologyNodePID";
    const char* BusTopologyPage::sk_labelBusTopologyNodeVoltages = "labelBusTopologyNodeVoltages";
	const char* BusTopologyPage::sk_labelBusTopologyNodeCurrents = "labelBusTopologyNodeCurrents";
    const char* BusTopologyPage::sk_eventBoxLegendPC = "eventboxLegendPC";
    const char* BusTopologyPage::sk_eventBoxLegendBus = "eventboxLegendBus";
    const char* BusTopologyPage::sk_eventBoxLegendNode = "eventboxLegendNode";
    const char* BusTopologyPage::sk_eventBoxLegendCamera = "eventboxLegendCamera";

    const Glib::ustring BusTopologyPage::sk_toggleButtonName = "togglebuttonPhyRegisters";
    const Glib::ustring BusTopologyPage::sk_framePhyRegisters = "framePhyRegisters";
    const char* BusTopologyPage::sk_phyScrolledWindowRegister = "scrolledwindowRegister";
	const char* BusTopologyPage::sk_phyScrolledWindowNode = "scrolledwindowNode";
    const char* BusTopologyPage::sk_nodeList = "treeviewNodeList";
    const char* BusTopologyPage::sk_registerList = "treeviewRegisterList";
    const Glib::ustring BusTopologyPage::sk_nodeColumnName = "Node";
    const Glib::ustring BusTopologyPage::sk_registerColumnName = "Register";
    const Glib::ustring BusTopologyPage::sk_valueColumnName = "Value"; 
    
	BusTopologyPage::BusTopologyPage()
	{
        m_pDrawingArea = NULL;
        SetColors();      
	}
	
	BusTopologyPage::BusTopologyPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
	{
        m_pDrawingArea = NULL;
        SetColors();
	}
	
	BusTopologyPage::~BusTopologyPage()
	{
        if ( m_pDrawingArea != NULL )
        {
            delete m_pDrawingArea;
            m_pDrawingArea = NULL;
        }
	}
			
	void BusTopologyPage::UpdateWidgets()
	{       
        OnRefreshTopology();
	}
	
	void BusTopologyPage::GetWidgets()
	{
        m_refXml->get_widget_derived( sk_drawingAreaTopology, m_pDrawingArea );
        m_refXml->get_widget( sk_buttonRefreshTopology, m_pButtonRefreshTopology );

		m_refXml->get_widget( sk_nodeList, m_pNodeList );
		m_refXml->get_widget( sk_registerList, m_pRegisterList );
		m_refXml->get_widget(sk_phyScrolledWindowNode, m_pPhyScrolledWindowNode);
		m_refXml->get_widget(sk_phyScrolledWindowRegister, m_pPhyScrolledWindowRegister);
        m_refXml->get_widget( sk_toggleButtonName, m_pTogglePhyRegisters);
        m_refXml->get_widget( sk_framePhyRegisters, m_pFramePhyRegisters);

		m_pPhyScrolledWindowNode->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		m_pPhyScrolledWindowRegister->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

        m_refXml->get_widget( sk_labelBusTopologyNodeName, m_pLabelBusTopologyNodeName );
        m_refXml->get_widget( sk_labelBusTopologyNodePID, m_pLabelBusTopologyNodePID );
        m_refXml->get_widget( sk_labelBusTopologyNodeVoltages, m_pLabelBusTopologyNodeVoltages );
		m_refXml->get_widget( sk_labelBusTopologyNodeCurrents, m_pLabelBusTopologyNodeCurrents );

        // phy registers
        m_nodeColumnRecord.add( m_nodeColumn);
		m_registerColumnRecord.add( m_registerColumn );
		m_registerColumnRecord.add( m_valueColumn );
		
		// Create node and register model
		m_refNodeModel = Gtk::TreeStore::create(m_nodeColumnRecord);
		m_pNodeList->set_model( m_refNodeModel );

		m_refRegisterModel = Gtk::ListStore::create( m_registerColumnRecord );
		m_pRegisterList->set_model( m_refRegisterModel );

		m_pNodeList->append_column(sk_nodeColumnName, m_nodeColumn);
		m_pRegisterList->append_column(sk_registerColumnName, m_registerColumn);
		m_pRegisterList->append_column(sk_valueColumnName, m_valueColumn);

		m_refTreeSelection = m_pNodeList->get_selection();

        // legend area
        m_refXml->get_widget( sk_eventBoxLegendPC, m_pEventBoxLegendPC );
        m_refXml->get_widget( sk_eventBoxLegendBus, m_pEventBoxLegendBus );
        m_refXml->get_widget( sk_eventBoxLegendNode, m_pEventBoxLegendNode );
        m_refXml->get_widget( sk_eventBoxLegendCamera, m_pEventBoxLegendCamera );

        m_pEventBoxLegendPC->modify_bg( Gtk::STATE_NORMAL, m_colorPC );      
        m_pEventBoxLegendBus->modify_bg( Gtk::STATE_NORMAL, m_colorBus );      
        m_pEventBoxLegendNode->modify_bg( Gtk::STATE_NORMAL, m_colorNode );      
        m_pEventBoxLegendCamera->modify_bg( Gtk::STATE_NORMAL, m_colorCamera );   


        // reading phy registers not currently supported under linux
#if !defined(WIN32) && !defined(WIN64)
        m_pTogglePhyRegisters->hide();
#endif
	}
	
	void BusTopologyPage::AttachSignals()
	{
        m_pButtonRefreshTopology->signal_clicked().connect(
            sigc::mem_fun( *this, &BusTopologyPage::OnRefreshTopology) );

        m_pDrawingArea->signal_node_selection_changed().connect(
            sigc::mem_fun( *this, &BusTopologyPage::OnNodeSelectionChanged ) );

		m_refTreeSelection->signal_changed().connect(
            sigc::mem_fun(*this, &BusTopologyPage::OnPhyNodeChanged));
        
#if defined(WIN32) || defined(WIN64)
        m_pTogglePhyRegisters->signal_toggled().connect(
            sigc::mem_fun(*this, &BusTopologyPage::OnTogglePhyRegisters));
#endif
    }

    void BusTopologyPage::OnRefreshTopology()
    {
        Error error;
        error = m_busMgr.GetTopology( &m_baseNode );
        if ( error != PGRERROR_OK )
        {
            // Error
        }
        m_pDrawingArea->Redraw( &m_baseNode );

#if defined(WIN32)  || defined(WIN64)
        if(!isFireProPresent())
        {
            m_pTogglePhyRegisters->set_sensitive(false);
            m_pFramePhyRegisters->hide();
        }
        else
        {
            m_pTogglePhyRegisters->set_sensitive(true);
        }
#endif
    }

    bool BusTopologyPage::isFireProPresent()
    {
        // for now non-firepro cards use a guid of 0
        PGRGuid nullGuid;
        TopologyNode parentNode;
        unsigned int numChildren = m_baseNode.GetNumChildren();

        unsigned int i;
        for(i = 0; i < numChildren; i++)
        {
            parentNode = m_baseNode.GetChild(i);
            
            if(parentNode.GetInterfaceType() == INTERFACE_IEEE1394)
                break;
        }

        if(i < numChildren)
        {
            numChildren = parentNode.GetNumChildren();

            TopologyNode cardNode;
            for (i = 0; i < numChildren; i++)
            {
                cardNode = parentNode.GetChild(i);

                if(cardNode.GetGuid() != nullGuid)
                    return true;
            }
        }
        return false;
    }

    void BusTopologyPage::OnTogglePhyRegisters()
    {
        if(m_pTogglePhyRegisters->get_active())
        {
            m_pFramePhyRegisters->show();
            m_pTogglePhyRegisters->set_label("Hide Phy Registers");
        }
        else 
        {
            m_pFramePhyRegisters->hide();
            m_pTogglePhyRegisters->set_label("Show Phy Registers");
        }
    }

    void BusTopologyPage::OnNodeSelectionChanged( PGRGuid guid )
    {        
        Glib::ustring nodeName = GenerateNodeName( guid );
        m_pLabelBusTopologyNodeName->set_text( nodeName );

        Glib::ustring productId = GenerateProductId( guid );
        m_pLabelBusTopologyNodePID->set_text( productId );

        Glib::ustring voltage = GenerateVoltageStr( guid );
        m_pLabelBusTopologyNodeVoltages->set_text( voltage );

		Glib::ustring current = GenerateCurrentStr( guid );
        m_pLabelBusTopologyNodeCurrents->set_text( current );

#if defined(WIN32) || defined(WIN64)
        // update phy register page
        updatePhyRegisters(guid);
#endif
    }

    void BusTopologyPage::updatePhyRegisters(PGRGuid guid)
    {
        PGRGuid nullGuid;
        if(guid == m_nodeDesc.guid)
            return;
         
        resetRegisterPane();

        if (guid == nullGuid)
        {
            m_nodeDesc.guid = nullGuid;
            return;
        }

        Field vendorid;
		if(!getVendorID(guid, &vendorid))
        {
            m_nodeDesc.guid = nullGuid;
            resetRegisterPane();
			return; 
        }
		// construct node string
		Glib::ustring vendorName = Glib::ustring::compose("%1 Chipset", vendorid.name);

		// insert into model
		Gtk::TreeModel::Row row = *(m_refNodeModel->append());
		row[m_nodeColumn] = vendorName;

		//NodeDesc nodeDesc;
		m_nodeDesc.guid = guid;
		m_nodeDesc.name = vendorName;

		// get number of ports
		unsigned int numPorts = 0;
		getNumberOfPorts(guid, &numPorts);

		// add base register heading
		Gtk::TreeModel::Row childrow = *(m_refNodeModel->append(row.children()));
		childrow[m_nodeColumn] = "Base Registers";

		// add port headings
		char tempBuffer[64];
		for (unsigned int t = 0; t < numPorts; t++)
		{
			sprintf(tempBuffer, "Port %d", t);
			childrow = *(m_refNodeModel->append(row.children()));
			childrow[m_nodeColumn] = tempBuffer;
		}
        m_pNodeList->expand_all();
    }

    void BusTopologyPage::getField(const Register* field, unsigned int* regVal, unsigned int* fieldVal)
	{
		unsigned int shiftCount = 0;
		unsigned int tempMask = field->mask;

		while ((tempMask & 0x01) == 0)
		{
			tempMask = tempMask >> 1;
			shiftCount++;
		}
		*fieldVal = (*regVal & field->mask) >> shiftCount;
	}

	void BusTopologyPage::getNumberOfPorts(PGRGuid guid, unsigned int* numPorts)
	{
		BusManager busMgr;
		unsigned int regVal;

		// read totalPorts field
		Error error = busMgr.ReadPhyRegister(guid, k_basePage, 0, k_baseRegisterInfo[k_totalPortsField].addr, &regVal);
		getField(&(k_baseRegisterInfo[k_totalPortsField]), &regVal, numPorts);
	}

	bool BusTopologyPage::getVendorID(PGRGuid guid, Field* vendorID)
	{
		BusManager busMgr;
		unsigned int regVal = 0;
		unsigned int vendorVal = 0;

		// read vendor id registers
		for (int i = 10; i <= 12; i++)
		{
			Error error = busMgr.ReadPhyRegister(guid, k_vendorPage, 0, i, &regVal);
			if(error != PGRERROR_OK)
			{
				return false;
			}
			else
			{
				vendorVal = (vendorVal << 8) | regVal;
			}
		}

		unsigned int count = 0;

		// lookup vendor id in string table
		while (k_vendorList[count].id != 0)
		{
			if (vendorVal == k_vendorList[count].id)
				break;
			count++;
		}

		vendorID->id = vendorVal;
		vendorID->name = k_vendorList[count].name;
		return true;
	}

    void BusTopologyPage::resetRegisterPane()
	{
        m_refNodeModel->clear();
		m_refRegisterModel->clear();
	}

	void BusTopologyPage::OnPhyNodeChanged()
	{
		std::vector<Gtk::TreeModel::Path> path = m_refTreeSelection->get_selected_rows();

		if(path.empty())
			return;

		std::vector<Gtk::TreeModel::Path>::iterator pathIter;
		pathIter = path.begin();
		Glib::ustring strText = (*pathIter).to_string();

        m_refRegisterModel->clear();

		PGRGuid guid;
		unsigned int port;
		
		if(getNodeFromPath(&strText, &guid, &port))
		{
			populateRegisterPage(&guid, port);
		}
		else
		{
//			resetRegisterPane();
		}
	}

	bool BusTopologyPage::getNodeFromPath(Glib::ustring* strText, PGRGuid* guid, unsigned int* port)
	{
		size_t pos = strText->find(':');

		if(pos == Glib::ustring::npos)
		{
			return false;
		}

		Glib::ustring nodeNum = strText->substr(0, pos);
		Glib::ustring statusRegister = strText->substr(pos+1);

		*port = atoi(statusRegister.c_str());
		*guid = m_nodeDesc.guid;

        return true;
	}

	void BusTopologyPage::populateRegisterPage(PGRGuid* guid, unsigned int port)
	{
		Gtk::ListStore::Row row;
		BusManager busMgr;
		std::vector<Register>* regVector;

		if (port == 0)	// base register
			regVector = new std::vector<Register>(k_baseRegisterInfo, k_baseRegisterInfo + sizeof(k_baseRegisterInfo) / sizeof(Register));
		else	
		{
			// port status register
			regVector = new std::vector<Register>(k_portStatusInfo, k_portStatusInfo + sizeof(k_portStatusInfo) / sizeof(Register));
			port--;  // port numbers start at 0
		}	

		// populate register page
		unsigned int regVal;
		unsigned int fieldVal;
		bool isFirst = true;
		std::vector<Register>::iterator iter;
		unsigned int currAddr = 0, prevAddr = 0;

		for(iter = regVector->begin(); iter != regVector->end(); iter++)
		{	
			prevAddr = currAddr;
			currAddr = (*iter).addr;

			if (isFirst || currAddr != prevAddr)
			{
				busMgr.ReadPhyRegister(*guid, 0, port, (*iter).addr, &regVal);
				isFirst = false;
			}

			// get field
			getField(& (*iter), &regVal, &fieldVal);

			// insert into page
			row = *(m_refRegisterModel->append());
			row[m_registerColumn] = (*iter).name;
			row[m_valueColumn] = fieldVal;
		}
	}

    Glib::ustring BusTopologyPage::GenerateNodeName( PGRGuid guid )
    {
        Camera cam;
        Glib::ustring errorStr = "N/A";
        Error error = cam.Connect( &guid );
        if ( error != PGRERROR_OK )
        {
            return errorStr;
        }

        CameraInfo camInfo;
        error = cam.GetCameraInfo(&camInfo);
        if ( error != PGRERROR_OK )
        {
            return errorStr;
        }

        char nodeName[256];
        sprintf( 
            nodeName,
            "%s %s (%u)",
            camInfo.vendorName,
            camInfo.modelName,
            camInfo.serialNumber );

        return nodeName;
    }

    Glib::ustring BusTopologyPage::GenerateProductId( PGRGuid /*guid*/ )
    {
        // TODO: Get product id and figure out the revision
        return "N/A";
    }

    Glib::ustring BusTopologyPage::GenerateVoltageStr( PGRGuid guid )
    {
        Error error;
        Glib::ustring errorStr = "N/A";

        Camera cam;
        error = cam.Connect( &guid );
        if ( error != PGRERROR_OK )
        {
            return errorStr;
        }

        const unsigned int k_voltageReg = 0x1A50;
        unsigned int voltageRegVal = 0;
        error = cam.ReadRegister( k_voltageReg, &voltageRegVal );
        if ( error != PGRERROR_OK )
        {
            return errorStr;
        }

        if ( voltageRegVal >> 31 == 0 )
        {
            return errorStr;
        }

        int numAvailableVoltages = (voltageRegVal & 0x00FFF000) >> 12;

        if ( numAvailableVoltages == 0 )
        {
            return errorStr;
        }

        const unsigned int k_voltageOffsetReg = 0x1A54;
        unsigned int voltageOffsetRegVal = 0;

        error = cam.ReadRegister( k_voltageOffsetReg, &voltageOffsetRegVal );
        if ( error != PGRERROR_OK )
        {
            return errorStr;
        }

        std::vector<float> vecVoltages;
        unsigned int properVoltageOffset = (voltageOffsetRegVal * 4) & 0xFFFF;

        for ( int i=0; i < numAvailableVoltages; i++ )
        {            
            unsigned int currVoltageOffset =  properVoltageOffset + (i*4);
            unsigned int currVoltageRegVal = 0;
            error = cam.ReadRegister( currVoltageOffset, &currVoltageRegVal );
            if ( error != PGRERROR_OK )
            {
                return errorStr;
            }            

            float voltage = static_cast<float>(Convert32bitIEEEToFloat( currVoltageRegVal ));
            vecVoltages.push_back( voltage );
        }

        Glib::ustring voltageStr;
        for ( unsigned int i=0; i < vecVoltages.size(); i++ )
        {
            char currVoltageStr[16];
            sprintf( currVoltageStr, "%.3fV", vecVoltages[i] );

            voltageStr += currVoltageStr;

            if ( i != vecVoltages.size() - 1 )
            {
                voltageStr += " | ";
            }            
        }

        return voltageStr;
    }

    Glib::ustring BusTopologyPage::GenerateCurrentStr( PGRGuid guid )
    {
        Error error;
        Glib::ustring errorStr = "N/A";

        Camera cam;
        error = cam.Connect( &guid );
        if ( error != PGRERROR_OK )
        {
            return errorStr;
        }

		CameraInfo camInfo;
		error = cam.GetCameraInfo(&camInfo);
		if ( error != PGRERROR_OK || camInfo.iidcVer < sk_IIDC132Version)
        {
            return errorStr;
        }

        const unsigned int k_currentReg = 0x1A58;
        unsigned int currentRegVal = 0;
        error = cam.ReadRegister( k_currentReg, &currentRegVal );
        if ( error != PGRERROR_OK )
        {
            return errorStr;
        }

        if ( currentRegVal >> 31 == 0 )
        {
            return errorStr;
        }

        int numAvailableCurrents = (currentRegVal & 0x00FFF000) >> 12;

        if ( numAvailableCurrents == 0 )
        {
            return errorStr;
        }

        const unsigned int k_currentOffsetReg = 0x1A5C;
        unsigned int currentOffsetRegVal = 0;

        error = cam.ReadRegister( k_currentOffsetReg, &currentOffsetRegVal );
        if ( error != PGRERROR_OK )
        {
            return errorStr;
        }

        std::vector<float> vecCurrents;
        unsigned int properCurrentOffset = (currentOffsetRegVal * 4) & 0xFFFF;

        for ( int i=0; i < numAvailableCurrents; i++ )
        {            
            unsigned int currCurrentOffset =  properCurrentOffset + (i*4);
            unsigned int currCurrentRegVal = 0;
            error = cam.ReadRegister( currCurrentOffset, &currCurrentRegVal );
            if ( error != PGRERROR_OK )
            {
                return errorStr;
            }            

            float current = static_cast<float>(Convert32bitIEEEToFloat( currCurrentRegVal ));
            vecCurrents.push_back( current );
        }

        Glib::ustring currentStr;
        for ( unsigned int i=0; i < vecCurrents.size(); i++ )
        {
            char currCurrentStr[16];
            sprintf( currCurrentStr, "%.3fA", vecCurrents[i] );

            currentStr += currCurrentStr;

            if ( i != vecCurrents.size() - 1 )
            {
                currentStr += " | ";
            }            
        }
        return currentStr;
    }

    void BusTopologyPage::SetColors()
    {
        const gushort k_multiplier = 257;
        m_colorPC.set_rgb( 154*k_multiplier, 184*k_multiplier, 210*k_multiplier );
        m_colorBus.set_rgb( 153*k_multiplier, 0, 0 );
        m_colorNode.set_rgb( 209*k_multiplier, 148*k_multiplier, 12*k_multiplier );
        m_colorCamera.set_rgb( 197*k_multiplier, 210*k_multiplier, 200*k_multiplier );
    }
}
