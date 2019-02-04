/* reco_eval_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-2-4 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "reco_eval_plugin.h"
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"
#include "../AllenNeuron_postprocessing/sort_swc_IVSCC.h"
#include "volimg_proc.h"

using namespace std;
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))

Q_EXPORT_PLUGIN2(reco_eval, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
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
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2019-2-4"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("eval"))
	{
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        QString inimg_file = infiles[0];
//        QString output_folder = outfiles[0];
        int k=0;
        QString swc_name = paras.empty() ? "" : paras[k]; if(swc_name == "NULL") swc_name = ""; k++;
        NeuronTree nt = readSWC_file(swc_name);
        QVector<QVector<V3DLONG> > childs;
        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }
        int Wx = 16;
        int Wy = 16;
        int Wz = 16;
        QList<NeuronSWC> list = nt.listNeuron;
        double corr = 0;
        int cnt = 1;
        for (V3DLONG i=0;i<list.size();i++)
        {
            if (childs[i].size()==0)
            {
                cnt++;
                V3DLONG tmpx = nt.listNeuron.at(i).x;
                V3DLONG tmpy = nt.listNeuron.at(i).y;
                V3DLONG tmpz = nt.listNeuron.at(i).z;
                V3DLONG xb = tmpx-Wx;
                V3DLONG xe = tmpx+Wx;
                V3DLONG yb = tmpy-Wy;
                V3DLONG ye = tmpy+Wy;
                V3DLONG zb = tmpz-Wz;
                V3DLONG ze = tmpz+Wz;
//                QString outimg_file;
//                outimg_file = output_folder + QString("/x%1_y%2_z%3.tif").arg(tmpx).arg(tmpy).arg(tmpz);

                unsigned char * data1d = 0;
                data1d = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,yb,ye+1,zb,ze+1);
                V3DLONG im_cropped_sz[4];
                im_cropped_sz[0] = xe - xb + 1;
                im_cropped_sz[1] = ye - yb + 1;
                im_cropped_sz[2] = ze - zb + 1;
                im_cropped_sz[3] = 1;

//                simple_saveimage_wrapper(callback,outimg_file.toStdString().c_str(),data1d,im_cropped_sz,1);

                NeuronTree nt_tps;
                QList <NeuronSWC> & listNeuron = nt_tps.listNeuron;
                for(V3DLONG j =0; j < list.size();j++)
                {
                    NeuronSWC t;
                    t = list.at(j);
                    t.x = list[j].x - xb;
                    t.y = list[j].y - yb;
                    t.z = list[j].z - zb;
                    if(t.x >= 0 && t.x < im_cropped_sz[0] && t.y >= 0 && t.y < im_cropped_sz[1] && t.z >= 0 && t.z < im_cropped_sz[2])
                    {
                        listNeuron << t;
                    }
                }
                nt_tps = SortSWC_pipeline(nt_tps.listNeuron,VOID,0);
//                QString outimg_swc = output_folder + QString("/x%1_y%2_z%3.swc").arg(tmpx).arg(tmpy).arg(tmpz);
//                writeSWC_file(outimg_swc,nt_tps);


                V3DLONG stacksz = im_cropped_sz[0] * im_cropped_sz[1] * im_cropped_sz[2];
                unsigned char *data1d_mask = new unsigned char [stacksz];
                memset(data1d_mask,0,stacksz*sizeof(unsigned char));
                double margin=0;//by PHC 20170531
                ComputemaskImage(nt_tps, data1d_mask, im_cropped_sz[0], im_cropped_sz[1], im_cropped_sz[2], margin);
                listNeuron.clear();
                for(V3DLONG i=0; i<stacksz; i++)
                    data1d_mask[i] = data1d_mask[i]==255?data1d[i]:0;
//                outimg_file = output_folder + QString("/x%1_y%2_z%3_v2.tif").arg(tmpx).arg(tmpy).arg(tmpz);
//                simple_saveimage_wrapper(callback,outimg_file.toStdString().c_str(),data1d_mask,im_cropped_sz,1);

                corr += calCorrelation(data1d,data1d_mask,stacksz)*NTDIS(list.at(0),list.at(i));
                if(data1d_mask) { delete []data1d_mask; data1d_mask = 0;}
                if(data1d) { delete []data1d; data1d = 0;}
//                v3d_msg(QString("%1").arg(corr));
            }
        }
        double corr_total = corr/cnt;
        if (output.size() == 1)
        {
            char *outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

            ofstream myfile;
            myfile.open (outimg_file,ios::out | ios::app );
            myfile << swc_name.toStdString()<<"\t"<<corr_total <<endl;
            myfile.close();
        }
        return true;
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

