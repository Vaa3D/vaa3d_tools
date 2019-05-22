#include "scoreinput.h"
#include<iostream>
using namespace std;
#include<QFileDialog>

ScoreInput::ScoreInput(QWidget *parent) : QMainWindow(parent)
{
    this->setWindowTitle("SWC Post Score Control");
    this->resize(1200, 650);
    centralWidget = new QWidget(parent);
//    cout<<423<<endl;

    vertical_layout_widget=new QWidget(centralWidget);
    vertical_layout=new QVBoxLayout(vertical_layout_widget);
    vertical_layout->setSpacing(5);
    vertical_layout_widget->resize(1100,600);
//    vertical_layout->on

//    cout<<424<<endl;
    horizental_layout_widget=new QWidget(centralWidget);
    horizental_layout=new QHBoxLayout(horizental_layout_widget);
//    cout<<4241<<endl;
    table_widget=new QTableWidget(vertical_layout_widget);
//    cout<<4221<<endl;
    vertical_layout->addWidget(horizental_layout_widget);
    QStringList header;
    header<<"number"<<"problem"<<"swc score"<<"match_score"<<" confidence score"<<"score_final"<<"operation";
    table_widget->setRowCount(1); //设置行数为10
    table_widget->setColumnCount(7); //设置列数为5
    table_widget->resize(400, 300); //设置表格
    table_widget->setHorizontalHeaderLabels(header);
    table_widget->setSortingEnabled(true);
//    cout<<425<<endl;
    table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_widget->setSelectionBehavior(QAbstractItemView::SelectRows); //整行选中的方式
    vertical_layout->addWidget(table_widget);
//    cout<<426<<endl;
    compare_button=new QPushButton(horizental_layout_widget);
    compare_button->setText("compare Whit Init");
    horizental_layout->addWidget(compare_button);

    get_neuron_button=new QPushButton(horizental_layout_widget);
    get_neuron_button->setText("getNeuronTree");
    horizental_layout->addWidget(get_neuron_button);
//    cout<<427<<endl;
    confirm_button=new QPushButton(vertical_layout_widget);
    confirm_button->setText("Save");
    vertical_layout->addWidget(confirm_button);

    weight_confidence_score=1.0;
    weight_match_score=1.0;
    weight_swc_score=1.0;
//    cout<<1<<endl;


////    horizental_layout.addItem();
    neuron_name=new QLabel();
    neuron_name->setText("neuron Name");
//    cout<<2<<endl;
    horizental_layout->addWidget(neuron_name);
    horizental_layout->addWidget(compare_button);
//cout<<4<<endl;
    slider_horizental_layout_widget=new QWidget(vertical_layout_widget);
    slider_horizental_layout=new QHBoxLayout(slider_horizental_layout_widget);
    slider_horizental_layout->setSpacing(3);
    label_slider=new QLabel(slider_horizental_layout_widget);
    label_slider->setText("Minumu Score Reserve");
    score_slider=new QSlider(slider_horizental_layout_widget);
    score_slider->setRange(0,100);
    score_slider->setValue(50);
    score_slider->setOrientation(Qt::Horizontal);
    score_slider->setTickPosition(QSlider::TicksAbove);
    score_slider_num=new QLabel(slider_horizental_layout_widget);
    score_slider_num->setText(QString::number(score_slider->value()/100.0));
    slider_horizental_layout->addWidget(label_slider);
    slider_horizental_layout->addWidget(score_slider);
    slider_horizental_layout->addWidget(score_slider_num);
    vertical_layout->addWidget(slider_horizental_layout_widget);

    weight_horizental_layout_widget=new QWidget(vertical_layout_widget);
    weight_horizental_layout=new QHBoxLayout(weight_horizental_layout_widget);
    weight_horizental_layout->setSpacing(7);

    weight_confidence_score_label=new QLabel(weight_horizental_layout_widget);
    weight_confidence_score_label->setText("weight of confidence_score ");
    weight_mathch_score_label=new QLabel(weight_horizental_layout_widget);
    weight_mathch_score_label->setText("weight of match_score ");
    weight_swc_score_label=new QLabel(weight_horizental_layout_widget);
    weight_swc_score_label->setText("weight of swc_score ");

    weight_confidence_score_edit=new QSpinBox(weight_horizental_layout_widget);
    weight_confidence_score_edit->setRange(0,100);
    weight_confidence_score_edit->setValue(1);
    weight_mathch_score_edit=new QSpinBox(weight_horizental_layout_widget);
    weight_mathch_score_edit->setRange(0,100);
    weight_mathch_score_edit->setValue(1);
    weight_swc_score_edit=new QSpinBox(weight_horizental_layout_widget);
    weight_swc_score_edit->setRange(0,100);
    weight_swc_score_edit->setValue(1);

    changeWeight=new QPushButton(weight_horizental_layout_widget);
    changeWeight->setText("confirm weight");

    weight_horizental_layout->addWidget(weight_confidence_score_label);
    weight_horizental_layout->addWidget(weight_confidence_score_edit);
    weight_horizental_layout->addWidget(weight_mathch_score_label);
    weight_horizental_layout->addWidget(weight_mathch_score_edit);
    weight_horizental_layout->addWidget(weight_swc_score_label);
    weight_horizental_layout->addWidget(weight_swc_score_edit);
    weight_horizental_layout->addWidget(changeWeight);

    vertical_layout->addWidget(weight_horizental_layout_widget);

    this->setCentralWidget(centralWidget);
//cout<<5<<endl;
    connect(get_neuron_button, SIGNAL(clicked()), this, SLOT(getNeuronTree()));
    connect(compare_button, SIGNAL(clicked()), this, SLOT(compare()));
    connect(confirm_button, SIGNAL(clicked()), this, SLOT(save()));
    connect(score_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanger(int)));
    connect(changeWeight, SIGNAL(clicked()), this, SLOT(changeWeights()));
//cout<<6<<endl;

//    this->getNeuronTree();
    score_slider_num->setFixedWidth(5*10);
}
void ScoreInput::changeWeights()
{
    weight_confidence_score=weight_confidence_score_edit->value();
    weight_match_score=weight_mathch_score_edit->value();
    weight_swc_score=weight_swc_score_edit->value();
    this->calFinalScore();

}
void ScoreInput::sliderValueChanger(int value)
{
    score_slider_num->setText(QString::number(score_slider->value()/100.0));
//    cout<<"sliderValueChanger "<<score_slider->value()<<endl;
   this->calFinalScore();
    emit signal_slider_valueChanged();
}

void ScoreInput::save()
{
//    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
//            "",
//            QObject::tr("Supported file (*.swc)"
//                ";;Neuron structure	(*.swc)"
//                ));
//    if(fileSaveName.isEmpty())
    if(mTreeList==NULL){return;}
    QString  cc=mTreeList->at(mTreeList->size()-1).comment;
    cout<<"mTreeList->at(mTreeList->size()-1).comment:"<<cc.toUtf8().data()<<endl;
    if(mTreeList->at(mTreeList->size()-1).comment.compare("temp_tree"))
    {
        writeSWC_file("", mTreeList->at(mTreeList->size()-1));
    }




}
void ScoreInput::compare()
{
    // do in initTree
    //save to dealedTree
    if(mTreeList==NULL||mTreeList->size()<=0)
    {
        cout<<"neuron tree is empty"<<endl;
        return ;
    }
    if(mTreeList->size()<deal_tree_num)
    {
        deal_tree_num=mTreeList->size();
    }
    if(mTreeList->size()==deal_tree_num)
    {
        mTreeList->append(dealedTree);
    }else if(mTreeList->size()>deal_tree_num)
    {
        (*mTreeList)[deal_tree_num]=dealedTree;
    }
//    this->hide();
    this->setWindowState(Qt::WindowMinimized);
    emit signal_compare();


}

void ScoreInput::setV3DPluginCallback2(V3DPluginCallback2 *callback2)
{
    if(callback2==NULL)
    {cout<<"input a wrong V3DPluginCallback2"<<endl;
        v3d_msg("input a wrong V3DPluginCallback2");
        return;}
    callback=callback2;
    //get neurontree
    cout<<"catch callback success"<<endl;
//    this->getNeuronTree();
}
bool ScoreInput::getNeuronTree()
{
    if(callback==NULL){cout<<" Callback invalid"<<endl;}
    QList <V3dR_MainWindow *> list_3dviewer = callback->getListAll3DViewers();
    if (list_3dviewer.size() < 1)
    {
        v3d_msg("Please open  a SWC file from the main menu first! list_3dviewer");
        return false;
    }
    V3dR_MainWindow *surface_win = list_3dviewer[0];
    if (!surface_win){
        v3d_msg("Please open up a SWC file from the main menu first!");
        return false;
    }
//    cout<<"read SWC from 3d Viewer  "<<endl;
    mTreeList = callback->getHandleNeuronTrees_Any3DViewer(surface_win);
    if(mTreeList->size()<=0)
    {
        return false;
    }

    initTree=mTreeList->at(0);
    dealedTree=initTree;
    deal_tree_num=mTreeList->size();
    get_neuron_button->hide();
    neuron_name->setText(initTree.file);
    return true;

}
void ScoreInput::setWight(float weight_confidence_score1,float weight_match_score1,float weight_swc_score1)
{
    weight_confidence_score=weight_confidence_score1;
    weight_match_score=weight_match_score1;
    weight_swc_score=weight_swc_score1;

    weight_confidence_score_edit->setValue(weight_confidence_score);
    weight_mathch_score_edit->setValue(weight_match_score);
    weight_swc_score_edit->setValue(weight_swc_score);

}

float* ScoreInput::return_result()
{
    return result;

}
void ScoreInput::calFinalScore()
{
    if(problemsString.size()<number_data)
    {cout<<"error problems_before.size()<number_data"<<endl;}
    if(1)
    {
        for(int i=0;i<table_widget->rowCount();i++)
        {
            for(int j=0;j<table_widget->columnCount();j++)
            {
                delete table_widget->itemAt(i,j);
            }


        }
    }


    if(number_data>0)
    {
       table_widget->setRowCount(number_data);
    }
    else{table_widget->setRowCount(1);return ;}


    float sum_weight=weight_confidence_score+weight_match_score+weight_swc_score;
    if(sum_weight==0){sum_weight=0.001;}
    for(int i=0;i<number_data;i++)
    {
        final_score[i]=weight_confidence_score*condidence_score[i]+
                weight_match_score*match_score[i]+
                weight_swc_score*no_other_swc_score[i]
                ;
        final_score[i]=final_score[i]/sum_weight;
        QTableWidgetItem* item_number = new QTableWidgetItem();
        item_number->setData(Qt::DisplayRole, i+1);//plus one
        table_widget->setItem(i,0,item_number);
//        table_widget->setItem(i,0,new QTableWidgetItem(QString::number(i+1)));//plus one

        table_widget->setItem(i,1,new QTableWidgetItem(problemsString.at(i)));

        QTableWidgetItem* item_swc_score = new QTableWidgetItem();
        item_swc_score->setData(Qt::DisplayRole, no_other_swc_score[i]);
        table_widget->setItem(i,2,item_swc_score);
        QTableWidgetItem* item_match_score = new QTableWidgetItem();
        item_match_score->setData(Qt::DisplayRole, match_score[i]);
        table_widget->setItem(i,3,item_match_score);

        QTableWidgetItem* item_confidence_score = new QTableWidgetItem();
        item_confidence_score->setData(Qt::DisplayRole, condidence_score[i]);
//        cout<<"set in widget i:"<<i<<" item_confidence_score :"<<condidence_score[i]<<endl;
        table_widget->setItem(i,4,item_confidence_score);

        QTableWidgetItem* item_final_score = new QTableWidgetItem();
        item_final_score->setData(Qt::DisplayRole, final_score[i]);
        table_widget->setItem(i,5,item_final_score);



//        table_widget->setItem(i,2,new QTableWidgetItem(QString::number(no_other_swc_score[i])));
//        table_widget->setItem(i,3,new QTableWidgetItem(QString::number(condidence_score[i])));
//        table_widget->setItem(i,4,new QTableWidgetItem(QString::number(match_score[i])));
//        table_widget->setItem(i,5,new QTableWidgetItem(QString::number(final_score[i])));
        if(final_score[i]*100>score_slider->value())
        {table_widget->setItem(i,6,new QTableWidgetItem("reserve"));
        result[i]=1;}
        else
        {table_widget->setItem(i,6,new QTableWidgetItem("need process"));
            result[i]=0;}
//        cout<<"i:"<<i<<endl;
//        cout<<"no_other_swc_score[i] "<<no_other_swc_score[i]
//            <<" condidence_score[i] "<<condidence_score[i]
//            <<" match_score[i] "<<match_score[i]<<endl
//            <<" final_score[i] "<<final_score[i]<<endl;
    }

}

void ScoreInput::setData(int numbers,QStringList problem,float * score_swc,float * score_confidence,float * score_match)
{
    cout<<"begin to setData"<<endl;
    if(problem.size()!=numbers)
    {cout<<"error problem.size()<numbers"<<endl;}

    try{final_score=new float [numbers];}//result
    catch(...) {v3d_msg("cannot allocate memory for final_score."); return ;}

    if(result){delete[] result;result=0;}

    try{result=new float [numbers];}
    catch(...) {v3d_msg("cannot allocate memory for result."); return ;}
//    try{no_other_swc_score=new float [numbers];}
//    catch(...) {v3d_msg("cannot allocate memory for no_other_swc_score."); return ;}
//    try{condidence_score=new float [numbers];}
//    catch(...) {v3d_msg("cannot allocate memory for condidence_score."); return ;}
//    try{match_score=new float [numbers];}
//    catch(...) {v3d_msg("cannot allocate memory for match_score."); return ;}

//    for(int i=0;i<numbers;i++)
//    {
//        no_other_swc_score[i]=score_swc[i];
//        condidence_score[i]=score_confidence[i];
//        match_score[i]=score_match[i];
//    }
    cout<<"begin to set date"<<endl;
    no_other_swc_score=score_swc;
    condidence_score=score_confidence;
    match_score=score_match;
    cout<<"success in set date"<<endl;

    number_data=numbers;
    problemsString=problem;

    this->calFinalScore();
}
