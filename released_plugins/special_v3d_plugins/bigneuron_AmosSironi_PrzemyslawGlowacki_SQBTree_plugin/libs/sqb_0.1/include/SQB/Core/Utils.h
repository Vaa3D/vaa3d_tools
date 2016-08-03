#ifndef _UTILS_SQB_H_
#define _UTILS_SQB_H_

// This file is part of SQBlib, a gradient boosting/boosted trees implementation.
//
// Copyright (C) 2012 Carlos Becker, http://sites.google.com/site/carlosbecker
//
// SQBlib is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SQBlib is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with SQBlib.  If not, see <http://www.gnu.org/licenses/>.

#include <vector>
#include <cstdlib>

namespace SQB
{
    // sample M indeces from 0..(N-1)
    static void sampleWithoutReplacement( unsigned M, unsigned N, std::vector<unsigned> *idxs )
    {
        if (M > N)  M = N;

        unsigned  max = N-1;

        std::vector<unsigned> toSample(N);
        for (unsigned i=0; i < N; i++)
            toSample[i] = i;

        idxs->resize(M);

        for (unsigned i=0; i < M; i++)
        {
            const unsigned idx = (((unsigned long)rand()) * max) / RAND_MAX;
            (*idxs)[i] = toSample[idx];

            //printf("Idx: %d / %d\n", idx, toSample[idx]);

            toSample[idx] = toSample[max];
            max = max - 1;
        }
    }
}

// real timer (only Linux!)
#ifdef CLOCK_REALTIME
class TimerRT
{
private:
    struct timespec ts1;
public:
    void Reset() {
        clock_gettime( CLOCK_REALTIME, &ts1 );
    }
    
    static bool implemented() { return true; }

    TimerRT() { Reset(); }

    double  elapsed() const
    {
        struct timespec ts2;
        clock_gettime( CLOCK_REALTIME, &ts2 );

        return (double) ( 1.0*(1.0*ts2.tv_nsec - ts1.tv_nsec*1.0)*1e-9
                         + 1.0*ts2.tv_sec - 1.0*ts1.tv_sec );
    }
};
#else
class TimerRT
{
public:
    void Reset() {
    }
    
    static bool implemented() { return false; }

    TimerRT() { Reset(); }

    double  elapsed() const
    {
        return 1.0; // not working
    }
};
#endif

#endif
