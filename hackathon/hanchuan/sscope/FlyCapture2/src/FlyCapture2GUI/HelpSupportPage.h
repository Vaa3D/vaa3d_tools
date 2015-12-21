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
// $Id: HelpSupportPage.h,v 1.6 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#ifndef _HELPSUPPORTPAGE_H_
#define _HELPSUPPORTPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
	class HelpSupportPage : public BasePage 
	{
	public:
		HelpSupportPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );
		~HelpSupportPage();
			
		void UpdateWidgets();
			
		void OnLinkButtonClicked(
            Gtk::LinkButton* button, 
            const Glib::ustring& uri);

	protected:
		void GetWidgets();
		void AttachSignals();		

	private:
		static const char* sk_linkButtonKB;
		static const char* sk_linkButtonSupportTickets;
		static const char* sk_linkButtonDownloads;			
		
		Gtk::LinkButton* m_pLinkButtonKB;
		Gtk::LinkButton* m_pLinkButtonSupportTickets;
		Gtk::LinkButton* m_pLinkButtonDownloads;

        HelpSupportPage();
        HelpSupportPage( const HelpSupportPage& );
        HelpSupportPage& operator=( const HelpSupportPage& );
	};
}

#endif // _HELPSUPPORTPAGE_H_
