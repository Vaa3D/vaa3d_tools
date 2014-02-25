#ifndef TIMESERIES_H
#define TIMESERIES_H

#include <vector>
#include "VirtualVolume.h"


class TimeSeries : public VirtualVolume
{
    protected:

        std::vector<VirtualVolume*> frames;     // each time frame corresponds to a complete volumetric image

        // pure virtual method inherithed from abstract class <VirtualVolume>
        void initChannels() throw (iim::IOException);

        // removing access to default constructor
        TimeSeries(void);

    public:

        //CONSTRUCTORS-DESTRUCTOR
        TimeSeries(const char* rootDir, std::string frames_format = "") throw (iim::IOException);
        ~TimeSeries(void) throw (iim::IOException);

        // get methods
        std::vector<VirtualVolume*> getFrames() {return frames;}
        std::vector<VirtualVolume*> getActiveFrames() throw (iim::IOException);
        VirtualVolume* getFrameAt(int t) throw (iim::IOException);
        iim::uint32 getNFrames(){return static_cast<iim::uint32>(frames.size());}

        // returns a unique ID that identifies the volume format
        std::string getPrintableFormat(){return iim::TIME_SERIES;}

        // added by Alessandro on 2014-02-18: additional info on the reference system (where available)
        float getVXL_1() {return frames.empty() ? 0 : frames[0]->getVXL_1();}
        float getVXL_2() {return frames.empty() ? 0 : frames[0]->getVXL_2();}
        float getVXL_3() {return frames.empty() ? 0 : frames[0]->getVXL_3();}
        iim::axis getAXS_1() {return frames.empty() ? iim::axis_invalid : frames[0]->getAXS_1();}
        iim::axis getAXS_2() {return frames.empty() ? iim::axis_invalid : frames[0]->getAXS_2();}
        iim::axis getAXS_3() {return frames.empty() ? iim::axis_invalid : frames[0]->getAXS_3();}

        // set active channels (@OVERRIDES VirtualVolume.h by Alessandro on 2014-02-23)
        void setActiveChannels ( iim::uint32 *_active, int _n_active );


        // pure virtual methods inherithed from abstract class <VirtualVolume>
        iim::real32 *loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1)  throw (iim::IOException);
        iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                   int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException);
};

#endif // TIMESERIES_H
