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
// $Id: SystemInfoPage.h,v 1.7 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#ifndef _SYSTEMINFOPAGE_H_
#define _SYSTEMINFOPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
	class SystemInfoPage : public BasePage 
	{
	public:
		SystemInfoPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );
		~SystemInfoPage();
			
		void UpdateWidgets();

	protected:
        static const char* sk_lblCPU;
        static const char* sk_lblNumCores;
        static const char* sk_lblAvailableMemory;
        static const char* sk_lblByteOrder;
        static const char* sk_lblOS;
        static const char* sk_lblGraphics;
        static const char* sk_lblScreenResolution;
        static const char* sk_btnCopySystemInfo;

        Gtk::Label* m_pLblCPU;
        Gtk::Label* m_pLblNumCores;
        Gtk::Label* m_pLblAvailableMemory;
        Gtk::Label* m_pLblByteOrder;
        Gtk::Label* m_pLblOS;
        Gtk::Label* m_pLblGraphics;
        Gtk::Label* m_pLblScreenResolution;
        Gtk::Button* m_pBtnCopySystemInfo;

        SystemInfoPage();
        SystemInfoPage( const SystemInfoPage& );
        SystemInfoPage& operator=( const SystemInfoPage& );

		void GetWidgets();
		void AttachSignals();		

	private:    
        void OnCopySystemInfo();
	};
}

#endif // _SYSTEMINFOPAGE_H_
