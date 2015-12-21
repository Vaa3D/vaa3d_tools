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
// $Id: Precompiled.h,v 1.3 2009/07/20 19:03:20 soowei Exp $
//=============================================================================

#ifndef PRECOMPILED_H 
#define PRECOMPILED_H 

#if defined(WIN32) || defined(WIN64)
#pragma warning( push, 0 )
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <gtkmm.h>
#include <libglademm/xml.h>
#include <giomm.h>
#include <glibmm.h>
#include "PGRIcon.h"

#if defined(WIN32) || defined(WIN64)
#pragma warning(pop)
#endif

#endif
