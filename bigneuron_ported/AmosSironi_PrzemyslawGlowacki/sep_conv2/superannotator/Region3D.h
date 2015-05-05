#ifndef REGION3D_H
#define REGION3D_H

#include <Matrix3D.h>
#include <SuperVoxeler.h>

/** Region where supervoxels were computed
  * This is to avoid computing supervoxels for the whole volume, which could
  * be infeasible or too expensive
  */
struct Region3D
{
    bool valid; //if region is valid

    UIntPoint3D corner; // (x,y,z) corner
    UIntPoint3D size;   // (width,height,depth) from corner

    Region3D() {
        valid = false;
    }

    // initialization from two corners
    Region3D( const UIntPoint3D &c1, const UIntPoint3D &c2 ) {
        if ( c2.x < c1.x || c2.y < c1.y || c2.z < c1.z ) {
            qWarning("Region3D constructur: No proper corners given!");
            valid = false;
        }

        valid = true;
        corner = c1;
        size = UIntPoint3D( c2.x - c1.x + 1, c2.y - c1.y + 1, c2.z - c1.z + 1 );
    }

    Region3D( const QRect &qrect, unsigned int startZ, unsigned int depth )
    {
        valid = true;
        corner.x = qrect.x();
        corner.y = qrect.y();

        size.x = qrect.width();
        size.y = qrect.height();

        corner.z = startZ;
        size.z = depth;

        valid = true;
    }

    // returns the length of the minimum side of the cube
    unsigned int minSideLength() const
    {
        unsigned int m = size.x;
        if (m > size.y) m = size.y;
        if (m > size.z) m = size.z;

        return m;
    }

    unsigned int maxSideLength() const
    {
        unsigned int m = size.x;
        if (m < size.y) m = size.y;
        if (m < size.z) m = size.z;

        return m;
    }

    // use this region to crop a volume
    template<typename T>
    void useToCrop( const Matrix3D<T> &whole, Matrix3D<T> *cropped ) const
    {
        if (!valid)
            qFatal("Tried to crop volume with invalid region");

        whole.cropRegion( corner.x, corner.y, corner.z, size.x, size.y, size.z, cropped );
    }

    // returns if pt is in the given region and if withoutOffset != 0 => the un-offsetted value of pt
    bool inRegion( const UIntPoint3D &pt, UIntPoint3D *withoutOffset  )
    {
        if ( (pt.x < corner.x) || (pt.y < corner.y) || (pt.z < corner.z) )
            return false;

        unsigned int ox = pt.x - corner.x;
        unsigned int oy = pt.y - corner.y;
        unsigned int oz = pt.z - corner.z;

        if ( (ox >= size.x) || (oy >= size.y) || (oz >= size.z) )
            return false;

        if ( withoutOffset != 0 ) {
            withoutOffset->x = ox;
            withoutOffset->y = oy;
            withoutOffset->z = oz;
        }

        return true;
    }

    // converts pixel list form whole image to the non-offset one (local to the region)
    // assumes that all the pixels are inside the region, othewise there will be problems with negative values!
    template<typename T>
    void croppedToWholePixList( const Matrix3D<T> &wholeVolume, const PixelInfoList &cropped, PixelInfoList &whole)
    {
        whole.resize(cropped.size());
        for (unsigned int i=0; i < whole.size(); i++)
        {
            unsigned int dx, dy, dz;
            whole[i].coords.x = dx = cropped[i].coords.x + corner.x;
            whole[i].coords.y = dy = cropped[i].coords.y + corner.y;
            whole[i].coords.z = dz = cropped[i].coords.z + corner.z;

            whole[i].index = wholeVolume.coordToIdx( dx, dy, dz );
        }
    }

    inline unsigned int totalVoxels() {
        if (!valid)
            return 0;

        return size.x*size.y*size.z;
    }

};

#endif // REGION3D_H
