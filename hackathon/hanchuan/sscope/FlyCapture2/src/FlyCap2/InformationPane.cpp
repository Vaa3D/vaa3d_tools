//=============================================================================
// Copyright � 2009 Point Grey Research, Inc. All Rights Reserved.
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
// $Id: InformationPane.cpp,v 1.11 2009/04/24 21:31:39 soowei Exp $
//=============================================================================

#include <time.h>

#include "Precompiled.h"
#include "InformationPane.h"

InformationPane::InformationPane(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::HPaned(cobject)
{
    m_refXml = refGlade;
}

InformationPane::~InformationPane()
{
}

void InformationPane::Initialize()
{
    GetWidgets();
}

void InformationPane::GetWidgets()
{
    // FPS    
    m_refXml->get_widget("lblDisplayedFPS", m_pLblDisplayedFPS);
    m_refXml->get_widget("lblProcessedFPS", m_pLblProcessedFPS);
    m_refXml->get_widget("lblRequestedFPS", m_pLblRequestedFPS);

    // Timestamp   
    m_refXml->get_widget("lblTimestampSeconds", m_pLblTimestampSeconds);
    m_refXml->get_widget("lblTimestampMicroseconds", m_pLblTimestampMicroseconds);
    m_refXml->get_widget("lbl1394CycleTimeSeconds", m_pLbl1394CycleTimeSeconds);
    m_refXml->get_widget("lbl1394CycleTimeCount", m_pLbl1394CycleTimeCount);
    m_refXml->get_widget("lbl1394CycleTimeOffset", m_pLbl1394CycleTimeOffset);

    // Image info
    m_refXml->get_widget("lblImageWidth", m_pLblImageWidth);
    m_refXml->get_widget("lblImageHeight", m_pLblImageHeight);
    m_refXml->get_widget("lblImagePixFmt", m_pLblImagePixFmt);
    m_refXml->get_widget("lblImageBitsPerPixel", m_pLblImageBitsPerPixel);

    // Embedded image info
    m_refXml->get_widget("labelEmbeddedGain", m_pLblEmbeddedGain);
    m_refXml->get_widget("labelEmbeddedShutter", m_pLblEmbeddedShutter);
    m_refXml->get_widget("labelEmbeddedBrightness", m_pLblEmbeddedBrightness);
    m_refXml->get_widget("labelEmbeddedExposure", m_pLblEmbeddedExposure);
    m_refXml->get_widget("labelEmbeddedWhiteBalance", m_pLblEmbeddedWhiteBalance);
    m_refXml->get_widget("labelEmbeddedFrameCounter", m_pLblEmbeddedFrameCounter);
    m_refXml->get_widget("labelEmbeddedStrobePattern", m_pLblEmbeddedStrobePattern);
    m_refXml->get_widget("labelEmbeddedGPIOPinState", m_pLblEmbeddedGPIOPinState);
    m_refXml->get_widget("labelEmbeddedROIPosition", m_pLblEmbeddedROIPosition);
}

void InformationPane::UpdateInformationPane( InformationPaneStruct infoStruct )    
{  
    UpdateFrameRateInfo( infoStruct.fps);
    UpdateTimestampInfo( infoStruct.timestamp);
    UpdateImageInfo( infoStruct.imageInfo);
    UpdateEmbeddedInfo( infoStruct.embeddedInfo );    
}

void InformationPane::UpdateFrameRateInfo( FPSStruct fps )
{
    char info[512];

    sprintf( info, "%3.2fHz", fps.processedFrameRate );
    m_pLblProcessedFPS->set_text( info );    

    sprintf( info, "%3.2fHz", fps.displayedFrameRate );
    m_pLblDisplayedFPS->set_text( info );    

    sprintf( info, "%3.2fHz", fps.requestedFrameRate );
    m_pLblRequestedFPS->set_text( info );
}

void InformationPane::UpdateTimestampInfo( TimeStamp timestamp )
{
    char timestampBuff[512];   

    if ( timestamp.seconds != 0)
    {
        const time_t t = timestamp.seconds;
        char *ctimeStr = ctime( (const time_t *)&t);
        ctimeStr[ strlen( ctimeStr) - 1] = 0; // remove the line feed at the end of the string
        ctimeStr[ 10] = 0x0a; // insert a line feed after date
        ctimeStr[ 19] = 0x0a; // insert a line feed after time

        m_pLblTimestampSeconds->set_text( ctimeStr );

        sprintf( timestampBuff, "%u", timestamp.microSeconds );
        m_pLblTimestampMicroseconds->set_text( timestampBuff );
    }

    sprintf( timestampBuff, "%u", timestamp.cycleSeconds );
    m_pLbl1394CycleTimeSeconds->set_text( timestampBuff );

    sprintf( timestampBuff, "%04u", timestamp.cycleCount );
    m_pLbl1394CycleTimeCount->set_text( timestampBuff );

    sprintf( timestampBuff, "%04u", timestamp.cycleOffset );
    m_pLbl1394CycleTimeOffset->set_text( timestampBuff );   
}

void InformationPane::UpdateImageInfo( ImageInfoStruct imageInfo )
{
    double bitsPerPixel;

    char info[512];

    bitsPerPixel = (imageInfo.stride * 8) / static_cast<double>(imageInfo.width);

    sprintf( info, "%u", imageInfo.width );
    m_pLblImageWidth->set_text( info );

    sprintf( info, "%u", imageInfo.height );
    m_pLblImageHeight->set_text( info );

    char pixFmtBuff[128];
    GetPixelFormatStr( imageInfo.pixFmt, pixFmtBuff );
    m_pLblImagePixFmt->set_text( pixFmtBuff );

    sprintf( info, "%2.0f", bitsPerPixel );
    m_pLblImageBitsPerPixel->set_text( info );
}

void InformationPane::UpdateEmbeddedInfo( EmbeddedImageInfoStruct embeddedInfo )
{
    char entry[128];

    sprintf( entry, "%08X", embeddedInfo.Individual.gain );
    m_pLblEmbeddedGain->set_text( entry );

    sprintf( entry, "%08X", embeddedInfo.Individual.shutter );
    m_pLblEmbeddedShutter->set_text( entry );

    sprintf( entry, "%08X", embeddedInfo.Individual.brightness );
    m_pLblEmbeddedBrightness->set_text( entry );

    sprintf( entry, "%08X", embeddedInfo.Individual.exposure );
    m_pLblEmbeddedExposure->set_text( entry );

    sprintf( entry, "%08X", embeddedInfo.Individual.whiteBalance );
    m_pLblEmbeddedWhiteBalance->set_text( entry );

    sprintf( entry, "%08X", embeddedInfo.Individual.frameCounter );
    m_pLblEmbeddedFrameCounter->set_text( entry );

    sprintf( entry, "%08X", embeddedInfo.Individual.strobePattern );
    m_pLblEmbeddedStrobePattern->set_text( entry );

    sprintf( entry, "%08X", embeddedInfo.Individual.GPIOPinState );
    m_pLblEmbeddedGPIOPinState->set_text( entry );

    sprintf( entry, "%08X", embeddedInfo.Individual.ROIPosition );
    m_pLblEmbeddedROIPosition->set_text( entry );
}

void InformationPane::GetPixelFormatStr( PixelFormat pixFmt, char* pPixFmt )
{
    switch( pixFmt )
    {
    case PIXEL_FORMAT_MONO8:
        strcpy( pPixFmt, "Mono 8" ); break;
    case PIXEL_FORMAT_MONO12:
        strcpy( pPixFmt, "Mono 12" ); break;
    case PIXEL_FORMAT_MONO16:
        strcpy( pPixFmt, "Mono 16" ); break;
    case PIXEL_FORMAT_RAW8:
        strcpy( pPixFmt, "Raw 8" ); break;
    case PIXEL_FORMAT_RAW12:
        strcpy( pPixFmt, "Raw 12" ); break;
    case PIXEL_FORMAT_RAW16:
        strcpy( pPixFmt, "Raw 16" ); break;
    case PIXEL_FORMAT_411YUV8:
        strcpy( pPixFmt, "YUV 411" ); break;
    case PIXEL_FORMAT_422YUV8:
        strcpy( pPixFmt, "YUV 422" ); break;
    case PIXEL_FORMAT_444YUV8:
        strcpy( pPixFmt, "YUV 444" ); break;
    case PIXEL_FORMAT_RGB8: // Also RGB
        strcpy( pPixFmt, "RGB 8" ); break;
    case PIXEL_FORMAT_S_MONO16:
        strcpy( pPixFmt, "Signed Mono 16" ); break;
    case PIXEL_FORMAT_S_RGB16:
        strcpy( pPixFmt, "Signed RGB 16" ); break;
    case PIXEL_FORMAT_BGR:
        strcpy( pPixFmt, "BGR" ); break;
    case PIXEL_FORMAT_BGRU:
        strcpy( pPixFmt, "BGRU" ); break;
    case PIXEL_FORMAT_RGBU:
        strcpy( pPixFmt, "RGBU" ); break;
    default:
        strcpy( pPixFmt, "Unknown" ); break;
    }
}
