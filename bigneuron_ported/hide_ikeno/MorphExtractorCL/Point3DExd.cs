using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MorphExtractorCL
{
    class Point3DExd : Point3Di
    {
        //public Point3D coord;
        public bool flag { get; private set; }
        public IList<Point3DExd> cobj { get; private set; }
        private uint label;
        private uint distance;

        public Point3DExd(int x, int y, int z)
        {
            Z = z;
            Y = y;
            X = x;

            flag = false;
            label = 0;
            distance = 0;
            cobj = new List<Point3DExd>();
        }

        public void AddConnectedObj(Point3DExd obj)
        {
            cobj.Add(obj);
        }

        public void ResetFlag() {
            flag = false;
        }

        public void SetFlag() {
            flag = true;
        }

        public uint Label
        {
            get { return label; }
            set { flag = true; label = value; }
        }

        public uint Distance
        {
            get { return distance; }
            set { flag = true; distance = value; }
        }
    }
}
