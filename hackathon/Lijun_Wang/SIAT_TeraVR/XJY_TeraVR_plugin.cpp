/* XJY_TeraVR_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2022-6-5 : by YourName
 */

#include "PMain.h"
#include <v3d_interface.h>
#include "v3d_message.h"
#include <vector>
#include "XJY_TeraVR_plugin.h"
#include <QWidget>
#include "windows.h"
#include "CConverter.h"
#include "PConverter.h"
#include <QFileDialog>

#include <iostream>
#include "CConverter.cpp"
using namespace std;
//Q_EXPORT_PLUGIN2(XJY_TeraVR, XJY_TeraVR);
 
QStringList XJY_TeraVR::menulist() const
{
	return QStringList() 
        <<tr("TeraConverter")
        <<tr("TeraVR")
		<<tr("about");
}

QStringList XJY_TeraVR::funclist() const
{
	return QStringList()
        <<tr("TeraConverter")
        <<tr("TeraVR")
		<<tr("help");
}



void XJY_TeraVR::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("TeraConverter"))
	{
       QString datapath = QFileDialog::getOpenFileName(NULL, "选择需要转换的数据","*.tif");
       string inVolPath = datapath.toStdString();
       cout<<inVolPath<<endl;
       string inVolFormat = "TIFF (3D)";
       bool inFileMode = true;
       bool outFileMode = true;
       bool time_series = false;
       bool conversionMode = false;
       int resolutionsSize = 3;
       int stackWidth = 256;
       int stackHeight = 256;
       int stackDepth = 256;
       int downsamplingMethod = 1;
       QString outpath = QFileDialog::getExistingDirectory(NULL,"选择输出地址",".");
       string outVolPath = outpath.toStdString();
       string outVolFormat = "Vaa3D raw (tiled, 3D)";
       terafly::CConverter *conv = terafly::CConverter::instance();//调用CConverter类的单例
       conv->setMembers(inVolPath,inVolFormat,inFileMode,outFileMode,time_series,conversionMode, resolutionsSize, stackWidth, stackHeight,stackDepth,downsamplingMethod, outVolPath, outVolFormat);//这一步如何重构
      // conv->start();

	}
    else if (menu_name == tr("TeraVR"))
	{
           QString currentPath;
           QDir dir;
           currentPath=dir.currentPath();
           terafly::PMain *pm = terafly::PMain::instance(&callback, parent);
           pm->show();
           QString path = QFileDialog::getExistingDirectory(NULL,"select data folder",".");
           QString qpath = QDir::fromNativeSeparators(path);//将"\"转换为"/"
           int index = qpath.lastIndexOf("/");
           QString deletePath = qpath.left(index);//回到上一层目录删除vmap.bin文件
           QFile fileTemp(deletePath +"//vmap.bin");//删除vmap.bin文件
           if(fileTemp.exists())
           {
               fileTemp.remove();
           }
           QFile fileTemp1(currentPath +"//vmap.bin");//删除vmap.bin文件
           if(fileTemp1.exists())
           {
               fileTemp1.remove();
           }
           pm->openImage(1,qpath.toStdString());
           QString anoPath = QFileDialog::getOpenFileName(NULL, "select Annotations","*.ano");
           pm->loadAnnotations(anoPath.toStdString());


	}
	else
	{

		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2022-6-5"));
	}
}

bool XJY_TeraVR::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("TeraConverter"))
	{
        //QString datapath = QFileDialog::getOpenFileName(NULL, "选择需要转换的数据","*.tif");
        string inVolPath = "D:/xianjinyuan/new_data/R2_N2_2_blocks/2304-4864-8832.tif";
        inVolPath = infiles[0];
        cout<<inVolPath<<endl;
        string inVolFormat = "TIFF (3D)";
        bool inFileMode = true;
        bool outFileMode = true;
        bool time_series = false;
        bool conversionMode = false;
        int resolutionsSize = 2;
        resolutionsSize = atoi(infiles[1]);
        int stackWidth = 256;
        int stackHeight = 256;
        int stackDepth = 256;
        int downsamplingMethod = 1;
        string outVolPath = "C:/Users/82700/Desktop/tttttt";
        outVolPath = outfiles[0];
        cout<<outVolPath<<endl;
        string outVolFormat = "Vaa3D raw (tiled, 3D)";
        terafly::CConverter *conv = terafly::CConverter::instance();//调用CConverter类的单例
        conv->setMembers(inVolPath,inVolFormat,inFileMode,outFileMode,time_series,conversionMode, resolutionsSize, stackWidth, stackHeight,stackDepth,downsamplingMethod, outVolPath, outVolFormat);//这一步如何重构
        conv->start();
        //int n = 100000;
        while(!conv->isend)//保持主程序运行等待多线程程序结束
        {
            QCoreApplication::processEvents();
        }
	}
    else if (func_name == tr("TeraVR"))
	{
        terafly::PMain *pm = terafly::PMain::instance(&callback, parent);
        pm->show();
        string path = infiles[0];
        QString qpath = QDir::fromNativeSeparators(QString::fromStdString(path));//把windows的"\"转换为"/"
        int index = qpath.lastIndexOf("/");
        QString deletePath = qpath.left(index);//回到上一层目录删除vmap.bin文件
        QFile fileTemp(deletePath +"//vmap.bin");//删除vmap.bin文件
        if(fileTemp.exists())
        {
            fileTemp.remove();
        }
        pm->openImage(1,qpath.toStdString());
        bool loadano = false;
        string anoPath = "";
        while(!pm->isend)//保持主程序运行等待多线程程序结束
        {
            QCoreApplication::processEvents();
            if(CViewer::getCurrent()&&!loadano)
            {
                if(infiles.size()>1)
                {
                    if(QString::fromStdString(infiles[1]).contains(".swc")&&outfiles.size()>0)//如果输入的是swc文件则需要转换成eswc并生成相应的ano文件
                    {
                        /*将swc转成eswc并生成需要的ano文件*/
                        string swcpath = infiles[1];
                        string anopath = outfiles[0];//一个文件夹地址，最好是一个空文件夹
                        QString qswcpath = QDir::fromNativeSeparators(QString::fromStdString(swcpath));
                        int s_index = qswcpath.lastIndexOf ("/"); //从后面查找"/"位置
                        QString swc_title = qswcpath.right (qswcpath.length ()-s_index-1); //从右边截取
                        QString eswc_title=swc_title.section(".", 0, 0);
                        string eswcpath = anopath + "//" + eswc_title.toStdString() + ".eswc";
                        string ano_path = eswc_title.toStdString() + ".ano";
                        swc_eswc(swcpath,eswcpath,0);
                        eswc2ano(anopath,ano_path);
                        anoPath = anopath + "//" + ano_path;
                    }//v3d /x XJY_TeraVR.dll /f TeraVR /i .terafly .swc /o .//ano//
                    else if(QString::fromStdString(infiles[1]).contains(".swc")&&outfiles.size()<=0)
                    {
                        v3d_msg("Missing output parameter");
                        break;
                    }
                    else//如果输入的是ano文件则直接获取输入参数
                    {
                        anoPath = infiles[1];
                    }//v3d /x XJY_TeraVR.dll /f TeraVR /i .terafly .ano
                    QString qanoPath = QDir::fromNativeSeparators(QString::fromStdString(anoPath));
                    pm->loadAnnotations(qanoPath.toStdString());
                    qDebug()<<"ano is loaded";
                    loadano = true;
                }
            }
        }
        //pm->saveAnnotations();//保存swc
        if(pm->isend)//isend在PMain的closeEvent()函数中赋值为正值
        {
           QCoreApplication::quit();//结束线程
        }
	}
    else if (func_name == tr("swc2ano"))
    {
        string swcpath = infiles[0];
        string anopath = outfiles[0];//一个文件夹地址，最好是一个空文件夹
        QString qswcpath = QDir::fromNativeSeparators(QString::fromStdString(swcpath));
        int index = qswcpath.lastIndexOf ("/"); //从后面查找"/"位置
        QString swc_title = qswcpath.right (qswcpath.length ()-index-1); //从右边截取
        QString eswc_title=swc_title.section(".", 0, 0);
        string eswcpath = anopath + "//" + eswc_title.toStdString() + ".eswc";
        string ano_path = eswc_title.toStdString() + ".ano";
        swc_eswc(swcpath,eswcpath,0);
        eswc2ano(anopath,ano_path);

    }
    else if(func_name == tr("ano2swc"))
    {
        string anopath = infiles[0];
        //string swcpath = outfiles[0];
        QString qanopath = QDir::fromNativeSeparators(QString::fromStdString(anopath));
        QString eswc_path = qanopath + ".eswc";
        string swc_path = anopath + ".swc";
        swc_eswc(swc_path,eswc_path.toStdString(),1);

    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void XJY_TeraVR::swc_eswc(string swcpath, string eswcpath, int model)
{
    QString currentPath;
    QDir dir;
    currentPath=dir.currentPath();
    qDebug()<<currentPath;
    QString qt = currentPath.toLatin1()+"//v3d_external//bin//v3d_qt6.exe";
    QString plugin = currentPath.toLatin1()+"//v3d_external//bin//plugins//neuron_utilities//Enhanced_SWC_Format_Converter//eswc_converter.dll";
    QString para = "";
    QString in = "";
    QString out = "";
    if(model == 0)//swc to eswc
    {
        para = "swc_to_eswc";
        in = QString::fromStdString(swcpath);
        out =QString::fromStdString(eswcpath);
    }
    else if(model == 1)
    {
        para = "eswc_to_swc";
        in = QString::fromStdString(eswcpath);
        out =QString::fromStdString(swcpath);
    }
    QString cmd = qt + " /x " + plugin + " /f " + para + " /i "+in+" /o "+ out;
    system(cmd.toLatin1());
}

void XJY_TeraVR::eswc2ano(string eswc_folder_path, string anofile)
{
    QString currentPath;
    QDir dir;
    currentPath=dir.currentPath();
    qDebug()<<currentPath;
    QString qt = currentPath.toLatin1()+"//v3d_external//bin//v3d_qt6.exe";
    QString plugin = currentPath.toLatin1()+"//v3d_external//bin//plugins//linker_file//Linker_File_Generator//linker_file_generator.dll";
    QString para = "linker";
    QString in = QString::fromStdString(eswc_folder_path);
    string ano_path = eswc_folder_path + "//" + anofile;
    QString out =QString::fromStdString(ano_path);

    QString cmd = qt + " /x " + plugin + " /f " + para + " /i "+in+" /o "+ out+" /p 1";
    system(cmd.toLatin1());

}


