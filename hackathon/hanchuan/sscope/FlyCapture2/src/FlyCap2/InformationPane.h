//=============================================================================
// Copyright © 2009 Point Grey Research, Inc. All Rights Reserved.
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
// $Id: InformationPane.h,v 1.4 2010/07/14 17:51:11 soowei Exp $
//=============================================================================

#ifndef PGR_FC2_INFORMATIONPANE_H
#define PGR_FC2_INFORMATIONPANE_H

#include "FlyCapture2.h"

using namespace FlyCapture2;

class InformationPane : public Gtk::HPaned
{
public:
    struct FPSStruct
    {
        double processedFrameRate;
        double displayedFrameRate;
        double requestedFrameRate;

        FPSStruct()
        {
            processedFrameRate = 0.0;
            displayedFrameRate = 0.0;
            requestedFrameRate = 0.0;
        }
    };

    struct ImageInfoStruct
    {
        unsigned int width;
        unsigned int height;
        unsigned int stride;
        PixelFormat pixFmt;

        ImageInfoStruct()
        {
            width = 0;
            height = 0;
            stride = 0;
            pixFmt = NUM_PIXEL_FORMATS;
        }        
    };

    struct EmbeddedImageInfoStruct
    {
        union
        {            
            unsigned int arEmbeddedInfo[10];

            struct
            {
                unsigned int timestamp;
                unsigned int gain;
                unsigned int shutter;
                unsigned int brightness;
                unsigned int exposure;
                unsigned int whiteBalance;
                unsigned int frameCounter;
                unsigned int strobePattern;
                unsigned int GPIOPinState;
                unsigned int ROIPosition;
            } Individual;
        };

        EmbeddedImageInfoStruct()
        {
            memset(arEmbeddedInfo, 0x0, 10);
        }
    };

    struct InformationPaneStruct
    {
        FPSStruct fps; 
        TimeStamp timestamp;
        ImageInfoStruct imageInfo;
        EmbeddedImageInfoStruct embeddedInfo;
    };

    InformationPane(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
    virtual ~InformationPane(void);

    void Initialize();

    void UpdateInformationPane( InformationPaneStruct infoStruct );    

protected:

private:   
    static void GetPixelFormatStr( PixelFormat pixFmt, char* pPixFmtBuffer );

    Glib::RefPtr<Gnome::Glade::Xml> m_refXml;    

    // FPS    
    Gtk::Label* m_pLblDisplayedFPS;
    Gtk::Label* m_pLblProcessedFPS;
    Gtk::Label* m_pLblRequestedFPS;

    // Timestamp   
    Gtk::Label* m_pLblTimestampSeconds;
    Gtk::Label* m_pLblTimestampMicroseconds;
    Gtk::Label* m_pLbl1394CycleTimeSeconds;
    Gtk::Label* m_pLbl1394CycleTimeCount;
    Gtk::Label* m_pLbl1394CycleTimeOffset;

    // Image info
    Gtk::Label* m_pLblImageWidth;
    Gtk::Label* m_pLblImageHeight;
    Gtk::Label* m_pLblImagePixFmt;
    Gtk::Label* m_pLblImageBitsPerPixel;

    // Embedded image info
    Gtk::Label* m_pLblEmbeddedGain;
    Gtk::Label* m_pLblEmbeddedShutter;
    Gtk::Label* m_pLblEmbeddedBrightness;
    Gtk::Label* m_pLblEmbeddedExposure;
    Gtk::Label* m_pLblEmbeddedWhiteBalance;
    Gtk::Label* m_pLblEmbeddedFrameCounter;
    Gtk::Label* m_pLblEmbeddedStrobePattern;
    Gtk::Label* m_pLblEmbeddedGPIOPinState;
    Gtk::Label* m_pLblEmbeddedROIPosition;

    void GetWidgets();    

    void UpdateFrameRateInfo( FPSStruct fps );
    void UpdateTimestampInfo( TimeStamp timestamp );
    void UpdateImageInfo( ImageInfoStruct imageInfo );
    void UpdateEmbeddedInfo( EmbeddedImageInfoStruct embeddedInfo );
};

#endif // PGR_FC2_INFORMATIONPANE_H
