/* seg_dendrite_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-1-6 : by heping
 */
 
#include "v3d_message.h"
#include <vector>
#include "seg_dendrite_plugin.h"
#include "seg_dendrite.h"
using namespace std;
Q_EXPORT_PLUGIN2(seg_dendrite, SegDendrite);
 
QStringList SegDendrite::menulist() const
{
	return QStringList() 
        <<tr("seg_dendrite")
		<<tr("about");
}

QStringList SegDendrite::funclist() const
{
	return QStringList()
        <<tr("segDendrite")
		<<tr("help");
}

void SegDendrite::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("seg_dendrite"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by heping, 2020-1-6"));
	}
}

bool SegDendrite::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("segDendrite"))
	{
        QString swcfile=infiles[0];
        QString segfile=outfiles[0];
        NeuronTree nt=readSWC_file(swcfile);

        QFileInfo eswcfileinfo;
        eswcfileinfo=QFileInfo(swcfile);
        QString eswcfile=eswcfileinfo.fileName();
        qDebug()<<eswcfile.toStdString().c_str();
        seg_dendrite(segfile+"//seg_"+eswcfile,nt);

	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

