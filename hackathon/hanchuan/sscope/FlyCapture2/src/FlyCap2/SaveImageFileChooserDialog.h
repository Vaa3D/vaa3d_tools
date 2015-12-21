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
// $Id: SaveImageFileChooserDialog.h,v 1.4 2010/04/20 18:03:57 soowei Exp $
//=============================================================================

#include "FlyCapture2.h"

struct FilterData
{
    FlyCapture2::ImageFileFormat format;
    std::string name;
    std::string extension;
};

class SaveImageFileChooserDialog
{
public:   
    SaveImageFileChooserDialog();
    SaveImageFileChooserDialog( 
        Gtk::Window* pWindow, 
        std::string defaultFileName,
        FlyCapture2::ImageFileFormat format,
        std::string saveImageLocation );
    virtual ~SaveImageFileChooserDialog();

    /**
     * Display a file chooser dialog.
     *
     * @param filename The filename selected.
     * @param format The file format selected.
     */
    void Run( 
        std::string& filename, 
        FlyCapture2::ImageFileFormat& format, 
        std::string& saveImageLocation );

protected:        

private:    
    SaveImageFileChooserDialog( const SaveImageFileChooserDialog& );
    SaveImageFileChooserDialog& operator=( const SaveImageFileChooserDialog& );

    Gtk::Window* m_pParentWindow;

    std::vector<FilterData> m_vecFilters;

    std::string m_defaultFileName;
    FlyCapture2::ImageFileFormat m_format;
    std::string m_saveImageLocation;

    void PopulateFilterData();
};
