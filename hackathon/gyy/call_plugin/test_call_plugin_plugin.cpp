/* test_call_plugin_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-20 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "test_call_plugin_plugin.h"
#include <iostream>
#include "vn_app2.h"
#include "fastmarching_tree.h"
#include "fastmarching_dt.h"
#include "hierarchy_prune.h"
#include "marker_radius.h"
#include "basic_surf_objs.h"
#include "swc_convert.h"
#include "vn_imgpreprocess.h"
#include "volimg_proc.h"
using namespace std;
Q_EXPORT_PLUGIN2(test_call_plugin, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("call_function")
        <<tr("call_plugin")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("call_function")
        <<tr("call_plugin")
		<<tr("help");
}

//bool app2_dialog(PARA_APP2 p2);

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("call_function"))
	{

        PARA_APP2 p2;
        //if(!app2_dialog(p2)) return;
        QString versionStr = "v2.621";
        QString imagePath = QFileDialog::getOpenFileName(parent, QString("open image file:"));
        QString markerPath = QFileDialog::getOpenFileName(parent, QString("open marker file:"));
        p2.inimg_file = imagePath;
        p2.inmarker_file = markerPath;
        QString path_tmp("/home/balala/Desktop/call_plugin/");
        p2.outswc_file = path_tmp+"outswc"+".swc";

        proc_app2(callback, p2, versionStr);
	}

    else if (menu_name == tr("call_plugin"))
	{
        //v3d_msg("To be implemented.");
        //V3DPluginCallback2 v3d;
        //PARA_APP2 p2;
        //if(!app2_dialog(p2)) return;
        V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;
        QString imagepath = QFileDialog::getOpenFileName(parent, QString("open image file:"));
        QString markerpath = QFileDialog::getOpenFileName(parent, QString("open marker file:"));
        //QString tmppath = "/home/balala/Desktop/call_plugin";
        QString path_tmp("/home/balala/Desktop/call_plugin/");
        QString outswc_file = path_tmp+"outswc"+".swc";

        arg.type = "random"; std::vector<char*> arg_input_app2;
        std:: string fileName_string1(imagepath.toStdString());
        char* fileName_char1 =  new char[fileName_string1.length() + 1];
        strcpy(fileName_char1, fileName_string1.c_str());
        arg_input_app2.push_back(fileName_char1);
        arg.p = (void *) & arg_input_app2;
        input << arg;

        arg.type = "random"; vector<char*> arg_paras_app2;
        string fileName_string2(markerpath.toStdString());
        char *fileName_char2 = new char[fileName_string2.length() + 1];
        strcpy(fileName_char2, fileName_string2.c_str());
        arg_paras_app2.push_back(fileName_char2);
        arg.p = (void *) & arg_paras_app2;
        input << arg;

        arg.type = "random"; vector<char*> arg_output_app2;
        string fileName_string0(outswc_file.toStdString());
        char* fileName_char0 = new char[fileName_string0.length() + 1];
        strcpy(fileName_char0, fileName_string0.c_str());
        arg_output_app2.push_back(fileName_char0);
        arg.p = (void *) & arg_output_app2;
        output << arg;

        //QString plugin_name = "/home/balala/vaa3d_tools/released_plugins/v3d_plugins/vaa3dneuron2";
        QString plugin_name = "Vaa3D_Neuron2";
        QString func_name = "app2";
        callback.callPluginFunc(plugin_name, func_name, input, output);

	}

	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-20"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> * )input[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> * )input[1].p : 0;
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> * )output[0].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? *pinfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? *pparas : vector<char*>();
    vector<char*> outfiles = (poutfiles !=0) ? *poutfiles : vector<char*>();

    if (func_name == tr("call_function"))
	{
        //v3d_msg("To be implemented.");
        PARA_APP2 p2;
        QString versionStr = "v2.621";
        //cout << __LINE__ << endl;

        p2.inimg_file = infiles.empty() ? "" : infiles[0];
//        p2.inmarker_file = paras[0];
        if(p2.inimg_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }
        p2.outswc_file = outfiles.empty() ? "" : outfiles[0];
        int k=0;

        QString inmarker_file = paras.empty() ? "" : paras[0];

        if(inmarker_file.isEmpty())
        {
            cerr<<"Need a marker file"<<endl;
            return false;
        }

        p2.inmarker_file = inmarker_file;


        proc_app2(callback, p2, versionStr);
	}

    else if (func_name == tr("call_plugin"))
	{
        //v3d_msg("To be implemented.");
        PARA_APP2 p2;
        QString plugin_name = "Vaa3D_Neuron2";
        QString func_name = "app2";
        callback.callPluginFunc(plugin_name, func_name, input, output);

	}

	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

//bool app2_dialog(PARA_APP2 p2)
//{
//    if(!p2.p4dImage || !p2.p4dImage->valid())
//        return false;
//    int chn_num = p2.p4dImage->getCDim();

//    QDialog *dialog = new QDialog();
//    dialog->setWindowTitle("auto_tracing based on APP2");
//    QGridLayout *layout = new QGridLayout();

//    QSpinBox *channel_spinbox = new QSpinBox();
//    channel_spinbox->setRange(1, chn_num);
//    channel_spinbox->setValue(1);
//    QSpinBox *cnntype_spinbox = new QSpinBox();
//    cnntype_spinbox->setRange(1, 3);
//    cnntype_spinbox->setValue(2);
//    QSpinBox *bkgthresh_spinbox = new QSpinBox();
//    bkgthresh_spinbox->setRange(-2, 255);
//    bkgthresh_spinbox->setValue(10);

//    QLineEdit *lenthresh_editor = new QLineEdit(QString("").setNum(p2.length_thresh));
//    QLineEdit *srratio_editor = new QLineEdit(QString("").setNum(p2.SR_ratio));

//    QCheckBox *isgsdt_checker = new QCheckBox();
//    isgsdt_checker->setChecked(p2.is_gsdt);
//    QCheckBox *iswb_checker = new QCheckBox();
//    iswb_checker->setChecked(p2.is_break_accept);
//    QCheckBox *b256_checker = new QCheckBox();
//    b256_checker->setChecked(p2.b_256cube);
//    QCheckBox *b_radius2Dchecker = new QCheckBox();
//    b_radius2Dchecker->setChecked(p2.b_RadiusFrom2D);
//    QCheckBox *bresample_checker = new QCheckBox();
//    bresample_checker->setChecked(p2.b_resample);
//    QCheckBox *b_intensity_checker = new QCheckBox();
//    b_intensity_checker->setChecked(p2.b_intensity);
//    QCheckBox *b_brightfiled_checker = new QCheckBox();
//    b_brightfiled_checker->setChecked(p2.b_brightfiled);

//    layout->addWidget(new QLabel("color channel"),0,0);
//    layout->addWidget(channel_spinbox, 0,1,1,5);
//    layout->addWidget(new QLabel("background_threshold \n(if set as -1, \nthen auto-thresholding)"), 1,0);
//    layout->addWidget(bkgthresh_spinbox, 1,1,1,5);

//    QHBoxLayout *hbox1 = new QHBoxLayout();
//    hbox1->addWidget(new QLabel("auto-downsample"));
//    hbox1->addWidget(b256_checker);
//    hbox1->addWidget(new QLabel("use GSDT"));
//    hbox1->addWidget(isgsdt_checker);
//    hbox1->addWidget(new QLabel("allow gap"));
//    hbox1->addWidget(iswb_checker);
//    hbox1->addWidget(new QLabel("radius from 2D?"));
//    hbox1->addWidget(b_radius2Dchecker);

//    QHBoxLayout *hbox2 = new QHBoxLayout();
//    hbox2->addWidget(new QLabel("auto-resample SWC"));
//    hbox2->addWidget(bresample_checker);
//    hbox2->addWidget(new QLabel("high intensity background"));
//    hbox2->addWidget(new QLabel("bright filed"));
//    hbox2->addWidget(b_brightfiled_checker);

//    layout->addLayout(hbox1, 2,0,1,6);
//    layout->addLayout(hbox2, 3,0,1,6);

//    layout->addWidget(new QLabel("cnn_type"), 4,0);
//    layout->addWidget(cnntype_spinbox, 4,1,1,5);
//    layout->addWidget(new QLabel("length_thresh"), 5,0);
//    layout->addWidget(lenthresh_editor, 5,1,1,5);
//    layout->addWidget(new QLabel("SR_ratio"), 6,0);
//    layout->addWidget(srratio_editor, 6,1,1,5);

//    QHBoxLayout *hbox3 = new QHBoxLayout();
//    QPushButton *ok = new QPushButton(" ok ");
//    ok->setDefault(true);
//    QPushButton *cancel = new QPushButton("cancel");
//    hbox3->addWidget(cancel);
//    hbox3->addWidget(ok);

//    layout->addLayout(hbox3, 7,0,1,6);
//    dialog->setLayout(layout);
//    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
//    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(accept()));

//    if(dialog->exec() != QDialog::Accepted)
//            return false;

//    p2.channel = channel_spinbox->value() - 1;
//    p2.cnn_type = cnntype_spinbox->value();
//    p2.bkg_thresh = bkgthresh_spinbox->value();
//    p2.length_thresh = atof(lenthresh_editor->text().toStdString().c_str());
//    p2.SR_ratio = atof(srratio_editor->text().toStdString().c_str());
//    p2.is_gsdt = isgsdt_checker->isChecked();
//    p2.is_break_accept = iswb_checker->isChecked();
//    p2.b_256cube = b256_checker->isChecked();
//    p2.b_RadiusFrom2D = b_radius2Dchecker->isChecked();
//    p2.b_resample = bresample_checker->isChecked();
//    p2.b_intensity = b_intensity_checker->isChecked();
//    p2.b_brightfiled = b_brightfiled_checker->isChecked();

//    if(dialog){delete dialog; dialog = 0;}

//    return true;
//}
