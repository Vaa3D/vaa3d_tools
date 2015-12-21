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
// $Id: FrameBufferPage.h,v 1.11 2010/03/23 21:50:56 soowei Exp $
//=============================================================================

#ifndef _FRAMEBUFFERPAGE_H_
#define _FRAMEBUFFERPAGE_H_

#include "BasePage.h"

namespace FlyCapture2
{
    class FrameBufferPage : public BasePage 
    {
    public:
        /** Constructor. Initialize the object with the specified parameters. */
        FrameBufferPage( CameraBase* pCamera, Glib::RefPtr<Gnome::Glade::Xml> refXml );

        /** Destructor. */
        ~FrameBufferPage();

        /** Update widgets on page. */
        void UpdateWidgets();
                
    protected:
        /** Load widgets. */
        void GetWidgets();

        /** Attach signals to widgets. */
        void AttachSignals();	

    private:
        static const unsigned int sk_frameBufferRegIIDC;
        static const unsigned int sk_frameBufferReg;

        static const char* sk_checkbuttonEnableImageHold;
        static const char* sk_labelFrameBufferType;
        static const char* sk_labelFrameBufferTotal;
        static const char* sk_labelFrameBufferUsed;
        static const char* sk_hscaleFrameBuffer;
        static const char* sk_buttonTransmitSelectedImage;
        static const char* sk_buttonRetransmitLastImage;

        Gtk::CheckButton* m_pCheckbuttonEnableImageHold;
        Gtk::Label* m_pLabelFrameBufferType;
        Gtk::Label* m_pLabelFrameBufferTotal;
        Gtk::Label* m_pLabelFrameBufferUsed;
        Gtk::HScale* m_pHscaleFrameBuffer;
        Gtk::Adjustment* m_pAdjustmentFrameBuffer;
        Gtk::Button* m_pButtonTransmitSelectedImage;
        Gtk::Button* m_pButtonRetransmitLastImage;

        // Interval between timer fire
        static const int sk_timeout = 500; // in milliseconds

        bool m_quit;

        bool m_isIIDC;

        FrameBufferPage();
        FrameBufferPage( const FrameBufferPage& );
        FrameBufferPage& operator=( const FrameBufferPage& );        

        bool OnTimer();

        void OnEnableFrameBufferToggled();
        void OnTransmitSelectedImage();
        void OnRetransmitSelectedImage();

        bool IsFrameBufferSupported();
        bool IsFrameBufferEnabled();

        unsigned int GetNumBuffers();
        unsigned int GetNumUsedBuffers();

        void EnableWidgets();
        void DisableWidgets();        

        void EnableIso();
        void DisableIso();
    };
}

#endif // _FRAMEBUFFERPAGE_H_
