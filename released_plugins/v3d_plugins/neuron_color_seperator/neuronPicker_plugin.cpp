	/* neuronPicker_plugin.cpp
 * 
 * 2014-12-01 : by Xiang Li, Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuronPicker_plugin.h"
#include "neuron_seperator_explorer.h"
#include <math.h>
#include "string"
#include "sstream"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <basic_landmark.h>
using namespace std;


#define const_length_histogram 90
#define const_max_voxelValue 255

static neuronPickerDialog * npdiag = 0;
static neuron_seperator_explorer * nediag = 0;
void finishnpdiag()
{
    if (npdiag)
    {
        delete npdiag;
        npdiag=0;
    }
}

Q_EXPORT_PLUGIN2(neuronPicker, neuronPicker);

QStringList neuronPicker::menulist() const
{
	return QStringList() 
        <<tr("pick_neuron_by_color")
        <<tr("seperate_result_explorer")
		<<tr("about");
}

QStringList neuronPicker::funclist() const
{
	return QStringList()
        <<tr("auto_seperate_neurons")
		<<tr("help");
}

void neuronPicker::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("pick_neuron_by_color"))
	{
        if(npdiag){
            npdiag->show();
            npdiag->raise();
            npdiag->activateWindow();
        }else{
            npdiag = new neuronPickerDialog(&callback);
            npdiag->show();
            npdiag->raise();
            npdiag->activateWindow();
        }
    }
    else if (menu_name == tr("seperate_result_explorer"))
    {
        if(nediag){
            nediag->show();
            nediag->raise();
            nediag->activateWindow();
        }else{
            nediag = new neuron_seperator_explorer(&callback, parent);
            nediag->show();
            nediag->raise();
            nediag->activateWindow();
        }
    }
	else
	{
		v3d_msg(tr(". "
            "Developed by Hanbo Chen, Xiang Li, 2014-12-01"));
	}
}

bool neuronPicker::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("auto_seperate_neurons"))
    {
        cout<<"==== color neuron picker ===="<<endl;
        if(infiles.size()!=1 || outfiles.size()!=1)
        {
            qDebug("ERROR: please set input and output!");
            printHelp();
            return false;
        }

        //load image
        QString fname_input = ((vector<char*> *)(input.at(0).p))->at(0);
        QString fname_output = ((vector<char*> *)(output.at(0).p))->at(0);

        unsigned char * image1Dc_in = 0;
        V3DLONG sz_img[4];
        int intype;
        if(!simple_loadimage_wrapper(callback, fname_input.toStdString().c_str(), image1Dc_in, sz_img, intype))
        {
          v3d_msg("load image "+fname_input+" error!");
          return false;
        }

        if(sz_img[3]>3){
            sz_img[3]=3;
            qDebug()<<"NeuronPicker: More than 3 channels were loaded. The first 3 channel will be applied for analysis.";
        }

        neuronPickerMain2 pickerObj;
        if (intype == 1) //V3D_UINT8;
        {
            pickerObj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
        }
        else if (intype == 2) //V3D_UINT16;
        {
            pickerObj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
        }
        else if(intype == 4) //V3D_FLOAT32;
        {
            pickerObj.pushNewData<float>((float*)image1Dc_in, sz_img);
        }
        else
        {
            v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.", 0);
            return false;
        }

        //get parameters
        int cubSize=11;
        int conviter=10;
        int fgthr=100;
        int bgthr=10;
        int sizethr=2000;
        int margin_size=15;
        float scale=4;
        float sparsthr=0.35;
        float touchthr=0.5;

        if(input.size()>1){
            vector<char*> paras = (*(vector<char*> *)(input.at(1).p));

            if(paras.size()>0){
                float tmp=atof(paras.at(0));
                if(tmp>0)
                    scale=tmp;
                else
                    cerr<<"error: illigal scale parameter: "<<tmp<<"; use default value "<<scale<<endl;
            }
            if(paras.size()>1){
                int tmp=atoi(paras.at(1));
                if(tmp>1)
                    cubSize=tmp;
                else
                    cerr<<"error: illigal neighbor size: "<<tmp<<"; use default value "<<cubSize<<endl;
            }
            if(paras.size()>2){
                int tmp=atoi(paras.at(2));
                if(tmp>=0)
                    conviter=tmp;
                else
                    cerr<<"error: illigal convolution iteration: "<<tmp<<"; use default value "<<conviter<<endl;
            }
            if(paras.size()>3){
                int tmp=atoi(paras.at(3));
                if(tmp>0)
                    fgthr=tmp;
                else
                    cerr<<"error: illigal seed intensity threshold: "<<tmp<<"; use default value "<<fgthr<<endl;
            }
            if(paras.size()>4){
                int tmp=atoi(paras.at(4));
                if(tmp>=0)
                    bgthr=tmp;
                else
                    cerr<<"error: illigal background intensity threshold: "<<tmp<<"; use default value "<<bgthr<<endl;
            }
            if(paras.size()>5){
                int tmp=atoi(paras.at(5));
                if(tmp>=0)
                    sizethr=tmp;
                else
                    cerr<<"error: illigal neuron size threshold: "<<tmp<<"; use default value "<<sizethr<<endl;
            }
            if(paras.size()>6){
                int tmp=atoi(paras.at(6));
                if(tmp>=0)
                    margin_size=tmp;
                else
                    cerr<<"error: illigal output border margin: "<<tmp<<"; use default value "<<margin_size<<endl;
            }
            if(paras.size()>7){
                float tmp=atof(paras.at(7));
                if(tmp>0 && tmp<1)
                    sparsthr=tmp;
                else
                    cerr<<"error: illigal sparsity threshold: "<<tmp<<"; use default value "<<sparsthr<<endl;
            }
            if(paras.size()>8){
                float tmp=atof(paras.at(8));
                if(tmp>0 && tmp<1)
                    touchthr=tmp;
                else
                    cerr<<"error: illigal edge touching outlier threshold: "<<tmp<<"; use default value "<<touchthr<<endl;
            }
        }

        qDebug()<<"NeuronPicker: searching, extracting, and saving starts.";
        pickerObj.innerScale=scale;
        V3DLONG neuronNum = pickerObj.autoAll(fname_output, &callback, cubSize, conviter, fgthr, bgthr, sizethr, margin_size, sparsthr, touchthr);
        qDebug()<<"NeuronPicker: Found "<<neuronNum<<" seperate nuerons in "<<fname_input;
    }
	else if (func_name == tr("help"))
	{
        printHelp();
	}
	else return false;

	return true;
}

void neuronPicker::printHelp()
{
    cout<<"\n==== Color Neuron Picker ===="<<endl;
    cout<<"\nUsage: v3d -x dllName -f neuron_picker -i <input_raw_file> -o <output_prefix> "
       <<"-p [<scale output (4)> [<neighbor cubic size (11)> [<convolute iteration (10)> [<seed intensity threshold (100)> [<background threshold (10)> [<neuron size threshold (2000)> [<output margin size (15)> [<sparsity threshold (0.35)> [<face touch treshold (0.5)>]]]]]]]]]"<<endl;
    cout<<"\n";
}

void neuronPickerDialog::shiftTwoBits2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    unsigned short* pPre = (unsigned short*)pre1d;
    for(V3DLONG i=0; i<imsz; i++)
    {
        pPost[i] = (unsigned char)MIN(pPre[i]/4,255);
    }
}

// func convert2UINT8
void neuronPickerDialog::convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
	unsigned short* pPre = (unsigned short*)pre1d;
    double max_v=0, min_v = 4095;
	for(V3DLONG i=0; i<imsz; i++)
	{
		if(max_v<pPre[i]) max_v = pPre[i];
		if(min_v>pPre[i]) min_v = pPre[i];
	}
	max_v -= min_v;
	if(max_v>0)
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
            pPost[i] = (unsigned char) 255.0*(double)(pPre[i] - min_v)/max_v;
		}
	}
	else
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
			pPost[i] = (unsigned char) pPre[i];
		}
	}
}
void neuronPickerDialog::convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
{
	float* pPre = (float*)pre1d;
	float max_v=0, min_v = 65535;
	for(V3DLONG i=0; i<imsz; i++)
	{
		if(max_v<pPre[i]) max_v = pPre[i];
		if(min_v>pPre[i]) min_v = pPre[i];
	}
	max_v -= min_v;
	if(max_v>0)
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
            pPost[i] = (unsigned char) 255.0*(double)(pPre[i] - min_v)/(double)max_v;
		}
	}
	else
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
			pPost[i] = (unsigned char) pPre[i];
		}
	}
}

//Hanbo
//dialog
neuronPickerDialog::neuronPickerDialog(V3DPluginCallback2 * cb)
{
    callback = cb;
    fname_previnput="";
    image1Dc_in=0;
    image1Dc_out=0;
    intype=0;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
    LList.clear();

    creat();
    initDlg();
    checkButtons();
}

neuronPickerDialog::~neuronPickerDialog()
{
    QSettings settings("V3D plugin","neuronPicker");

    settings.setValue("fname_input",this->fname_previnput);
    settings.setValue("fname_output",this->edit_output->text());
    settings.setValue("thr_bg",this->spin_bgthr->value());
    settings.setValue("iter_conv",this->spin_conviter->value());
    settings.setValue("size_mask",this->spin_distance->value());
    settings.setValue("thr_fg",this->spin_fgthr->value());
    settings.setValue("thr_size",this->spin_sizethr->value());
}

void neuronPickerDialog::reject()
{
    //release memory
    if(npdiag){
        if(image1Dc_in != 0){
            delete []image1Dc_in; image1Dc_in=0;
        }
        if(image1Dc_out != 0){
            neuronPickerMain::memory_free_uchar1D(image1Dc_out);
            image1Dc_out=0;
        }
    }
    finishnpdiag();
}

void neuronPickerDialog::creat()
{
    QGridLayout *gridLayout = new QGridLayout();

    //I/O zone
    QLabel* label_load = new QLabel(QObject::tr("Input Image:"));
    gridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit();
    edit_load->setText(""); edit_load->setReadOnly(true);
    gridLayout->addWidget(edit_load,0,1,1,4);
    btn_load = new QPushButton("...");
    gridLayout->addWidget(btn_load,0,5,1,1);

    QLabel* label_output = new QLabel(QObject::tr("Output Prefix:"));
    gridLayout->addWidget(label_output,1,0,1,1);
    edit_output = new QLineEdit();
    edit_output->setText(""); edit_output->setReadOnly(false);
    gridLayout->addWidget(edit_output,1,1,1,4);
    btn_output = new QPushButton("...");
    gridLayout->addWidget(btn_output,1,5,1,1);

    connect(btn_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(btn_output, SIGNAL(clicked()), this, SLOT(output()));

    //auto marker zone
    QFrame *line_0 = new QFrame();
    line_0->setFrameShape(QFrame::HLine);
    line_0->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_0,2,0,1,6);

    btn_autoMarkers = new QPushButton("Search Neurons");
    gridLayout->addWidget(btn_autoMarkers,3,0,1,1);
    spin_fgthr = new QSpinBox();
    spin_fgthr->setRange(0,255); spin_fgthr->setValue(100);
    spin_sizethr = new QSpinBox();
    spin_sizethr->setRange(0,100000); spin_sizethr->setValue(100);
    QLabel* label_a = new QLabel("seed intensity threshold (0~255):");
    gridLayout->addWidget(label_a,3,1,1,2,Qt::AlignRight);
    gridLayout->addWidget(spin_fgthr,3,3,1,1);
    QLabel* label_b = new QLabel("neuron min voxel #:");
    gridLayout->addWidget(label_b,3,4,1,1,Qt::AlignRight);
    gridLayout->addWidget(spin_sizethr,3,5,1,1);

    connect(btn_autoMarkers, SIGNAL(clicked()), this, SLOT(autoSeeds()));

    //marker zone
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_1,4,0,1,6);


    btn_update = new QPushButton("Load Markers =>");
    gridLayout->addWidget(btn_update,5,0,1,1);
    cb_marker = new QComboBox();
    gridLayout->addWidget(cb_marker,5,1,1,1);
    btn_extract = new QPushButton("Extract Neuron");
    gridLayout->addWidget(btn_extract,5,2,1,1);
    btn_save = new QPushButton("Accept, Save, and Next");
    gridLayout->addWidget(btn_save,5,3,1,2);
    btn_next = new QPushButton("Reject and Next");
    gridLayout->addWidget(btn_next,5,5,1,1);

    connect(btn_update, SIGNAL(clicked()), this, SLOT(loadMarkers()));
    connect(btn_extract, SIGNAL(clicked()), this, SLOT(extract()));
    connect(btn_save, SIGNAL(clicked()), this, SLOT(saveFile()));
    connect(btn_next, SIGNAL(clicked()), this, SLOT(skip()));
    connect(cb_marker, SIGNAL(currentIndexChanged(int)), this, SLOT(syncMarkers(int)));

    //extract parameters
    spin_bgthr = new QSpinBox();
    spin_bgthr->setRange(0,255); spin_bgthr->setValue(10);
    spin_distance = new QSpinBox();
    spin_distance->setRange(0,100000); spin_distance->setValue(11);
    spin_conviter = new QSpinBox();
    spin_conviter->setRange(0,100); spin_conviter->setValue(5);
    spin_sizemargin = new QSpinBox();
    spin_sizemargin->setRange(0,100000); spin_sizemargin->setValue(15);
    QLabel* label_0 = new QLabel("background threshold (0~255):");
    gridLayout->addWidget(label_0,6,0,1,2);
    gridLayout->addWidget(spin_bgthr,6,2,1,1);
    QLabel* label_1 = new QLabel("neighbor(cubic) mask size: ");
    gridLayout->addWidget(label_1,6,3,1,2);
    gridLayout->addWidget(spin_distance,6,5,1,1);
    QLabel* label_2 = new QLabel("convolute iteration (contrast factor): ");
    gridLayout->addWidget(label_2,7,0,1,2);
    gridLayout->addWidget(spin_conviter,7,2,1,1);
    QLabel* label_3 = new QLabel("extract margin size: ");
    gridLayout->addWidget(label_3,7,3,1,2);
    gridLayout->addWidget(spin_sizemargin,7,5,1,1);

    //other
    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_2,10,0,1,6);

    btn_runall = new QPushButton("Extract All and Save");
    gridLayout->addWidget(btn_runall,11,3,1,2);
    btn_quit = new QPushButton("Quit");
    gridLayout->addWidget(btn_quit,11,5,1,1);

    connect(btn_runall, SIGNAL(clicked()), this, SLOT(runall()));
    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));

    setLayout(gridLayout);
}

void neuronPickerDialog::initDlg()
{
    QSettings settings("V3D plugin","neuronPicker");

    fname_previnput=settings.value("fname_input").toString();
//    this->edit_output->setText(settings.value("fname_output").toString());
//    if(settings.contains("thr_bg"))
//        this->spin_bgthr->setValue(settings.value("thr_bg").toInt());
//    if(settings.contains("thr_hue"))
//        this->spin_huedis->setValue(settings.value("thr_hue").toInt());
//    if(settings.contains("size_mask"))
//        this->spin_distance->setValue(settings.value("size_mask").toInt());
//    if(settings.contains("thr_fg"))
//        this->spin_fgthr->setValue(settings.value("thr_fg").toInt());
//    if(settings.contains("thr_size"))
//        this->spin_sizethr->setValue(settings.value("thr_size").toInt());

    this->edit_output->setText("");
    this->spin_bgthr->setValue(10);
    this->spin_conviter->setValue(10);
    this->spin_distance->setValue(11);
    this->spin_fgthr->setValue(150);
    this->spin_sizethr->setValue(1000);
}

bool neuronPickerDialog::load()
{
    QString fname_input;
    if(this->edit_load->text().isEmpty()){
        fname_input=fname_previnput;
    }else{
        fname_input=this->edit_load->text();
    }
    fname_input = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
                                               fname_input,
                                               QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    if(fname_input.isEmpty()){
        return false;
    }

    qDebug()<<"NeuronPicker: is going to load "<<fname_input;
    qDebug()<<"NeuronPicker: reset environment";
    {//reset previous content first
        if(image1Dc_in != 0){
            delete []image1Dc_in; image1Dc_in=0;
        }
        if(image1Dc_out != 0){
            neuronPickerMain::memory_free_uchar1D(image1Dc_out);
            image1Dc_out=0;
        }
        intype=0;
        sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
        LList.clear();
        cb_marker->clear();
        qDebug()<<"NeuronPicker: reset input window";
    }

    qDebug()<<"NeuronPicker: load new image";
    if(!simple_loadimage_wrapper(*callback, fname_input.toStdString().c_str(), image1Dc_in, sz_img, intype))
    {
      v3d_msg("load image "+fname_input+" error!");
      return false;
    }
    qDebug()<<"\t>>read image file "<< fname_input <<" complete.";
    qDebug()<<"\t\timage size: [w="<<sz_img[0]<<", h="<<sz_img[1]<<", z="<<sz_img[2]<<", c="<<sz_img[3]<<"]";
    qDebug()<<"\t\tdatatype: "<<intype;

//    if(sz_img[3]<3){
//        v3d_msg("Currently this program only supports image with 3 channels (RGB).", 0);
//        return false;
//    }
    if(sz_img[3]>3){
        sz_img[3]=3;
        qDebug()<<"More than 3 channels were loaded. The first 3 channel will be applied for analysis.";
    }

    qDebug()<<"NeuronPicker: initialize veriables";

    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

    if (intype == 1) //V3D_UINT8;
    {
        pickerObj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
    }
    else if (intype == 2) //V3D_UINT16;
    {
        pickerObj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
        neuronPickerDialog::convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else if(intype == 4) //V3D_FLOAT32;
    {
        pickerObj.pushNewData<float>((float*)image1Dc_in, sz_img);
        neuronPickerDialog::convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else
    {
        v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.", 0);
        return false;
    }

    this->edit_load->setText(fname_input);
    this->edit_output->setText(fname_input + ".extract");

    qDebug()<<"NeuronPicker: update visualization windows";
    updateInputWindow();
    updateOutputWindow();
    checkButtons();

    fname_previnput=fname_input;
}

void neuronPickerDialog::output()
{
    QString fname_outbase=edit_output->text();
    QString fname_output = QFileDialog::getSaveFileName(0, QObject::tr("Choose the output folder and prefix"),
                                               fname_outbase,
                                               "*.*");

    if(!fname_output.isEmpty()){
        fname_outbase=fname_output;
    }
    edit_output->setText(fname_outbase);
    checkButtons();
}

void neuronPickerDialog::autoSeeds()
{
    vector<V3DLONG> seeds;
    pickerObj.autoSeeds(seeds, spin_distance->value(), spin_conviter->value(), spin_fgthr->value(), spin_bgthr->value(), spin_sizethr->value());

    //load and update markers
    LList.clear();
    poss_landmark.clear();
    QStringList cb_items;
    for (int i=0;i<seeds.size();i++)
    {
        poss_landmark.push_back(seeds[i]);
        vector<V3DLONG> pos=neuronPickerMain::pos2xyz(seeds[i], sz_img[0], sz_img[0]*sz_img[1]);
        LocationSimple tmpLS(pos[0]+1,pos[1]+1,pos[2]+1);
        LList.append(tmpLS);
        LList[i].color.r=196;
        LList[i].color.g=LList[i].color.b=0;
        cb_items.append("marker: " + QString::number(i+1));
    }
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            callback->setLandmark(v3dhandleList_current[i],LList);
            callback->pushObjectIn3DWindow(v3dhandleList_current[i]);

            break;
        }
    }
    cb_marker->clear();
    cb_marker->addItems(cb_items);
    checkButtons();
}

void neuronPickerDialog::runall()
{
    //load and update markers
    LList.clear();
    QStringList cb_items;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            LandmarkList LList_in = callback->getLandmark(v3dhandleList_current[i]);
            for(int i=0; i<LList_in.size(); i++){
                LList.append(LList_in.at(i));
                LList[i].color.r=196;
                LList[i].color.g=LList[i].color.b=0;
                cb_items.append("marker: " + QString::number(i+1));
            }

            break;
        }
    }
    poss_landmark=neuronPickerMain::landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    cb_marker->clear();
    cb_marker->addItems(cb_items);
    checkButtons();
    if(LList.size()<=0){
        return;
    }

    //extract by markers and save
    V3DLONG success=0;
    for(int idx_landmark=0; idx_landmark<poss_landmark.size(); idx_landmark++){
        V3DLONG pos_landmark=poss_landmark[idx_landmark];
        pos_out=pos_landmark;
        V3DLONG rsize=pickerObj.extractMargin_uchar(image1Dc_out,sz_out, pos_landmark, spin_conviter->value(), spin_distance->value(), spin_bgthr->value(), spin_sizemargin->value());

        if(rsize>0){
            QString fname_output=this->edit_output->text() + "_" + QString::number((int)LList.at(idx_landmark).x) + "_" +
                    QString::number((int)LList.at(idx_landmark).y) + "_" + QString::number((int)LList.at(idx_landmark).z) + ".v3dpbd";
            if(!simple_saveimage_wrapper(*callback, qPrintable(fname_output),image1Dc_out,sz_out,1)){
                v3d_msg("failed to save file to " + fname_output);
                return;
            }else{
                qDebug()<<"NeuronPicker: "<<fname_output;
            }
            fname_output=this->edit_output->text() + "_" + QString::number((int)LList.at(idx_landmark).x) + "_" +
                    QString::number((int)LList.at(idx_landmark).y) + "_" + QString::number((int)LList.at(idx_landmark).z) + ".marker";
            neuronPickerMain::saveSingleMarker(poss_landmark[idx_landmark],fname_output,sz_img);

            success++;
        }
    }

    QString myMessage = "Extracted and saved "+QString::number(success) + " neurons from " + QString::number(poss_landmark.size()) + " markers.";
    v3d_msg(myMessage);
}

int neuronPickerDialog::loadMarkers()
{
    LList.clear();
    QStringList cb_items;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            LandmarkList LList_in = callback->getLandmark(v3dhandleList_current[i]);
            for(int i=0; i<LList_in.size(); i++){
                LList.append(LList_in.at(i));
                LList[i].color.r=196;
                LList[i].color.g=LList[i].color.b=0;
                cb_items.append("marker: " + QString::number(i+1));
            }

            break;
        }
    }
    poss_landmark=neuronPickerMain::landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    cb_marker->clear();
    cb_marker->addItems(cb_items);
    if(LList.size()>0){
        extract();
    }
    checkButtons();
    return(LList.size());
}

void neuronPickerDialog::syncMarkers(int c)
{
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            for(int i=0; i<LList.size(); i++){
                if(i==cb_marker->currentIndex()){
                    LList[i].color.b=0;
                }else{
                    LList[i].color.b=196;
                }
            }
            callback->setLandmark(v3dhandleList_current[i],LList);
            callback->pushObjectIn3DWindow(v3dhandleList_current[i]);
        }
    }
}

void neuronPickerDialog::extract()
{
    if(LList.size()<1 || cb_marker->count()<1 || cb_marker->currentIndex()+1>LList.size() ){
        return;
    }
    int idx_landmark=cb_marker->currentIndex();
    V3DLONG pos_landmark=poss_landmark[idx_landmark];

    qDebug()<<"start extracting";
    pos_out=pos_landmark;
    V3DLONG rsize=pickerObj.extractMargin_uchar(image1Dc_out,sz_out, pos_landmark, spin_conviter->value(), spin_distance->value(), spin_bgthr->value(), spin_sizemargin->value());
    if(rsize>0){
        qDebug()<<"NeuronPicker: push for visualization: "<<image1Dc_out<<":"<<sz_out[0]<<":"<<sz_out[1]<<":"<<sz_out[2]<<":"<<sz_out[3];
        updateOutputWindow();
    }else
        v3d_msg("Nothing were found. Please change marker or adjust parameters.");

    checkButtons();
}

void neuronPickerDialog::saveFile()
{
    int idx_landmark=cb_marker->currentIndex();
    V3DLONG sz_img_sub[4];
//    sz_img_sub[0]=sz_img[0];
//    sz_img_sub[1]=sz_img[1];
//    sz_img_sub[2]=sz_img[2];
//    sz_img_sub[3]=3;
    sz_img_sub[0]=sz_out[0];
    sz_img_sub[1]=sz_out[1];
    sz_img_sub[2]=sz_out[2];
    sz_img_sub[3]=sz_out[3];
    QString fname_output=this->edit_output->text() + "_" + QString::number((int)LList.at(idx_landmark).x) + "_" +
            QString::number((int)LList.at(idx_landmark).y) + "_" + QString::number((int)LList.at(idx_landmark).z) + ".v3dpbd";
    if(!simple_saveimage_wrapper(*callback, qPrintable(fname_output),image1Dc_out,sz_img_sub,1)){
        v3d_msg("failed to save file to " + fname_output);
        return;
    }else{
        qDebug()<<"NeuronPicker: "<<fname_output;
    }
    fname_output=this->edit_output->text() + "_" + QString::number((int)LList.at(idx_landmark).x) + "_" +
            QString::number((int)LList.at(idx_landmark).y) + "_" + QString::number((int)LList.at(idx_landmark).z) + ".marker";
    neuronPickerMain::saveSingleMarker(poss_landmark[idx_landmark],fname_output,sz_img);
    LList[idx_landmark].color.r = 0;
    LList[idx_landmark].color.g = 255;
    if(idx_landmark+1<cb_marker->count()){
        cb_marker->setCurrentIndex(idx_landmark+1);
        extract();
    }else{
        v3d_msg("Reach the end of marker list.");
    }
    checkButtons();
}

void neuronPickerDialog::skip()
{
    int idx_landmark=cb_marker->currentIndex();
    if(idx_landmark+1<cb_marker->count()){
        cb_marker->setCurrentIndex(idx_landmark+1);
        extract();
    }else{
        v3d_msg("Reach the end of marker list.");
    }
    checkButtons();
}


void neuronPickerDialog::checkButtons()
{
    if(image1Dc_in==0){
        btn_update->setEnabled(false);
        btn_extract->setEnabled(false);
        btn_save->setEnabled(false);
        btn_next->setEnabled(false);
        btn_runall->setEnabled(false);
        btn_autoMarkers->setEnabled(false);
    }else{
        bool winfound=false;
        v3dhandleList v3dhandleList_current=callback->getImageWindowList();
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
            {
                winfound=true;
            }
        }
        if(winfound){
            btn_update->setEnabled(true);
            btn_runall->setEnabled(true);
            btn_autoMarkers->setEnabled(true);
        }
        if(cb_marker->count()>0){
            btn_extract->setEnabled(true);
            if(this->edit_output->text().isEmpty() || image1Dc_out==0){
                btn_save->setEnabled(false);
                btn_next->setEnabled(false);
            }else{
                btn_save->setEnabled(true);
                btn_next->setEnabled(true);
            }
        }else{
            btn_extract->setEnabled(false);
            btn_next->setEnabled(false);
            btn_save->setEnabled(false);
        }
    }
}

void neuronPickerDialog::updateInputWindow()
{
    //search in open windows
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            winfound=true;
        }
    }
    if(image1Dc_in != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d;
        image4d.setFileName(this->edit_load->text().toStdString().c_str());
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1D_input=neuronPickerMain::memory_allocate_uchar1D(size_page);
        memcpy(image1D_input, image1Dc_in, size_page*sizeof(unsigned char));
        image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);
        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            //update the image
            callback->setImage(v3dhandle_main, &image4d);
            callback->setImageName(v3dhandle_main, NAME_INWIN);
            callback->updateImageWindow(v3dhandle_main);
            callback->open3DWindow(v3dhandle_main);
        }else{
            v3dhandleList_current=callback->getImageWindowList();
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
                {
                    //update the image
                    if(!callback->setImage(v3dhandleList_current[i], &image4d)){
                        v3d_msg("Failed to update input image");
                    }
                    callback->updateImageWindow(v3dhandleList_current[i]);
                    //callback->open3DWindow(v3dhandleList_current[i]);
                    callback->pushImageIn3DWindow(v3dhandleList_current[i]);
                    callback->open3DWindow(v3dhandleList_current[i]);
                }
            }
        }
    }else if(winfound){
        //no image loaded, but there is a window? clean up the contents
        //clean up landmarks
        LandmarkList LList_empty;
        //clean up window
        Image4DSimple image4d;
        unsigned char *image1D_input=neuronPickerMain::memory_allocate_uchar1D(1);
        image1D_input[0]=0;
        image4d.setData(image1D_input,1,1,1,1,V3D_UINT8);
        v3dhandleList_current=callback->getImageWindowList();
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
            {
                callback->setLandmark(v3dhandleList_current[i], LList_empty);
                callback->setImage(v3dhandleList_current[i], &image4d);
                callback->updateImageWindow(v3dhandleList_current[i]);
                callback->pushImageIn3DWindow(v3dhandleList_current[i]);
            }
        }
    }
}

void neuronPickerDialog::updateOutputWindow()
{
    //search in open windows
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            winfound=true;
        }
    }
    if(image1Dc_out != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d;
        V3DLONG size_page = sz_out[0]*sz_out[1]*sz_out[2]*sz_out[3];
        unsigned char* image1Dc_input=neuronPickerMain::memory_allocate_uchar1D(size_page);
        memcpy(image1Dc_input, image1Dc_out, size_page*sizeof(unsigned char));
        image4d.setData(image1Dc_input, sz_out[0], sz_out[1], sz_out[2], sz_out[3], V3D_UINT8);
        LandmarkList LList_single;
        vector<V3DLONG> coord=neuronPickerMain::pos2xyz(pos_out, sz_out[0], sz_out[0]*sz_out[1]);
        LocationSimple Ltmp(coord[0],coord[1],coord[2]);
        LList_single.append(Ltmp);

        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            callback->setImage(v3dhandle_main, &image4d);
            callback->setLandmark(v3dhandle_main, LList_single);
            callback->setImageName(v3dhandle_main, NAME_OUTWIN);
            callback->updateImageWindow(v3dhandle_main);
            callback->open3DWindow(v3dhandle_main);
            callback->pushObjectIn3DWindow(v3dhandle_main);
        }else{
            //update the image
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
                {
                    callback->setImage(v3dhandleList_current[i], &image4d);
                    callback->setLandmark(v3dhandleList_current[i], LList_single);
                    callback->updateImageWindow(v3dhandleList_current[i]);
                    //callback->open3DWindow(v3dhandleList_current[i]);
                    callback->pushImageIn3DWindow(v3dhandleList_current[i]);
                    callback->open3DWindow(v3dhandleList_current[i]);
                    callback->pushObjectIn3DWindow(v3dhandleList_current[i]);
                }
            }
        }
    }else if(winfound){
        //no image loaded, but there is a window? clean up the window content
        Image4DSimple image4d;
        LandmarkList LList_empty;
        unsigned char *image1D_input=neuronPickerMain::memory_allocate_uchar1D(1);
        image1D_input[0]=0;
        image4d.setData(image1D_input,1,1,1,1,V3D_UINT8);
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
            {
                callback->setImage(v3dhandleList_current[i], &image4d);
                callback->setLandmark(v3dhandleList_current[i], LList_empty);
                callback->updateImageWindow(v3dhandleList_current[i]);
                //callback->close3DWindow(v3dhandleList_current[i]);
                callback->pushImageIn3DWindow(v3dhandleList_current[i]);
                callback->pushObjectIn3DWindow(v3dhandleList_current[i]);
            }
        }
    }
}
