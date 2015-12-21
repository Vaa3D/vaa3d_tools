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
// $Id: SerialPortWindow.h,v 1.7 2009/08/20 22:47:14 soowei Exp $
//=============================================================================

#ifndef PGR_FC2_SERIALPORTWINDOW_H
#define PGR_FC2_SERIALPORTWINDOW_H

#include <iostream>
#include <bitset>
#include <queue>

#include "FlyCapture2.h"
#include "FlyCapture2GUI.h"

using namespace FlyCapture2;

class SerialPortWindow
{
public:
    SerialPortWindow();
    ~SerialPortWindow();

    bool Initialize();    
    bool Run( PGRGuid guid );        
    bool Cleanup();

protected:

private:
    /** Interval between timer events, in milliseconds. */
    static const int sk_timeout = 100;

    Glib::RefPtr<Gnome::Glade::Xml> m_refXml;
    Gtk::Window* m_pWindow;		

    Gtk::ImageMenuItem* m_pMenuQuit;
    Gtk::ImageMenuItem* m_pMenuAbout;

    Gtk::Label* m_pLabelSerialNumber;
    Gtk::Label* m_pLabelModel;
    Gtk::Label* m_pLabelSensor;
    Gtk::Label* m_pLabelInterface;

    Gtk::Label* m_pLabelBaudRate;
    Gtk::Label* m_pLabelParity;
    Gtk::Label* m_pLabelCharacterLength;
    Gtk::Label* m_pLabelStopBits;
    Gtk::Label* m_pLabelBufferSize;

    Gtk::Entry* m_pEntryTransmit;
    Gtk::Button* m_pButtonTransmit;

    Gtk::TextView* m_pTextViewReceiveData;
    Glib::RefPtr<Gtk::TextBuffer> m_refTextBuffer;

    Gtk::Button* m_pButtonQuit;

    /** Pixbuf for the PGR icon. */
    Glib::RefPtr<Gdk::Pixbuf> m_iconPixBuf;   

    /** Pointer to the receive loop thread handle. */
    Glib::Thread* m_pReceiveLoop;

    /** The camera object. */
    Camera m_camera;

    /** Whether to exit the receive loop. */
    bool m_run;

    /** Message queue for messages to be displayed. */
    std::queue<Glib::ustring> m_messageQueue;

    /** Mutex protecting access to the message queue. */
    Glib::Mutex m_messageMutex;

    /** Load widgets. */
    void GetWidgets();

    /** Attach signals to widgets. */
    void AttachSignals();

    /** Populate the camera information widgets. */
    void PopulateCameraInfo();

    /** Populate the connection parameter widgets. */
    void PopulateConnectionParameters();

    /** Load the PGR icon. */
    void LoadIcon();

    /** Launch the receive thread. */
    void LaunchReceiveLoop();

    /** Receive thread function. */
    void ReceiveLoop( SerialPortWindow* pWindow );

    /** Timer event handler. */
    bool OnTimer();

    /** Transmit button handler. */
    void OnTransmit();

    /** Quit button handler. */
    void OnQuit();

    /** Destroy handler. Called when clicking "X" on the window. */
    bool OnDestroy( GdkEventAny* event );   

    /** Quit menu item handler. */
    void OnMenuQuit();

    /** About menu item handler. */
    void OnMenuAbout();

    /**
     * Get current values in the serial control register.     
     *
     * @return A bitset containing the register value.
     */
    std::bitset<32> GetSerialControlBS();

    /**
     * Set the specified value to the serial control register.
     *
     * @param serialControlBS Bitset to be set to register.
     *
     * @return Whether the write to the camera was successful.
     */
    bool SetSerialControlBS( std::bitset<32> serialControlBS );

    /**
     * Push a message on to message queue.
     *
     * @param message Message to be pushed.
     */
    void PushMessage( Glib::ustring message );

    /**
     * Pop a message off the message queue.     
     *
     * @return The message at the front of the message queue.
     */
    Glib::ustring PopMessage();

    /**
     * Reverse the endianness of a specified string.
     *
     * @param original Original string.
     *
     * @return The string, reversed in endianness.
     */
    Glib::ustring ReverseStrEndianness( Glib::ustring original );

    /**
     * Get the current baud rate setting, given the serial control register.
     *
     * @param serialRegVal Serial control register value.
     *
     * @return Current baud rate.
     */
    static unsigned int GetBaudRate( unsigned int serialRegVal );

    /**
     * Get the character length setting, given the serial control register.
     *
     * @param serialRegVal Serial control register value.
     *
     * @return Current character length.
     */
    static unsigned int GetCharacterLength( unsigned int serialRegVal );

    /**
     * Get the current parity setting, given the serial control register.
     *
     * @param serialRegVal Serial control register value.
     *
     * @return Current parity setting.
     */
    static unsigned int GetParity( unsigned int serialRegVal );

    /**
     * Get the current stop bit setting, given the serial control register.
     *
     * @param serialRegVal Serial control register value.
     *
     * @return Current stop bit setting.
     */
    static float GetStopBit( unsigned int serialRegVal );

    /**
     * Get the current buffer size setting, given the serial control register.
     *
     * @param serialRegVal Serial control register value.
     *
     * @return Current buffer size setting.
     */
    static unsigned int GetBufferSize( unsigned int serialRegVal );

    static int ShowErrorMessageDialog( Glib::ustring mainTxt, Error error );
    static int ShowErrorMessageDialog( Glib::ustring mainTxt, Glib::ustring secondaryTxt );
};

#endif // PGR_FC2_SERIALPORTWINDOW_H
