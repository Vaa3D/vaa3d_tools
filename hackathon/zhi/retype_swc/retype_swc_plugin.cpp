/* retype_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-03-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "retype_swc_plugin.h"

#include "basic_surf_objs.h"
#include <iostream>
#include "y_imglib.h"
#include "my_surf_objs.h"

using namespace std;
Q_EXPORT_PLUGIN2(retype_swc, retype_swc);
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(fileOpenName));
#endif
        return false;
    }
    QString info;
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
           info = buf;
           myfile<< info.remove('\n') <<endl;
        }

    }

    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};

template <class T> T pow2(T a)
{
    return a*a;

}
 
QStringList retype_swc::menulist() const
{
	return QStringList() 
		<<tr("retype")
		<<tr("about");
}

QStringList retype_swc::funclist() const
{
	return QStringList()
		<<tr("help");
}

void retype_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("retype"))
    {

       /*ofstream myfile;
        myfile.open ("/local3/slide01_section11/slide01_section11_3D/stitched_image.tc",ios::out | ios::app );
        myfile << "# thumbnail file \n";
        myfile << "NULL \n\n";
        myfile << "# tiles \n";
        myfile << "98" << " \n\n";
        myfile << "# dimensions (XYZC) \n";
        myfile << "7630" << " " << "16918" << " " << "751" << " " << 1 << " ";
        myfile << "\n\n";
        myfile << "# origin (XYZ) \n";
        myfile << "0.000000 0.000000 0.000000 \n\n";
        myfile << "# resolution (XYZ) \n";
        myfile << "1.000000 1.000000 1.000000 \n\n";
        myfile << "# image coordinates look up table \n";
        myfile.close();



        int gapx = 101;
        int gapy = 88;
        int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        int x1_new, x2_new, y1_new, y2_new;
        int tiles[18] = {   137,   129,   121,   113,   105,    97,    89,    81,    73,    65,    57,    49,    41,    33,    25,    17,     9,     1};
        int  i,j;
        int shift = 0;
        for(i = 0; i < 18 ; i++)
        {
            if(i ==0)
            {
                y1_new = 0;y2_new = 1023;
            }
            else
            {
                y1_new = y2 -gapy; y2_new = y1_new+1023;
            }
            y1 = y1_new;
            y2 = y2_new;

            for(j = tiles[i]; j <= tiles[i]+ 7; j++)
            {
                if(j == tiles[i])
                {
                      x1_new = shift;
                      x2_new = 1023 + shift;
                }else
                {
                      x1_new = x2 - gapx;
                      x2_new = x2 - gapx + 1023;
                 }
                  x1 = x1_new;
                  x2 = x2_new;
                  myfile.open ("/local3/slide01_section11/slide01_section11_3D/stitched_image.tc",ios::out | ios::app );
                  QString outputilefull;
                  if(j <10)
                    outputilefull.append(QString("00%1.raw").arg(QString::number(j)));
                  else if(j<100)
                      outputilefull.append(QString("0%1.raw").arg(QString::number(j)));
                  else
                      outputilefull.append(QString("%1.raw").arg(QString::number(j)));
                  outputilefull.append(QString("   ( %1, %2, 0) ( %3, %4, 750)").arg(QString::number(x1)).arg(QString::number(y1)).arg(QString::number(x2)).arg(QString::number(y2)));
                  myfile << outputilefull.toStdString();
                  myfile << "\n";
                  myfile.close();

            }
            shift = shift + 9;
        }

        myfile.open ("/media/My_Book/125160/slide--S01/slide1_section10_rename/X00_Y00/stitched_image.tc",ios::out | ios::app );
        myfile << "\n# MST LUT\n";
        myfile.close();


        ofstream myfile;
                myfile.open ("/media/My_Book/125160/slide--S01/slide1_section13_rename/X00_Y00/stitched_image.tc",ios::out | ios::app );
                myfile << "# thumbnail file \n";
                myfile << "NULL \n\n";
                myfile << "# tiles \n";
                myfile << "261" << " \n\n";
                myfile << "# dimensions (XYZC) \n";
                myfile << "1024" << " " << "1024" << " " << "261" << " " << 3 << " ";
                myfile << "\n\n";
                myfile << "# origin (XYZ) \n";
                myfile << "0.000000 0.000000 0.000000 \n\n";
                myfile << "# resolution (XYZ) \n";
                myfile << "1.000000 1.000000 1.000000 \n\n";
                myfile << "# image coordinates look up table \n";
                myfile.close();


                int  i;
                for(i = 0; i < 261 ; i++)
                {
                                             myfile.open ("/media/My_Book/125160/slide--S01/slide1_section13_rename/X00_Y00/stitched_image.tc",ios::out | ios::app );
                          QString outputilefull;
                          if(i <10)
                            outputilefull.append(QString("00%1.tif").arg(QString::number(i)));
                          else if(i<100)
                              outputilefull.append(QString("0%1.tif").arg(QString::number(i)));
                          else
                              outputilefull.append(QString("%1.tif").arg(QString::number(i)));
                          outputilefull.append(QString("   ( 0, 0, %1) ( 1023, 1023, %1)").arg(QString::number(i)).arg(QString::number(i)));
                          myfile << outputilefull.toStdString();
                          myfile << "\n";
                          myfile.close();

                }

                myfile.open ("/media/My_Book/125160/slide--S01/slide1_section13_rename/X00_Y00/stitched_image.tc",ios::out | ios::app );
                myfile << "\n# MST LUT\n";
                myfile.close();*/

        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileOpenName.isEmpty())
            return;
        int type = 2;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
             bool ok;
             nt = readSWC_file(fileOpenName);
             type = QInputDialog::getInteger(parent, "Please specify the node type","type:",1,0,256,1,&ok);
             if (!ok)
                 return;
            for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
            {
               //if(nt.listNeuron[i].parent == -1)
                //   type = type + 1;
                //  nt.listNeuron[i].type = type;
                  nt.listNeuron[i].type = type;

            }
            QString fileDefaultName = fileOpenName+QString("_retype.swc");
            //write new SWC to file
            QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                    fileDefaultName,
                    QObject::tr("Supported file (*.swc)"
                        ";;Neuron structure	(*.swc)"
                        ));
            if (!export_list2file(nt.listNeuron,fileSaveName,fileOpenName))
            {
                v3d_msg("fail to write the output swc file.");
                return;
            }


//            NeuronTree nt = readSWC_file(fileOpenName);
//            QVector<QVector<V3DLONG> > childs;
//            V3DLONG neuronNum = nt.listNeuron.size();
//            childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
//            for (V3DLONG i=0;i<neuronNum;i++)
//            {
//                V3DLONG par = nt.listNeuron[i].pn;
//                if (par<0) continue;
//                childs[nt.hashNeuron.value(par)].push_back(i);
//            }

//            vector<MyMarker*> final_out_swc = readSWC_file(fileOpenName.toStdString());
//            vector<MyMarker*> final_out_swc_updated;
//            final_out_swc_updated.push_back(final_out_swc[0]);


//            for(int j = 1; j < final_out_swc.size(); j++)
//            {
//                int flag_prun = 0;
//                int par_x = final_out_swc[j]->parent->x;
//                int par_y = final_out_swc[j]->parent->y;
//                int par_z = final_out_swc[j]->parent->z;
//                int par_r = final_out_swc[j]->parent->radius;

//                int dis_prun = sqrt(pow2(final_out_swc[j]->x - par_x) + pow2(final_out_swc[j]->y - par_y) + pow2(final_out_swc[j]->z - par_z));
//                if( (final_out_swc[j]->radius + par_r - dis_prun)/dis_prun > 0.3)
//                {
//                    if(childs[j].size() > 0)
//                    {
//                        for(int jj = 0; jj < childs[j].size(); jj++)
//                        final_out_swc[childs[j].at(jj)]->parent = final_out_swc[j]->parent;
//                    }
//                    flag_prun = 1;
//                }

//                if(flag_prun == 0)
//                {
//                   final_out_swc_updated.push_back(final_out_swc[j]);
//                }
//            }


//            QString fileDefaultName = fileOpenName+QString("_retype.swc");
//            QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
//                    fileDefaultName,
//                    QObject::tr("Supported file (*.swc)"
//                        ";;Neuron structure	(*.swc)"
//                        ));

//            saveSWC_file(fileSaveName.toStdString(), final_out_swc_updated);

        }
	}
	else
	{
        v3d_msg(tr("This is a plugin to retype the swc file (first node to be root, and the rest to be dendrites). "
			"Developed by Zhi Zhou, 2014-03-13"));
    }
}

bool retype_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, paras, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) paras = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("retype"))
    {
        cout<<"Welcome to swc retyping plugin"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need input swc file"<<endl;
            return false;
        }

        QString  inswc_file =  infiles[0];
        int k=0;

        int type = (paras.size() >= k+1) ? atoi(paras[k]) : 3;  k++;//0;

        QString  outswc_file =  outfiles[0];
        cout<<"inswc_file = "<<inswc_file.toStdString().c_str()<<endl;
        cout<<"node type = "<<type<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;

        NeuronTree nt;
        nt = readSWC_file(inswc_file);
        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            if(nt.listNeuron[i].parent == -1)
                nt.listNeuron[i].type = 1;
            else
                nt.listNeuron[i].type = type;
        }
        if (!export_list2file(nt.listNeuron,outswc_file,inswc_file))
        {
            v3d_msg("fail to write the output swc file.");
            return false;
        }



    }
	else return false;

	return true;
}

