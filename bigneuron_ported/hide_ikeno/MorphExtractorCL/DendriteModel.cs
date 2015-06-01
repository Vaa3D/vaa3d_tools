using System;
using System.Collections.Generic;

namespace Sigen.Model
{
    public class VoxelCollection
    {
        public VoxelCollection()
        {
        }
    }

    public class VoxelItem
    {
        public double X { get; private set; }
        public double Y { get; private set; }
        public double Z { get; private set; }

        public VoxelItem(double x, double y, double z)
        {
            X = x;
            Y = y;
            Z = z;
        }
    }
}
