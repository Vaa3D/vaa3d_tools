/* Neuron Modifer_plugin.cpp
 * This plugin modifies neuron morphology in terms of  basic neuron descriptors.
 * 2015-4-20 : by Xiaoxiao Liu
 */
 
#include "v3d_message.h"
#include <vector>
#include "NeuronModifier_plugin.h"
#include "eswc_core.h"
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(NeuronModifer, NeuronModifer);

 
QStringList NeuronModifer::menulist() const
{
	return QStringList() 
		<<tr("Neuron Modifier")
		<<tr("about");
}

QStringList NeuronModifer::funclist() const
{
	return QStringList()
                <<tr("modify")
		<<tr("help");
}

void NeuronModifer::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
        if (menu_name == tr("about"))
	{
		v3d_msg(tr("This plugin modifies neuron morphology in terms of  basic neuron descriptors.. "
			"Developed by Xiaoxiao Liu, 2015-4-20"));
	}
}

bool NeuronModifer::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

        if (func_name == tr("modify"))
	{
                std::cout<<"You can use this plugin to modify neuron morphologies."<< std::endl;

                if(infiles.empty())
                {
                    std::cout<<"Need input swc file"<< std::endl;
                    return false;
                }

                QString  inswc_file =  infiles[0];
                QString  outswc_file =  outfiles[0];

                double r_scale, x_scale, y_scale, z_scale, prune_level ;
                if (input.size() >= 2)
                {

                    vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
                    if(paras.size() >= 5)
                    {
                        cout<<paras.size()<<endl;
                        r_scale = atof(paras.at(0));
                        x_scale = atof(paras.at(1));
                        y_scale = atof(paras.at(2));
                        z_scale = atof(paras.at(3));
                        prune_level = atof(paras.at(4));

                    }
                    else
                    {
                        std::cout<<"Need five parameters"<< std::endl;
                        return false;
                    }


                    std::cout<<"inswc_file = "<<inswc_file.toStdString().c_str()<< std::endl;
                    std::cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<< std::endl;

                    NeuronTree nt = readSWC_file(inswc_file);

                    //obtain layer order
                    vector<V3DLONG> std_seg_id, std_seg_layer;
                    if (!swc2eswc(nt,std_seg_id, std_seg_layer))
                    {
                            cout<<"Cannot convert swc to eswc."<< endl;
                            return -1;
                    }

                    NeuronTree nt_modified;
                    QList <NeuronSWC> listNeuron;
                    QHash <int, int>  hashNeuron;
                    listNeuron.clear();
                    hashNeuron.clear();

                    NeuronSWC S;
                    QList<NeuronSWC> list = nt.listNeuron;

                    // scale radius, xyz
                    for(V3DLONG i = 0; i < list.size();i++)
                    {
                        if (std_seg_layer[i] < prune_level) // prune all branches with bigger order
                        {
                            S.n =  nt.listNeuron.at(i).n;
                            S.x = x_scale *  nt.listNeuron.at(i).x;
                            S.y = y_scale *  nt.listNeuron.at(i).y;
                            S.z = z_scale *  nt.listNeuron.at(i).z;
                            S.r = r_scale *  nt.listNeuron.at(i).r;
                            S.pn =  nt.listNeuron.at(i).pn;
                            S.type =  nt.listNeuron.at(i).type;
                            listNeuron.append(S);
                            hashNeuron.insert(S.n, listNeuron.size()-1);
                        }
                    }

                    nt_modified.n = -1;
                    nt_modified.on = true;
                    nt_modified.listNeuron = listNeuron;
                    nt_modified.hashNeuron = hashNeuron;

                    writeSWC_file(outswc_file,nt_modified);
                }
                else
                {
                     std::cout<<"Need five factors"<< std::endl;
                    return false;
                }


	}
	else if (func_name == tr("help"))
	{
                 std::cout<<"Usage : v3d -x dllname -f modify -i <inswc_folder> -o <outswc_file> -p <radius_factor> <x_factor> <y_factor> <z_factor> <prune_level>"
                   << std::endl;
	}
	else return false;

	return true;
}

