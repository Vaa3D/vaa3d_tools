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
// $Id: FrameRateCounter.cpp,v 1.7 2009/03/25 01:27:14 soowei Exp $
//=============================================================================

#include "stdafx.h"
#include "FrameRateCounter.h"

FrameRateCounter::FrameRateCounter( unsigned long queueLength )
{
    m_frameRate   = 0.0;
    m_prevTime = 0;

    for( unsigned int i = 0; i < queueLength; i++ )
    {
        // 100 milliseconds
        m_frameTime.push_back( 1.0 );
    }

   ::QueryPerformanceFrequency( (LARGE_INTEGER*)&m_frequency );
   ::QueryPerformanceCounter( (LARGE_INTEGER*)&m_prevTime );


}

FrameRateCounter::~FrameRateCounter()
{
}


double FrameRateCounter::GetFrameRate()
{
    return m_frameRate;
}


void FrameRateCounter::SetFrameRate( double frameRate )
{
    m_frameRate = frameRate;
}

void FrameRateCounter::NewFrame()
{
    //DWORD currTime = timeGetTime();
    __int64 currTime;
    ::QueryPerformanceCounter( (LARGE_INTEGER*)&currTime );


    __int64 timeDiff64 = currTime - m_prevTime;
    double  timeDiff = timeDiff64/static_cast<double>(m_frequency);
    if ( timeDiff != 0 )
    {
#if 0
        long queueSize = (unsigned long)(1.0/timeDiff) + 1;
        float absVal = (float)abs( (long)(queueSize - m_frameTime.size()));
        if( (absVal/(float)m_frameTime.size() ) > 0.1 )
        {
            // update queue size due to framerate change.
            for( int i = 0; i < (abs( (long)(queueSize - m_frameTime.size()))); i++ )
            {
                if( queueSize > m_frameTime.size() )
                {
                    // add items
                    m_frameTime.push_back( timeDiff );
                }
                else
                {
                    // dump items
                    m_frameTime.pop_front();
                }
            }
        }
#endif
        m_frameTime.pop_front();
        m_frameTime.push_back( timeDiff );
        double totalTime = 0.0;
        for( unsigned int i = 0; i < m_frameTime.size(); i++ )
        {
            totalTime += m_frameTime[i];
        }

        m_frameRate = 1/(totalTime/m_frameTime.size());

        m_prevTime = currTime;
    }
}
