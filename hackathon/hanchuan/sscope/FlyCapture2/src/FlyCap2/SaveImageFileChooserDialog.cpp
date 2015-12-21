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
// $Id: SaveImageFileChooserDialog.cpp,v 1.9 2010/04/20 18:03:57 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "SaveImageFileChooserDialog.h"
   
SaveImageFileChooserDialog::SaveImageFileChooserDialog()
{
    m_pParentWindow = NULL;
    m_defaultFileName = "Untitled Image";

    PopulateFilterData();
}

SaveImageFileChooserDialog::SaveImageFileChooserDialog( 
    Gtk::Window* pWindow, 
    std::string defaultFileName,
    FlyCapture2::ImageFileFormat format,
    std::string saveImageLocation )
{        
    m_pParentWindow = pWindow;
    m_defaultFileName = defaultFileName;
    m_format = format;
    m_saveImageLocation = saveImageLocation;

    PopulateFilterData();
}

SaveImageFileChooserDialog::~SaveImageFileChooserDialog()
{
}    

void SaveImageFileChooserDialog::PopulateFilterData()
{
    FilterData bmpData;
    bmpData.format = FlyCapture2::BMP;
    bmpData.name = "Windows Bitmap (*.bmp)";
    bmpData.extension = ".bmp";

    FilterData ppmData;
    ppmData.format = FlyCapture2::PPM;
    ppmData.name = "Portable Pixelmap (*.ppm)";
    ppmData.extension = ".ppm";

    FilterData pgmData;
    pgmData.format = FlyCapture2::PGM;
    pgmData.name = "Portable Greymap (*.pgm)";
    pgmData.extension = ".pgm";

    FilterData jpgData;
    jpgData.format = FlyCapture2::JPEG;
    jpgData.name = "JPEG (*.jpg, *.jpeg)";
    jpgData.extension = ".jpg";

    FilterData tiffData;
    tiffData.format = FlyCapture2::TIFF;
    tiffData.name = "Tagged Image File Format (*.tiff)";
    tiffData.extension = ".tiff";

    FilterData pngData;
    pngData.format = FlyCapture2::PNG;
    pngData.name = "Portable Network Graphics (*.png)";
    pngData.extension = ".png";

    FilterData rawData;
    rawData.format = FlyCapture2::RAW;
    rawData.name = "Raw data (*.raw)";
    rawData.extension = ".raw";

    FilterData allData;
    allData.format = FlyCapture2::FROM_FILE_EXT;
    allData.name = "All files (*.*)";
    allData.extension = "";

    m_vecFilters.push_back( bmpData );
    m_vecFilters.push_back( ppmData );
    m_vecFilters.push_back( pgmData );
    m_vecFilters.push_back( jpgData );
    m_vecFilters.push_back( tiffData );
    m_vecFilters.push_back( pngData );
    m_vecFilters.push_back( rawData );
    m_vecFilters.push_back( allData );
}

void SaveImageFileChooserDialog::Run( 
    std::string& filename, 
    FlyCapture2::ImageFileFormat& format, 
    std::string& saveImageLocation )
{
    Gtk::FileChooserDialog dialog( 
        "Please enter a filename", 
        Gtk::FILE_CHOOSER_ACTION_SAVE );

    if ( m_pParentWindow != NULL )
    {
        dialog.set_transient_for( *m_pParentWindow );
    }

    // Add response buttons to the dialog:
    dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
    dialog.add_button( Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK );        

    // Set the default filename that appears
    dialog.set_current_name( m_defaultFileName );

    // Move to the save image location folder if available
    if ( m_saveImageLocation.length() != 0 )
    {
        dialog.set_current_folder(m_saveImageLocation);
    }

    // Prompt to overwrite an existing file
    dialog.set_do_overwrite_confirmation( true );

    // Only allow single selection of files
    dialog.set_select_multiple( false );

    Gtk::FileFilter bmpFilter;
    bmpFilter.set_name("Windows Bitmap (*.bmp)");
    bmpFilter.add_pattern("*.bmp");

    Gtk::FileFilter ppmFilter;
    ppmFilter.set_name("Portable Pixelmap (*.ppm)");
    ppmFilter.add_pattern("*.ppm");

    Gtk::FileFilter pgmFilter;
    pgmFilter.set_name("Portable Greymap (*.pgm)");
    pgmFilter.add_pattern("*.pgm");

    Gtk::FileFilter jpgFilter;
    jpgFilter.set_name("JPEG (*.jpg, *.jpeg)");
    jpgFilter.add_pattern("*.jpg");
    jpgFilter.add_pattern("*.jpeg");

    Gtk::FileFilter tiffFilter;
    tiffFilter.set_name("Tagged Image File Format (*.tiff)");
    tiffFilter.add_pattern("*.tiff");

    Gtk::FileFilter pngFilter;
    pngFilter.set_name("Portable Network Graphics (*.png)");
    pngFilter.add_pattern("*.png");

    Gtk::FileFilter rawFilter;
    rawFilter.set_name("Raw data (*.raw)");
    rawFilter.add_pattern("*.raw");

    Gtk::FileFilter allFilter;
    allFilter.set_name("All files (*.*)");
    allFilter.add_pattern("*");

    dialog.add_filter(bmpFilter);
    dialog.add_filter(ppmFilter);
    dialog.add_filter(pgmFilter);
    dialog.add_filter(jpgFilter);
    dialog.add_filter(tiffFilter);
    dialog.add_filter(pngFilter);
    dialog.add_filter(rawFilter);
    dialog.add_filter(allFilter);

    switch (m_format)
    {
    case FlyCapture2::FROM_FILE_EXT: dialog.set_filter(allFilter); break;
    case FlyCapture2::PGM: dialog.set_filter(pgmFilter); break;
    case FlyCapture2::PPM: dialog.set_filter(ppmFilter); break;
    case FlyCapture2::BMP: dialog.set_filter(bmpFilter); break;
    case FlyCapture2::JPEG: dialog.set_filter(jpgFilter); break;
    case FlyCapture2::JPEG2000: dialog.set_filter(jpgFilter); break;
    case FlyCapture2::TIFF: dialog.set_filter(tiffFilter); break;
    case FlyCapture2::PNG: dialog.set_filter(pngFilter); break;
    case FlyCapture2::RAW: dialog.set_filter(rawFilter); break;
    default: dialog.set_filter(allFilter); break;
    }
    
    const int result = dialog.run();    

    if ( result != Gtk::RESPONSE_OK )
    {
        filename = "";
        format = FlyCapture2::FROM_FILE_EXT;
        return;
    }

    filename = dialog.get_filename();  
    saveImageLocation = dialog.get_current_folder();

    const Gtk::FileFilter* pSelectedFilter = dialog.get_filter();
    Glib::ustring selectedFilterName = pSelectedFilter->get_name();

    bool foundFilter = false;

    for ( unsigned int i=0; i < m_vecFilters.size(); i++ )
    {
        if ( m_vecFilters[i].name == selectedFilterName )
        {
            // Assign the format that was found
            format = m_vecFilters[i].format;

            // Append the extension on to the filename
            filename += m_vecFilters[i].extension;

            foundFilter = true;
            break;
        }
    }        

    if ( foundFilter == false )
    {
        // Didn't find an extension
        if ( filename.find(".bmp") != std::string::npos )
        {
            format = FlyCapture2::BMP;
        }
        else if ( filename.find(".ppm") != std::string::npos)
        {
            format = FlyCapture2::PPM;
        }
        else if ( filename.find(".pgm") != std::string::npos)
        {
            format = FlyCapture2::PGM;
        }
        else if ( ( filename.find(".jpg") != std::string::npos) || 
            ( filename.find(".jpeg") != std::string::npos) )
        {
            format = FlyCapture2::JPEG;
        }
        else if ( filename.find(".tiff") != std::string::npos)
        {
            format = FlyCapture2::TIFF;
        }
        else if ( filename.find(".png") != std::string::npos)
        {
            format = FlyCapture2::PNG;
        }
        else if ( filename.find(".raw") != std::string::npos)
        {
            format = FlyCapture2::RAW;
        }    
        else
        {
            // No extension and no file format.
            // If we get here, save it as .png
            format = FlyCapture2::PNG;
        }
    }    
}