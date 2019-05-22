#include "BrainVesselCPR_spline.h"

Coor3D catmull_rom_spline(double t, Coor3D p1, Coor3D p2, Coor3D p3, Coor3D p4)
{
    Coor3D res;
    res.x = 0.5 * ((-p1.x + 3*p2.x -3*p3.x + p4.x)*t*t*t
                + (2*p1.x -5*p2.x + 4*p3.x - p4.x)*t*t
                + (-p1.x+p3.x)*t
                + 2*p2.x);
    res.y = 0.5 * ((-p1.y + 3*p2.y -3*p3.y + p4.y)*t*t*t
                + (2*p1.y -5*p2.y + 4*p3.y - p4.y)*t*t
                + (-p1.y+p3.y)*t
                + 2*p2.y);
    res.z = 0.5 * ((-p1.z + 3*p2.z -3*p3.z + p4.z)*t*t*t
                + (2*p1.z -5*p2.z + 4*p3.z - p4.z)*t*t
                + (-p1.z+p3.z)*t
                + 2*p2.z);
    return res;
}
