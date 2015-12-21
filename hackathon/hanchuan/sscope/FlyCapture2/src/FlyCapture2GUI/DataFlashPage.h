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
// $Id: DataFlashPage.h,v 1.12 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#ifndef _DATAFLASHPAGE_H_
#define _DATAFLASHPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
	class DataFlashPage : public BasePage 
	{
	public:
		DataFlashPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );
		~DataFlashPage();
			
		void UpdateWidgets();
        
	protected:
		void GetWidgets();
		void AttachSignals();		

	private:
        static const unsigned int sk_dataFlashReg = 0x1240;
        static const unsigned int sk_dataFlashOffsetReg = 0x1244;

        static const char* sk_labelDataFlashStatus;
        static const char* sk_labelDataFlashSize;
        static const char* sk_buttonFileToFlash;
        static const char* sk_buttonFlashToFile;
        static const char* sk_buttonEraseFlash;
        static const char* sk_progressbarDataFlash;
        static const char* sk_textviewDataFlashPreview;
        static const char* sk_buttonRefreshDataFlashPreview;

        Gtk::Label* m_pLabelDataFlashStatus;
        Gtk::Label* m_pLabelDataFlashSize;
        Gtk::Button* m_pButtonFileToFlash;
        Gtk::Button* m_pButtonFlashToFile;
        Gtk::Button* m_pButtonEraseFlash;
        Gtk::ProgressBar* m_pProgressbarDataFlash;
        Gtk::TextView* m_pTextviewDataFlashPreview;
        Gtk::Button* m_pButtonRefreshDataFlashPreview;

        Glib::RefPtr<Gtk::TextBuffer> m_refTextBuffer;

        Glib::Mutex m_progressMutex;
        Glib::Dispatcher* m_pProgressEvent;
        Glib::Dispatcher* m_pFlashEraseCompleteEvent;
        Glib::Dispatcher* m_pFlashReadCompleteEvent;
        Glib::Dispatcher* m_pFlashWriteCompleteEvent;
        Glib::Dispatcher* m_pFlashErrorEvent;

        std::string m_saveFilename;
        std::string m_loadFilename;

        unsigned int m_currentPage;
        unsigned int m_numPages;

        unsigned int* m_arFlashData;

        DataFlashPage();
        DataFlashPage( const DataFlashPage& );
        DataFlashPage& operator=( const DataFlashPage& );

        bool IsDataFlashSupported();

        unsigned int GetDataFlashSize();
        unsigned int GetPageSize();
        unsigned int GetNumPages();

        void RefreshTextViewPreview();

        void EnableWidgets();
        void DisableWidgets();  

        void OnFileToFlash();
        void OnFlashToFile();
        void OnEraseFlash();
        void OnRefreshDataFlashPreview();

        void OnUpdateProgressBar();
        void OnFlashEraseComplete();
        void OnFlashReadComplete();
        void OnFlashWriteComplete();
        void OnFlashError();

        /** Worker thread function for erasing the data flash. */
        void EraseFlashWorker();

        /** Worker thread function for reading the data flash to a file. */
        void FlashToFileWorker();

        /** Worker thread function for writing a file to the data flash. */
        void FileToFlashWorker();

        /**
         * Add file filters to the specified dialog.
         *
         * @param dialog The FileChooserDialog to add file filters to.
         */
        void AddFileFilters( Gtk::FileChooserDialog& dialog );
	};
}

#endif // _DATAFLASHPAGE_H_
