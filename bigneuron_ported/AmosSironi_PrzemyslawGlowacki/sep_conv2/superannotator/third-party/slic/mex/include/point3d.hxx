#ifndef POINT3D_HXX
#define POINT3D_HXX

#include <cstdlib>

template<typename T>
struct Point3D
{
    typedef T   ScalarType;

        T x;
        T y;
        T z;

        inline void add(T dx, T dy, T dz) {
                x += dx;
                y += dy;
                z += dz;
        }

        inline void add( const Point3D<T> &pt ) {
            add( pt.x, pt.y, pt.z );
        }

        inline void divideBy(T N) {
                x /= N;
                y /= N;
                z /= N;
        }

        inline double sqDist(const Point3D<T> &p2) {
            double dx = (p2.x - x);
            double dy = (p2.y - y);
            double dz = (p2.z - z);

            return dx*dx + dy*dy + dz*dz;
        }

        inline T manhDist(const Point3D<T> &p2) {
            return abs(x - p2.x) + abs(y - p2.y) + abs(z - p2.z);
        }

        inline Point3D(T _x, T _y, T _z) {
            x = _x; y = _y; z = _z;
        }

        inline Point3D() {
            x = y = z = 0;
        }
        
        inline float dot( const Point3D<T> &p2 )
        {
			return ((float)x) * ((float)p2.x) + ((float)y) * ((float)p2.y) + ((float)z) * ((float)p2.z);
		}
};

typedef	Point3D<unsigned int> UIntPoint3D;
typedef	Point3D<float> FloatPoint3D;
typedef std::vector<UIntPoint3D>	UIntPoint3DList;

typedef	Point3D<float> FloatPoint3D;

#endif // POINT3D_HXX
