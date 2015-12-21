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
// $Id: Format7Page.h,v 1.28 2010/07/15 17:48:37 soowei Exp $
//=============================================================================

#ifndef _FORMAT7PAGE_H_
#define _FORMAT7PAGE_H_

#include "BasePage.h"
#include "Format7DrawingArea.h"

namespace FlyCapture2
{
    /** Model for Format7 mode combo box. */
	class Format7ModesColumns : public Gtk::TreeModelColumnRecord
	{
	public:
        /** Text representation of mode. */
		Gtk::TreeModelColumn<Glib::ustring> m_colModeStr;

        /** Actual mode value. */
		Gtk::TreeModelColumn<Mode> m_colMode;	  	
					
        /** Constructor. */
		Format7ModesColumns()
		{ 
			add( m_colModeStr ); 
			add( m_colMode ); 
		}							 
	};
	
    /** Model for Format7 pixel format combo box. */
	class Format7PixelFormatsColumns : public Gtk::TreeModelColumnRecord
	{
	public:
        /** Text representation of pixel format. */
		Gtk::TreeModelColumn<Glib::ustring> m_colPixelFormatStr;

        /** Actual pixel format value. */
		Gtk::TreeModelColumn<PixelFormat> m_colPixelFormat;	  	
				
        /** Constructor. */
		Format7PixelFormatsColumns()
		{ 
			add( m_colPixelFormatStr ); 
			add( m_colPixelFormat ); 
		}							 
	};

	class Format7Page : public BasePage 
	{
	public:
        /** 
        * Constructor. Initialize the object with the specified parameters. 
        *
        * @param pCamera Pointer to a valid Camera object.
        * @param refXml RefPtr to a Glade XML object.
        */
		Format7Page( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );

        /** Destructor. */
		~Format7Page();
			
        /** Update widgets on page. */
		void UpdateWidgets();

        /**
         * Get a signal that can be used to receive events when the Apply
         * button on the page is clicked.
         *
         * @return Signal that can be connected to in order to receive
         *         Format7 applied events.
         */
        sigc::signal<void> signal_format7_applied();

	protected:
        /** Load widgets. */
		void GetWidgets();

        /** Attach signals to widgets. */
		void AttachSignals();	

        sigc::signal<void> m_signal_format7_applied;

	private:
        static const char* sk_vboxMode; 
		static const char* sk_comboBoxMode;

		static const char* sk_comboBoxPixelFormat;

        static const char* sk_vboxGigEBinning;
        static const char* sk_spinGigEHorzBinning;
        static const char* sk_spinGigEVertBinning;
        static const char* sk_checkGigEAveragingEnabled;  

		static const char* sk_labelCursor;
		static const char* sk_labelMaxImageSize;
		static const char* sk_labelImageSizeUnits;
		static const char* sk_labelOffsetUnits;
		static const char* sk_spinF7Left;
		static const char* sk_spinF7Top;
		static const char* sk_spinF7Width;
		static const char* sk_spinF7Height;
		static const char* sk_btnApply;	
        static const char* sk_drawingareaFormat7;

        static const char* sk_vboxF7PacketSize;
        static const char* sk_labelMaxPacketSize;
        static const char* sk_labelMinPacketSize;
        static const char* sk_spinCurrPacketSize;
        static const char* sk_hscalePacketSize;

        static const char* sk_vboxGigEPacketSize;
        static const char* sk_spinGigECurrPacketSize;
        static const char* sk_hscaleGigEPacketSize;

        static const char* sk_vboxGigEPacketDelay;
        static const char* sk_spinGigECurrPacketDelay;
        static const char* sk_hscaleGigEPacketDelay;
			
        // F7 Mode
        Gtk::VBox* m_pVBoxF7Mode;
		Gtk::ComboBox* m_pComboBoxMode;

        // Pixel format
		Gtk::ComboBox* m_pComboBoxPixelFormat;	

        // GigE binning controls
        Gtk::VBox* m_pVBoxGigEBinning;
        Gtk::SpinButton* m_pSpinGigEHorzBinning;
        Gtk::SpinButton* m_pSpinGigEVertBinning;
        Gtk::Adjustment* m_pAdjustmentGigEHorzBinning;
        Gtk::Adjustment* m_pAdjustmentGigEVertBinning;
        Gtk::CheckButton* m_pCheckGigEAveragingEnabled;        

        // Image size
        Gtk::SpinButton* m_pSpinF7Left;
        Gtk::SpinButton* m_pSpinF7Top;
        Gtk::SpinButton* m_pSpinF7Width;
        Gtk::SpinButton* m_pSpinF7Height;
        Gtk::Adjustment* m_pAdjustmentF7Left;
        Gtk::Adjustment* m_pAdjustmentF7Top;
        Gtk::Adjustment* m_pAdjustmentF7Width;
        Gtk::Adjustment* m_pAdjustmentF7Height;
        Gtk::Label* m_pLabelMaxImageSize;
        Gtk::Label* m_pLabelImageSizeUnits;
        Gtk::Label* m_pLabelOffsetUnits;			

        // F7 Packet size
        Gtk::VBox* m_pVboxF7PacketSize;
        Gtk::SpinButton* m_pSpinCurrPacketSize;
        Gtk::HScale* m_pHScalePacketSize;
        Gtk::Adjustment* m_pAdjustmentPacketSize;
        Gtk::Label* m_pLabelMaxPacketSize;
        Gtk::Label* m_pLabelMinPacketSize;		

        // GigE packet size
        Gtk::VBox* m_pVboxGigEPacketSize;
        Gtk::SpinButton* m_pSpinGigEPacketSize;
        Gtk::HScale* m_pHScaleGigEPacketSize;
        Gtk::Adjustment* m_pAdjustmentGigEPacketSize;	

        // GigE packet delay
        Gtk::VBox* m_pVboxGigEPacketDelay;
        Gtk::SpinButton* m_pSpinGigEPacketDelay;
        Gtk::HScale* m_pHScaleGigEPacketDelay;
        Gtk::Adjustment* m_pAdjustmentGigEPacketDelay;

        Gtk::Button* m_pBtnApply;

        Format7DrawingArea* m_pDrawingArea;
			
		Format7ModesColumns m_modeColumns;
		Format7PixelFormatsColumns m_pixelFormatColumns;
			
		Glib::RefPtr<Gtk::ListStore> m_refListStoreModes;
		Glib::RefPtr<Gtk::ListStore> m_refListStorePixelFormats;

        Format7Page();
        Format7Page( const Format7Page& );
        Format7Page& operator=( const Format7Page& );
			
		void PopulateModeComboBox();
		void PopulatePixelFormatComboBox( unsigned int pixelFormats );
			
		void SetActiveModeComboBox( Mode activeMode );
		void SetActivePixelFormatComboBox( PixelFormat activePixelFormat );
			
		Mode GetActiveModeComboBox();
		PixelFormat GetActivePixelFormatComboBox();
			
		void AddToPixelFormatComboBox( Glib::ustring szPixelFormat, PixelFormat pixFmt );
			
		void UpdateF7Info( Mode mode );
		void UpdateF7PacketInfo( Format7ImageSettings imageSettings, Format7PacketInfo info );

        void UpdateGigEImageInfo();
        void UpdateGigEPacketInfo();
        void UpdateGigEPacketDelay();

        // Get IMAGE_POSITION and IMAGE_SIZE for specified mode
        bool GetF7ImageParametersFromCamera(
            Mode mode,
            unsigned int* pLeft,
            unsigned int* pTop,
            unsigned int* pWidth,
            unsigned int* pHeight );
			
		void GetF7InfoFromPage( Format7ImageSettings* pImageSettings );
        void GetGigEImageInfoFromPage( GigEImageSettings* pImageSettings );

        bool ValidateImageSize();
        bool ValidateImageLeft( unsigned int left, unsigned int width, unsigned int offsetHStepSize, unsigned int maxWidth );
        bool ValidateImageTop( unsigned int top, unsigned int height, unsigned int offsetVStepSize, unsigned int maxHeight );
        bool ValidateImageWidth( unsigned int left, unsigned int width, unsigned int imageHStepSize, unsigned int maxWidth );
        bool ValidateImageHeight( unsigned int top, unsigned int height, unsigned int offsetVStepSize, unsigned int maxHeight );
        bool ValidatePacketSize();
			
		bool IsCurrentlyInFormat7();
			
		Mode GetLowestMode();
		PixelFormat GetLowestPixelFormat( Mode mode );

        void ApplyF7Settings();
        void ApplyGigESettings();

        void EnableWidgets();
        void DisableWidgets();

        void OnComboBoxModeChanged();
        void OnComboBoxPixelFormatChanged();

        // Binning event handlers
        bool OnSpinBinningValueFocusLost( GdkEventFocus* event );
        void OnSpinBinningValueChanged();

        // Image size event handlers
        bool OnSpinImageSizeFocusLost( GdkEventFocus* event );
        void OnSpinImageSizeChanged();

        // Packet size event handlers
        bool OnSpinF7PacketSizeFocusLost( GdkEventFocus* event );
        void OnF7PacketSizeChanged();
        void OnApply();

        void OnImageSizeChanged( 
            unsigned int left, 
            unsigned int top, 
            unsigned int width, 
            unsigned int height );
	};
}

#endif // _FORMAT7PAGE_H_
