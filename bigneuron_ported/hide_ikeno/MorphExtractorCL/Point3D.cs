using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MorphExtractorCL
{
    public class Point3Di
    {
        public int X { get; protected set; }
        public int Y { get; protected set; }
        public int Z { get; protected set; }

        public Point3Di()
        {
            X = 0; Y = 0; Z = 0;
        }

        public Point3Di(int ix, int iy, int iz)
        {
            X = ix; Y = iy; Z = iz;
        }
    }

    public class Point3Dd
    {
        public double X { get; protected set; }
        public double Y { get; protected set; }
        public double Z { get; protected set; }

        public Point3Dd(double x, double y, double z)
        {
            X = x; Y = y; Z = z;
        }

        public double EuclideanDistanceTo(Point3Dd other)
        {
            return Math.Sqrt(Math.Pow(other.X - X, 2) + Math.Pow(other.Y - Y, 2) + Math.Pow(other.Z - Z, 2));
        }
    }
}
