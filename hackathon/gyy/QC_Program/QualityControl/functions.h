#ifndef FUNCTIONS_H
#define FUNCTIONS_H


class Functions
{
public:
    static void calculate_YZ_shift_via_MIP();
    static void calculate_overlap_shift_one_plane();
    static void calculate_overlap_shift_all_images();

    static void shift_global_optimization();
    static void stitch_one_plane();
    static void XZ_projection_20px_MIP();
    static void YZ_projection_20px_MIP();

};

#endif // FUNCTIONS_H
