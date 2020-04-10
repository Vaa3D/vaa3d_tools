#include "point_class.h"
#include <math.h>

Point_class::Point_class()
{
    reference_range = 5;
    filter_range = 3;
    flag_range = 3;
    mean_SD_para = 3;
    range_20_data = 0;
    filter_data = 0;
    filter_shift_data = 0;
    filter_thres_data1 = 0;
    filter_thres_data2 = 0;
    flag_data1 = 0;
    flag_data2 = 0;
}

void Point_class:: set_filter_data(Point_class point_arr[2])
{
    double sum = range_20_data;
    for(int i = 0; i < 2; i ++)
    {
        sum += point_arr[i].get_range_20_data();
    }
    filter_data = int(sum/filter_range+0.5);
}

void Point_class:: set_filter_shift_data(Point_class point)
{
    filter_shift_data = filter_data - point.filter_data;
}

void Point_class:: set_filter_thres_data1(Point_class point_arr[5])
{
    double sum = 0, d_sum = 0;
    double mean = 0, SD = 0;
    for(int i = 0; i < reference_range; i ++)
    {
        sum += point_arr[i].filter_shift_data;
    }
    mean = sum/reference_range;

    for(int i = 0; i < reference_range; i ++)
    {
        d_sum += (point_arr[i].filter_shift_data - mean) * (point_arr[i].filter_shift_data - mean);
    }
    SD =sqrt(d_sum/(reference_range - 1));

    filter_thres_data1 = mean + mean_SD_para * SD;
}

void Point_class:: set_filter_thres_data2(Point_class point_arr[5])
{
    double sum = 0, d_sum = 0;
    double mean = 0, SD = 0;
    for(int i = 0; i < reference_range; i ++)
    {
        sum += point_arr[i].filter_shift_data;
    }
    mean = sum/reference_range;

    for(int i = 0; i < reference_range; i ++)
    {
        d_sum += (point_arr[i].filter_shift_data - mean) * (point_arr[i].filter_shift_data - mean);
    }
    SD =sqrt(d_sum/(reference_range - 1));

    filter_thres_data2 = mean_SD_para * SD - mean;
}


void Point_class::set_flag_data1(Point_class point)
{
    if(filter_shift_data > point.filter_thres_data1)
        flag_data1 = 1;
}
void Point_class::set_flag_data2(Point_class point1, Point_class point2)
{
    if((point1.filter_shift_data + point2.filter_thres_data2)<0)
        flag_data2 = 1;
}

Point_class & Point_class::operator = (const Point_class & point)
{

    range_20_data = point.range_20_data;
    filter_data = point.filter_data;
    filter_shift_data = point.filter_shift_data;
    filter_thres_data1 = point.filter_thres_data1;
    filter_thres_data2 = point.filter_thres_data2;
    flag_data1 = point.flag_data1;
    flag_data2 = point.flag_data2;

    return *this;
}

void Point_class:: copy_point_class(Point_class point)
{
    range_20_data = point.range_20_data;
    filter_data = point.filter_data;
    filter_shift_data = point.filter_shift_data;
    filter_thres_data1 = point.filter_thres_data1;
    filter_thres_data2 = point.filter_thres_data2;
    flag_data1 = point.flag_data1;
    flag_data2 = point.flag_data2;
}
