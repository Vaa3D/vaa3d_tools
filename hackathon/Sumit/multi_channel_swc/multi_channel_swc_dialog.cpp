#include "multi_channel_swc_dialog.h"
#include "multi_channel_swc_template.h"
#include "multi_channel_swc_func.h"
#include <fstream>

multi_channel_swc_dialog::multi_channel_swc_dialog(V3DPluginCallback2 *cb, QWidget *parent) :
    QDialog(parent)
{
    callback=cb;
    create();
    initDlg();
    checkbtn();
    p_img = 0;
    nt = 0;
}

void multi_channel_swc_dialog::create()
{
    gridLayout = new QGridLayout();

    //input zone
    edit_image = new QLineEdit();
    edit_swc = new QLineEdit();
    edit_output = new QLineEdit();
    btn_loadimage = new QPushButton("...");
    btn_loadswc = new QPushButton("...");
    btn_setoutput = new QPushButton("...");
    QLabel * label_loadimage;
    label_loadimage = new QLabel(QObject::tr("Input image file:"));
    QLabel * label_loadswc;
    label_loadswc = new QLabel(QObject::tr("Input swc file:"));
    QLabel * label_setoutput;
    label_setoutput = new QLabel(QObject::tr("Output file:"));

    connect(btn_loadimage, SIGNAL(clicked()), this, SLOT(load_image()));
    connect(btn_loadswc, SIGNAL(clicked()), this, SLOT(load_swc()));
    connect(btn_setoutput, SIGNAL(clicked()), this, SLOT(set_output()));

    gridLayout->addWidget(label_loadimage,0,0,1,6);
    gridLayout->addWidget(edit_image,1,0,1,5);
    gridLayout->addWidget(btn_loadimage,1,5,1,1);
    gridLayout->addWidget(label_loadswc,2,0,1,6);
    gridLayout->addWidget(edit_swc,3,0,1,5);
    gridLayout->addWidget(btn_loadswc,3,5,1,1);
    gridLayout->addWidget(label_setoutput,4,0,1,6);
    gridLayout->addWidget(edit_output,5,0,1,5);
    gridLayout->addWidget(btn_setoutput,5,5,1,1);

    //parameter zone
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_1,8,0,1,6);

    spin_threshold1 = new QDoubleSpinBox();
    spin_threshold1->setValue(15);
    spin_threshold2 = new QDoubleSpinBox();
    spin_threshold2->setValue(15);
    combo_channel1 = new QComboBox();
    combo_channel2 = new QComboBox();

    QLabel* label_c1 = new QLabel("primary channel:");
    QLabel* label_t1 = new QLabel("threshold:");
    QLabel* label_c2 = new QLabel("secondary channel:");
    QLabel* label_t2 = new QLabel("threshold:");

    gridLayout->addWidget(label_c1,9,0,1,3);
    gridLayout->addWidget(combo_channel1,9,3,1,1);
    gridLayout->addWidget(label_t1,9,4,1,1);
    gridLayout->addWidget(spin_threshold1,9,5,1,1);

    gridLayout->addWidget(label_c2,10,0,1,3);
    gridLayout->addWidget(combo_channel2,10,3,1,1);
    gridLayout->addWidget(label_t2,10,4,1,1);
    gridLayout->addWidget(spin_threshold2,10,5,1,1);

    //operation zone
    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_2,15,0,1,6);
    btn_run = new QPushButton("Run");
    btn_quit = new QPushButton("Quit");
    gridLayout->addWidget(btn_run,16,4,1,1);
    gridLayout->addWidget(btn_quit,16,5,1,1);

    connect(btn_run, SIGNAL(clicked()), this, SLOT(run()));
    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));

    setLayout(gridLayout);
}

multi_channel_swc_dialog::~multi_channel_swc_dialog()
{
    QSettings settings("V3D plugin","multiChannelSWC");

    settings.setValue("fname_image",fname_img);
    settings.setValue("fname_swc",fname_neuronTree);
    settings.setValue("fname_output",this->edit_output->text());
    settings.setValue("thr1",this->spin_threshold1->value());
    settings.setValue("thr2",this->spin_threshold2->value());
}

void multi_channel_swc_dialog::checkbtn()
{
    if(this->nt==0 || this->p_img==0){
        btn_run->setEnabled(false);
    }else{
        btn_run->setEnabled(true);
    }
}

void multi_channel_swc_dialog::initDlg()
{
    QSettings settings("V3D plugin","multiChannelSWC");

    if(settings.contains("fname_image"))
        fname_img=settings.value("fname_image").toString();
    if(settings.contains("fname_swc"))
        fname_neuronTree=settings.value("fname_swc").toString();
    if(settings.contains("fname_output"))
        this->edit_output->setText(settings.value("fname_output").toString());
    if(settings.contains("thr1"))
        this->spin_threshold1->setValue(settings.value("thr1").toDouble());
    if(settings.contains("thr2"))
        this->spin_threshold1->setValue(settings.value("thr2").toDouble());
}

void multi_channel_swc_dialog::load_image()
{
    //get file name
    QString fileOpenName;
    if(edit_image->text().isEmpty()){
        fileOpenName = fname_img;
    }else{
        fileOpenName = edit_image->text();
    }
    QString fname_input = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
                                               fileOpenName,
                                               QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    if(fname_input.isEmpty()){
        return;
    }

    //try to load image
    qDebug()<<"NeuronPicker: is going to load "<<fname_input;
    if(!simple_loadimage_wrapper(*callback, fname_input.toStdString().c_str(), p_img, sz_img, intype))
    {
        v3d_msg("load image "+fname_input+" error!");
        return;
    }

    //check image channel information
    if(sz_img[3]<2){
        v3d_msg("Image only has 1 channel!");
        return;
    }

    combo_channel1->clear();
    combo_channel2->clear();
    for(int i=0; i<sz_img[3]; i++){
        combo_channel1->addItem(QString::number(i+1));
        combo_channel2->addItem(QString::number(i+1));
    }
    combo_channel1->setCurrentIndex(1);
    combo_channel2->setCurrentIndex(0);

    fname_img=fname_input;
    edit_image->setText(fname_input);

    checkbtn();
}

void multi_channel_swc_dialog::load_swc()
{
    QString fileOpenName;
    if(edit_swc->text().isEmpty()){
        fileOpenName = fname_neuronTree;
    }else{
        fileOpenName = edit_swc->text();
    }
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    nt = new NeuronTree();
    if(!fileOpenName.isEmpty()){
        *nt = readSWC_file_multichannel(fileOpenName);
    }
    if(nt->listNeuron.size()>0){
        edit_swc->setText(fileOpenName);
        edit_output->setText(fileOpenName+"_processed.swc");
        fname_neuronTree=fileOpenName;
    }else{
        v3d_msg("Error: cannot read file "+fileOpenName);
        return;
    }

    checkbtn();
}

void multi_channel_swc_dialog::set_output()
{
    QString fileOpenName = this->edit_output->text();
    fileOpenName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(!fileOpenName.isEmpty()){
        edit_output->setText(fileOpenName);
    }

    checkbtn();
}

void multi_channel_swc_dialog::run()
{
    if(nt==0 || p_img==0){
        return;
    }
    int ch1=combo_channel1->currentIndex();
    int ch2=combo_channel2->currentIndex();
    float thr1=spin_threshold1->value();
    float thr2=spin_threshold2->value();
    vector<float> mean, stdev, ratio;
    calculate_info(p_img, sz_img, nt, ch1, thr1, ch2, thr2, ratio, mean, stdev);
    save_result(edit_output->text(), nt, ratio, mean, stdev);

    accept();
}

void save_result(const QString fname, NeuronTree* nt, vector<float>& ratio, vector<float>& mean, vector<float>& stdev)
{
    if(ratio.size()!=nt->listNeuron.size()){
        qDebug()<<"ERROR: ratio number does not match with neuron tree";
        return;
    }
    if(mean.size()!=nt->listNeuron.size()){
        qDebug()<<"ERROR: mean number does not match with neuron tree";
        return;
    }
    if(stdev.size()!=nt->listNeuron.size()){
        qDebug()<<"ERROR: standard deviation number does not match with neuron tree";
        return;
    }

    ofstream fp(fname.toStdString().c_str());
    if(!fp.is_open()){
        v3d_msg("Error: failed to open file to write: "+fname);
        return;
    }
    fp<<"# "<<fname.toStdString().c_str()<<endl;
    fp<<"# This file is generated by multi channel swc plugin in Vaa3D, built by Sumit Nanda (snanda2@gmu.edu) and Hanbo Chen (cojoc.chen@gmail.com)"<<endl;
    fp<<"#CHANNELSWC"<<endl;
    fp<<"#"<<ratio.size()<<" 3"<<endl;
//    for(V3DLONG nid=0; nid<ratio.size(); nid++){
//        fp<<"#"<<ratio.at(nid)<<" "<<mean.at(nid)<<" "<<stdev.at(nid)<<endl;
//    }
    fp<<"# id,type,x,y,z,r,pid,segid,level,ratio,mean,stdev"<<endl;
    for(V3DLONG nid=0; nid<ratio.size(); nid++){
        fp<<nt->listNeuron.at(nid).n<<" ";
        fp<<nt->listNeuron.at(nid).type<<" ";
        fp<<nt->listNeuron.at(nid).x<<" ";
        fp<<nt->listNeuron.at(nid).y<<" ";
        fp<<nt->listNeuron.at(nid).z<<" ";
        fp<<nt->listNeuron.at(nid).r<<" ";
        fp<<nt->listNeuron.at(nid).pn<<" ";
        fp<<"0 0 "<<ratio.at(nid)<<" "<<mean.at(nid)<<" "<<stdev.at(nid);
        fp<<endl;
    }
    fp.close();
}

bool compute_swc_render(QString filename, NeuronTree& nt_rend)
{
    return true;
   }
