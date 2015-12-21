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
// $Id: CamSettingsPage.h,v 1.27 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#ifndef _CAMSETTINGSPAGE_H_
#define _CAMSETTINGSPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
	class CamSettingsPage : public BasePage 
	{
	public:
        /** 
        * Constructor. Initialize the object with the specified parameters. 
        *
        * @param pCamera Pointer to a valid Camera object.
        * @param refXml RefPtr to a Glade XML object.
        */
		CamSettingsPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );

        /** Destructor. */
		~CamSettingsPage();
			
        /** Update widgets on page. */
        void UpdateWidgets();
								
		void OnAutoClicked( PropertyType property, Gtk::CheckButton* pButton );
		void OnOnOffClicked( PropertyType property, Gtk::CheckButton* pButton );
		void OnOnePushClicked( PropertyType property, Gtk::CheckButton* pButton );
		void OnHScroll( PropertyType property, Gtk::Adjustment* pAdjustment );
		
		void OnPowerClicked( Gtk::CheckButton* pButton );

        void OnAbsModeClicked( Gtk::CheckButton* pButton );

	protected:
        /** Load widgets. */
        void GetWidgets();

        /** Attach signals to widgets. */
        void AttachSignals();	

	private:
		/**
		 * Internal struct to store the widgets associated with a single camera 
         * property. The second hscale, adjustment and label is only used for 
         * white balance.
		 */           
		struct WidgetProp
		{		
			Gtk::HScale* pHScale1;
            Gtk::HScale* pHScale2;
			Gtk::CheckButton* pAuto;
			Gtk::CheckButton* pOnOff;
			Gtk::CheckButton* pOnePush;
			Gtk::Adjustment* pAdjustment1;
            Gtk::Adjustment* pAdjustment2;
            Gtk::SpinButton* pSpinButton1;
            Gtk::SpinButton* pSpinButton2;
			Gtk::Label* pLabel1;
            Gtk::Label* pLabel2;
		};
		
        /** Number of camera properties. */
		static const unsigned int sk_numProps = 18;

        /** Array of widget properties. */
		WidgetProp m_widgetPropArray[sk_numProps];		

        static const unsigned int sk_timeout = 500;
				
		static const char* sk_checkbuttonOnOffPower;
        static const char* sk_checkbuttonAbsMode;					
			
		Gtk::CheckButton* m_pCheckbuttonPower;
        Gtk::CheckButton* m_pCheckbuttonAbsMode;

        /** Whether the absolute values should be displayed. */
        bool m_absMode;

        /** Map of property type to property name. */
        std::map<PropertyType, const char*> m_mapProperty;

        Glib::Mutex m_hScrollMutex;
        Glib::Mutex m_updateWidgetMutex;
			
        CamSettingsPage();
        CamSettingsPage( const CamSettingsPage& );
        CamSettingsPage& operator=( const CamSettingsPage& );
        
        bool OnTimer();

        void UpdateSpinButton( Gtk::SpinButton* pButton, PropertyInfo* pPropInfo );

		void UpdateAdjustment( Gtk::Adjustment* pAdjustment, PropertyInfo* pPropInfo, Property* pProp );
		void UpdateLabel( Gtk::Label* pLabel, PropertyInfo* pPropInfo, Property* pProp );
			
		void UpdateAutoCheckButton( Gtk::CheckButton* pButton, bool autoSupported, bool autoManualMode );
		void UpdateOnOffCheckButton( Gtk::CheckButton* pButton, bool onOffSupported, bool onOff );
		void UpdateOnePushCheckButton( Gtk::CheckButton* pButton, bool onePushSupported, bool onePush );
			
		void UpdateCheckButton( Gtk::CheckButton* pButton, bool supported, bool val );        
			
		void HideProperty( PropertyType property );

        void HideWidget( Gtk::Widget* pWidget );

		/** Update the power check button with the value on the camera. */
		void UpdateCameraPowerCheckButton();	

		/**
		 * Check if the camera supports the power button.
		 *
		 * @return Whether the camera supports the power button.
		 */
		bool IsCameraPowerAvailable();

		/**
		 * Get the status of the camera's power register.
		 *
		 * @return Whether the power is on or off.
		 */
		bool GetCameraPower();

		/**
		 * Set the camera's power register.
		 *
		 * @param on Whether to set the power to on or off.
		 */
		void SetCameraPower( bool on );

        void CreatePropertyMap();        
	};
}

#endif // _CAMSETTINGSPAGE_H_
