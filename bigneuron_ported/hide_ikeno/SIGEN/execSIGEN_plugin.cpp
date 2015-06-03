/* execSIGEN_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-6-2 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

//

#include <QtGui>
#include <stdio.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>

//

#include <QtGui>
#include <cmath>
#include <stdlib.h>
#include "../plugin_loader/v3d_plugin_loader.h"
#include <boost/lexical_cast.hpp>

//

//

#include "execSIGEN_plugin.h"
Q_EXPORT_PLUGIN2(execSIGEN, SIGEN);

using namespace std;

QString getAppPath();


struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
void reconstruction_func2(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

QStringList SIGEN::menulist() const
{
	return QStringList() 
        <<tr("tracing_menu")
        <<tr("extract")
		<<tr("about");
}

QStringList SIGEN::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
        <<tr("extract_func")
		<<tr("help");
}

void SIGEN::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{

        bool bmenu = true;
        input_PARA PARA;

        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (menu_name == tr("extract"))
    {

        bool bmenu = true;
        input_PARA PARA;

        reconstruction_func2(callback,parent,PARA,bmenu);
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by Hidetoshi Ikeno, 2015-6-2"));
	}
}

bool SIGEN::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
   if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("extract_func"))
    {
       bool bmenu = false;
       input_PARA PARA;

       vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
       vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
       vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
       vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

       if(infiles.empty())
       {
           fprintf (stderr, "Need input image. \n");
           return false;
       }
       else
           PARA.inimg_file = infiles[0];
       int k=0;
       PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
       reconstruction_func2(callback,parent,PARA,bmenu);

    }
    else if (func_name == tr("help"))
        {

	  ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN                    
	  printf("**** Usage of execSIGEN tracing **** \n");
	  printf("vaa3d -x execSIGEN -f tracing_func -i <inimg_file> -p <channel> <other pa\
rameters>\n");
	  printf("inimg_file       The input image\n");
	  printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

	  printf("outswc_file      Will be named automatically based on the input image file name, \
so you don't have to specify it.\n\n");
	}
     else 
        return false;

	return true;
}

int fexist(const char *filename)
{
    FILE  *fp;

    if ((fp = fopen(filename, "r")) == NULL)
        return (0);
    fclose(fp);
    return (1);
}


void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE


    //    v3d_msg(QString("Image file: %1").arg(PARA.inimg_file),bmenu);
    
    //
    // GUI setting
    bool ok1, ok2, ok3, ok4, ok5;
    unsigned int th=20, dt=10, vt=1, ns=0, nc=0;

    th = QInputDialog::getInteger(parent, "Binarization Threshold ",
                                  "Enter Threhold for binarization (0-255):",
                                  30, 0, 255, 1, &ok1);
    if (!ok1) return;

    dt = QInputDialog::getInteger(parent, "Distance Threshold ",
                                  "Enter Threhold for distance:",
                                  10, 0, 100, 1, &ok2);
    if (!ok2) return;

    vt = QInputDialog::getInteger(parent, "Volume Threshold ",
                                  "Enter Threhold for fragment volume:",
                                  1, 0, 100, 1, &ok3);
    if (!ok3) return;

    ns = QInputDialog::getInteger(parent, "Smoothing Level",
                                  "Enter Smoothing level:",
                                  5, 0, 50, 1, &ok4);
    if (!ok4) return;

    nc = QInputDialog::getInteger(parent, "Clipping Level",
                                  "Enter Clipping level:",
                                  0, 0, 50, 1, &ok5);
    if (!ok5) return;

    /*
    bool ok6;
    QInputDialog* inputDialog = new QInputDialog();
    inputDialog->setOptions(QInputDialog::NoButtons);

    QString pathFiji =  QInputDialog::getText(NULL ,"QInputDialog::getText() Example",
                                          "Path to Fiji:", QLineEdit::Normal,
                                          QDir::home().dirName(), &ok6);

    if (!ok6) return;
    */

    // using /var/tmp area for temporary saving of image files

    system("mkdir /var/tmp/vaa3D-SIGEN");

    // using Fiji in /Applications/Fiji.app/
    // in the case of different directory, we have to change line 261

    ofstream macro_file;

    macro_file.open("/var/tmp/fileOut.ijm");

    QString fiji_com = "open(\""+PARA.inimg_file+"\");\n";
    macro_file << fiji_com.toStdString();

    fiji_com = "run(\"Image Sequence... \", \"format=BMP name=in start=0 digits=4 save=/var/tmp/vaa3D-SIGEN/\");\n";
    macro_file << fiji_com.toStdString();

    fiji_com = "run(\"Quit\")";
    macro_file << fiji_com.toStdString();

    macro_file.close();
    
    // using Fiji
    // Fiji have to be loacted in /Applications
    
    system("/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx -macro /var/tmp/fileOut.ijm");

    // executing MorphExtractorCL.exe on mono
    QString strTh, strDt, strVt, strNs, strNc;
    QString command = "mono " + getAppPath() + "/../../vaa3d_tools/bigneuron_ported/hide_ikeno/SIGEN/MorphExtractorCL.exe -i /var/tmp/vaa3D-SIGEN -o out -b "+strTh.setNum(th)+" -t 1 -v "+strVt.setNum(vt)+" -d "+strDt.setNum(dt)+" -s "+strNs.setNum(ns)+" -a "+strNc.setNum(nc);
    system(command.toStdString().c_str());

    //    system("mono ./MorphExtractorCL.exe -i ./tmp -o out -b 128 -t -v 2 -d 10 -s 10 -a 10");
    system("rm /var/tmp/vaa3D-SIGEN/*");
    system("rmdir /var/tmp/vaa3D-SIGEN");

    //Output


    NeuronTree nt;
    QString swc_name = PARA.inimg_file + "_SIGEN.swc";
    nt.name = "execSIGEN";
    //    writeSWC_file(swc_name.toStdString().c_str(),nt);

    if (fexist("out.swc")) {
        command = "cp out.swc "+swc_name;
            system(command.toStdString().c_str());

        system("rm out.swc");
    } else {
        v3d_msg(QString("Install Mono at http://www.mono-project.com/ & copy MorphExtractorCL.exe in your vaa3D directory"));
    }

    ofstream log_file;
    QString log_name = PARA.inimg_file+"_SIGEN.txt";
    log_file.open(log_name.toStdString().c_str());

    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    log_file << "Date_Time: ";
    log_file << pnow->tm_year + 1900 << "/" << pnow->tm_mon + 1 << "/" << pnow->tm_mday <<
                " " << pnow->tm_hour << ":" << pnow->tm_min << ":" << pnow->tm_sec << "\n";

    log_file << "Input: ";
    log_file << PARA.inimg_file.toStdString().c_str() << "\n";
    log_file << "Output: ";
    log_file << swc_name.toStdString().c_str();
    log_file << "\n";
    log_file << "Binarization_threshold: ";
    log_file << strTh.setNum(th).toStdString().c_str();
    log_file << "\n";
    log_file << "Volume_threshold: ";
    log_file << strTh.setNum(vt).toStdString().c_str();
    log_file << "\n";
    log_file << "Distance_threshold: ";
    log_file << strTh.setNum(dt).toStdString().c_str();
    log_file << "\n";
    log_file << "Smoothing_level: ";
    log_file << strTh.setNum(ns).toStdString().c_str();
    log_file << "\n";
    log_file << "Clipping_level: ";
    log_file << strTh.setNum(nc).toStdString().c_str();

    log_file.close();

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}


void reconstruction_func2(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE

    // v3d_msg(QString("Image file: %1").arg(PARA.inimg_file),bmenu);

    QDialog dialog;
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("SIGEN segmentation conditions"));

    // Add the lineEdits with their respective labels
    QList<QLineEdit *> fields;

    QLineEdit *lineEdit1 = new QLineEdit(&dialog);
    lineEdit1->setText("20");
    QString label1 = QString("Binarization Th").arg(1);
    form.addRow(label1, lineEdit1);
    fields << lineEdit1;

    QLineEdit *lineEdit2 = new QLineEdit(&dialog);
    lineEdit2->setText("10");
    QString label2 = QString("Distance Th").arg(2);
    form.addRow(label2, lineEdit2);
    fields << lineEdit2;

    QLineEdit *lineEdit3 = new QLineEdit(&dialog);
    lineEdit3->setText("1");
    QString label3 = QString("Volume Th").arg(3);
    form.addRow(label3, lineEdit3);
    fields << lineEdit3;

    QLineEdit *lineEdit4 = new QLineEdit(&dialog);
    lineEdit4->setText("0");
    QString label4 = QString("Smoothing Level").arg(4);
    form.addRow(label4, lineEdit4);
    fields << lineEdit4;

    QLineEdit *lineEdit5 = new QLineEdit(&dialog);
    lineEdit5->setText("0");
    QString label5 = QString("Clipping Level").arg(5);
    form.addRow(label5, lineEdit5);
    fields << lineEdit5;
/*
    QLineEdit *lineEdit6 = new QLineEdit(&dialog);
    lineEdit6->setText("/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx");
    QString label6 = QString("Clipping Level").arg(6);
    form.addRow(label6, lineEdit6);
    fields << lineEdit6;
*/
    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields
        foreach(QLineEdit * lineEdit1, fields) {
            qDebug() << lineEdit1->text();
        }
    } else return;

    int th=20, dt=10, vt=1, ns=0, nc=0;

    th = atoi(lineEdit1->text().toStdString().c_str());
    dt = atoi(lineEdit2->text().toStdString().c_str());
    vt = atoi(lineEdit3->text().toStdString().c_str());
    ns = atoi(lineEdit4->text().toStdString().c_str());
    nc = atoi(lineEdit5->text().toStdString().c_str());

    // using /var/tmp area for temporary saving of image files

    system("mkdir /var/tmp/vaa3D-SIGEN");

    // using Fiji in /Applications/Fiji.app/
    // in the case of different directory, we have to change line 261

    ofstream macro_file;

//    macro_file.open("/Applications/Fiji.app/macros/fileOut.ijm");
    macro_file.open("/var/tmp/fileOut.ijm");

    QString fiji_com = "open(\""+PARA.inimg_file+"\");\n";
    macro_file << fiji_com.toStdString();

    fiji_com = "run(\"Image Sequence... \", \"format=BMP name=in start=0 digits=4 save=/var/tmp/vaa3D-SIGEN/\");\n";
    macro_file << fiji_com.toStdString();

    fiji_com = "run(\"Quit\")";
    macro_file << fiji_com.toStdString();

    macro_file.close();

    // using Fiji
    // Fiji have to be loacted in /Applications

    std:string fiji_command;
  //  fiji_command = lineEdit6->text().toStdString().c_str();

    fiji_command = "/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx /var/tmp/fileOut.ijm";
//    system("/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx /var/tmp/fileOut.ijm");
    system(fiji_command.c_str());
    system("/var/tmp/fileOut.ijm");
    system("rm /var/tmp/fileOut.ijm");

    // executing MorphExtractorCL.exe on mono
    QString strTh, strDt, strVt, strNs, strNc;
    QString command = "mono ./MorphExtractorCL.exe -i /var/tmp/vaa3D-SIGEN -o out -b "+strTh.setNum(th)+" -t 1 -v "+strVt.setNum(vt)+" -d "+strDt.setNum(dt)+" -s "+strNs.setNum(ns)+" -a "+strNc.setNum(nc);
    system(command.toStdString().c_str());

    //    system("mono ./MorphExtractorCL.exe -i ./tmp -o out -b 128 -t -v 2 -d 10 -s 10 -a 10");
    system("rm /var/tmp/vaa3D-SIGEN/*");
    system("rmdir /var/tmp/vaa3D-SIGEN");

    //Output


    NeuronTree nt;
    QString swc_name = PARA.inimg_file + "_SIGEN.swc";
    nt.name = "execSIGEN";
    //    writeSWC_file(swc_name.toStdString().c_str(),nt);

    if (fexist("out.swc")) {
        command = "cp out.swc "+swc_name;
            system(command.toStdString().c_str());

        system("rm out.swc");
    } else {
        v3d_msg(QString("Install Mono at http://www.mono-project.com/ & copy MorphExtractorCL.exe in your vaa3D directory"));
    }

    ofstream log_file;
    QString log_name = PARA.inimg_file+"_SIGEN.txt";
    log_file.open(log_name.toStdString().c_str());

    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    log_file << "Date_Time: ";
    log_file << pnow->tm_year + 1900 << "/" << pnow->tm_mon + 1 << "/" << pnow->tm_mday <<
                " " << pnow->tm_hour << ":" << pnow->tm_min << ":" << pnow->tm_sec << "\n";

    log_file << "Input: ";
    log_file << PARA.inimg_file.toStdString().c_str() << "\n";
    log_file << "Output: ";
    log_file << swc_name.toStdString().c_str();
    log_file << "\n";
    log_file << "Binarization_threshold: ";
    log_file << strTh.setNum(th).toStdString().c_str();
    log_file << "\n";
    log_file << "Volume_threshold: ";
    log_file << strTh.setNum(vt).toStdString().c_str();
    log_file << "\n";
    log_file << "Distance_threshold: ";
    log_file << strTh.setNum(dt).toStdString().c_str();
    log_file << "\n";
    log_file << "Smoothing_level: ";
    log_file << strTh.setNum(ns).toStdString().c_str();
    log_file << "\n";
    log_file << "Clipping_level: ";
    log_file << strTh.setNum(nc).toStdString().c_str();

    log_file.close();

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}

QString getAppPath()
{
    QString v3dAppPath("~/Work/v3d_external/v3d");
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    v3dAppPath = testPluginsDir.absolutePath();
    return v3dAppPath;
}
