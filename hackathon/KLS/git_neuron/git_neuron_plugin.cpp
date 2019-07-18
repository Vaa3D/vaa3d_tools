
#include "v3d_message.h"
#include <vector>
#include <map>
#include "git_neuron_plugin.h"
#include "git_neuron_func.h"
#include <iostream>

#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif

using namespace std;

Q_EXPORT_PLUGIN2(git_neuron, git_neuron_swc);
 
QStringList git_neuron_swc::menulist() const
{
	return QStringList() 
		<<tr("compare_neuron")
		<<tr("merge_neuron")
		<<tr("about");
}

QStringList git_neuron_swc::funclist() const
{
	return QStringList()
        <<tr("compare_neuron")
		<<tr("help");
}

void git_neuron_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("compare_neuron"))
	{
        do_compare(callback,parent);
	}
	else if (menu_name == tr("merge_neuron"))
	{
        do_merge(callback,parent);
	}else
	{
		v3d_msg(tr("This plugin will compare two neurons.. "
			"Developed by Lingsheng Kong, 2019-6-27"));
	}
}

bool git_neuron_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	return true;
}


void git_neuron_swc::do_compare(V3DPluginCallback2 &callback, QWidget *parent)
{
    git_neuron_dialog connectfunc;
	//cout<<"test1"<<endl;
    connectfunc.exec();
}

void git_neuron_swc::do_merge(V3DPluginCallback2 &callback, QWidget *parent)
{
    git_neuron_dialog connectfunc(1);
    connectfunc.exec();
}

git_neuron_dialog::git_neuron_dialog()
{
    creat();
	//cout<<"test2"<<endl;
    initDlg();
    checkbtn();
}

git_neuron_dialog::git_neuron_dialog(int n)
{
    creat1();
	cout<<"test"<<endl;
    //initDlg1();
    checkbtn1();
}

git_neuron_dialog::~git_neuron_dialog()
{
    QSettings settings("V3D plugin","git_neuron");

    //settings.setValue("fname_swc",edit_load0->text());
    //settings.setValue("fname_output",edit_load1->text());
}

void git_neuron_dialog::initDlg()
{
    QSettings settings("V3D plugin","git_neuron");
	cout<<"test3"<<endl;
    if(settings.contains("fname_swc"))
        this->edit_load0->setText(settings.value("fname_swc").toString());
    if(settings.contains("fname_output"))
        this->edit_load1->setText(settings.value("fname_output").toString());
}

bool git_neuron_dialog::checkbtn()
{
	cout<<"test4"<<endl;
    if(this->edit_load0->text().isEmpty() || this->edit_load1->text().isEmpty()){
       
		btn_run->setEnabled(false);
        return false;
    }else{
        btn_run->setEnabled(true);
        return true;
    }
}

bool git_neuron_dialog::checkbtn1()
{
	//cout<<"test3"<<endl;
    if(this->edit_load2->text().isEmpty() || this->edit_load3->text().isEmpty() || this->edit_load4->text().isEmpty()){
		btn_merge->setEnabled(false);
        return false;
    }else{
		btn_merge->setEnabled(true);
        return true;
    }
}

void git_neuron_dialog::creat()
{
    gridLayout = new QGridLayout();

    //input zone
    label_load0 = new QLabel(QObject::tr("Input SWC file1:"));
    gridLayout->addWidget(label_load0,0,0,1,8);
    edit_load0 = new QLineEdit();
    edit_load0->setReadOnly(true);
    gridLayout->addWidget(edit_load0,1,0,1,6);
    btn_load0 = new QPushButton("Select");
    gridLayout->addWidget(btn_load0,1,6,1,1);

    label_load1 = new QLabel(QObject::tr("Input SWC file2:"));
    gridLayout->addWidget(label_load1,2,0,1,8);
    edit_load1 = new QLineEdit();
    edit_load1->setReadOnly(false);
    gridLayout->addWidget(edit_load1,3,0,1,6);
    btn_load1 = new QPushButton("Select");
    gridLayout->addWidget(btn_load1,3,6,1,1);

    connect(btn_load0, SIGNAL(clicked()), this, SLOT(load0()));
    connect(btn_load1, SIGNAL(clicked()), this, SLOT(load1()));	
	
    //operation zone
    QFrame *line_2 = new QFrame();
	
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);	
    gridLayout->addWidget(line_2,15,0,1,7);

    btn_run = new QPushButton("Compare");
    connect(btn_run,     SIGNAL(clicked()), this, SLOT(run()));
    gridLayout->addWidget(btn_run,16,2,1,1);


    btn_quit = new QPushButton("Quit");
    connect(btn_quit,     SIGNAL(clicked()), this, SLOT(reject()));
    gridLayout->addWidget(btn_quit,16,6,1,1);

    setLayout(gridLayout);
}



void git_neuron_dialog::initDlg1()
{
    QSettings settings("V3D plugin","git_neuron");
	cout<<"test1"<<endl;
    if(settings.contains("fname_swc"))
        this->edit_load2->setText(settings.value("fname_swc").toString());
    if(settings.contains("fname_output"))
        this->edit_load3->setText(settings.value("fname_output").toString());
}

void git_neuron_dialog::creat1()
{
    gridLayout = new QGridLayout();
	cout<<"test2"<<endl;
    //input zone
    label_load3 = new QLabel(QObject::tr("Input SWC file1:"));
    gridLayout->addWidget(label_load3,0,0,1,8);
    edit_load3 = new QLineEdit();
    edit_load3->setReadOnly(true);
    gridLayout->addWidget(edit_load3,1,0,1,6);
    btn_load3 = new QPushButton("Select");
    gridLayout->addWidget(btn_load3,1,6,1,1);

    label_load4 = new QLabel(QObject::tr("Input SWC file2:"));
    gridLayout->addWidget(label_load4,2,0,1,8);
    edit_load4 = new QLineEdit();
    edit_load4->setReadOnly(false);
    gridLayout->addWidget(edit_load4,3,0,1,6);
    btn_load4 = new QPushButton("Select");
    gridLayout->addWidget(btn_load4,3,6,1,1);

	label_load2 = new QLabel(QObject::tr("Input parent SWC file:"));
    gridLayout->addWidget(label_load2,4,0,1,8);
    edit_load2 = new QLineEdit();
    edit_load2->setReadOnly(true);
    gridLayout->addWidget(edit_load2,5,0,1,6);
    btn_load2 = new QPushButton("Select");
    gridLayout->addWidget(btn_load2,5,6,1,1);


    connect(btn_load3, SIGNAL(clicked()), this, SLOT(load3()));
    connect(btn_load4, SIGNAL(clicked()), this, SLOT(load4()));	
	connect(btn_load2, SIGNAL(clicked()), this, SLOT(load2()));	

    //operation zone
    QFrame *line_2 = new QFrame();
	
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);	
    gridLayout->addWidget(line_2,15,0,1,7);

    btn_merge = new QPushButton("Merge");
    connect(btn_merge,     SIGNAL(clicked()), this, SLOT(merge()));
    gridLayout->addWidget(btn_merge,16,2,1,1);

    btn_quit2 = new QPushButton("Quit2");
    connect(btn_quit2,     SIGNAL(clicked()), this, SLOT(reject()));
    gridLayout->addWidget(btn_quit2,16,6,1,1);
	cout<<"test 5 "<<endl;
    setLayout(gridLayout);
}





bool git_neuron_dialog::load0()
{
    QString fileOpenName = this->edit_load0->text();
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(!fileOpenName.isEmpty()){
        edit_load0->setText(fileOpenName);
    }

    checkbtn();
    return true;
}


bool git_neuron_dialog::load1()
{
    QString fileOpenName = this->edit_load1->text();
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(!fileOpenName.isEmpty()){
        edit_load1->setText(fileOpenName);
    }

    checkbtn();
    return true;
}


bool git_neuron_dialog::load2()
{
    QString fileOpenName = this->edit_load2->text();
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(!fileOpenName.isEmpty()){
        edit_load2->setText(fileOpenName);
    }

    checkbtn1();
    return true;
}


bool git_neuron_dialog::load3()
{
    QString fileOpenName = this->edit_load3->text();
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(!fileOpenName.isEmpty()){
        edit_load3->setText(fileOpenName);
    }

    checkbtn1();
    return true;
}

bool git_neuron_dialog::load4()
{
    QString fileOpenName = this->edit_load4->text();
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(!fileOpenName.isEmpty()){
        edit_load4->setText(fileOpenName);
    }

    checkbtn1();
    return true;
}

void git_neuron_dialog::run()
{
    if(!checkbtn()){
        addinfo(">> please specify input and output\n");
        return;
    }

    qDebug()<<"load and initialize";
    NeuronTree* nt;
	NeuronTree* nt2;
    //load neuron
    nt = new NeuronTree();
	nt2 = new NeuronTree();
    QString fileOpenName = this->edit_load0->text();
	QString fileOpenName2 = this->edit_load1->text();
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
        *nt = readSWC_file(fileOpenName);
    }
	if (fileOpenName2.toUpper().endsWith(".SWC") || fileOpenName2.toUpper().endsWith(".ESWC"))
    {
        *nt2 = readSWC_file(fileOpenName2);
    }
    if(nt->listNeuron.size()<=0){
        addinfo("failed to read SWC file: "+fileOpenName,true);
    }
	neuron_compare(nt,nt2,fileOpenName + "_compare.eswc",fileOpenName2 + "_compare.eswc");

}

void git_neuron_dialog::merge()
{
    if(!checkbtn1()){
        addinfo(">> please specify input and output\n");
        return;
    }

    qDebug()<<"load and initialize";
    NeuronTree* nt;
	NeuronTree* nt2;
	NeuronTree* nt3;
    //load neuron
    nt = new NeuronTree();
	nt2 = new NeuronTree();
	nt3 = new NeuronTree();
    QString fileOpenName = this->edit_load3->text();
	QString fileOpenName2 = this->edit_load4->text();
	QString fileOpenName3 = this->edit_load2->text();
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
        *nt = readSWC_file(fileOpenName);
    }
	if (fileOpenName2.toUpper().endsWith(".SWC") || fileOpenName2.toUpper().endsWith(".ESWC"))
    {
        *nt2 = readSWC_file(fileOpenName2);
    }

	if (fileOpenName3.toUpper().endsWith(".SWC") || fileOpenName3.toUpper().endsWith(".ESWC"))
    {
        *nt3 = readSWC_file(fileOpenName3);
    }
    if(nt->listNeuron.size()<=0){
        addinfo("failed to read SWC file: "+fileOpenName,true);
    }
	neuron_merge_test(nt,nt2,nt3,fileOpenName + "_merge_test.eswc");

}

void git_neuron_dialog::addinfo(QString info, bool bold)
{
    QTextCursor textcursor = this->text_info->textCursor();
    textcursor.movePosition(QTextCursor::Start);
    if(bold)
        textcursor.insertHtml(">> <b>"+info+"</b><br>");
    else
        textcursor.insertText(">> "+info+"\n");
}
