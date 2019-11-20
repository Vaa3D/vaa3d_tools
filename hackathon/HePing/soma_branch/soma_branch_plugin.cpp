/* soma_branch_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-11-20 : by heping
 */
 
#include "v3d_message.h"
#include <vector>
#include "soma_branch_plugin.h"
#include "branch_count_soma.h"
using namespace std;
Q_EXPORT_PLUGIN2(soma_branch, BranchCount);
 
QStringList BranchCount::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList BranchCount::funclist() const
{
	return QStringList()
        <<tr("branch_count")
		<<tr("func2")
		<<tr("help");
}

void BranchCount::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by heping, 2019-11-20"));
	}
}

bool BranchCount::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("branch_count"))
	{
        QString swcfile=(infiles.size()>=1)?infiles[0]:"";
        NeuronTree nt=readSWC_file(swcfile);
        SWCTree t;
        QList<ImageMarker> markers;
        vector<location> points;
        QFileInfo eswcfileinfo;
        eswcfileinfo=QFileInfo(swcfile);
        QString eswcfile=eswcfileinfo.fileName();
        eswcfile.mid(0,eswcfile.indexOf("."));
        t.count_branch_location(nt,markers,points);
        bool flag=four_point(points);
        if(flag==true) qDebug()<<"yes!";
        else qDebug()<<"no!";
        qDebug()<<"branch num:"<<markers.size();
        QString apofile=(outfiles.size()>=1)?outfiles[0]:"";
        writeMarker_file(apofile+"//"+eswcfile+".marker",markers);

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

