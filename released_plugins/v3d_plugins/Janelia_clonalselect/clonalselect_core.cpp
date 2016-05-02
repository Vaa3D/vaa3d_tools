/* clonalselect_core.cpp
 * 2013-01-13: create this program by Yang Yu
 */


#ifndef __CLONALSELECT_CORE_CPP__
#define __CLONALSELECT_CORE_CPP__

//
#include "clonalselect_core.h"

//
/// point clouds class
//

PointClouds::PointClouds(){thresh=0.1;}
PointClouds::~PointClouds(){}

//
int PointClouds::read(string fn)
{
    ifstream pFile(fn.c_str());
    string str, buf;
    char letter;
    long l;
    unsigned char u;

    if(pFile.is_open())
    {
        //
        pFile >> letter;
        if(letter=='#')
            getline(pFile, str); // read comments
        if(strcmp(str.c_str(), PCD_COMMENT))
        {
            cout<<"Invalid PCD file"<<endl;
            return -1;
        }
        else
        {
            // header
            if(!pFile.eof()) getline(pFile, str); // \n

            if(!pFile.eof()) getline(pFile, str);
            istringstream iss(str);
            iss >> buf; // FIELDS

            if(!pFile.eof()) getline(pFile, str);
            iss.str(str);
            iss >> buf; // TYPE
            iss >> u;
            pcdheadinfo.x = (DataSize)u;
            iss >> u;
            pcdheadinfo.y = (DataSize)u;
            iss >> u;
            pcdheadinfo.z = (DataSize)u;
            iss >> u;
            pcdheadinfo.v = (DataSize)u;

            if(!pFile.eof()) getline(pFile, str);
            iss.str(str);
            iss >> buf; // DIMENSIONS
            iss >> pcdheadinfo.width; iss >> pcdheadinfo.height; iss >> pcdheadinfo.depth;

            if(!pFile.eof()) getline(pFile, str);
            iss.str(str);
            iss >> buf; // POINTS
            iss >> pcdheadinfo.len;

            if(!pFile.eof()) getline(pFile, str);
            iss.str(str);
            iss >> buf; // MAXVALUE
            iss >> pcdheadinfo.maxv;

            if(!pFile.eof()) getline(pFile, str);
            iss.str(str);
            iss >> buf; // DATA
            iss >> buf;

            if(strcmp(buf.c_str(), "ASCII")==0)
            {
                pcdheadinfo.st = (StoreType)0;
            }
            else if(strcmp(buf.c_str(), "BINARY")==0)
            {
                pcdheadinfo.st = (StoreType)1;
            }
            else
            {
                cout << "Invalid DATA Type"<<endl;
                return -3;
            }

            // point cloud data
            long count=0;
            Point<unsigned short, unsigned short> pt; //

            if((int)(pcdheadinfo.st)==0) // ascii
            {
                while( !pFile.eof() && getline(pFile, str) && count<pcdheadinfo.len)
                {
                    iss.str(str);

                    if(iss.fail() || iss.eof())
                        continue;

                    iss >> pt.x; iss >> pt.y; iss >> pt.z; iss >> pt.v;
                    points.push_back(pt);

                    count++;
                }
            }
            else // binary
            {
                long length = pcdheadinfo.len * 4 * 2; // short int, x, y, z, v

                char *p = NULL;

                try
                {
                    p = new char [length];
                }
                catch(...)
                {
                    cout<<"Invalid allocate memory"<<endl;
                    return -2;
                }

                pFile.read(p, length);

                unsigned short *pUS = (unsigned short *)p;

                long count=0;
                for(long i=0; i<pcdheadinfo.len; i++)
                {
                    pt.x = pUS[count++]; pt.y = pUS[count++]; pt.z = pUS[count++]; pt.v = pUS[count++];
                    points.push_back(pt);
                }

                // de-alloc
                y_del<char>(p);

            }
        }
    }
    else
    {
        cout << "Unable to open the PCD file"<<endl;
        pFile.close();
        return false;
    }

    pFile.close();

    return 0;
}

int PointClouds::write(string fn)
{
    FILE *pFile=0;
    pFile = fopen(fn.c_str(),"wt");

    // Header
    fprintf(pFile, "# Point Clouds Data version 1.0 \n\n"); // PCD_COMMENT
    fprintf(pFile, "FIELDS x y z v \n");
    fprintf(pFile, "TYPE %d %d %d %d \n", (int)(pcdheadinfo.x), (int)(pcdheadinfo.y), (int)(pcdheadinfo.z), (int)(pcdheadinfo.v));
    fprintf(pFile, "DIMENSIONS %ld %ld %ld \n", pcdheadinfo.width, pcdheadinfo.height, pcdheadinfo.depth);
    fprintf(pFile, "POINTS %ld \n", pcdheadinfo.len);
    fprintf(pFile, "MAXVALUE %d \n", pcdheadinfo.maxv);

    // Point Cloud Data
    if((int)(pcdheadinfo.st)==0) // ascii
    {
        fprintf(pFile, "DATA ASCII \n");
        for(long i=0; i<pcdheadinfo.len; i++)
        {
            fprintf(pFile, "%d %d %d %d \n", points.at(i).x, points.at(i).y, points.at(i).z, points.at(i).v);
        }
    }
    else // binary
    {
        fprintf(pFile, "DATA BINARY \n");
        size_t length = pcdheadinfo.len * 4;

        unsigned short *p = NULL;

        try
        {
            p = new unsigned short [length];
        }
        catch(...)
        {
            cout<<"Invalid allocate memory"<<endl;
            return -2;
        }

        long idx = 0;
        for(long i=0; i<pcdheadinfo.len; i++)
        {
            p[idx++] = points.at(i).x;
            p[idx++] = points.at(i).y;
            p[idx++] = points.at(i).z;
            p[idx++] = points.at(i).v;
        }

        fwrite((unsigned char*)p, 2, length, pFile);

        // de-alloc
        y_del<unsigned short>(p);
    }

    fclose(pFile);

    return 0;
}

int PointClouds::convert(unsigned char* p, long sx, long sy, long sz)
{

    // convert the mask images into point clouds
    if(points.size()>0) points.clear();

    pcdheadinfo.width = sx;
    pcdheadinfo.height = sy;
    pcdheadinfo.depth = sz;
    pcdheadinfo.len = 0;

    pcdheadinfo.x = (DataSize)2;
    pcdheadinfo.y = (DataSize)2;
    pcdheadinfo.z = (DataSize)2;
    pcdheadinfo.v = (DataSize)1;

    pcdheadinfo.maxv = 0;

    pcdheadinfo.st = (StoreType)1;

    Point<unsigned short, unsigned short> pt;
    for(long k=0; k<sz; k++)
    {
        long offset_k = k*sx*sy;
        for(long j=0; j<sy; j++)
        {
            long offset_j = offset_k + j*sx;
            for(long i=0; i<sx; i++)
            {
                unsigned char val = p[offset_j + i];

                if(val>0)
                {
                    if(val > pcdheadinfo.maxv)
                        pcdheadinfo.maxv = val;

                    pt.x = i;
                    pt.y = j;
                    pt.z = k;

                    pt.v = val;

                    points.push_back(pt);

                    pcdheadinfo.len++;
                }
            }
        }
    }

    //
    return 0;
}

#endif // __CLONALSELECT_CORE_CPP__



