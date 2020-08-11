#ifndef LOC_INPUT_H
#define LOC_INPUT_H

#include<QTableWidget>
#include <QMainWindow>
#include<QGridLayout>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QPushButton>
#include<QLabel>
#include<QSlider>
#include"v3d_interface.h"


class loc_input : public QDialog
{
    Q_OBJECT
public:
    explicit loc_input(QWidget *parent = NULL);
    void setV3DPluginCallback2(V3DPluginCallback2 *callback2);
    void show_area();


    LandmarkList area;

public slots:
    void sliderValueChanger(int value);
    void save();
private:

    V3DPluginCallback2 *callback;
    QLabel *x_slider_label;
    QLabel *y_slider_label;
    QSlider *x_slider;
    QSlider *y_slider;
    QPushButton *confirm_button;

    v3dhandle curwin;
    Image4DSimple *p4DImage;
    V3DLONG sz[4];
    QSpinBox * x_location_spinbox;
    QSpinBox * y_location_spinbox;
    QSpinBox * z_location_spinbox;

    V3DLONG x_coordinate;
    V3DLONG y_coordinate;
    V3DLONG z_coordinate;








};
#endif // LOC_INPUT_H
