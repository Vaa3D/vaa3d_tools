
/* xjy_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2022-3-25 : by Lijun Wang
 */
 
#include "v3d_message.h"
#include <vector>
#include "xjy_plugin.h"
#include "..//..//..//..//v3d_external//v3d_main//basic_c_fun//basic_surf_objs.cpp"
#include <iostream>
#include "fstream"
#include "string"
#include <io.h>
#include <QFileDialog>
#include <Python.h>

using namespace std;
//Q_EXPORT_PLUGIN2(xjy, xjy_refine);
 
QStringList xjy_refine::menulist() const
{
	return QStringList() 
        <<tr("refine")
        <<tr("refine without stitching")
		<<tr("about");
}

QStringList xjy_refine::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void xjy_refine::splitString(const string& org_string,QVector<string>& strings, const string& seq) {
    string::size_type p1 = 0;
    string::size_type p2 = org_string.find(seq);

    while (p2 != string::npos) {
        if (p2 == p1) {
            ++p1;
            p2 = org_string.find(seq, p1);
            continue;
        }
        strings.push_back(org_string.substr(p1, p2 - p1));
        p1 = p2 + seq.size();
        p2 = org_string.find(seq, p1);
    }

    if (p1 != org_string.size()) {
        strings.push_back(org_string.substr(p1));
    }
}

void xjy_refine::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("refine"))
	{
        /**********convert tiff to 8-bit v3draw*************/
        QString currentPath;
        QDir dir;
        currentPath=dir.currentPath();
        //v3d_msg(currentPath);
        system(currentPath.toLatin1()+"//imagej//Fiji.app//ImageJ-win64.exe -macro "+currentPath.toLatin1()+"//imagej//Fiji.app//ConvertTo8bit.ijm");

        /***********split swc***********/
        Py_SetPythonHome(L"C:\\Users\\82700\\anaconda3\\envs\\tensorflow");//需要配置为本地的python环境
        Py_Initialize();
        if ( !Py_IsInitialized() )
        {
        }
        PyObject* pModule = PyImport_ImportModule("split_swc");  // 这里的test_py就是创建的python文件

        if (!pModule) {
                cout<< "Cant open python file!\n" << endl;
            }
       PyObject* pFunsplitswc= PyObject_GetAttrString(pModule,"split_swc");  // 这里的hellow就是python文件定义的函数
        if(!pFunsplitswc){
            cout<<"Get function split_swc failed"<<endl;
        }
        /*创建一个python类型的变量，这里均可以创建成tuple数据类型*/
        PyObject* pArgs = PyTuple_New(3);
        /*200是开辟的字符串是路径长度，若字符串较长需将其变大*/
        QString datapath = QFileDialog::getExistingDirectory(NULL,"select data folder",".");//datapath是v3draw格式数据存放的地址
        PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", datapath.toStdString().c_str()));
        QString swcpath = QFileDialog::getOpenFileName(NULL, "select the swc to split","*.swc");//swcpath是完整的需要拆分的swc存放的地址
        PyTuple_SetItem(pArgs, 1, Py_BuildValue("s", swcpath.toStdString().c_str()));
        QString savepath = QFileDialog::getExistingDirectory(NULL,"select a folder to save the splited swc",".");//savepath是拆分后的swc存放的地址
        PyTuple_SetItem(pArgs, 2, Py_BuildValue("s", savepath.toStdString().c_str()));
        PyObject* pValue = PyObject_CallObject(pFunsplitswc, pArgs);
        cout<<pValue;
        Py_Finalize();


        /******swc2marker*************/
        ofstream outfile_s2m;
        outfile_s2m.open(currentPath.toLatin1()+"//dir_s2m.txt",ios::app);//需要转成marker文件的swc文件目录
        long Handle_s2m;
        struct _finddata_t FileInfo_s2m;
        //QString file_name_s2m = QFileDialog::getExistingDirectory(NULL,"select swc folder",".");
        string path_s2m = savepath.toStdString()+"//*.swc";
        //std::string str(path);
        if ((Handle_s2m = _findfirst(path_s2m.c_str(), &FileInfo_s2m)) == -1L)
            printf("没有找到匹配的项目\n");
        else
        {
            outfile_s2m<<FileInfo_s2m.name<<endl;
            while (_findnext(Handle_s2m, &FileInfo_s2m) == 0)
                outfile_s2m<<FileInfo_s2m.name<<endl;
            _findclose(Handle_s2m);
        }
        outfile_s2m.close();

        ifstream infile_s2m;
        infile_s2m.open(currentPath.toLatin1()+"//dir_s2m.txt",ios::in);
        string line_s2m;
        QDir *marker = new QDir;
        marker->mkdir(currentPath+"//MARKER");
        while(getline(infile_s2m,line_s2m))
        {
            QVector<string> s2m;
            splitString(line_s2m,s2m,".");
            QString inswc_file = savepath.toLatin1()+"//"+QString::fromStdString(line_s2m);
            QString  outmarker_file = currentPath+"//MARKER//"+QString::fromStdString(s2m[0])+".swc.marker";
            NeuronTree nt=readSWC_file(inswc_file);
            QList <ImageMarker> listLandmarks;
            if(!nt.listNeuron.size())
            {

            }


            double scale=1;

            for(int i=0; i<nt.listNeuron.size();i++)
            {
                ImageMarker m;
                m.x=nt.listNeuron[i].x*scale;
                m.y=nt.listNeuron[i].y*scale;
                m.z=nt.listNeuron[i].z*scale;
                if(m.x == 0)
                    m.x = 0.5;
                if(m.y == 0)
                    m.y = 0.5;
                if(m.z == 0)
                    m.z = 0.5;
                m.n=nt.listNeuron[i].n;
                m.name="";
                m.type=0;
                m.shape=0;
                m.radius=1;
                m.color.r = 255;
                m.color.g = 0;
                m.color.b = 0;
                listLandmarks.append(m);
            }

            writeMarker_file(outmarker_file,listLandmarks);
        }


        /*************mean_shift***************/
        FILE *filepath;
        ofstream outfile_ms;
        outfile_ms.open(currentPath.toLatin1()+"//dir_ms.txt",ios::app);//创建需要refine的数据目录
        long Handle;
        struct _finddata_t FileInfo;
        string path = datapath.toStdString()+"//*.v3draw";
        if ((Handle = _findfirst(path.c_str(), &FileInfo)) == -1L)
            printf("没有找到匹配的项目\n");
        else
        {
            outfile_ms<<FileInfo.name<<endl;
            while (_findnext(Handle, &FileInfo) == 0)
                outfile_ms<<FileInfo.name<<endl;
            _findclose(Handle);
        }
        outfile_ms.close();

        ifstream infile_ms;
        infile_ms.open(currentPath.toLatin1()+"//dir_ms.txt",ios::in);
        string line_ms;
        QString qt = currentPath.toLatin1()+"//v3d_qt6.exe";
        QString plugin = currentPath.toLatin1()+"//plugins//image_analysis//mean_shift_center_finder//mean_shift_center_finder.dll";
        QString para = "mean_shift";
        QDir *meanshift_marker = new QDir;
        meanshift_marker->mkdir(currentPath+"//Meanshift_MARKER");
        while(getline(infile_ms,line_ms))
        {
            QVector<string> a_ms;
            splitString(line_ms,a_ms,".");
            QString in_data = datapath+ "//"+QString::fromStdString(line_ms);
            QString in_marker = currentPath+"//MARKER//"+ QString::fromStdString(a_ms[0]) +".swc.marker";
            QFileInfo fi(in_marker);
            if(!fi.isFile())//如果不存在与数据对应的marker文件则跳过这个数据
                continue;
            QString out = currentPath+"//Meanshift_MARKER//" + QString::fromStdString(a_ms[0]) + "_mid.marker";

            //选择参数
            int r = 15;//default

            //当只有两三个点时，让带宽参数最小
            ifstream cho;
            cho.open(in_marker.toLatin1(),ios::in);
            string c;
            int cnt_len = 0;
            while(getline(cho,c))
            {
                cnt_len++;
            }
            if(cnt_len<=3)
                r = 2;

            QString cmd = qt + " /x " + plugin + " /f " + para + " /i "+in_data+" "+in_marker+" /p "+ QString::number(r)+" /o "+ out;
            //cout<<cmd;
            system(cmd.toLatin1());

            //处理一下meanshift后的marker文件，把文件中原始的点去掉。
            ifstream process;
            process.open(currentPath.toLatin1()+"//Meanshift_MARKER//" + a_ms[0].c_str() + "_mid.marker",ios::in);
            ofstream processed;
            processed.open(currentPath.toLatin1()+"//Meanshift_MARKER//" + a_ms[0].c_str() + "_meanshift.marker",ios::app);
            string pro;
            int cnt = 0;
            while(getline(process,pro))
            {
                cnt++;
                if(cnt>3)
                {
                    QVector<string> p;
                    splitString(pro,p,",");
                    if(p[5] == "ms")
                    {
                        processed<<pro<<endl;
                    }
                }
            }
            process.close();
            processed.close();
            QFile fileTemp_pro(currentPath.toLatin1()+"//Meanshift_MARKER//" + a_ms[0].c_str() + "_mid.marker");
            fileTemp_pro.remove();
            QFile fileTemp_ori(currentPath.toLatin1()+"//MARKER//"+ a_ms[0].c_str() +".swc.marker");
            fileTemp_ori.remove();
        }
        infile_ms.close();
        QFile fileTemp_ms(currentPath.toLatin1()+"//dir_ms.txt");//删除v3draw数据目录
        fileTemp_ms.remove();
        QDir *remove_dir = new QDir;
        remove_dir->setPath(currentPath+"//MARKER");//删除MARKER文件夹
        remove_dir->removeRecursively();



        /*****************marker2swc********************/
        ifstream infile1;
        ifstream infile2;
        ofstream outfile;
        ifstream infile_m2s;
        infile_m2s.open(currentPath.toLatin1()+"//dir_s2m.txt",ios::in);
        string line_m2s;
        while(getline(infile_m2s,line_m2s))
        {
            QVector<string> m2s;
            splitString(line_m2s,m2s,".");
            infile1.open(savepath.toLatin1()+"//"+line_m2s.c_str(),ios::in);//打开原来的swc文件
            infile2.open(currentPath.toLatin1()+"//Meanshift_MARKER//" + m2s[0].c_str() + "_meanshift.marker",ios::in);//打开meanshift后的marker文件
            if(!infile1.is_open())
                cout<<"open file failure"<<endl;
            QDir *msswc = new QDir;
            msswc->mkdir(currentPath+"//Meanshift_swc");
             outfile.open(currentPath.toLatin1()+"//Meanshift_swc//"+m2s[0].c_str()+"_meanshift.swc",ios::app);//创建转换后的swc文件
             string line1,line2,line3;
             while(getline(infile1,line1)&&getline(infile2,line2))
             {
                QVector<string> sv1;
                QVector<string> sv2;
                splitString(line1,sv1," ");
                splitString(line2,sv2,",");
                line3 = sv1[0]+" "+sv1[1]+" "+sv2[0]+" "+sv2[1]+" "+sv2[2]+" "+sv1[5]+" "+sv1[6];
                outfile<<line3;
                outfile<<endl;
             }
             infile1.close();
             infile2.close();
             outfile.close();

        }
        QFile fileTemp_m2s(currentPath.toLatin1()+"//dir_s2m.txt");//删除v3draw数据目录
        fileTemp_m2s.remove();
        QDir *remove_MSMARKER = new QDir;
        remove_MSMARKER->setPath(currentPath+"//Meanshift_MARKER");//删除Meanshift_MARKER文件夹
        remove_MSMARKER->removeRecursively();


         /****************stitch swc*******************/
         FILE *filepath_cb;
         ofstream outfile_cb;
         outfile_cb.open(currentPath.toLatin1()+"//dir_cbswc.txt",ios::app);
         long Handle_cb;
         struct _finddata_t FileInfo_cb;
         if ((Handle_cb = _findfirst(currentPath.toLatin1()+"//Meanshift_swc//*.swc", &FileInfo_cb)) == -1L)
             printf("没有找到匹配的项目\n");
         else
         {
             outfile_cb<<FileInfo_cb.name<<endl;
             while (_findnext(Handle_cb, &FileInfo_cb) == 0)
                 outfile_cb<<FileInfo_cb.name<<endl;
             _findclose(Handle_cb);
         }
         outfile_cb.close();

             ifstream infile3,infile4;
             ofstream outfile1;
             infile3.open(currentPath.toLatin1()+"//dir_cbswc.txt",ios::in);
             outfile1.open(currentPath.toLatin1()+"//Meanshift_swc//result.swc",ios::app);
             string line4;
             string line5;
             string str;
             while(getline(infile3,line4))
             {
                 string filenm = currentPath.toStdString()+"//Meanshift_swc//"+line4;
                 infile4.open(filenm,ios::in);
                 QVector<string> a;
                 a.clear();
                 splitString(line4,a,"-");
                 float numx = atof(a[0].c_str());
                 float numy = atof(a[1].c_str());
                 QVector<string> b;
                 splitString(a[2],b,".");
                 float numz = atof(b[0].c_str());
                 while(getline(infile4,line5))//把每个小块内部的相对坐标恢复成完整数据中的坐标
                 {

                     QVector<string> a2;
                     splitString(line5,a2," ");
                     cout<<a2[0]<<endl;
                     float posx = atof(a2[2].c_str());
                     float posy = atof(a2[3].c_str());
                     float posz = atof(a2[4].c_str());
                     string s1 = std::to_string(posx+numx);
                     string s2 = std::to_string(posy+numy);
                     string s3 = std::to_string(posz+numz);
                     str = a2[0]+' '+a2[1]+' '+s1+' '+s2+' '+s3+' '+a2[5]+' '+a2[6];
                     outfile1<<str;
                     outfile1<<endl;

                 }
                 infile4.close();

             }
             infile3.close();
             outfile1.close();
             QFile fileTemp_cb(currentPath.toLatin1()+"//dir_cbswc.txt");//删除v3draw数据目录
             fileTemp_cb.remove();
             v3d_msg("done");
	}
    else if (menu_name == tr("refine without stitching"))
	{
        /**********convert tiff to 8-bit v3draw*************/
        QString currentPath;
        QDir dir;
        currentPath=dir.currentPath();
        QString datapath = QFileDialog::getExistingDirectory(NULL,"select the data folder",".");//datapath是v3draw格式数据存放的地址
        QString savepath = QFileDialog::getExistingDirectory(NULL,"select the swc folder",".");//savepath是拆分后的swc存放的地址

        /******swc2marker*************/
        ofstream outfile_s2m;
        outfile_s2m.open(currentPath.toLatin1()+"//dir_s2m.txt",ios::app);//需要转成marker文件的swc文件目录
        long Handle_s2m;
        struct _finddata_t FileInfo_s2m;
        string path_s2m = savepath.toStdString()+"//*.swc";
        if ((Handle_s2m = _findfirst(path_s2m.c_str(), &FileInfo_s2m)) == -1L)
            printf("没有找到匹配的项目\n");
        else
        {
            outfile_s2m<<FileInfo_s2m.name<<endl;
            while (_findnext(Handle_s2m, &FileInfo_s2m) == 0)
                outfile_s2m<<FileInfo_s2m.name<<endl;
            _findclose(Handle_s2m);
        }
        outfile_s2m.close();

        ifstream infile_s2m;
        infile_s2m.open(currentPath.toLatin1()+"//dir_s2m.txt",ios::in);
        string line_s2m;
        QDir dir_test(currentPath+"//MARKER_NOSTITCH");
        if(dir_test.exists())
            dir_test.removeRecursively();
        QDir *marker = new QDir;
        marker->mkdir(currentPath+"//MARKER_NOSTITCH");
        while(getline(infile_s2m,line_s2m))
        {
            QVector<string> s2m;
            splitString(line_s2m,s2m,".");
            QString inswc_file = savepath.toLatin1()+"//"+QString::fromStdString(line_s2m);
            QString  outmarker_file = currentPath+"//MARKER_NOSTITCH//"+QString::fromStdString(s2m[0])+".swc.marker";
            NeuronTree nt=readSWC_file(inswc_file);
            QList <ImageMarker> listLandmarks;
            if(!nt.listNeuron.size())
            {

            }


            double scale=1;

            for(int i=0; i<nt.listNeuron.size();i++)
            {
                ImageMarker m;
                m.x=nt.listNeuron[i].x*scale;
                m.y=nt.listNeuron[i].y*scale;
                m.z=nt.listNeuron[i].z*scale;
                if(m.x == 0)
                    m.x = 0.5;
                if(m.y == 0)
                    m.y = 0.5;
                if(m.z == 0)
                    m.z = 0.5;
                m.n=nt.listNeuron[i].n;
                m.name="";
                m.type=0;
                m.shape=0;
                m.radius=1;
                m.color.r = 255;
                m.color.g = 0;
                m.color.b = 0;
                listLandmarks.append(m);
            }

            writeMarker_file(outmarker_file,listLandmarks);
        }


        /*************mean_shift***************/
        FILE *filepath;
        ofstream outfile_ms;
        outfile_ms.open(currentPath.toLatin1()+"//dir_ms.txt",ios::app);//创建需要refine的数据目录
        long Handle;
        struct _finddata_t FileInfo;
        string path = datapath.toStdString()+"//*.v3draw";
        if ((Handle = _findfirst(path.c_str(), &FileInfo)) == -1L)
            printf("没有找到匹配的项目\n");
        else
        {
            outfile_ms<<FileInfo.name<<endl;
            while (_findnext(Handle, &FileInfo) == 0)
                outfile_ms<<FileInfo.name<<endl;
            _findclose(Handle);
        }
        outfile_ms.close();

        ifstream infile_ms;
        infile_ms.open(currentPath.toLatin1()+"//dir_ms.txt",ios::in);
        string line_ms;
        QString qt = currentPath.toLatin1()+"//v3d_qt6.exe";
        QString plugin = currentPath.toLatin1()+"//plugins//image_analysis//mean_shift_center_finder//mean_shift_center_finder.dll";
        QString para = "mean_shift";
        QDir *meanshift_marker = new QDir;
        meanshift_marker->mkdir(currentPath+"//Meanshift_MARKER_NOSTITCH");
        while(getline(infile_ms,line_ms))
        {
            QVector<string> a_ms;
            splitString(line_ms,a_ms,".");
            QString in_data = datapath+ "//"+QString::fromStdString(line_ms);
            QString in_marker = currentPath+"//MARKER_NOSTITCH//"+ QString::fromStdString(a_ms[0]) +".swc.marker";
            QFileInfo fi(in_marker);
            if(!fi.isFile())//如果不存在与数据对应的marker文件则跳过这个数据
                continue;
            QString out = currentPath+"//Meanshift_MARKER_NOSTITCH//" + QString::fromStdString(a_ms[0]) + "_mid.marker";

            //选择参数
            int r = 15;//default

            //当只有两三个点时，让带宽参数最小
            ifstream cho;
            cho.open(in_marker.toLatin1(),ios::in);
            string c;
            int cnt_len = 0;
            while(getline(cho,c))
            {
                cnt_len++;
            }
            if(cnt_len<=3)
                r = 2;

            QString cmd = qt + " /x " + plugin + " /f " + para + " /i "+in_data+" "+in_marker+" /p "+ QString::number(r)+" /o "+ out;
            //cout<<cmd;
            system(cmd.toLatin1());

            //处理一下meanshift后的marker文件，把文件中原始的点去掉。
            ifstream process;
            process.open(currentPath.toLatin1()+"//Meanshift_MARKER_NOSTITCH//" + a_ms[0].c_str() + "_mid.marker",ios::in);
            ofstream processed;
            processed.open(currentPath.toLatin1()+"//Meanshift_MARKER_NOSTITCH//" + a_ms[0].c_str() + "_meanshift.marker",ios::app);
            string pro;
            int cnt = 0;
            while(getline(process,pro))
            {
                cnt++;
                if(cnt>3)
                {
                    QVector<string> p;
                    splitString(pro,p,",");
                    if(p[5] == "ms")
                    {
                        processed<<pro<<endl;
                    }
                }
            }
            process.close();
            processed.close();
            QFile fileTemp_pro(currentPath.toLatin1()+"//Meanshift_MARKER_NOSTITCH//" + a_ms[0].c_str() + "_mid.marker");
            fileTemp_pro.remove();
            QFile fileTemp_ori(currentPath.toLatin1()+"//MARKER_NOSTITCH//"+ a_ms[0].c_str() +".swc.marker");
            fileTemp_ori.remove();
        }
        infile_ms.close();
        QFile fileTemp_ms(currentPath.toLatin1()+"//dir_ms.txt");//删除v3draw数据目录
        fileTemp_ms.remove();
        QDir *remove_dir = new QDir;
        remove_dir->setPath(currentPath+"//MARKER_NOSTITCH");//删除MARKER文件夹
        remove_dir->removeRecursively();



        /*****************marker2swc********************/
        ifstream infile1;
        ifstream infile2;
        ofstream outfile;
        ifstream infile_m2s;
        infile_m2s.open(currentPath.toLatin1()+"//dir_s2m.txt",ios::in);
        string line_m2s;
        while(getline(infile_m2s,line_m2s))
        {
            QVector<string> m2s;
            splitString(line_m2s,m2s,".");
            infile1.open(savepath.toLatin1()+"//"+line_m2s.c_str(),ios::in);//打开原来的swc文件
            infile2.open(currentPath.toLatin1()+"//Meanshift_MARKER_NOSTITCH//" + m2s[0].c_str() + "_meanshift.marker",ios::in);//打开meanshift后的marker文件
            if(!infile1.is_open())
                cout<<"open file failure"<<endl;
            QDir *msswc = new QDir;
            msswc->mkdir(currentPath+"//Meanshift_swc_NOSTITCH");
             outfile.open(currentPath.toLatin1()+"//Meanshift_swc_NOSTITCH//"+m2s[0].c_str()+"_meanshift.swc",ios::app);//创建转换后的swc文件
             string line1,line2,line3;
             while(getline(infile1,line1)&&getline(infile2,line2))
             {
                QVector<string> sv1;
                QVector<string> sv2;
                splitString(line1,sv1," ");
                splitString(line2,sv2,",");
                line3 = sv1[0]+" "+sv1[1]+" "+sv2[0]+" "+sv2[1]+" "+sv2[2]+" "+sv1[5]+" "+sv1[6];
                outfile<<line3;
                outfile<<endl;
             }
             infile1.close();
             infile2.close();
             outfile.close();

        }
        QFile fileTemp_m2s(currentPath.toLatin1()+"//dir_s2m.txt");//删除v3draw数据目录
        fileTemp_m2s.remove();
        QDir *remove_MSMARKER = new QDir;
        remove_MSMARKER->setPath(currentPath+"//Meanshift_MARKER_NOSTITCH");//删除Meanshift_MARKER文件夹
        remove_MSMARKER->removeRecursively();
        v3d_msg("done");
	}
	else
	{
        v3d_msg(tr("This is a plugin for Shenzhen Institute of Advanced Technology Chinese Academy of Sciences "
            "Developed by Lijun Wang, 2022"));
	}
}



bool xjy_refine::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

