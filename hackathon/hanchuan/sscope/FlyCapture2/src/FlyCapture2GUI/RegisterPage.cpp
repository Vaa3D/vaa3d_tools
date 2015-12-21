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
// $Id: RegisterPage.cpp,v 1.35 2010/03/24 23:29:06 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "RegisterPage.h"
#include <string.h>

namespace FlyCapture2
{
    const char* RegisterPage::sk_blankRegOffset = "0";
    const char* RegisterPage::sk_blankRegVal = "00";
    const char* RegisterPage::sk_blankIEEEVal = "0.000";

    const char* RegisterPage::sk_vboxRegisterSet = "vboxRegisterSet";
    const char* RegisterPage::sk_radioRegisterSetIIDC = "radioRegisterSetIIDC";
    const char* RegisterPage::sk_radioRegisterSetGEV = "radioRegisterSetGEV";

	const char* RegisterPage::sk_entryRegister = "entryRegister";
	const char* RegisterPage::sk_entryIEEE754 = "entryIEEE754";
	const char* RegisterPage::sk_labelRegisterName = "lblRegisterName";			
	const char* RegisterPage::sk_entry0To7 = "entry0To7";
	const char* RegisterPage::sk_entry8To15 = "entry8To15";
	const char* RegisterPage::sk_entry16To23 = "entry16To23";
	const char* RegisterPage::sk_entry24To31 = "entry24To31";
	const char* RegisterPage::sk_btnSetRegister = "btnSetRegister";
	const char* RegisterPage::sk_btnGetRegister = "btnGetRegister";

    const char* RegisterPage::sk_entryRegisterBlock = "entryRegisterBlock";
    const char* RegisterPage::sk_spinbuttonNumQuadlets = "spinbuttonNumQuadlets";
    const char* RegisterPage::sk_btnWriteRegisterBlock = "btnWriteRegisterBlock";
    const char* RegisterPage::sk_btnReadRegisterBlock = "btnReadRegisterBlock";
    const char* RegisterPage::sk_treeviewRegisterBlock = "treeviewRegisterBlock";
	
    RegisterPage::RegisterPage()
	{
        m_registerSet = REGISTER_SET_IIDC;
        m_lastRegBlockOffset = 0;
        m_pAdjustmentNumQuadlets = NULL;
	}
	
	RegisterPage::RegisterPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
	{
        m_registerSet = REGISTER_SET_IIDC;
        m_lastRegBlockOffset = 0;
        m_pAdjustmentNumQuadlets = NULL;
	}
	
	RegisterPage::~RegisterPage()
	{
        if ( m_pAdjustmentNumQuadlets != NULL )
        {
            delete m_pAdjustmentNumQuadlets;
        }
	}
			
	void RegisterPage::UpdateWidgets()
	{
        if ( m_camInfo.interfaceType == INTERFACE_GIGE )
        {
            m_pVboxRegisterSet->show();
        }
        else
        {
            m_pVboxRegisterSet->hide();
            m_registerSet = REGISTER_SET_IIDC;
        }

        m_pEntryRegister->set_text( sk_blankRegOffset );
        m_pEntryIEEE754->set_text( sk_blankIEEEVal );        
        m_pEntry0To7->set_text( sk_blankRegVal );
        m_pEntry8To15->set_text( sk_blankRegVal );
        m_pEntry16To23->set_text( sk_blankRegVal );
        m_pEntry24To31->set_text( sk_blankRegVal );
        ZeroRegBitValues();

        m_pEntryRegisterBlock->set_text( sk_blankRegOffset );
	}
	
	void RegisterPage::GetWidgets()
	{
        m_refXml->get_widget( sk_vboxRegisterSet, m_pVboxRegisterSet );
        m_refXml->get_widget( sk_radioRegisterSetIIDC, m_pRadioRegisterSetIIDC );
        m_refXml->get_widget( sk_radioRegisterSetGEV, m_pRadioRegisterSetGEV );
		m_refXml->get_widget( sk_entryRegister, m_pEntryRegister );
		m_refXml->get_widget( sk_entryIEEE754, m_pEntryIEEE754 );
		m_refXml->get_widget( sk_labelRegisterName, m_pLabelRegisterName );			
		m_refXml->get_widget( sk_entry0To7, m_pEntry0To7 );
		m_refXml->get_widget( sk_entry8To15, m_pEntry8To15 );
		m_refXml->get_widget( sk_entry16To23, m_pEntry16To23 );
		m_refXml->get_widget( sk_entry24To31, m_pEntry24To31 );
		m_refXml->get_widget( sk_btnSetRegister, m_pBtnWriteRegister );
		m_refXml->get_widget( sk_btnGetRegister, m_pBtnReadRegister );

        for ( unsigned int i=0; i < sk_numBits; i++ )
        {
            char eventBoxName[64];
            sprintf( eventBoxName, "eventboxRegBit%u", i);
            m_refXml->get_widget( eventBoxName, m_arRegBit[i].pEventBox );
            m_arRegBit[i].pEventBox->modify_bg( Gtk::STATE_NORMAL, GetColorAccentGreen() );            
            
            char labelName[64];
            sprintf( labelName, "lblRegBit%u", i);            
            m_refXml->get_widget( labelName, m_arRegBit[i].pLabel );
        }

        m_refXml->get_widget( sk_entryRegisterBlock, m_pEntryRegisterBlock );
        m_refXml->get_widget( sk_spinbuttonNumQuadlets, m_pSpinbuttonNumQuadlets );
        m_refXml->get_widget( sk_btnReadRegisterBlock, m_pBtnReadRegisterBlock );
        m_refXml->get_widget( sk_btnWriteRegisterBlock, m_pBtnWriteRegisterBlock );
        m_refXml->get_widget( sk_treeviewRegisterBlock, m_pTreeviewRegisterBlock );

        // Create the number of quadlets adjustment
        m_pAdjustmentNumQuadlets = new Gtk::Adjustment( 1, 1, 1000 );

        m_pSpinbuttonNumQuadlets->set_adjustment( *m_pAdjustmentNumQuadlets );

        // Create the list store
        m_refListStoreRegBlock = Gtk::ListStore::create( m_regBlockColumns );    

        // Set the tree view model
        m_pTreeviewRegisterBlock->set_model( m_refListStoreRegBlock );

        // Create column headers
        CreateTreeViewColumnHeaders();              
	}
	
	void RegisterPage::AttachSignals()
	{
        m_pRadioRegisterSetIIDC->signal_toggled().connect( 
            sigc::bind<RegisterSet, Gtk::RadioButton*>(
            sigc::mem_fun( *this, &RegisterPage::OnRegisterSetChange ), REGISTER_SET_IIDC, m_pRadioRegisterSetIIDC));

        m_pRadioRegisterSetGEV->signal_toggled().connect( 
            sigc::bind<RegisterSet, Gtk::RadioButton*>(
            sigc::mem_fun( *this, &RegisterPage::OnRegisterSetChange ), REGISTER_SET_GEV, m_pRadioRegisterSetGEV));

		m_pBtnWriteRegister->signal_clicked().connect( sigc::mem_fun(*this, &RegisterPage::OnWriteRegister) );
		m_pBtnReadRegister->signal_clicked().connect( sigc::mem_fun(*this, &RegisterPage::OnReadRegister) );
		
		m_pEntryRegister->signal_changed().connect( sigc::mem_fun(*this, &RegisterPage::OnEntryRegisterChange) );
		
		m_pEntryIEEE754->signal_focus_out_event().connect( sigc::mem_fun(*this, &RegisterPage::OnEntryIEEE754Change) );
		
		m_pEntry0To7->signal_changed().connect( sigc::mem_fun(*this, &RegisterPage::OnEntryRegValChange) );
		m_pEntry8To15->signal_changed().connect( sigc::mem_fun(*this, &RegisterPage::OnEntryRegValChange) );
		m_pEntry16To23->signal_changed().connect( sigc::mem_fun(*this, &RegisterPage::OnEntryRegValChange) );
		m_pEntry24To31->signal_changed().connect( sigc::mem_fun(*this, &RegisterPage::OnEntryRegValChange) );

        m_pEntryRegisterBlock->signal_changed().connect( sigc::mem_fun(*this, &RegisterPage::OnEntryRegisterBlockChange ) );
        m_pBtnReadRegisterBlock->signal_clicked().connect( sigc::mem_fun(*this, &RegisterPage::OnReadRegisterBlock ) );
        m_pBtnWriteRegisterBlock->signal_clicked().connect( sigc::mem_fun(*this, &RegisterPage::OnWriteRegisterBlock ) );

        for ( unsigned int i=0; i < sk_numBits; i++ )
        {        
            m_arRegBit[i].pEventBox->set_events(Gdk::BUTTON_PRESS_MASK);
            m_arRegBit[i].pEventBox->signal_button_press_event().connect(
                sigc::bind<unsigned int>(
                sigc::mem_fun(*this, &RegisterPage::OnRegBitClicked), i ) );
        }
	}
	
	void RegisterPage::OnWriteRegister()
	{
        unsigned int registerVal = GetRegisterOffsetFromPage();
		unsigned int value = GetRegisterValueFromPage();

        if ( m_registerSet == REGISTER_SET_IIDC )
        {
            Error error = m_pCamera->WriteRegister( registerVal, value );
            if ( error != PGRERROR_OK )
            {
                char error[512];
                sprintf( error, "There was an error writing IIDC register %X with the value %X.", registerVal, value );

                ShowMessageDialog(
                    "Error writing IIDC register",
                    error,
                    Gtk::MESSAGE_ERROR);		
            }
        }
        else if ( m_registerSet == REGISTER_SET_GEV )
        {            
            if ( IsGEV() != true )
            {
                return;
            }

            GigECamera* pCamera = dynamic_cast<GigECamera*>(m_pCamera);
            Error error = pCamera->WriteGVCPRegister( registerVal, value );
            if ( error != PGRERROR_OK )
            {
                char error[512];
                sprintf( error, "There was an error writing GVCP register %X with the value %X.", registerVal, value );

                ShowMessageDialog(
                    "Error writing GVCP register",
                    error,
                    Gtk::MESSAGE_ERROR);		
            }
        }				
	}
	
	void RegisterPage::OnReadRegister()
	{		
		// Get the text in the box		
		unsigned int registerVal = GetRegisterOffsetFromPage();
		unsigned int value = 0;

        if ( m_registerSet == REGISTER_SET_IIDC )
        {
            Error error = m_pCamera->ReadRegister( registerVal, &value );
            if ( error != PGRERROR_OK )
            {
                char error[512];
                sprintf( error, "There was an error reading register %X.", registerVal );

                ShowMessageDialog(
                    "Error reading register",
                    error,
                    Gtk::MESSAGE_ERROR );			

                return;
            }		
        }
        else if ( m_registerSet == REGISTER_SET_GEV )
        {            
            if ( IsGEV() != true )
            {
                return;
            }

            GigECamera* pCamera = dynamic_cast<GigECamera*>(m_pCamera);
            Error error = pCamera->ReadGVCPRegister( registerVal, &value );
            if ( error != PGRERROR_OK )
            {
                char error[512];
                sprintf( error, "There was an error reading GVCP register %X.", registerVal );

                ShowMessageDialog(
                    "Error reading GVCP register",
                    error,
                    Gtk::MESSAGE_ERROR);		
            }
        }	
		
		SetRegisterValueToPage( value );
	}
	
	void RegisterPage::OnEntryRegisterChange()
	{
        if ( m_registerSet == REGISTER_SET_IIDC )
        {
            // Get the text in the box		
            unsigned int registerVal = GetRegisterOffsetFromPage();

            // Get the corresponding register string
            const char* registerString = GetRegisterString( registerVal );

            if ( strcmp( registerString, "Invalid") != 0 )
            {
                // Register seems valid
                m_pEntryRegister->modify_base( Gtk::STATE_NORMAL, GetColorAccentGreen() );
            }
            else
            {
                // Register seems invalid
                m_pEntryRegister->modify_base( Gtk::STATE_NORMAL, GetColorAccentRed() );
            }

            m_pLabelRegisterName->set_text( registerString );         
        }
        else if ( m_registerSet == REGISTER_SET_GEV )
        {
            m_pLabelRegisterName->set_text( "Unknown" );      
            m_pEntryRegister->modify_base( Gtk::STATE_NORMAL, GetColorAccentGreen() );
        }        
	}
	
	bool RegisterPage::OnEntryIEEE754Change(GdkEventFocus* /*focus*/)
	{        
        char* end;        
        double value = strtod( m_pEntryIEEE754->get_text().c_str(), &end );

        unsigned int valueIEEE;
        valueIEEE = ConvertFloatTo32bitIEEE( static_cast<float>(value) );

        SetRegisterValueToPage( valueIEEE );

        return true;
	}
	
	void RegisterPage::OnEntryRegValChange()
	{        
        unsigned int value = GetRegisterValueFromPage();       

        char floatString[128];
        sprintf(
            floatString,
            "%f",
            Convert32bitIEEEToFloat(value) );

        m_pEntryIEEE754->set_text( floatString );

        UpdateRegBitValues( value );
	}

    bool RegisterPage::OnRegBitClicked( GdkEventButton* /*event*/, unsigned int bit )
    {
        Glib::ustring textVal = m_arRegBit[bit].pLabel->get_text();
        if ( textVal == "0" )
        {
            m_arRegBit[bit].pLabel->set_text( "1" );
        }
        else if ( textVal == "1" )
        {
            m_arRegBit[bit].pLabel->set_text( "0" );
        }

        unsigned int newVal = GetRegBitValues();
        SetRegisterValueToPage( newVal );

        return true;
    }

    void RegisterPage::OnEntryRegisterBlockChange()
    {
        Gdk::Color color_green = GetColorAccentGreen();
        Gdk::Color color_red = GetColorAccentRed();

        // Get the text in the box		
        unsigned int registerVal = GetRegisterBlockOffsetFromPage();

        // Get the corresponding register string
        const char* registerString = GetRegisterString( registerVal );

        if ( strcmp( registerString, "Invalid") != 0 )
        {
            // Register seems valid
            m_pEntryRegisterBlock->modify_base( Gtk::STATE_NORMAL, color_green );
        }
        else
        {
            // Register seems invalid
            m_pEntryRegisterBlock->modify_base( Gtk::STATE_NORMAL, color_red );
        }     
    }

    void RegisterPage::OnReadRegisterBlock()
    {
        // Get the register offset
        unsigned int regBlockOffset = GetRegisterBlockOffsetFromPage();

        // Get the number of quadlets
        int numQuadlets = static_cast<int>(m_pAdjustmentNumQuadlets->get_value());

        // No need to error check the number of quadlets, because the lower
        // limit of the adjustment is set to 1

        unsigned int* regBlockArray = new unsigned int[ numQuadlets ];

        if ( m_registerSet == REGISTER_SET_IIDC )
        {
            // Perform the block read
            Error error = m_pCamera->ReadRegisterBlock(
                0xFFFF,
                regBlockOffset + 0xF0F00000,
                regBlockArray,
                numQuadlets );        
            if ( error != PGRERROR_OK )
            {
                char error[512];
                sprintf( 
                    error, 
                    "There was an error reading IIDC register block %X - %X.", 
                    regBlockOffset,
                    regBlockOffset + (4*numQuadlets) );

                ShowMessageDialog(
                    "Error reading IIDC register block",
                    error,
                    Gtk::MESSAGE_ERROR );		

                return;
            }
        }
        else if ( m_registerSet == REGISTER_SET_GEV )
        {
            if ( IsGEV() != true )
            {
                return;
            }

            GigECamera* pCamera = dynamic_cast<GigECamera*>(m_pCamera);
            Error error = pCamera->ReadGVCPRegisterBlock(
                regBlockOffset,
                regBlockArray,
                numQuadlets );
            if ( error != PGRERROR_OK )
            {
                char error[512];
                sprintf( 
                    error, 
                    "There was an error reading GVCP register block %X - %X.", 
                    regBlockOffset,
                    regBlockOffset + (4*numQuadlets) );

                ShowMessageDialog(
                    "Error reading GVCP register block",
                    error,
                    Gtk::MESSAGE_ERROR );		

                return;
            }
        }        

        m_lastRegBlockOffset = regBlockOffset;

        m_refListStoreRegBlock->clear();

        char offset[16];
        char value[16];

        for ( int i=0; i < numQuadlets; i++ )
        {
            // Append the individual register to the list
            Gtk::TreeModel::Row row = *(m_refListStoreRegBlock->append());

            unsigned int currOffset = regBlockOffset + (4*i);
            const char* currRegName = GetRegisterString( currOffset );
            
            sprintf( offset, "0x%X", currOffset );            
            sprintf( value, "0x%08X", regBlockArray[i] );

            row[m_regBlockColumns.m_colRegOffset] = offset;
            row[m_regBlockColumns.m_colRegValue] = value;
            row[m_regBlockColumns.m_colRegName] = currRegName;
        }

        delete [] regBlockArray;
    }

    void RegisterPage::OnWriteRegisterBlock()
    {
        Gtk::TreeModel::Children children = m_refListStoreRegBlock->children();
        unsigned int numQuadlets = children.size();

        unsigned int* regBlockArray = new unsigned int[numQuadlets];

        // Get the values from the tree view
        Gtk::TreeModel::Children::iterator iter;
        unsigned int iIdx = 0;
        for ( iter = children.begin(); iter != children.end(); iter++, iIdx++ )
        {
            Gtk::TreeModel::Row row = *iter;

            Glib::ustring valueString = row[m_regBlockColumns.m_colRegValue];
            unsigned int value = 0;

            int retVal = sscanf( valueString.c_str(), "0x%X", &value );
            if( retVal != 1 )
            {
                ShowMessageDialog(
                    "Error parsing register value",
                    "There was an invalid register value.",
                    Gtk::MESSAGE_ERROR );	

                delete [] regBlockArray;
                return;
            }

            regBlockArray[iIdx] = value;
        }

        Error error;
        error = m_pCamera->WriteRegisterBlock(
            0xFFFF,
            m_lastRegBlockOffset + 0xF0F00000,
            regBlockArray,
            numQuadlets );
        if ( error != PGRERROR_OK )
        {
            char error[512];
            sprintf( 
                error, 
                "There was an error writing register block %X - %X.", 
                m_lastRegBlockOffset,
                m_lastRegBlockOffset + (4*numQuadlets) );

            ShowMessageDialog(
                "Error writing register block",
                error,
                Gtk::MESSAGE_ERROR );	

            delete [] regBlockArray;

            return;
        }

        if ( m_registerSet == REGISTER_SET_IIDC )
        {
            Error error;
            error = m_pCamera->WriteRegisterBlock(
                0xFFFF,
                m_lastRegBlockOffset + 0xF0F00000,
                regBlockArray,
                numQuadlets );
            if ( error != PGRERROR_OK )
            {
                char error[512];
                sprintf( 
                    error, 
                    "There was an error writing register block %X - %X.", 
                    m_lastRegBlockOffset,
                    m_lastRegBlockOffset + (4*numQuadlets) );

                ShowMessageDialog(
                    "Error writing register block",
                    error,
                    Gtk::MESSAGE_ERROR );	

                delete [] regBlockArray;

                return;
            }
        }
        else if ( m_registerSet == REGISTER_SET_GEV )
        {
            if ( IsGEV() != true )
            {
                return;
            }

            GigECamera* pCamera = dynamic_cast<GigECamera*>(m_pCamera);
            Error error = pCamera->WriteGVCPRegisterBlock(
                m_lastRegBlockOffset,
                regBlockArray,
                numQuadlets );
            if ( error != PGRERROR_OK )
            {
                char error[512];
                sprintf( 
                    error, 
                    "There was an error writing GVCP register block %X - %X.", 
                    m_lastRegBlockOffset,
                    m_lastRegBlockOffset + (4*numQuadlets) );

                ShowMessageDialog(
                    "Error writing GVCP register block",
                    error,
                    Gtk::MESSAGE_ERROR );		

                delete [] regBlockArray;

                return;
            }
        }

        delete [] regBlockArray;
    }

    unsigned int RegisterPage::GetRegisterOffsetFromPage()
    {
        // Get the text in the box		
        unsigned int registerVal = strtoul( m_pEntryRegister->get_text().c_str(), NULL, 16 );

        return registerVal;
    }

    unsigned int RegisterPage::GetRegisterValueFromPage()
    {
        unsigned int reg0 = 0;
        unsigned int reg8 = 0;
        unsigned int reg16 = 0;
        unsigned int reg24 = 0;

        reg0 = strtoul( m_pEntry0To7->get_text().c_str(), NULL, 16 );
        reg8 = strtoul( m_pEntry8To15->get_text().c_str(), NULL, 16 );
        reg16 = strtoul( m_pEntry16To23->get_text().c_str(), NULL, 16 );
        reg24 = strtoul( m_pEntry24To31->get_text().c_str(), NULL, 16 );
        	
        unsigned int value = (reg0 << 24) + (reg8 << 16) + (reg16 << 8) + (reg24 << 0);

        return value;
    }

    unsigned int RegisterPage::GetRegisterBlockOffsetFromPage()
    {
        // Get the text in the box		
        unsigned int registerVal = strtoul( m_pEntryRegisterBlock->get_text().c_str(), NULL, 16 );

        return registerVal;
    }
	
	void RegisterPage::SetRegisterValueToPage( unsigned int value )
    {
        char regVal[16];		

        sprintf( regVal, "%02X", value >> 24 & 0xFF );
        m_pEntry0To7->set_text( regVal );

        sprintf( regVal, "%02X", value >> 16 & 0xFF );
        m_pEntry8To15->set_text( regVal );

        sprintf( regVal, "%02X", value >> 8 & 0xFF );
        m_pEntry16To23->set_text( regVal );

        sprintf( regVal, "%02X", value >> 0 & 0xFF );
        m_pEntry24To31->set_text( regVal );	
    }            

    void RegisterPage::CreateTreeViewColumnHeaders()
    {
        m_pTreeviewRegisterBlock->append_column( "Offset", m_regBlockColumns.m_colRegOffset );
        m_pTreeviewRegisterBlock->append_column_editable( "Value", m_regBlockColumns.m_colRegValue );
        m_pTreeviewRegisterBlock->append_column( "Name", m_regBlockColumns.m_colRegName );
    }        

    void RegisterPage::ZeroRegBitValues()
    {
        const char* zero = "0";
        for ( unsigned int i=0; i < sk_numBits; i++ )
        {
            m_arRegBit[i].pLabel->set_text( zero );
        }
    }

    void RegisterPage::UpdateRegBitValues( unsigned int regVal )
    {
        const char* zero = "0";
        const char* one = "1";

        for ( unsigned int i=0; i < sk_numBits; i++ )
        {
            bool enabled = (regVal & (0x80000000 >> i)) != 0 ? true : false;
            m_arRegBit[i].pLabel->set_text( enabled ? one : zero );
        } 
    }

    unsigned int RegisterPage::GetRegBitValues()
    {
        unsigned int regVal = 0;
        for ( unsigned int i=0; i < sk_numBits; i++ )
        {
            Glib::ustring textVal = m_arRegBit[i].pLabel->get_text();
            if ( textVal == "1" )
            {
                regVal |= (0x80000000 >> i);
            }
        } 

        return regVal;
    }

    void RegisterPage::OnRegisterSetChange( RegisterSet selectedSet, Gtk::RadioButton* pRadioButton )
    {
        if ( pRadioButton->get_active() == true )
        {
            m_registerSet = selectedSet;
            OnEntryRegisterChange();
        }        
    }
}
