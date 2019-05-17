/* test_app2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-13 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "test_app2_plugin.h"
#include <QFileInfo>
#include "function_.cpp"
#include "v3d_basicdatatype.h"
#include <iostream>
#include "block.h"
#include "app2_block.h"
using namespace std;
Q_EXPORT_PLUGIN2(test_app2, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
    //QFileInfo file;

}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
        //v3d_msg("To be implemented.");
        QString pathSwc=QFileDialog::getOpenFileName(parent,QString(QObject::tr(",,,")),"/home/balala/Documents");
        QString pathEswc=QFileDialog::getOpenFileName(parent,QString(QObject::tr(",,,")),"/home/balala/Desktop");
        NeuronTree ntSwc = readSWC_file(pathSwc);
        NeuronTree ntEswc = readSWC_file(pathEswc);
        blockTree btSwc = getBlockTree(ntSwc, 64, 64, 32);
        blockTree btEswc = getBlockTree(ntEswc, 64, 64, 32);
        int numSwc = btSwc.blocklist.size();
        int numEswc = btEswc.blocklist.size();
        if(numEswc != numSwc)
        {
            //vector<NeuronSWC> tipSwc = tip_detection1(ntSwc);
            //vector<NeuronSWC> tipEswc = tip_detection1(ntEswc);
            for(V3DLONG i = numSwc; i < numEswc; i++)
            {
                //process_signal(btEswc.blocklist[i]);
                app2_block(btEswc.blocklist[i]);
            }
        }
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-13"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

