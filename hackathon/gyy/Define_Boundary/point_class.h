#ifndef POINT_CLASS_H
#define POINT_CLASS_H


class Point_class
{
public:
    int range_20_data;
    int filter_data;
    int filter_shift_data;
    double filter_thres_data1;
    double filter_thres_data2;
    int flag_data1;
    int flag_data2;
    int reference_range;
    int filter_range;
    int flag_range;
    int mean_SD_para;

public:
    Point_class();

    int get_range_20_data(){return range_20_data;}
    void set_range_20_data(int data){range_20_data = data;}

    void set_filter_data(Point_class point_arr[4]);
    void set_filter_shift_data(Point_class point);
    void set_filter_thres_data1(Point_class point_arr[5]);
    void set_filter_thres_data2(Point_class point_arr[5]);

    void set_reference_range(int a){reference_range = a;}
    void set_filter_range(int a){filter_range = a;}
    void set_flag_range(int a){flag_range = a;}
    void set_mean_SD_para(int a){mean_SD_para = a;}

    void set_flag_data1(Point_class point);
    void set_flag_data2(Point_class point1, Point_class point2);

    void copy_point_class(Point_class point);
    Point_class &operator = (const Point_class & point);

    ~Point_class(){}
};




#endif // POINT_CLASS_H
