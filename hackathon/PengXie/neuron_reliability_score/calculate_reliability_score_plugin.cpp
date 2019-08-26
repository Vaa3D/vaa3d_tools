/* calculate_reliability_score_plugin.cpp
 * This plugin will generate the reliability score of neuron reconstruction.
 * 2015-4-27 : by Hanbo Chen, Hang Xiao, Hanchuan Peng
 */
 

#include "calculate_reliability_score_plugin.h"


//using namespace std;
Q_EXPORT_PLUGIN2(calculate_reliability_score, neuronScore);

void printHelp()
{
    qDebug()<<"vaa3d -x libname -f calculate_score -i <input_image> <input_swc> -o <output> -p [<score type> [<radius_factor>]]";
    qDebug()<<"score type (default 1): 0/1 segment between brach node. 0: node wise comparison; 1: segment wise comparison.";
    qDebug()<<"\t\t2/3 straight line segment. 2: node wise comparison; 3: segment wise comparison.";
    qDebug()<<"radius factor (RF) (default 2): the area around the reconstruction within distance RF*radius will be masked before searching for alternation pathways.";
}


QStringList neuronScore::menulist() const
{
	return QStringList() 
		<<tr("calculate_score")
        <<tr("calculate_score_terafly")
		<<tr("about");
}

QStringList neuronScore::funclist() const
{
	return QStringList()
		<<tr("calculate_score")
		<<tr("help");
}

void neuronScore::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("calculate_score"))
	{
        neuronScoreDialog myDlg(callback, parent);
        myDlg.exec();
        if(myDlg.isaccept){
            QString fname_img=myDlg.edit_img->text();
            QString fname_swc=myDlg.edit_swc->text();
            QString fname_output=myDlg.edit_output->text();
            int type=myDlg.cb_scoreType->currentIndex();
            float radiusFactor=myDlg.spin_radiusRate->value();
            doCalculateScore(callback, fname_img,fname_swc,fname_output,type,radiusFactor,1);
            v3d_msg("Done! files saved to "+fname_output+"_score.txt");
        }
	}
    else if (menu_name == tr("calculate_score_terafly"))
    {
        QString imagePath = callback.getPathTeraFly();
        NeuronTree nt = callback.getSWCTeraFly();
        NeuronTree nt_scored = calculateScoreTerafly(callback,imagePath,nt,1,2);
        callback.setSWCTeraFly(nt_scored);
    }
	else
	{
		v3d_msg(tr("This plugin will generate the reliability score of neuron reconstruction.. "
			"Developed by Hanbo Chen, Hang Xiao, Hanchuan Peng, 2015-4-27"));
	}
}

bool neuronScore::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("calculate_score"))
	{
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input image and swc. \n");
            printHelp();
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file name. \n");
            printHelp();
            return false;
        }

        int k=0;
        int scoreType = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        float radiusFactor = (paras.size() >= k+1) ? atof(paras[k]) : 2;  k++;

        doCalculateScore(callback, infiles[0],infiles[1],outfiles[0],scoreType,radiusFactor,0);
	}
    else if (func_name == tr("calculate_score_terafly"))
    {
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input image and swc. \n");
            printHelp();
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file name. \n");
            printHelp();
            return false;
        }

        int k=0;
        int scoreType = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        float radiusFactor = (paras.size() >= k+1) ? atof(paras[k]) : 2;  k++;
        NeuronTree nt = readSWC_file(QString(infiles[1]));
        V_NeuronSWC_list nt_decomposed = NeuronTree__2__V_NeuronSWC_list(nt);
        NeuronTree nt_new = V_NeuronSWC_list__2__NeuronTree(nt_decomposed);
        NeuronTree nt_scored = calculateScoreTerafly(callback,infiles[0],nt_new,scoreType,radiusFactor);
        writeESWC_file(QString(outfiles[0]),nt_scored);
    }
	else if (func_name == tr("help"))
	{
        printHelp();
	}
	else return false;

	return true;
}

neuronScoreDialog::neuronScoreDialog(V3DPluginCallback2 &cb, QWidget *pt)
{
    callback=&cb;
    parent=pt;

    isaccept=0;
    creat();
    initDlg();
    checkButtons();
}

neuronScoreDialog::~neuronScoreDialog()
{
    QSettings settings("V3D plugin","neuronReliablilityScore");

    settings.setValue("fname_img",edit_img->text());
    settings.setValue("fname_swc",edit_swc->text());
    settings.setValue("fname_output",edit_output->text());
    settings.setValue("scoreType",cb_scoreType->currentIndex());
    settings.setValue("radiusFactor",spin_radiusRate->value());
}

void neuronScoreDialog::initDlg()
{
    QSettings settings("V3D plugin","neuronReliablilityScore");

    if(settings.contains("fname_img"))
        this->edit_img->setText(settings.value("fname_img").toString());
    if(settings.contains("fname_swc"))
        this->edit_swc->setText(settings.value("fname_swc").toString());
    if(settings.contains("fname_output"))
        this->edit_output->setText(settings.value("fname_output").toString());
    if(settings.contains("scoreType"))
        this->cb_scoreType->setCurrentIndex(settings.value("scoreType").toInt());
    if(settings.contains("radiusFactor"))
        this->spin_radiusRate->setValue(settings.value("radiusFactor").toInt());
}
void neuronScoreDialog::creat()
{
    QGridLayout *gridLayout = new QGridLayout();

    //I/O zone
    QLabel* label_load = new QLabel(QObject::tr("Input Image:"));
    gridLayout->addWidget(label_load,0,0,1,1);
    edit_img = new QLineEdit();
    edit_img->setText("");
    gridLayout->addWidget(edit_img,0,1,1,4);
    btn_img = new QPushButton("...");
    gridLayout->addWidget(btn_img,0,5,1,1);

    QLabel* label_swc = new QLabel(QObject::tr("Input SWC File:"));
    gridLayout->addWidget(label_swc,1,0,1,1);
    edit_swc = new QLineEdit();
    edit_swc->setText("");
    gridLayout->addWidget(edit_swc,1,1,1,4);
    btn_swc = new QPushButton("...");
    gridLayout->addWidget(btn_swc,1,5,1,1);

    QLabel* label_output = new QLabel(QObject::tr("Output Prefix:"));
    gridLayout->addWidget(label_output,2,0,1,1);
    edit_output = new QLineEdit();
    edit_output->setText("");
    gridLayout->addWidget(edit_output,2,1,1,4);
    btn_output = new QPushButton("...");
    gridLayout->addWidget(btn_output,2,5,1,1);

    connect(btn_img, SIGNAL(clicked()), this, SLOT(set_img()));
    connect(btn_swc, SIGNAL(clicked()), this, SLOT(set_swc()));
    connect(btn_output, SIGNAL(clicked()), this, SLOT(set_output()));

    //parameter zone
    QFrame *line_0 = new QFrame();
    line_0->setFrameShape(QFrame::HLine);
    line_0->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_0,3,0,1,6);
    spin_radiusRate = new QDoubleSpinBox();
    spin_radiusRate->setRange(0,10000); spin_radiusRate->setValue(2);
    QLabel* label_a = new QLabel("scale radius:");
    gridLayout->addWidget(label_a,4,0,1,2,Qt::AlignRight);
    gridLayout->addWidget(spin_radiusRate,4,2,1,1);
    cb_scoreType = new QComboBox();
    cb_scoreType->addItem("topology segment, node wise");
    cb_scoreType->addItem("topology segment, segment wise");
    cb_scoreType->addItem("straight line, node wise");
    cb_scoreType->addItem("straight line, segment wise");
    gridLayout->addWidget(cb_scoreType,4,3,1,3);
    cb_scoreType->setCurrentIndex(1);

    //button zone
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_1,7,0,1,6);
    btn_quit = new QPushButton("Cancel");
    gridLayout->addWidget(btn_quit,8,4,1,1);
    btn_ok = new QPushButton("OK");
    gridLayout->addWidget(btn_ok,8,5,1,1);

    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btn_ok, SIGNAL(clicked()), this, SLOT(accept()));

    //info zone
    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_2,10,0,1,6);
    edit_info = new QTextEdit();
    edit_info->insertPlainText("The scores are calculated as follows:\n");
    edit_info->insertPlainText("1) Find the segments in reconstructions;\n");
    edit_info->insertPlainText("2) For each segment, mask the image by radius*radius_factor and redo the tracing using fast martching;\n");
    edit_info->insertPlainText("3) The average intensity along original segment (x) and the new segment (y) will be calculated;\n");
    edit_info->insertPlainText("4) For segment wise comparison, the score will be y/x. For node wise comparison, the score will be y/node_intensity.\n");

    gridLayout->addWidget(edit_info,11,0,1,6);

    setLayout(gridLayout);
}


void neuronScoreDialog::checkButtons()
{
    if(edit_img->text().isEmpty() ||
            edit_swc->text().isEmpty() ||
            edit_output->text().isEmpty()){
        btn_ok->setEnabled(false);
    }else{
        btn_ok->setEnabled(true);
    }
}

void neuronScoreDialog::accept()
{
    isaccept = 1;

    QDialog::accept();
}

void neuronScoreDialog::set_img()
{
    QString fname_input;
    if(this->edit_img->text().isEmpty()){
        fname_input="";
    }else{
        fname_input=this->edit_img->text();
    }
    fname_input = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
                                               fname_input,
                                               QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    if(fname_input.isEmpty()){
        return;
    }else{
        this->edit_img->setText(fname_input);
    }

    checkButtons();
}

void neuronScoreDialog::set_swc()
{
    QString fname_input;
    if(this->edit_swc->text().isEmpty()){
        fname_input="";
    }else{
        fname_input=this->edit_swc->text();
    }
    fname_input = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input reconstruction "),
                                               fname_input,
                                               QObject::tr("reconstruction (*.swc);;All(*)"));
    if(fname_input.isEmpty()){
        return;
    }else{
        this->edit_swc->setText(fname_input);
        if(this->edit_output->text().isEmpty())
            this->edit_output->setText(fname_input);
    }

    checkButtons();
}

void neuronScoreDialog::set_output()
{
    QString fname_output=edit_output->text();
    fname_output = QFileDialog::getSaveFileName(0, QObject::tr("Choose the output folder and prefix"),
                                               fname_output,
                                               "*.*");
    if(!fname_output.isEmpty()){
        edit_output->setText(fname_output);
    }
    checkButtons();
}


