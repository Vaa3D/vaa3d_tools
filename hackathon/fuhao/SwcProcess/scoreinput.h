#ifndef SCOREINPUT_H
#define SCOREINPUT_H
#include<QTableWidget>
#include <QMainWindow>
#include<QGridLayout>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QPushButton>
#include<QLabel>
#include<QSlider>
#include"v3d_interface.h"
#include"v3dr_mainwindow.h"
#include"v3dr_glwidget.h"
//#include<QLayout>


class ScoreInput : public QMainWindow
{
    Q_OBJECT
public:
    explicit ScoreInput(QWidget *parent = nullptr);
    void setV3DPluginCallback2(V3DPluginCallback2 *callback2);

    void setWight(float weight_confidence_score1,float weight_match_score1,float weight_swc_score1);
    void setData(int numbers,QStringList problem,float * score_swc,float * score_confidence,float * score_match);
    float* return_result();
signals:
    void signal_compare();
    void save_temp_tree();
    void signal_slider_valueChanged();

public slots:
    bool getNeuronTree();
    void save();
    void compare();
    void sliderValueChanger(int value);
    void changeWeights();
private:
    QWidget *centralWidget ;

    QTableWidget *table_widget;
    QPushButton *compare_button;
    QPushButton *get_neuron_button;
    QPushButton *confirm_button;

    QLabel *neuron_name;
    QLabel *label_slider;
    QSlider *score_slider;
    QLabel *score_slider_num;
    QWidget *slider_horizental_layout_widget;
    QHBoxLayout *slider_horizental_layout;
    QVBoxLayout *vertical_layout;
    QHBoxLayout *horizental_layout;
    QWidget *vertical_layout_widget;
    QWidget *horizental_layout_widget;

    QLabel *weight_confidence_score_label;
    QLabel *weight_mathch_score_label;
    QLabel *weight_swc_score_label;

    QSpinBox *weight_confidence_score_edit;
    QSpinBox *weight_mathch_score_edit;
    QSpinBox *weight_swc_score_edit;
    QWidget *weight_horizental_layout_widget;
    QHBoxLayout *weight_horizental_layout;
    QPushButton *changeWeight;


    V3DPluginCallback2 *callback;
    NeuronTree initTree;
    int deal_tree_num;
    NeuronTree dealedTree;
    QList<NeuronTree> * mTreeList;
    V3dR_MainWindow * surface_win;

    float weight_confidence_score;
    float weight_match_score;
    float weight_swc_score;
    int number_data;
    float * no_other_swc_score;
    float* condidence_score;
    float* match_score;
    float* result;
    float *final_score;
    QStringList problemsString;

    void calFinalScore();



};

#endif // SCOREINPUT_H
