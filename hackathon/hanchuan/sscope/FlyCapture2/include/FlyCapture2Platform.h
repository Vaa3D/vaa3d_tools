//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with Point Grey Research, Inc. (PGR).
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

//=============================================================================
// $Id: FlyCapture2Platform.h,v 1.4 2010/08/12 23:34:17 soowei Exp $
//=============================================================================

#ifndef PGR_FC2_FLYCAPTURE2PLATFORM_H
#define PGR_FC2_FLYCAPTURE2PLATFORM_H

//=============================================================================
// Platform-specific header file for FlyCapture2. 
//
// All the platform-specific code that is required by individual compilers
// to produce the appropriate code for each platform.
//=============================================================================

#if defined(WIN32) || defined(WIN64)

// Windows 32-bit and 64-bit
#ifdef FLYCAPTURE2_EXPORT
#define FLYCAPTURE2_API __declspec( dllexport )
#elif FLYCAPTURE2_STATIC
#define FLYCAPTURE2_API 
#else
#define FLYCAPTURE2_API __declspec( dllimport )
#endif

#if _MSC_VER > 1000
#pragma once
#endif

#elif defined(MAC_OSX)

// Mac OSX

#else

#define FLYCAPTURE2_API
// Linux and all others

#endif

#endif // PGR_FC2_FLYCAPTURE2PLATFORM_H

