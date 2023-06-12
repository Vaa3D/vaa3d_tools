/* SuperPlugin_plugin.cpp
 * 
 * 2023-3-7 : by Csz
 */
 
#include "v3d_message.h"
#include <vector>
#include "SuperPlugin_plugin.h"
//#include "TaskManage.h"
#include "PluginPipeline.h"
#include "WorkShop.h"
#include "GlobalConfig.h"
#include <QDateTime>

using namespace std;
 
QString PluginRunStartTime;

QStringList SuperPlugin::menulist() const
{
	return QStringList() 
		<<tr("start")
		<<tr("about");
}

QStringList SuperPlugin::funclist() const
{
	return QStringList()
		<<tr("start")
		<<tr("help");
}

void SuperPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("start"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr(". "
			"Developed by Csz, 2023-3-7"));
	}
}

bool SuperPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("start"))
	{

        QDateTime currentDateTime = QDateTime::currentDateTime();
        PluginRunStartTime = currentDateTime.toString("yyyy_MM_dd_hh_mm_ss");

        if(infiles.size()<1)
            return false;
        char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
        QString name=QString(inimg_file);

        QString ConfigPath;
        if(infiles.size()>1){
            ConfigPath=QString(infiles.at(1));

        }else{
            ConfigPath=QCoreApplication::applicationDirPath()+"/spconfig.cfg";
        }
        qDebug()<<ConfigPath;
        ReadConfigFile(ConfigPath);

        qDebug() << "Thread started, thread id1: " << QThread::currentThreadId();
        TaskManage::get_instance()->initThreadPool(ThreadPoolSize);
        TaskManage::get_instance()->starttimer();

        PluginPipeLine pp(callback);
        pp.getstartpoint(name,UnetIP,UnetPort);

        QEventLoop loop;

            // 连接请求完成的信号和事件循环的退出槽
        connect(&pp, SIGNAL(finished()), &loop, SLOT(quit()));

            // 等待请求完成
        loop.exec();

        vector<int> startpoint=pp.getCentroid();
        qDebug()<<startpoint[0]<<startpoint[1]<<startpoint[2];
//        QString name="D:/A_test/test1/140921c16.tif.v3dpbd.tiff.v3draw";
//        QString name="D:/A_test/test1/220217_F_060_0_1_01_05_RSGb_1298_gyc.v3draw";

        WorkShop ws(callback);
        ws.AssignImage(name);
        ws.newPipeline(startpoint[0]-64,startpoint[1]-64,startpoint[2]-64,startpoint[0]+64,startpoint[1]+64,startpoint[2]+64);

//        ws.newPipeline(234,196,8,362,324,136);
//        pp.CropRawImage(name,pp.getBoundingBox(0,0,0,128,128,128));
//        PluginPipeLine pp2(callback);
//        QString name2="D:/A_test/test1/220217_F_060_0_1_01_05_RSGb_1298_gyc.v3draw";
//        pp.CropRawImage(name2,pp2.getBoundingBox(10,10,10,138,138,138));
//        CallPlugin cp(callback,"vn2","help");

//        TaskManage::get_instance()->starttimer();

//        int n=0;
        while(1){
//            if(n<10){
//            TaskManage::get_instance()->addTask(cp);

//            n++;
//            }
            TaskManage::get_instance()->printStatus();
//            pp.CropRawImage(name,pp.getBoundingBox(0,0,0,128,128,128));
            QCoreApplication::processEvents();
        }
	}
	else if (func_name == tr("help"))
	{
        QString config="E:/Downloads/Vaa3D_v6.007_Windows_64bit/Vaa3D_v6.007_Windows_64bit/spconfig.cfg";
        ReadConfigFile(config);
        qDebug()<<QCoreApplication::applicationDirPath();
	}
	else return false;

	return true;
}

