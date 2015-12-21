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
// $Id: FrameRateCounter.cpp,v 1.12 2010/01/11 18:21:17 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "FrameRateCounter.h"

FrameRateCounter::FrameRateCounter( unsigned long queueLength )
{
    m_timer.start();    

    double seconds = m_timer.elapsed();

    for( unsigned int i = 0; i < queueLength; i++ )
    {
        m_frameTime.push_back( seconds );
    }

    m_timer.start();    
}

FrameRateCounter::~FrameRateCounter()
{
}


double FrameRateCounter::GetFrameRate()
{
    const unsigned int size = static_cast<unsigned int>(m_frameTime.size());

    Glib::Mutex::Lock lock( m_dequeMutex );

    const double start = m_frameTime[0];
    const double end = m_frameTime[size-1];
    const double diff = end - start;

    double frameRate = 1.0 / (diff / static_cast<double>(size-1));    

    return frameRate;
}


void FrameRateCounter::SetFrameRate( double /*frameRate*/ )
{
    // Nothing to do here 
}


void FrameRateCounter::Reset()
{
    const double seconds = m_timer.elapsed();

    Glib::Mutex::Lock lock( m_dequeMutex );

    const unsigned int queueSize = static_cast<unsigned int>(m_frameTime.size());
    m_frameTime.clear();

    for( unsigned int i = 0; i < queueSize; i++ )
    {
        m_frameTime.push_back( seconds );
    }
}

void FrameRateCounter::NewFrame()
{
    const double seconds = m_timer.elapsed();

    Glib::Mutex::Lock lock( m_dequeMutex );

    m_frameTime.pop_front();
    m_frameTime.push_back( seconds );
}
