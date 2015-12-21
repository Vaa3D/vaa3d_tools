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
// $Id: RegisterPage.h,v 1.17 2009/04/21 22:34:18 soowei Exp $
//=============================================================================

#ifndef _REGISTERPAGE_H_
#define _REGISTERPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
    class RegisterBlockColumns : public Gtk::TreeModelColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> m_colRegOffset;
        Gtk::TreeModelColumn<Glib::ustring> m_colRegValue;
        Gtk::TreeModelColumn<Glib::ustring> m_colRegName;

        RegisterBlockColumns()
        { 
            add( m_colRegOffset ); 
            add( m_colRegValue ); 
            add( m_colRegName ); 
        }							 
    };

	class RegisterPage : public BasePage 
	{
	public:
		RegisterPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );
		~RegisterPage();
			
		void UpdateWidgets();
					
	protected:		
		void GetWidgets();
		void AttachSignals();		

	private:      
        struct RegBit
        {
            Gtk::EventBox* pEventBox;
            Gtk::Label* pLabel;
        };

        enum RegisterSet
        {
            REGISTER_SET_IIDC,
            REGISTER_SET_GEV
        };

        static const unsigned int sk_numBits = 32;

        static const char* sk_blankRegOffset;
        static const char* sk_blankRegVal;
        static const char* sk_blankIEEEVal;

        static const char* sk_vboxRegisterSet;
        static const char* sk_radioRegisterSetIIDC;
        static const char* sk_radioRegisterSetGEV;

		static const char* sk_entryRegister;
		static const char* sk_entryIEEE754;
		static const char* sk_labelRegisterName;			
		static const char* sk_entry0To7;
		static const char* sk_entry8To15;
		static const char* sk_entry16To23;
		static const char* sk_entry24To31;
		static const char* sk_btnSetRegister;
		static const char* sk_btnGetRegister;

        static const char* sk_entryRegisterBlock;
        static const char* sk_spinbuttonNumQuadlets;
        static const char* sk_btnWriteRegisterBlock;
        static const char* sk_btnReadRegisterBlock;
        static const char* sk_treeviewRegisterBlock;    

        Gtk::VBox* m_pVboxRegisterSet;
        Gtk::RadioButton* m_pRadioRegisterSetIIDC;
        Gtk::RadioButton* m_pRadioRegisterSetGEV;
		
		Gtk::Entry* m_pEntryRegister;
		Gtk::Entry* m_pEntryIEEE754;
		Gtk::Label* m_pLabelRegisterName;
		Gtk::Entry* m_pEntry0To7;
		Gtk::Entry* m_pEntry8To15;
		Gtk::Entry* m_pEntry16To23;
		Gtk::Entry* m_pEntry24To31;
		Gtk::Button* m_pBtnWriteRegister;
		Gtk::Button* m_pBtnReadRegister;  

        RegBit m_arRegBit[sk_numBits];

        Gtk::Entry* m_pEntryRegisterBlock;
        Gtk::SpinButton* m_pSpinbuttonNumQuadlets;
        Gtk::Adjustment* m_pAdjustmentNumQuadlets;
        Gtk::Button* m_pBtnReadRegisterBlock;
        Gtk::Button* m_pBtnWriteRegisterBlock;
        Gtk::TreeView* m_pTreeviewRegisterBlock;

        Glib::RefPtr<Gtk::ListStore> m_refListStoreRegBlock;
        RegisterBlockColumns m_regBlockColumns;

        unsigned int m_lastRegBlockOffset;

        RegisterSet m_registerSet;

        RegisterPage();
        RegisterPage( const RegisterPage& );
        RegisterPage& operator=( const RegisterPage& );

        unsigned int GetRegisterOffsetFromPage();
        unsigned int GetRegisterValueFromPage();

        unsigned int GetRegisterBlockOffsetFromPage();
			
		void SetRegisterValueToPage( unsigned int value );

        void CreateTreeViewColumnHeaders();

        void ZeroRegBitValues();
        void UpdateRegBitValues( unsigned int regVal );
        unsigned int GetRegBitValues();

        void OnRegisterSetChange(RegisterSet selectedSet, Gtk::RadioButton* pRadioButton);

        void OnWriteRegister();
        void OnReadRegister();
        void OnEntryRegisterChange();
        bool OnEntryIEEE754Change(GdkEventFocus* focus);
        void OnEntryRegValChange();
        bool OnRegBitClicked( GdkEventButton* event, unsigned int bit );

        void OnEntryRegisterBlockChange();
        void OnReadRegisterBlock();
        void OnWriteRegisterBlock();        
	};
}


#endif // _REGISTERPAGE_H_
