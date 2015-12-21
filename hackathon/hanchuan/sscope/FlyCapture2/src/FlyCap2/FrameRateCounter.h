//=============================================================================
// Copyright © 2004 Point Grey Research, Inc. All Rights Reserved.
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
// $Id: FrameRateCounter.h,v 1.9 2009/08/26 18:30:35 soowei Exp $
//=============================================================================
#ifndef PGR_FC2_FRAMERATECOUNTER_H
#define PGR_FC2_FRAMERATECOUNTER_H

#include <deque>

/**
 * Frame rate counter class. Calculates frame rate using an average of n
 * images.
 */ 
class FrameRateCounter  
{
public:
    /** Constructor. */
    FrameRateCounter( unsigned long queueLength = 10 );

    /** Destructor. */
    virtual ~FrameRateCounter();

    /**
     * Get the frame rate.
     *
     * @return Frame rate.
     */
    double GetFrameRate();

    /**
     * Set the frame rate. This function is not implemented.
     *
     * @param frameRate The frame rate to set.
     */
    void SetFrameRate( double frameRate );

    /** Resets the counter. */
    void Reset();

    /** Inform the class that there is a new frame. */
    void NewFrame();

protected:
    std::deque<double>  m_frameTime;
    Glib::Timer m_timer;  

    Glib::Mutex m_dequeMutex;
};

#endif // #ifndef PGR_FC2_FRAMERATECOUNTER_H
