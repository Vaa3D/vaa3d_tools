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
// $Id: SystemInfoPage.cpp,v 1.8 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "SystemInfoPage.h"

namespace FlyCapture2
{
    const char* SystemInfoPage::sk_lblCPU = "lblSystemCPU";
    const char* SystemInfoPage::sk_lblNumCores = "lblSystemNumCores";
    const char* SystemInfoPage::sk_lblAvailableMemory = "lblSystemAvailableMemory";
    const char* SystemInfoPage::sk_lblByteOrder = "lblSystemByteOrder";
    const char* SystemInfoPage::sk_lblOS = "lblSystemOS";
    const char* SystemInfoPage::sk_lblGraphics = "lblSystemGraphics";
    const char* SystemInfoPage::sk_lblScreenResolution = "lblSystemScreenResolution";
    const char* SystemInfoPage::sk_btnCopySystemInfo = "btnCopySystemInfo";

	SystemInfoPage::SystemInfoPage()
	{
	}
	
	SystemInfoPage::SystemInfoPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
	{
	}
	
	SystemInfoPage::~SystemInfoPage()
	{
	}
			
	void SystemInfoPage::UpdateWidgets()
	{
        SystemInfo infoStruct;
        Utilities::GetSystemInfo( &infoStruct );

        m_pLblCPU->set_text( infoStruct.cpuDescription );

        char numCores[16];
        sprintf( 
            numCores, 
            "%u", 
            static_cast<unsigned int>(infoStruct.numCpuCores) );
        m_pLblNumCores->set_text( numCores );

        char availableMemory[64];
        sprintf( 
            availableMemory, 
            "%u MB", 
            static_cast<unsigned int>(infoStruct.sysMemSize) );
        m_pLblAvailableMemory->set_text( availableMemory );

        if ( infoStruct.byteOrder == BYTE_ORDER_LITTLE_ENDIAN )
        {
            m_pLblByteOrder->set_text( "Little endian" );
        }
        else if ( infoStruct.byteOrder == BYTE_ORDER_BIG_ENDIAN )
        {
            m_pLblByteOrder->set_text( "Big endian" );
        }
        else
        {
            m_pLblByteOrder->set_text( "Unknown" );
        }

        m_pLblOS->set_text( infoStruct.osDescription ); 

        m_pLblGraphics->set_text( infoStruct.gpuDescription );

        char resolution[32];
        if ( infoStruct.screenWidth > 0 && infoStruct.screenHeight > 0 )
        {
            sprintf( 
                resolution, 
                "%u x %u", 
                static_cast<unsigned int>(infoStruct.screenWidth), 
                static_cast<unsigned int>(infoStruct.screenHeight) );
        }
        else
        {
            sprintf( resolution, "Unknown" );
        }
        
        m_pLblScreenResolution->set_text( resolution );
	}
	
	void SystemInfoPage::GetWidgets()
	{
        m_refXml->get_widget( sk_lblCPU, m_pLblCPU );
        m_refXml->get_widget( sk_lblNumCores, m_pLblNumCores );
        m_refXml->get_widget( sk_lblAvailableMemory, m_pLblAvailableMemory );
        m_refXml->get_widget( sk_lblByteOrder, m_pLblByteOrder );
        m_refXml->get_widget( sk_lblOS, m_pLblOS );
        m_refXml->get_widget( sk_lblGraphics, m_pLblGraphics );
        m_refXml->get_widget( sk_lblScreenResolution, m_pLblScreenResolution );
        m_refXml->get_widget( sk_btnCopySystemInfo, m_pBtnCopySystemInfo );
	}
	
	void SystemInfoPage::AttachSignals()
	{
        m_pBtnCopySystemInfo->signal_clicked().connect( 
            sigc::mem_fun(*this, &SystemInfoPage::OnCopySystemInfo) );
	}
    
    void SystemInfoPage::OnCopySystemInfo()
    {                    
        SystemInfo infoStruct;
        Utilities::GetSystemInfo( &infoStruct );
                
        Glib::ustring clipboardStr = "*** FlyCapture2 System Information ***\n";
                
        clipboardStr += "CPU: ";
        clipboardStr += infoStruct.cpuDescription;
        clipboardStr += "\n";

        char numCores[128];
        sprintf( 
            numCores, 
            "Number of CPU cores: %u\n", 
            static_cast<unsigned int>(infoStruct.numCpuCores) );
        
        clipboardStr += numCores;

        char availableMemory[128];
        sprintf( 
            availableMemory, 
            "Available memory: %u MB\n", 
            static_cast<unsigned int>(infoStruct.sysMemSize) );
            
        clipboardStr += availableMemory;
        
        char endianness[128];
        sprintf( 
            endianness, 
            "Endianness: %s\n", 
            infoStruct.byteOrder == BYTE_ORDER_LITTLE_ENDIAN ? "Little endian" : "Big endian" );        
        
        clipboardStr += "OS: ";
        clipboardStr += infoStruct.osDescription;
        clipboardStr += "\n";
        
        clipboardStr += "GPU: ";
        clipboardStr += infoStruct.gpuDescription;
        clipboardStr += "\n";

        char resolution[128];
        if ( infoStruct.screenWidth > 0 && infoStruct.screenHeight > 0 )
        {
            sprintf( 
                resolution, 
                "Resolution: %u x %u\n", 
                static_cast<unsigned int>(infoStruct.screenWidth), 
                static_cast<unsigned int>(infoStruct.screenHeight) );
                
            clipboardStr += resolution;
        }
        else
        {
            clipboardStr += "Resolution: Unknown\n";
        }             
        
        Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
        refClipboard->set_text( clipboardStr );
    }
}
