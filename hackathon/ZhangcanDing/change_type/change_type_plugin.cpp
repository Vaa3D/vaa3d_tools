/* change_type_plugin.cpp
 * This is a plugin for change .eswc and .marker file
 * 2019-3-15 : by ZhangcanDing
 */
 
#include "v3d_message.h"
#include <vector>
#include "change_type_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(change_type, changetype);
 
QStringList changetype::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList changetype::funclist() const
{
	return QStringList()
		<<tr("swc_type")
		<<tr("marker_type")
		<<tr("help");
}

void changetype::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
		v3d_msg(tr("This is a plugin for change .eswc and .marker file. "
			"Developed by ZhangcanDing, 2019-3-15"));
	}
}

bool changetype::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("swc_type"))
	{
        /* type number and color:
            2:red 3: blue 4:purple 5:pale blue 6:yellow 7:green 8:orange 9:olive green 10:pink */


//        if(input.size()<2)
//        {
//            fprintf(stderr,"Need input -i swc file and -p type number. \n ");

//        }



        NeuronTree p= readSWC_file(QString(infiles[0]));
        NeuronTree nt_new;

        int newtype= atof(inparas[0]);
        for(int i=0;i<p.listNeuron.size();i++)
        {
            NeuronSWC S;
            S.n = p.listNeuron[i].n;
            S.type =newtype;
            S.x = p.listNeuron[i].x;
            S.y= p.listNeuron[i].y;
            S.z = p.listNeuron[i].z;
            S.r = p.listNeuron[i].r;
            S.pn = p.listNeuron[i].pn;
            S.seg_id = p.listNeuron[i].seg_id;
            S.level = p.listNeuron[i].level;
            S.creatmode = p.listNeuron[i].creatmode;  // Creation Mode LMG 8/10/2018
            S.timestamp = p.listNeuron[i].timestamp;  // Timestamp LMG 27/9/2018
            S.tfresindex = p.listNeuron[i].tfresindex; // TeraFly resolution index LMG 13/12/2018
            S.fea_val = p.listNeuron[i].fea_val;
            nt_new.listNeuron.append(S);
            nt_new.hashNeuron.insert(S.n, nt_new.listNeuron.size()-1);
        }

        QString filename=QString(infiles[0])+"_typechanged.eswc";
        writeESWC_file(filename, nt_new);







	}
	else if (func_name == tr("marker_type"))
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

