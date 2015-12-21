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
// $Id: FrameRateCounter.h,v 1.5 2009/01/14 22:58:43 release Exp $
//=============================================================================

#ifndef __FRAMERATE_H__
#define __FRAMERATE_H__

#include <deque>

class FrameRateCounter  
{
public:
    FrameRateCounter( unsigned long queueLength = 120 );
    virtual ~FrameRateCounter();

    double GetFrameRate();
    void SetFrameRate( double frameRate );
    void NewFrame();

protected:
    double m_frameRate;
    std::deque<double>  m_frameTime;
    __int64  m_frequency;
    __int64  m_lastTime;
    __int64  m_prevTime;
};



#endif // #ifndef __FRAMERATE_H__
