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
// $Id: DataFlashPage.cpp,v 1.26 2010/07/14 17:51:11 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "DataFlashPage.h"
#include "string.h"
#include "memory.h"

namespace FlyCapture2
{
    const char* DataFlashPage::sk_labelDataFlashStatus = "labelDataFlashStatus";
    const char* DataFlashPage::sk_labelDataFlashSize = "labelDataFlashSize";
    const char* DataFlashPage::sk_buttonFileToFlash = "buttonFileToFlash";
    const char* DataFlashPage::sk_buttonFlashToFile = "buttonFlashToFile";
    const char* DataFlashPage::sk_buttonEraseFlash = "buttonEraseFlash";
    const char* DataFlashPage::sk_progressbarDataFlash = "progressbarDataFlash";
    const char* DataFlashPage::sk_textviewDataFlashPreview = "textviewDataFlashPreview";
    const char* DataFlashPage::sk_buttonRefreshDataFlashPreview = "buttonRefreshDataFlashPreview";

	DataFlashPage::DataFlashPage()
	{
        m_arFlashData = NULL;
        m_currentPage = 0;
        m_numPages = 0;
        m_pProgressEvent = NULL;
	}
	
	DataFlashPage::DataFlashPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml ) : BasePage( pCamera, refXml )
	{
        m_arFlashData = NULL;
        m_currentPage = 0;
        m_numPages = 0;
        m_pProgressEvent = NULL;
	}
	
	DataFlashPage::~DataFlashPage()
	{
        if( m_pProgressEvent != NULL )
        {
            delete m_pProgressEvent;
            m_pProgressEvent = NULL;
        }

        if ( m_pFlashEraseCompleteEvent != NULL )
        {
            delete m_pFlashEraseCompleteEvent;
            m_pFlashEraseCompleteEvent = NULL;
        }

        if ( m_pFlashReadCompleteEvent != NULL )
        {
            delete m_pFlashReadCompleteEvent;
            m_pFlashReadCompleteEvent = NULL;
        }

        if ( m_pFlashWriteCompleteEvent != NULL )
        {
            delete m_pFlashWriteCompleteEvent;
            m_pFlashWriteCompleteEvent = NULL;
        }

        if ( m_pFlashErrorEvent != NULL )
        {
            delete m_pFlashErrorEvent;
            m_pFlashErrorEvent = NULL;
        }
    }
			
	void DataFlashPage::UpdateWidgets()
	{       
        if ( m_pCamera == NULL || IsConnected() != true )
        {
            DisableWidgets();
            return;	
        }

        // Check if the camera is a Dragonfly (as it is not supported)
        if ( ( strstr( m_camInfo.modelName, "Dragonfly" ) != NULL ) &&
            ( strstr( m_camInfo.modelName, "Dragonfly2" ) == NULL ) )
        {
            DisableWidgets();
            return;
        }

        bool supported = IsDataFlashSupported();
        if ( supported == false )
        {
            DisableWidgets();
            return;
        }

        EnableWidgets();

        // Get flash size
        char dataFlashSize[32];
        sprintf( 
            dataFlashSize,
            "%u KB",
            GetDataFlashSize() / 1024 );
        m_pLabelDataFlashSize->set_text( dataFlashSize );

        RefreshTextViewPreview();
	}
	
	void DataFlashPage::GetWidgets()
	{
        m_refXml->get_widget( sk_labelDataFlashStatus, m_pLabelDataFlashStatus );
        m_refXml->get_widget( sk_labelDataFlashSize, m_pLabelDataFlashSize );
        m_refXml->get_widget( sk_buttonFileToFlash, m_pButtonFileToFlash );
        m_refXml->get_widget( sk_buttonFlashToFile, m_pButtonFlashToFile );
        m_refXml->get_widget( sk_buttonEraseFlash, m_pButtonEraseFlash );
        m_refXml->get_widget( sk_progressbarDataFlash, m_pProgressbarDataFlash );
        m_refXml->get_widget( sk_textviewDataFlashPreview, m_pTextviewDataFlashPreview );
        m_refXml->get_widget( sk_buttonRefreshDataFlashPreview, m_pButtonRefreshDataFlashPreview );

        m_refTextBuffer = Gtk::TextBuffer::create();

        // Set the text view to monospace
        m_pTextviewDataFlashPreview->modify_font( Pango::FontDescription("monospace") );

        m_pProgressEvent = new Glib::Dispatcher();
        m_pFlashEraseCompleteEvent = new Glib::Dispatcher();
        m_pFlashReadCompleteEvent = new Glib::Dispatcher();
        m_pFlashWriteCompleteEvent = new Glib::Dispatcher();
        m_pFlashErrorEvent = new Glib::Dispatcher();
	}
	
	void DataFlashPage::AttachSignals()
	{
        m_pButtonFileToFlash->signal_clicked().connect(
            sigc::mem_fun(*this, &DataFlashPage::OnFileToFlash) );

        m_pButtonFlashToFile->signal_clicked().connect(
            sigc::mem_fun(*this, &DataFlashPage::OnFlashToFile) );

        m_pButtonEraseFlash->signal_clicked().connect(
            sigc::mem_fun(*this, &DataFlashPage::OnEraseFlash) );

        m_pButtonRefreshDataFlashPreview->signal_clicked().connect(
            sigc::mem_fun(*this, &DataFlashPage::OnRefreshDataFlashPreview) );

        m_pProgressEvent->connect( 
            sigc::mem_fun(*this, &DataFlashPage::OnUpdateProgressBar) );

        m_pFlashEraseCompleteEvent->connect( 
            sigc::mem_fun(*this, &DataFlashPage::OnFlashEraseComplete) );

        m_pFlashReadCompleteEvent->connect( 
            sigc::mem_fun(*this, &DataFlashPage::OnFlashReadComplete) );

        m_pFlashWriteCompleteEvent->connect( 
            sigc::mem_fun(*this, &DataFlashPage::OnFlashWriteComplete) );

        m_pFlashErrorEvent->connect( 
            sigc::mem_fun(*this, &DataFlashPage::OnFlashError) ) ;
	}

    void DataFlashPage::OnFileToFlash()
    {
        Gtk::FileChooserDialog loadDialog( 
            "Please select a file", 
            Gtk::FILE_CHOOSER_ACTION_OPEN );

        // Add response buttons the the dialog:
        loadDialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
        loadDialog.add_button( Gtk::Stock::OPEN, Gtk::RESPONSE_OK );               

        // Only allow single selection of files
        loadDialog.set_select_multiple( false );

        AddFileFilters( loadDialog );

        int result = loadDialog.run();            
        if ( result != Gtk::RESPONSE_OK )
        {
            return;
        }

        m_loadFilename = loadDialog.get_filename();

        DisableWidgets();

        // Launch in a worker thread so that progress bar can be updated    
        Glib::Thread::create( 
            sigc::mem_fun(*this, &DataFlashPage::FileToFlashWorker),
            false );   
    }

    void DataFlashPage::OnFlashToFile()
    {        
        Gtk::FileChooserDialog saveDialog( 
            "Please enter a filename", 
            Gtk::FILE_CHOOSER_ACTION_SAVE );

        // Add response buttons the the dialog:
        saveDialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
        saveDialog.add_button( Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK );       

        // Set the default filename that appears
        char defaultFilename[128];
        sprintf(
            defaultFilename, 
            "%u-flash.dat",
            m_camInfo.serialNumber );
        saveDialog.set_current_name( defaultFilename );

        // Prompt to overwrite an existing file
        saveDialog.set_do_overwrite_confirmation( true );

        // Only allow single selection of files
        saveDialog.set_select_multiple( false );

        AddFileFilters( saveDialog );

        int result = saveDialog.run();            
        if ( result != Gtk::RESPONSE_OK )
        {
            return;
        }

        m_saveFilename = saveDialog.get_filename();

        DisableWidgets();
        
        // Launch in a worker thread so that progress bar can be updated    
        Glib::Thread::create( 
            sigc::mem_fun(*this, &DataFlashPage::FlashToFileWorker),
            false );        
    }

    void DataFlashPage::OnEraseFlash()
    {
        int retVal = 0;
        retVal = ShowMessageDialog( 
            "Permanent loss of data",
            "This process is irreversible! Do you wish to continue?",
            Gtk::MESSAGE_WARNING,
            Gtk::BUTTONS_YES_NO );

        if ( retVal == Gtk::RESPONSE_NO )
        {
            return;
        }     

        DisableWidgets();
        
        // Launch in a worker thread so that progress bar can be updated    
        Glib::Thread::create( 
            sigc::mem_fun(*this, &DataFlashPage::EraseFlashWorker),            
            false );
    }

    void DataFlashPage::OnRefreshDataFlashPreview()
    {
        RefreshTextViewPreview();
    }

    bool DataFlashPage::IsDataFlashSupported()
    {
        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( sk_dataFlashReg, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading data flash register", error );
            return false;
        }

        return (value >> 31) != 0;
    }

    unsigned int DataFlashPage::GetDataFlashSize()
    {
        return GetPageSize() * GetNumPages();
    }

    unsigned int DataFlashPage::GetPageSize()
    {
        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( sk_dataFlashReg, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading data flash register", error );
            return 0;
        }

        int exponent = static_cast<int>((value & 0x00FFF000) >> 12);
    
        return static_cast<int>(pow(2.0, exponent));
    }

    unsigned int DataFlashPage::GetNumPages()
    {
        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( sk_dataFlashReg, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading data flash register", error );
            return 0;
        }

        int exponent = static_cast<int>(value & 0x00000FFF);

        return static_cast<int>(pow(2.0, exponent));
    }

    void DataFlashPage::RefreshTextViewPreview()
    {
        Error error;
        unsigned int value;
        error = m_pCamera->ReadRegister( sk_dataFlashOffsetReg, &value );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error reading data flash offset register", error );
            return;
        }

        const unsigned int k_previewSize = 256;

        // Read the first kilobyte (1024 bytes/256 quadlets) of data and display it
        unsigned int flashData[k_previewSize];
        error = m_pCamera->ReadRegisterBlock(
            0xFFFF,
            0xF0000000 + ( value * 4 ),
            flashData,
            k_previewSize );
        if ( error != PGRERROR_OK )
        {
            // Error
            ShowErrorMessageDialog( "Error performing block read", error );
            return;
        }

        Glib::ustring previewString;

        for ( unsigned int i=0; i < k_previewSize-4; i += 4 )
        {
            char temp[64];
            sprintf(
                temp,
                "%08X %08X %08X %08X\r\n", 
                flashData[i+0],
                flashData[i+1],
                flashData[i+2],
                flashData[i+3] );

            previewString += temp;
        }

        m_refTextBuffer->set_text( previewString );
        m_pTextviewDataFlashPreview->set_buffer( m_refTextBuffer );
    }

    void DataFlashPage::EnableWidgets()
    {
        m_pButtonFileToFlash->set_sensitive( true );
        m_pButtonFlashToFile->set_sensitive( true );
        m_pButtonEraseFlash->set_sensitive( true );
        m_pTextviewDataFlashPreview->set_sensitive( true );
        m_pButtonRefreshDataFlashPreview->set_sensitive( true );

        m_pLabelDataFlashStatus->set_text("Available");
    }

    void DataFlashPage::DisableWidgets()
    {
        m_pButtonFileToFlash->set_sensitive( false );
        m_pButtonFlashToFile->set_sensitive( false );
        m_pButtonEraseFlash->set_sensitive( false );
        m_pTextviewDataFlashPreview->set_sensitive( false );
        m_pButtonRefreshDataFlashPreview->set_sensitive( false );

        m_pLabelDataFlashStatus->set_text("Not Available");
    }       

    void DataFlashPage::OnUpdateProgressBar()
    {
        Glib::Mutex::Lock lock(m_progressMutex);

        char caption[32];
        unsigned int currPage = m_currentPage + 1;
        double fraction = currPage / static_cast<float>(m_numPages);
        double percentage = fraction * 100.0f;

        sprintf(
            caption,
            "%u / %u (%.0f%%)",
            currPage,
            m_numPages,
            percentage );

        m_pProgressbarDataFlash->set_text( caption );
        m_pProgressbarDataFlash->set_fraction( fraction );            
    }

    void DataFlashPage::OnFlashEraseComplete()
    {
        EnableWidgets();
        RefreshTextViewPreview();
    }

    void DataFlashPage::OnFlashReadComplete()
    {
        // Open the file
        Glib::RefPtr<Gio::File> saveFile;
        saveFile = Gio::File::create_for_path( m_saveFilename );

        // Open output stream
        Glib::RefPtr<Gio::FileOutputStream> saveOutputStream;
        saveOutputStream = saveFile->replace();       

        unsigned int availableFlashSize = GetDataFlashSize();

        // Write the raw data flash to disk
        int retVal = saveOutputStream->write( (void*)m_arFlashData, availableFlashSize );
        if ( retVal == -1 || retVal != static_cast<int>(availableFlashSize) )
        {
            // Error
            ShowErrorMessageDialog( 
                "Error writing data flash to file", 
                "There was an error writing the data flash to file. Aborting file write." );
        }   

        if ( m_arFlashData != NULL )
        {
            delete [] m_arFlashData;
            m_arFlashData = NULL;
        }    

        // Close output stream
        saveOutputStream->close();    

        EnableWidgets();
        RefreshTextViewPreview();
    }

    void DataFlashPage::OnFlashWriteComplete()
    {
        EnableWidgets();
        RefreshTextViewPreview();
    }

    void DataFlashPage::OnFlashError()
    {
        ShowErrorMessageDialog( 
            "Error during flash operation", 
            "The operation was not completed successfully." );
    }

    void DataFlashPage::EraseFlashWorker()
    {
        Error error;
        unsigned int offsetVal = 0;
        error = m_pCamera->ReadRegister( sk_dataFlashOffsetReg, &offsetVal );
        if ( error != PGRERROR_OK )
        {
            // Error
            m_pFlashErrorEvent->emit();
        }        

        unsigned long startOffset = 0xF0000000 + ( offsetVal * 4 );
        unsigned long currOffset = startOffset;
        unsigned int pageSize = GetPageSize();
        unsigned int numPages = GetNumPages();

        unsigned int* flashData;
        flashData = new unsigned int[pageSize];
        memset( flashData, 0xFF, pageSize );        

        for ( unsigned int i=0; i < numPages; i++ )
        {
            Error error;
            error = m_pCamera->WriteRegisterBlock(
                0xFFFF,
                currOffset,                
                flashData,
                pageSize / 4 );
            if ( error != PGRERROR_OK )
            {
                // Error
                m_pFlashErrorEvent->emit();
                break;
            }

            currOffset += pageSize;

            Glib::Mutex::Lock lock(m_progressMutex);

            m_currentPage = i;
            m_numPages = numPages;

            m_pProgressEvent->emit(); 
        }

        delete [] flashData;     

        m_pFlashEraseCompleteEvent->emit();
    }    

    void DataFlashPage::FlashToFileWorker()
    {        
        Error error;
        unsigned int offsetVal = 0;
        error = m_pCamera->ReadRegister( sk_dataFlashOffsetReg, &offsetVal );
        if ( error != PGRERROR_OK )
        {
            // Error
            m_pFlashErrorEvent->emit();
        }        

        unsigned long startOffset = 0xF0000000 + ( offsetVal * 4 );
        unsigned long currOffset = startOffset;
        unsigned int pageSize = GetPageSize();
        unsigned int numPages = GetNumPages();

        unsigned int availableFlashSize = GetDataFlashSize();   

        if ( m_arFlashData != NULL )
        {
            delete [] m_arFlashData;
        }
        m_arFlashData = new unsigned int[availableFlashSize/4];
        memset( m_arFlashData, 0x0, availableFlashSize/4 );

        for ( unsigned int i=0; i < numPages; i++ )
        {      
            Error error;
            error = m_pCamera->ReadRegisterBlock(
                0xFFFF,
                currOffset,
                &m_arFlashData[(i*pageSize)/4],
                pageSize / 4 );
            if ( error != PGRERROR_OK )
            { 
                m_pFlashErrorEvent->emit();
                break;
            }   
            
            currOffset += pageSize;    

            Glib::Mutex::Lock lock(m_progressMutex);

            m_currentPage = i;
            m_numPages = numPages;

            m_pProgressEvent->emit(); 
        }

        m_pFlashReadCompleteEvent->emit();
    }    

    void DataFlashPage::FileToFlashWorker()
    {
        // Open the file
        Glib::RefPtr<Gio::File> loadFile;
        loadFile = Gio::File::create_for_path( m_loadFilename );

        unsigned int availableFlashSize = GetDataFlashSize();

        Glib::RefPtr<Gio::FileInfo> fileInfo = loadFile->query_info();
        unsigned int fileSize = static_cast<unsigned int>(fileInfo->get_size());

        if ( availableFlashSize < fileSize )
        {
            // Error
            m_pFlashErrorEvent->emit();
            return;
        }

        // Open output stream
        Glib::RefPtr<Gio::DataInputStream> loadInputStream;
        loadInputStream = Gio::DataInputStream::create( loadFile->read() );                

        if ( m_arFlashData != NULL )
        {
            delete [] m_arFlashData;
        }
        m_arFlashData = new unsigned int[availableFlashSize/4];
        memset( m_arFlashData, 0x0, availableFlashSize/4 );

        bool retVal;
        gsize bytesRead;
        retVal = loadInputStream->read_all( m_arFlashData, fileSize, bytesRead );
        if ( retVal != true || fileSize != bytesRead )
        {
            // Error
            m_pFlashErrorEvent->emit();
            return;
        }            

        Error error;
        unsigned int offsetVal = 0;
        error = m_pCamera->ReadRegister( sk_dataFlashOffsetReg, &offsetVal );
        if ( error != PGRERROR_OK )
        {
            // Error
            m_pFlashErrorEvent->emit();
            return;
        }        

        unsigned long startOffset = 0xF0000000 + ( offsetVal * 4 );
        unsigned long currOffset = startOffset;
        unsigned int pageSize = GetPageSize();
        unsigned int numPages = GetNumPages();
        for ( unsigned int i=0; i < numPages; i++ )
        {      
            Error error;
            error = m_pCamera->WriteRegisterBlock(                
                0xFFFF,
                currOffset,
                &m_arFlashData[(i*pageSize)/4],
                pageSize / 4 );
            if ( error != PGRERROR_OK )
            { 
                m_pFlashErrorEvent->emit();
                break;
            }   

            currOffset += pageSize;    

            Glib::Mutex::Lock lock(m_progressMutex);

            m_currentPage = i;
            m_numPages = numPages;

            m_pProgressEvent->emit(); 
        }

        if ( m_arFlashData != NULL )
        {
            delete [] m_arFlashData;
            m_arFlashData = NULL;
        }        

        // Close output stream
        loadInputStream->close();    

        m_pFlashWriteCompleteEvent->emit();
    }

    void DataFlashPage::AddFileFilters( Gtk::FileChooserDialog& dialog )
    {
        // Add file filters
        Gtk::FileFilter datFilter;
        datFilter.set_name( "Data flash files (*.dat)" );
        datFilter.add_pattern("*.dat");

        Gtk::FileFilter allFilter;
        allFilter.set_name( "All files (*.*)" );
        allFilter.add_pattern("*.*");

        dialog.add_filter( datFilter );
        dialog.add_filter( allFilter );
    }
}
