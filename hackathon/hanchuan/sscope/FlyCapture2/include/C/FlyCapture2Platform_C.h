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
// $Id: FlyCapture2Platform_C.h,v 1.5 2010/08/13 18:49:59 soowei Exp $
//=============================================================================

#ifndef PGR_FC2_FLYCAPTURE2PLATFORM_C_H
#define PGR_FC2_FLYCAPTURE2PLATFORM_C_H

//=============================================================================
// Platform-specific header file for FlyCapture2 C.  
//
// All the platform-specific code that is required by individual compilers
// to produce the appropriate code for each platform.
//=============================================================================

#if defined(WIN32) || defined(WIN64)

// Windows 32-bit and 64-bit
#ifdef FLYCAPTURE2_C_EXPORTS
#define FLYCAPTURE2_C_API __declspec( dllexport )
#elif FLYCAPTURE2_C_STATIC
#define FLYCAPTURE2_C_API
#else
#define FLYCAPTURE2_C_API __declspec( dllimport )
#endif

#if _MSC_VER > 1000
#pragma once
#endif

#elif defined(MAC_OSX)

// Mac OSX

#else

#define FLYCAPTURE2_C_API
#define FLYCAPTURE2_C_CALL_CONVEN
// Linux and all others

#endif

#endif // PGR_FC2_FLYCAPTURE2PLATFORM_C_H

