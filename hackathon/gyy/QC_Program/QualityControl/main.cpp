
#include "functions.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Functions:: calculate_YZ_shift_via_MIP();
    Functions:: calculate_overlap_shift_one_plane();
    Functions:: calculate_overlap_shift_all_images();

    Functions:: shift_global_optimization();
    Functions:: stitch_one_plane();
    Functions:: XZ_projection_20px_MIP();
    Functions:: YZ_projection_20px_MIP();


    return a.exec();
}
