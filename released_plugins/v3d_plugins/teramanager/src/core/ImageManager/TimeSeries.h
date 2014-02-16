#ifndef TIMESERIES_H
#define TIMESERIES_H

#include <vector>
#include "VirtualVolume.h"


class TimeSeries : public VirtualVolume
{
    protected:

        std::vector<VirtualVolume*> frames;     // each time frame corresponds to a complete volumetric image
        iim::uint32 t0, t1;                          // only time frames in [t0, t1] are active (default: all)

        // pure virtual method inherithed from abstract class <VirtualVolume>
        void initChannels() throw (iim::IOException);

        // removing access to default constructor
        TimeSeries(void);

    public:

        //CONSTRUCTORS-DESTRUCTOR
        TimeSeries(const char* rootDir) throw (iim::IOException);
        ~TimeSeries(void) throw (iim::IOException);

        // get methods
        std::vector<VirtualVolume*> getFrames() {return frames;}
        std::vector<VirtualVolume*> getActiveFrames() throw (iim::IOException);
        VirtualVolume* getFrameAt(int t) throw (iim::IOException);
        iim::uint32 getNFrames(){return static_cast<iim::uint32>(frames.size());}
        iim::uint32 getNActiveFrames(){return t1 -t0 +1;}


        // set methods
        void setActiveFrames(int _t0, int _t1){t0 = _t0; t1 = _t1;}

        // pure virtual methods inherithed from abstract class <VirtualVolume>
        iim::real32 *loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1)  throw (iim::IOException);
        iim::uint8 *loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException);
};

#endif // TIMESERIES_H
