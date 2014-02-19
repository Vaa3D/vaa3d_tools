//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

# ifndef _RAW_VOLUME_H
# define _RAW_VOLUME_H

# include "VirtualVolume.h" 
# include "RawFmtMngr.h"

typedef char BIT8_UNIT;
typedef short int BIT16_UNIT;
typedef int BIT32_UNIT;
typedef V3DLONG BIT64_UNIT;


class RawVolume : public VirtualVolume
{
    private:

        char *file_name;
        unsigned char *img;
        V3DLONG *sz;
        int datatype;
        int b_swap;
        int header_len;

        void *fhandle;

        void init ( ) throw (iim::IOException);

        // iannello returns the number of channels of images composing the volume
        void initChannels ( ) throw (iim::IOException);

    public:

        RawVolume(const char* _file_name)  throw (iim::IOException);

        ~RawVolume(void);

        // returns a unique ID that identifies the volume format
        std::string getPrintableFormat(){return iim::RAW_FORMAT;}

        // added by Alessandro on 2014-02-18: additional info on the reference system (where available)
        float getVXL_1() {return VXL_H;}
        float getVXL_2() {return VXL_V;}
        float getVXL_3() {return VXL_D;}
        iim::axis getAXS_1() {return iim::horizontal;}
        iim::axis getAXS_2() {return iim::vertical;}
        iim::axis getAXS_3() {return iim::depth;}

        iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);

        iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1, int *channels=0, int ret_type=iim::DEF_IMG_DEPTH)  throw (iim::IOException);
};		

# endif
