// Curve Drawing related codes  by Hang, May 19 2012
#ifndef __SMOOTH_CURVE_H__
#define __SMOOTH_CURVE_H__

// code modified from hanchuan

template <class T> //should be a struct included members of (x,y,z), like Coord3D
bool smooth_curve(std::vector<T *> & mCoord, int winsize)
{
    //std::cout<<" smooth_curve ";
    if (winsize<2) return true;

    std::vector<T*> mC = mCoord; // a copy
    V3DLONG N = mCoord.size();
    int halfwin = winsize/2;

    for (int i=1; i<N-1; i++) // don't move start & end point
    {
        std::vector<T*> winC;
        std::vector<double> winW;
        winC.clear();
        winW.clear();

        winC.push_back( mC[i] );
        winW.push_back( 1.+halfwin );
        for (int j=1; j<=halfwin; j++)
        {
            int k1 = i+j;   if(k1<0) k1=0;  if(k1>N-1) k1=N-1;
            int k2 = i-j;   if(k2<0) k2=0;  if(k2>N-1) k2=N-1;
            winC.push_back( mC[k1] );
            winC.push_back( mC[k2] );
            winW.push_back( 1.+halfwin-j );
            winW.push_back( 1.+halfwin-j );
        }
        //std::cout<<"winC.size = "<<winC.size()<<"\n";

        double s, x,y,z;
        s = x = y = z = 0;
        for (int ii=0; ii<winC.size(); ii++)
        {
            x += winW[ii]* winC[ii]->x;
            y += winW[ii]* winC[ii]->y;
            z += winW[ii]* winC[ii]->z;
            s += winW[ii];
        }
        if (s)
        {
            x /= s;
            y /= s;
            z /= s;
        }

        mCoord[i]->x = x; // output
        mCoord[i]->y = y; // output
        mCoord[i]->z = z; // output
    }
    return true;
}

template <class T> //should be a struct included members of (x,y,z), like Coord3D
bool smooth_curve_and_radius(std::vector<T *> & mCoord, int winsize)
{
    //std::cout<<" smooth_curve ";
    if (winsize<2) return true;

    std::vector<T*> mC = mCoord; // a copy
    V3DLONG N = mCoord.size();
    int halfwin = winsize/2;

    for (int i=1; i<N-1; i++) // don't move start & end point
    {
        std::vector<T*> winC;
        std::vector<double> winW;
        winC.clear();
        winW.clear();

        winC.push_back( mC[i] );
        winW.push_back( 1.+halfwin );
        for (int j=1; j<=halfwin; j++)
        {
            int k1 = i+j;   if(k1<0) k1=0;  if(k1>N-1) k1=N-1;
            int k2 = i-j;   if(k2<0) k2=0;  if(k2>N-1) k2=N-1;
            winC.push_back( mC[k1] );
            winC.push_back( mC[k2] );
            winW.push_back( 1.+halfwin-j );
            winW.push_back( 1.+halfwin-j );
        }
        //std::cout<<"winC.size = "<<winC.size()<<"\n";

        double s, x,y,z,r;
        s = x = y = z = 0;
        for (int ii=0; ii<winC.size(); ii++)
        {
//            x += winW[ii]* winC[ii]->x;
//            y += winW[ii]* winC[ii]->y;
//            z += winW[ii]* winC[ii]->z;
            r += winW[ii]* winC[ii]->radius;
            s += winW[ii];
        }
        if (s)
        {
//            x /= s;
//            y /= s;
//            z /= s;
			r /= s;
        }

//        mCoord[i]->x = x; // output
//        mCoord[i]->y = y; // output
//        mCoord[i]->z = z; // output
		mCoord[i]->radius = r; // output
    }
    return true;
}


#endif
