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
// $Id: Precompiled.h,v 1.12 2009/10/22 00:25:26 soowei Exp $
//=============================================================================

#ifndef PRECOMPILED_H 
#define PRECOMPILED_H 

#if defined(WIN32) || defined(WIN64)
#pragma warning( push, 0 )
#endif

#include <gtkmm.h>
#include <libglademm/xml.h>
#include <giomm.h>
#include <glibmm.h>
#include <gtkglmm.h>
#include "PGRIcon.h"
#include "PGRLogo.h"

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#undef max 
#undef min
#endif

#if defined(WIN32) || defined(WIN64)
#pragma warning(pop)
#endif

namespace Histogram
{
    /**
     * Mode of the histogram window.
     */ 
    enum DrawMode
    {
        MODE_HISTOGRAM, /**< Histogram mode. */
        MODE_ROWCOL /**< Row column plot mode. */
    };

    /**
     * Row column mode.
     */ 
    enum RowColMode
    {
        ROWCOL_COLUMN, /**< Column mode. */
        ROWCOL_ROW, /**< Row mode. */
    };

    /**
     * Row column channels to display.
     */ 
    enum RowColDisplay
    {
        ROWCOL_GREY, /**< Display grey channel. */
        ROWCOL_RED, /**< Display red channel. */
        ROWCOL_GREEN, /**< Display green channel. */
        ROWCOL_BLUE, /**< Display blue channel. */
    };

    /**
     * Row column functions to calculate for.
     */ 
    enum RowColFunction
    {
        ROWCOL_MAX, /**< Maximum channel value. */
        ROWCOL_MEAN, /**< Mean channel value. */
        ROWCOL_MIN, /**< Minimum channel value. */
    };
}

#endif
