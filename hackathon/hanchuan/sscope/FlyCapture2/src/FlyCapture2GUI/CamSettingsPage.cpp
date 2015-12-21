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
// $Id: CamSettingsPage.cpp,v 1.45 2010/05/13 18:36:25 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "CamSettingsPage.h"

namespace FlyCapture2
{	
	const char* CamSettingsPage::sk_checkbuttonOnOffPower = "checkbuttonOnOffPower";
    const char* CamSettingsPage::sk_checkbuttonAbsMode = "checkbuttonAbsMode";	
	
	CamSettingsPage::CamSettingsPage()
	{	
        m_absMode = true;

        CreatePropertyMap();
	}
	
	CamSettingsPage::CamSettingsPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
	{
        m_absMode = true;

        CreatePropertyMap();
	}
	
	CamSettingsPage::~CamSettingsPage()
	{	
        for ( unsigned int i = 0; i < sk_numProps; i++ )
        {
            Gtk::Adjustment* pAdjustment1 = m_widgetPropArray[i].pAdjustment1;
            Gtk::Adjustment* pAdjustment2 = m_widgetPropArray[i].pAdjustment2;

            if ( pAdjustment1 != NULL )
            {
                delete pAdjustment1;
                m_widgetPropArray[i].pAdjustment1 = NULL;
            }      

            if ( pAdjustment2 != NULL )
            {
                delete pAdjustment2;
                m_widgetPropArray[i].pAdjustment2 = NULL;
            }   
        }
	}
	
	void CamSettingsPage::GetWidgets()
	{	
        // Initialize everything to NULL
        for ( unsigned int i = 0; i < sk_numProps; i++ )
        {
            m_widgetPropArray[i].pHScale1 = NULL;
            m_widgetPropArray[i].pHScale2 = NULL;
            m_widgetPropArray[i].pAuto = NULL;
            m_widgetPropArray[i].pOnOff = NULL;
            m_widgetPropArray[i].pOnePush = NULL;
            m_widgetPropArray[i].pAdjustment1 = NULL;
            m_widgetPropArray[i].pAdjustment2 = NULL;
            m_widgetPropArray[i].pSpinButton1 = NULL;
            m_widgetPropArray[i].pSpinButton2 = NULL;
            m_widgetPropArray[i].pLabel1 = NULL;
            m_widgetPropArray[i].pLabel2 = NULL;
        }

		// Power button		
		m_refXml->get_widget( sk_checkbuttonOnOffPower, m_pCheckbuttonPower );

        // Abs mode
        m_refXml->get_widget( sk_checkbuttonAbsMode, m_pCheckbuttonAbsMode );

        // Get property widgets
        for ( unsigned int i=0; i < sk_numProps; i++ )
        {
            PropertyType currType = static_cast<PropertyType>(i);

            // These properties do not have any widgets associated with them
            if ( currType == TRIGGER_MODE || currType == TRIGGER_DELAY )
            {
                continue;
            }            

            // Make sure the property can be found in the property map
            if ( m_mapProperty.find(currType) == m_mapProperty.end() )
            {
                // Property not found
                continue;
            }

            // Retrieve the property name
            const char* currPropName = m_mapProperty.find(currType)->second;
            char widgetName[64];

            // Special case for temperature, because it only has a label and
            // nothing else
            if ( currType == TEMPERATURE )
            {
                sprintf( widgetName, "lbl%s", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pLabel1 );

                continue;
            }

            if ( currType == WHITE_BALANCE )
            {
                // Special case for white balance, since it has 2 hscales
                // and 2 labels for itself
                sprintf( widgetName, "hscale%sRed", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pHScale1 );

                sprintf( widgetName, "hscale%sBlue", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pHScale2 );

                sprintf( widgetName, "spinbutton%sRed", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pSpinButton1 );

                sprintf( widgetName, "spinbutton%sBlue", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pSpinButton2 );

                sprintf( widgetName, "lbl%sRed", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pLabel1 );

                sprintf( widgetName, "lbl%sBlue", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pLabel2 );
            }
            else
            {
                sprintf( widgetName, "hscale%s", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pHScale1 );

                sprintf( widgetName, "spinbutton%s", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pSpinButton1 );

                sprintf( widgetName, "lbl%s", currPropName );
                m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pLabel1 );
            }            

            sprintf( widgetName, "checkbuttonAuto%s", currPropName );
            m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pAuto );

            sprintf( widgetName, "checkbuttonOnOff%s", currPropName );
            m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pOnOff );

            sprintf( widgetName, "checkbuttonOnePush%s", currPropName );
            m_refXml->get_widget( widgetName, m_widgetPropArray[currType].pOnePush );            
        }        
		
		// Create the adjustments and connect them to the appropriate hscales
		const int k_adjMin = 0;
		const int k_adjMax = 100;
		const int k_adjCurr = 0;
		
        for ( unsigned int i = 0; i < sk_numProps; i++ )
        {
            switch (i)
            {
            case WHITE_BALANCE:
                {
                    Gtk::Adjustment* pNewAdjustment1 = new Gtk::Adjustment( k_adjCurr, k_adjMin, k_adjMax );
                    m_widgetPropArray[i].pAdjustment1 = pNewAdjustment1;
                    m_widgetPropArray[i].pHScale1->set_adjustment( *pNewAdjustment1 );
                    m_widgetPropArray[i].pSpinButton1->set_adjustment( *pNewAdjustment1 );

                    Gtk::Adjustment* pNewAdjustment2 = new Gtk::Adjustment( k_adjCurr, k_adjMin, k_adjMax );
                    m_widgetPropArray[i].pAdjustment2 = pNewAdjustment2;
                    m_widgetPropArray[i].pHScale2->set_adjustment( *pNewAdjustment2 );
                    m_widgetPropArray[i].pSpinButton2->set_adjustment( *pNewAdjustment2 );
                }                
                break;
            case TRIGGER_MODE:
            case TRIGGER_DELAY:
            case TEMPERATURE:
                {
                    m_widgetPropArray[i].pAdjustment1 = 0;
                }
                break;
            default:
                {
                    Gtk::Adjustment* pNewAdjustment = new Gtk::Adjustment( k_adjCurr, k_adjMin, k_adjMax );
                    m_widgetPropArray[i].pAdjustment1 = pNewAdjustment;
                    m_widgetPropArray[i].pHScale1->set_adjustment( *pNewAdjustment );
                    m_widgetPropArray[i].pSpinButton1->set_adjustment( *pNewAdjustment );
                }
                break;
            }
		}
	}
	
	void CamSettingsPage::AttachSignals()
	{						
		// Create a timer
		SetTimerConnection(
			Glib::signal_timeout().connect( sigc::mem_fun(*this, &CamSettingsPage::OnTimer), sk_timeout ));
		
		// Attach all the various camera properties
		// In general, all the functions here are bound using sigc::bind
		// to attach additional parameters, such as the property being modified,
		// as well as widget itself
		for ( unsigned int i = 0; i < sk_numProps; i++ )
		{					
			Gtk::CheckButton* pAuto = m_widgetPropArray[i].pAuto;
			Gtk::CheckButton* pOnOff = m_widgetPropArray[i].pOnOff;
			Gtk::CheckButton* pOnePush = m_widgetPropArray[i].pOnePush;
			Gtk::Adjustment* pAdjustment1 = m_widgetPropArray[i].pAdjustment1;
            Gtk::Adjustment* pAdjustment2 = m_widgetPropArray[i].pAdjustment2;
			
			if ( pAuto != 0 )
			{
				pAuto->signal_toggled().connect( 
					sigc::bind<PropertyType, Gtk::CheckButton*>( 
						sigc::mem_fun(*this, &CamSettingsPage::OnAutoClicked), 
						(PropertyType)i, 
						pAuto ) );
			}
			
			if ( pOnOff != 0 )
			{
				pOnOff->signal_toggled().connect( 
					sigc::bind<PropertyType, Gtk::CheckButton*>( 
						sigc::mem_fun(*this, &CamSettingsPage::OnOnOffClicked), 
						(PropertyType)i, 
						pOnOff ) );			
			}
			
			if ( pOnePush != 0 )
			{
				pOnePush->signal_toggled().connect( 
					sigc::bind<PropertyType, Gtk::CheckButton*>( 
						sigc::mem_fun(*this, &CamSettingsPage::OnOnePushClicked), 
						(PropertyType)i, 
						pOnePush ) );			
			}		
			
			if ( pAdjustment1 != 0 )
			{
				pAdjustment1->signal_value_changed().connect( 
					sigc::bind<PropertyType, Gtk::Adjustment*>( 
						sigc::mem_fun(*this, &CamSettingsPage::OnHScroll), 
						(PropertyType)i, 
						pAdjustment1 ) );
			}			

            if ( pAdjustment2 != 0 )
            {
                pAdjustment2->signal_value_changed().connect( 
                    sigc::bind<PropertyType, Gtk::Adjustment*>( 
                    sigc::mem_fun(*this, &CamSettingsPage::OnHScroll), 
                    (PropertyType)i, 
                    pAdjustment2 ) );
            }	
		}
		
		// Attach power check box
		m_pCheckbuttonPower->signal_toggled().connect( 
			sigc::bind<Gtk::CheckButton*>( 
				sigc::mem_fun(*this, &CamSettingsPage::OnPowerClicked), 
				m_pCheckbuttonPower ) );

        // Attach abs mode check box
        m_pCheckbuttonAbsMode->signal_toggled().connect( 
            sigc::bind<Gtk::CheckButton*>( 
            sigc::mem_fun(*this, &CamSettingsPage::OnAbsModeClicked), 
            m_pCheckbuttonAbsMode ) );
	}
	
	void CamSettingsPage::UpdateWidgets()
	{
        Glib::Mutex::Lock updateLock( m_updateWidgetMutex, Glib::NOT_LOCK );
        if ( updateLock.try_acquire() != true )
        {
            return;
        }

        if ( m_pCamera == NULL || IsConnected() != true )
        {
            return;	
        }
		
		UpdateCameraPowerCheckButton();

        if ( m_absMode != m_pCheckbuttonAbsMode->get_active() )
        {
            m_pCheckbuttonAbsMode->set_active( m_absMode );
        }        				
		
		for ( unsigned int i = 0; i < sk_numProps; i++ )
		{
			if ( m_pCamera == 0 )
			{
				return;
			}
            			
			Gtk::CheckButton* pAuto = m_widgetPropArray[i].pAuto;
			Gtk::CheckButton* pOnOff = m_widgetPropArray[i].pOnOff;
			Gtk::CheckButton* pOnePush = m_widgetPropArray[i].pOnePush;
            Gtk::HScale* pHScale1 = m_widgetPropArray[i].pHScale1;
            Gtk::HScale* pHScale2 = m_widgetPropArray[i].pHScale2;
            Gtk::SpinButton* pSpinButton1 = m_widgetPropArray[i].pSpinButton1;
            Gtk::SpinButton* pSpinButton2 = m_widgetPropArray[i].pSpinButton2;
			Gtk::Adjustment* pAdjustment1 = m_widgetPropArray[i].pAdjustment1;
            Gtk::Adjustment* pAdjustment2 = m_widgetPropArray[i].pAdjustment2;
            Gtk::Label* pLabel1 = m_widgetPropArray[i].pLabel1;
            Gtk::Label* pLabel2 = m_widgetPropArray[i].pLabel2;

            const PropertyType k_currPropType = (PropertyType)i;
						
            Property camProp;
            PropertyInfo camPropInfo;

			// Get the property and property info
			camProp.type = k_currPropType;
			camPropInfo.type = k_currPropType;

            Error getPropErr = m_pCamera->GetProperty( &camProp );
            Error getPropInfoErr = m_pCamera->GetPropertyInfo( &camPropInfo );
						
			if ( getPropErr != PGRERROR_OK || getPropInfoErr != PGRERROR_OK )
			{
				// Perhaps not supported, hide it and continue
				HideProperty( k_currPropType );
				continue;
			}		            
			
			if ( pAuto != 0 )
			{
				UpdateAutoCheckButton( pAuto, camPropInfo.autoSupported, camProp.autoManualMode );
			}
			
			if ( pOnOff != 0 )
			{
				UpdateOnOffCheckButton( pOnOff, camPropInfo.onOffSupported, camProp.onOff );
			}
			
			if ( pOnePush != 0 )
			{
				UpdateOnePushCheckButton( pOnePush, camPropInfo.onePushSupported, camProp.onePush );
			}
			
			if ( pAdjustment1 != 0 )
			{
				UpdateAdjustment( pAdjustment1, &camPropInfo, &camProp );
                pHScale1->show();  
                pHScale1->set_sensitive(true);

                UpdateSpinButton( pSpinButton1, &camPropInfo );
                pSpinButton1->show();
			}

            if ( pAdjustment2 != 0 )
            {
                UpdateAdjustment( pAdjustment2, &camPropInfo, &camProp );
                pHScale2->show(); 
                pHScale2->set_sensitive(true);

                UpdateSpinButton( pSpinButton2, &camPropInfo );
                pSpinButton2->show();
            }			

            if ( pLabel1 != 0 )
            {
                UpdateLabel(pLabel1, &camPropInfo, &camProp );
            }

            if ( pLabel2 != 0 )
            {
                UpdateLabel(pLabel2, &camPropInfo, &camProp );
            }
		}
	}
	
    bool CamSettingsPage::OnTimer()
    {
        if (m_pCamera == NULL || IsIconified())
        {
            return true;
        }

        // This is only so that the camera settings page doesn't hammer
        // the camera with async reads when it is not the visible page
        Gtk::Notebook* pNotebook;
        m_refXml->get_widget( "notebookCamCtl", pNotebook );
        if ( pNotebook->get_current_page() == 0 )
        {
            UpdateWidgets();	
        }

        return true;
    }

	void CamSettingsPage::OnAutoClicked( PropertyType property, Gtk::CheckButton* pButton )
	{	
        Error error;
		Property camProp;
		camProp.type = property;
		
		error = m_pCamera->GetProperty( &camProp );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting camera property", error );
        }

		camProp.autoManualMode = pButton->get_active();

		error = m_pCamera->SetProperty( &camProp, false );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting camera property", error );
        }
	}
	
	void CamSettingsPage::OnOnOffClicked( PropertyType property, Gtk::CheckButton* pButton )
	{		
        Error error;
		Property camProp;
		camProp.type = property;
		
		error = m_pCamera->GetProperty( &camProp );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting camera property", error );
        }
		
        camProp.onOff = pButton->get_active();
		
        error = m_pCamera->SetProperty( &camProp, false );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting camera property", error );
        }
	}
	
	void CamSettingsPage::OnOnePushClicked( PropertyType property, Gtk::CheckButton* pButton )
	{		
        Error error;
		Property camProp;
		camProp.type = property;
		
		error = m_pCamera->GetProperty( &camProp );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting camera property", error );
        }

		camProp.onePush = pButton->get_active();

		error = m_pCamera->SetProperty( &camProp, false );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting camera property", error );
        }
	}	
	
	void CamSettingsPage::OnHScroll( PropertyType property, Gtk::Adjustment* pAdjustment )
	{
        Glib::Mutex::Lock scrollLock( m_hScrollMutex, Glib::NOT_LOCK );
        if ( scrollLock.try_acquire() != true )
        {
            return;
        }

		double value = pAdjustment->get_value();
        unsigned int valueReg = static_cast<unsigned int>(value);
		
        Error error;
		Property camProp;
		PropertyInfo camPropInfo;
		
		camProp.type = property;
		camPropInfo.type = property;
		
        error = m_pCamera->GetProperty( &camProp );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting camera property", error );
        }

		error = m_pCamera->GetPropertyInfo( &camPropInfo );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error getting camera property information", error );
        }

        // Clamp the abs value
        if ( value > camPropInfo.absMax )
        {
            value = camPropInfo.absMax;
        }

        if ( value < camPropInfo.absMin )
        {
            value = camPropInfo.absMin;
        }

        // Clamp the relative value
        if ( valueReg > camPropInfo.max )
        {
            valueReg = camPropInfo.max;
        }

        if ( valueReg < camPropInfo.min )
        {
            valueReg = camPropInfo.min;
        }
        
        if ( property == WHITE_BALANCE )
        {          
            if ( pAdjustment == m_widgetPropArray[property].pAdjustment1 )
            {
                camProp.valueA = valueReg;
            }
            else if ( pAdjustment == m_widgetPropArray[property].pAdjustment2 )
            {
                camProp.valueB = valueReg;
            }
            else
            {
                // Error
                return;
            }

            camProp.absControl = false;
        }
        else
        {
            if ( camPropInfo.absValSupported == true && 
                m_absMode == true )
            {
                camProp.absValue = static_cast<float>(value);
                camProp.absControl = true;
            }
            else
            {			
                camProp.valueA = valueReg;
                camProp.absControl = false;
            }
        }		
		
		error = m_pCamera->SetProperty( &camProp, false );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error setting camera property", error );
        }
	}
	
	void CamSettingsPage::OnPowerClicked( Gtk::CheckButton* pButton )
	{
		bool on = pButton->get_active();
		SetCameraPower( on );
	}

    void CamSettingsPage::OnAbsModeClicked( Gtk::CheckButton* pButton )
    {
        m_absMode = pButton->get_active();

        // Refresh the unit label and spin buttons
        // The rest of the widgets don't need to be updated
        for ( unsigned int i = 0; i < sk_numProps; i++ )
        {
            if ( m_pCamera == 0 )
            {
                return;
            }           

            Gtk::SpinButton* pSpin1 = m_widgetPropArray[i].pSpinButton1;
            Gtk::SpinButton* pSpin2 = m_widgetPropArray[i].pSpinButton2;
            Gtk::Label* pLabel1 = m_widgetPropArray[i].pLabel1;
            Gtk::Label* pLabel2 = m_widgetPropArray[i].pLabel2;

            const PropertyType k_currPropType = (PropertyType)i;

            Property camProp;
            PropertyInfo camPropInfo;

            // Get the property and property info
            camProp.type = k_currPropType;
            camPropInfo.type = k_currPropType;

            if ( m_pCamera->GetProperty( &camProp ) != PGRERROR_OK ||
                m_pCamera->GetPropertyInfo( &camPropInfo ) != PGRERROR_OK )
            {
                // Perhaps not supported, hide it and continue
                HideProperty( k_currPropType );
                continue;
            }			            

            if ( pLabel1 != 0 )
            {
                UpdateLabel( pLabel1, &camPropInfo, &camProp );		                
            }		

            if ( pLabel2 != 0 )
            {
                UpdateLabel( pLabel2, &camPropInfo, &camProp );		                
            }	

            if ( pSpin1 != 0 )
            {
                UpdateSpinButton( pSpin1, &camPropInfo );
            }

            if ( pSpin2 != 0 )
            {
                UpdateSpinButton( pSpin2, &camPropInfo );
            }  
        }

        UpdateWidgets();
    }

    void CamSettingsPage::UpdateSpinButton( 
        Gtk::SpinButton* pButton, 
        PropertyInfo* pPropInfo )
    {
        if ( pButton == NULL || pPropInfo == NULL )
        {
            return;
        }

        if ( pPropInfo->absValSupported == true )
        {
            pButton->set_digits( m_absMode == true ? 3 : 0 );            
        }

        pButton->set_sensitive(true);
    }
	
	void CamSettingsPage::UpdateAdjustment(  
        Gtk::Adjustment* pAdjustment, 
        PropertyInfo* pPropInfo, 
        Property* pProp )
	{		
        if ( pAdjustment == NULL || 
            pPropInfo == NULL || 
            pProp == NULL )
        {
            return;
        }

        if ( pPropInfo->type == WHITE_BALANCE )
        {
            if ( pAdjustment->get_lower() != pPropInfo->min )
            {
                pAdjustment->set_lower( pPropInfo->min );
            }

            if ( pAdjustment->get_upper() != pPropInfo->max )
            {
                pAdjustment->set_upper( pPropInfo->max );
            }                    

            if ( pAdjustment == m_widgetPropArray[WHITE_BALANCE].pAdjustment1 )
            {
                pAdjustment->set_value( pProp->valueA );
            }
            else if ( pAdjustment == m_widgetPropArray[WHITE_BALANCE].pAdjustment2 )
            {
                pAdjustment->set_value( pProp->valueB );
            }
        }
        else
        {
            if ( pPropInfo->absValSupported == true && m_absMode == true )
            {
                // Set the adjustment using absolute values
                if ( pAdjustment->get_lower() != pPropInfo->absMin )
                {
                    pAdjustment->set_lower( pPropInfo->absMin );
                }

                if ( pAdjustment->get_upper() != pPropInfo->absMax )
                {
                    pAdjustment->set_upper( pPropInfo->absMax );
                }      

                if ( pAdjustment->get_value() != pProp->absValue )
                {
                    pAdjustment->set_value( pProp->absValue );
                }                
            }
            else
            {
                // Set the adjustment using regular values
                if ( pAdjustment->get_lower() != pPropInfo->min )
                {
                    pAdjustment->set_lower( pPropInfo->min );
                }

                if ( pAdjustment->get_upper() != pPropInfo->max )
                {
                    pAdjustment->set_upper( pPropInfo->max );
                }   

                if ( pAdjustment->get_value() != pProp->valueA )
                {
                    pAdjustment->set_value( pProp->valueA );
                }
            }		
        }		
	}
	
	void CamSettingsPage::UpdateLabel( 
        Gtk::Label* pLabel, 
        PropertyInfo* pPropInfo, 
        Property* pProp )
	{			
        if ( pLabel == NULL ||
            pPropInfo == NULL ||
            pProp == NULL )
        {
            return;
        }

		char label[64];
		if ( pPropInfo->type == TEMPERATURE )
		{
			double kelvins = 0.0;
			double celcius = 0.0;
			double fahrenheit = 0.0;
			
			kelvins = pProp->valueA / 10.0f;
			celcius = kelvins - 273.15f;
			fahrenheit = ((celcius * 9.0f ) / 5.0f) + 32.0f;
			
			sprintf( label, 
					"%3.1fK / %3.1fC / %3.1fF",
					kelvins,
					celcius,
					fahrenheit );

            pLabel->show();
            pLabel->set_text( label );
		}
        else if ( pPropInfo->type == WHITE_BALANCE )
        {
            pLabel->hide();
        }
		else
		{
			if ( pProp->present == true )
			{
				if ( pPropInfo->absValSupported == true && 
                    m_absMode == true )
				{	
                    sprintf( label, "%s", pPropInfo->pUnitAbbr );

                    pLabel->show();
                    pLabel->set_text( label );
				}
				else
				{
                    pLabel->hide();
				}
			}
			else
			{
                pLabel->hide();
			}		
		}												
	}
	
	void CamSettingsPage::UpdateAutoCheckButton( Gtk::CheckButton* pButton, bool autoSupported, bool autoManualMode )
	{		
		UpdateCheckButton( pButton, autoSupported, autoManualMode );
	}
	
	void CamSettingsPage::UpdateOnOffCheckButton( Gtk::CheckButton* pButton, bool onOffSupported, bool onOff )
	{
		UpdateCheckButton( pButton, onOffSupported, onOff );
	}
	
	void CamSettingsPage::UpdateOnePushCheckButton( Gtk::CheckButton* pButton, bool onePushSupported, bool onePush )
	{
		UpdateCheckButton( pButton, onePushSupported, onePush );
	}
	
	void CamSettingsPage::UpdateCheckButton( Gtk::CheckButton* pButton, bool bSupported, bool bVal )
	{
        if ( pButton == NULL )
        {
            return;
        }

		if ( bSupported == true )
		{
            if ( pButton->get_state() == Gtk::STATE_INSENSITIVE )
            {
                pButton->set_sensitive( true );
            }
			
            if ( pButton->is_visible() == false )
            {
                pButton->show();
            }

            if ( pButton->get_active() != bVal )
            {
                pButton->set_active( bVal );
            }			
		}
		else
		{
            if ( pButton->is_visible() == true )
            {
                pButton->hide();
            }
		}
	}
	
	void CamSettingsPage::HideProperty( PropertyType property )
	{
		Gtk::HScale* pHScale1 = m_widgetPropArray[property].pHScale1;
        Gtk::HScale* pHScale2 = m_widgetPropArray[property].pHScale2;
		Gtk::CheckButton* pAuto = m_widgetPropArray[property].pAuto;
		Gtk::CheckButton* pOnOff = m_widgetPropArray[property].pOnOff;
		Gtk::CheckButton* pOnePush = m_widgetPropArray[property].pOnePush;	
        Gtk::SpinButton* pSpin1 = m_widgetPropArray[property].pSpinButton1;
        Gtk::SpinButton* pSpin2 = m_widgetPropArray[property].pSpinButton2;
		Gtk::Label* pLabel1 = m_widgetPropArray[property].pLabel1;
        Gtk::Label* pLabel2 = m_widgetPropArray[property].pLabel2;
		
        HideWidget(pHScale1);
        HideWidget(pHScale2);
        HideWidget(pAuto);
        HideWidget(pOnOff);
        HideWidget(pOnePush);
        HideWidget(pSpin1);
        HideWidget(pSpin2);		
        HideWidget( pLabel1 );
        HideWidget( pLabel2 );		
	}

    void CamSettingsPage::HideWidget( Gtk::Widget* pWidget )
    {
        if ( pWidget == NULL )
        {
            return;
        }

        if ( pWidget->get_state() != Gtk::STATE_INSENSITIVE )
        {
            pWidget->set_sensitive( false );
        }

        if ( pWidget->is_visible() == true )
        {       
            pWidget->hide();
        }
    }
	
	void CamSettingsPage::UpdateCameraPowerCheckButton()
	{
		if ( IsCameraPowerAvailable() == true )
		{
            if ( m_pCheckbuttonPower->is_visible() == false )
            {
			    m_pCheckbuttonPower->show();
            }

			if ( GetCameraPower() != m_pCheckbuttonPower->get_active() )
			{
				m_pCheckbuttonPower->set_active( GetCameraPower() );
			}			
		}
		else
		{
            if ( m_pCheckbuttonPower->is_visible() == true )
            {
			    m_pCheckbuttonPower->hide();
            }
		}		
	}
	
	bool CamSettingsPage::IsCameraPowerAvailable()
	{
		const unsigned int powerReg = 0x400;
		unsigned int powerRegVal = 0;
		
		Error error = m_pCamera->ReadRegister( powerReg, &powerRegVal );
		if ( error != PGRERROR_OK )
		{
			// Error
            //ShowErrorMessageDialog( "Error reading camera power register", error );
			return false;
		}
		
		return ( (powerRegVal & 0x00008000 ) != 0 );		
	}
	
	bool CamSettingsPage::GetCameraPower()
	{
		const unsigned int powerReg = 0x610;
		unsigned int powerRegVal = 0 ;
		
		Error error = m_pCamera->ReadRegister( powerReg, &powerRegVal );
		if ( error != PGRERROR_OK )
		{
			// Error
            //ShowErrorMessageDialog( "Error reading camera power register", error );
			return false;
		}
		
		return ( (powerRegVal & (0x1 << 31)) != 0 );
	}
	
	void CamSettingsPage::SetCameraPower( bool on )
	{
		const unsigned int powerReg = 0x610;
		unsigned int powerRegVal = 0 ;

        powerRegVal = (on == true) ? 0x80000000 : 0x0;	
		
		Error error = m_pCamera->WriteRegister( powerReg, powerRegVal );
		if ( error != PGRERROR_OK )
		{
			// Error
            //ShowErrorMessageDialog( "Error writing camera power register", error );
			return;
		}
	}  

    void CamSettingsPage::CreatePropertyMap()
    {
        m_mapProperty[BRIGHTNESS] = "Brightness";
        m_mapProperty[AUTO_EXPOSURE] = "Exposure";
        m_mapProperty[SHARPNESS] = "Sharpness";
        m_mapProperty[WHITE_BALANCE] = "WB";
        m_mapProperty[HUE] = "Hue";
        m_mapProperty[SATURATION] = "Saturation";
        m_mapProperty[GAMMA] = "Gamma";
        m_mapProperty[IRIS] = "Iris";
        m_mapProperty[FOCUS] = "Focus";
        m_mapProperty[ZOOM] = "Zoom";
        m_mapProperty[PAN] = "Pan";
        m_mapProperty[TILT] = "Tilt";
        m_mapProperty[SHUTTER] = "Shutter";
        m_mapProperty[GAIN] = "Gain";         
        m_mapProperty[FRAME_RATE] = "FrameRate";  
        m_mapProperty[TEMPERATURE] = "Temperature";  
    }    
}
