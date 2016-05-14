/* pruning_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-05-02 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "pruning_swc_plugin.h"


#include "basic_surf_objs.h"
#include <iostream>
#include "my_surf_objs.h"

template <class T> T local_max(T a, T b)
{
    return (a>=b)?a:b;
}

template <class T> T local_min(T a, T b)
{
    return (a<=b)?a:b;
}

using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))


Q_EXPORT_PLUGIN2(pruning_swc, pruning_swc);

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


QStringList importSWCFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}
 
QStringList pruning_swc::menulist() const
{
	return QStringList() 
        <<tr("pruning")
        <<tr("pruning_nc")
         <<tr("caculate_distance")
           <<tr("rotation")

      //  <<tr("pruning_group")
      //  <<tr("aligning")
		<<tr("about");
}

QStringList pruning_swc::funclist() const
{
    return QStringList()
        <<tr("pruning_nc")
        <<tr("caculate_distance")
       <<tr("rotation")
		<<tr("help");
}

void pruning_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("pruning"))
	{
                QString fileOpenName;
                fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                        "",
                        QObject::tr("Supported file (*.swc *.eswc)"
                            ";;Neuron structure	(*.swc)"
                            ";;Extended neuron structure (*.eswc)"
                            ));
                if(fileOpenName.isEmpty())
                    return;
                double length = 0;
                NeuronTree nt;
                if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
                {
                     bool ok;
                     nt = readSWC_file(fileOpenName);
                     length = QInputDialog::getDouble(parent, "Please specify the maximum prunned segment length","segment length:",1,0,2147483647,0.1,&ok);
                     if (!ok)
                         return;
                }

                QVector<QVector<V3DLONG> > childs;


                V3DLONG neuronNum = nt.listNeuron.size();
                childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
                V3DLONG *flag = new V3DLONG[neuronNum];

                for (V3DLONG i=0;i<neuronNum;i++)
                {
                    flag[i] = -1;

                    V3DLONG par = nt.listNeuron[i].pn;
                    if (par<0) continue;
                    childs[nt.hashNeuron.value(par)].push_back(i);
                }

                QList<NeuronSWC> list = nt.listNeuron;

                for (int i=0;i<list.size();i++)
                {
//                    if (childs[i].size()==0)
//                    {
//                        int index_tip = 0;
//                        int parent_tip = getParent(i,nt);
//                        while(childs[parent_tip].size()<2)
//                        {
//                            parent_tip = getParent(parent_tip,nt);
//                            index_tip++;
//                            if(parent_tip == 1000000000)
//                                break;
//                        }
//                        if(index_tip < length)
//                        {
//                            flag[i] = -1;

//                            int parent_tip = getParent(i,nt);
//                            while(childs[parent_tip].size()<2)
//                           {
//                                flag[parent_tip] = -1;
//                                parent_tip = getParent(parent_tip,nt);
//                                if(parent_tip == 1000000000)
//                                    break;
//                           }
//                        }

//                    }
                    if (childs[i].size()>1)
                        flag[i] = 1;

                }

               //NeutronTree structure
               NeuronTree nt_prunned;
               QList <NeuronSWC> listNeuron;
               QHash <int, int>  hashNeuron;
               listNeuron.clear();
               hashNeuron.clear();

               //set node

               NeuronSWC S;
               for (int i=0;i<list.size();i++)
               {
                   if(flag[i] == 1)
                   {
                        NeuronSWC curr = list.at(i);
                        S.n 	= curr.n;
                        S.type 	= curr.type;
                        S.x 	= curr.x;
                        S.y 	= curr.y;
                        S.z 	= curr.z;
                        S.r 	= curr.r;
                        S.pn 	= curr.pn;
                        listNeuron.append(S);
                        hashNeuron.insert(S.n, listNeuron.size()-1);
                   }

              }
               nt_prunned.n = -1;
               nt_prunned.on = true;
               nt_prunned.listNeuron = listNeuron;
               nt_prunned.hashNeuron = hashNeuron;

               if(flag) {delete[] flag; flag = 0;}

               QString fileDefaultName = fileOpenName+QString("_pruned.swc");
               //write new SWC to file
               QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                       fileDefaultName,
                       QObject::tr("Supported file (*.swc)"
                           ";;Neuron structure	(*.swc)"
                           ));
               if (!export_list2file(nt_prunned.listNeuron,fileSaveName,fileOpenName))
               {
                   v3d_msg("fail to write the output swc file.");
                   return;
               }


    }else if (menu_name == tr("caculate_distance"))
    {
                QString fileOpenName;
                fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                        "",
                        QObject::tr("Supported file (*.swc *.eswc)"
                            ";;Neuron structure	(*.swc)"
                            ";;Extended neuron structure (*.eswc)"
                            ));
                if(fileOpenName.isEmpty())
                    return;
                NeuronTree nt1;
                if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
                {
                     nt1 = readSWC_file(fileOpenName);
                }

                fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                        "",
                        QObject::tr("Supported file (*.swc *.eswc)"
                            ";;Neuron structure	(*.swc)"
                            ";;Extended neuron structure (*.eswc)"
                            ));

                NeuronTree nt2;
                if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
                {
                     nt2 = readSWC_file(fileOpenName);
                }

                V3DLONG neuronNum1 = nt1.listNeuron.size();
                V3DLONG *dis1 = new V3DLONG[neuronNum1];
                V3DLONG *match1 = new V3DLONG[neuronNum1];

                V3DLONG neuronNum2 = nt2.listNeuron.size();
                V3DLONG *dis2 = new V3DLONG[neuronNum2];

                for(int i = 0; i < neuronNum1; i++)
                {
                    double x1 = nt1.listNeuron.at(i).x;
                    double y1 = nt1.listNeuron.at(i).y;
                    double z1 = nt1.listNeuron.at(i).z;
                    double dis_min = 10000000;
                    for(int j = 0; j < neuronNum2; j++)
                    {
                        double x2 = nt2.listNeuron.at(j).x;
                        double y2 = nt2.listNeuron.at(j).y;
                        double z2 = nt2.listNeuron.at(j).z;
                        double dis = sqrt(pow2(x1 -x2 ) + pow2(y1 - y2) + pow2(z1 - z2));
                        if(dis < dis_min)
                        {
                            dis1[i] = j;
                            match1[i] = dis;
                            dis_min = dis;
                        }
                    }

                }

                for(int j = 0; j < neuronNum2; j++)
                {
                    double x2 = nt2.listNeuron.at(j).x;
                    double y2 = nt2.listNeuron.at(j).y;
                    double z2 = nt2.listNeuron.at(j).z;

                    double dis_min = 10000000;
                    for(int i = 0; i < neuronNum1; i++)
                    {

                        double x1 = nt1.listNeuron.at(i).x;
                        double y1 = nt1.listNeuron.at(i).y;
                        double z1 = nt1.listNeuron.at(i).z;
                        double dis = sqrt(pow2(x1 -x2 ) + pow2(y1 - y2) + pow2(z1 - z2));
                        if(dis < dis_min)
                        {
                            dis2[j] = i;
                            dis_min = dis;

                        }
                    }
                }

                double dis_totle = 0;
                int d = 0;
                for(int i = 0; i < neuronNum1; i++)
                {
                    if(dis2[dis1[i]] == i)
                    {
                        dis_totle = dis_totle + match1[i];
                        d++;
                    }

                }

                double final_distance = dis_totle/d;
                v3d_msg(QString("distance is %1, number of pairs :%2").arg(final_distance).arg(d));


    }
    else if (menu_name == tr("pruning_nc"))
    {
                QString fileOpenName;
                fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                        "",
                        QObject::tr("Supported file (*.swc *.eswc)"
                            ";;Neuron structure	(*.swc)"
                            ";;Extended neuron structure (*.eswc)"
                            ));
                if(fileOpenName.isEmpty())
                    return;
                double length = 0;
                NeuronTree nt;
                if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
                {
                     bool ok;
                     nt = readSWC_file(fileOpenName);
                     length = QInputDialog::getDouble(parent, "Please specify the maximum prunned segment length","segment length:",1,0,2147483647,0.1,&ok);
                     if (!ok)
                         return;
                }

                QVector<QVector<V3DLONG> > childs;


                V3DLONG neuronNum = nt.listNeuron.size();
                childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
                V3DLONG *flag = new V3DLONG[neuronNum];

                for (V3DLONG i=0;i<neuronNum;i++)
                {
                    flag[i] = 1;

                    V3DLONG par = nt.listNeuron[i].pn;
                    if (par<0) continue;
                    childs[nt.hashNeuron.value(par)].push_back(i);
                }

                QList<NeuronSWC> list = nt.listNeuron;

                for (int i=0;i<list.size();i++)
                {

                    if (childs[i].size()==0)
                    {
                        int index_tip = 0;
                        int parent_tip = getParent(i,nt);
                        while(childs[parent_tip].size()<2)
                        {
                            parent_tip = getParent(parent_tip,nt);
                            index_tip++;
                            if(parent_tip == 1000000000)
                                break;
                        }

                        int type1 = list.at(parent_tip).type;
                        int type2;
                        if(getParent(parent_tip,nt) == 1000000000)
                            type2 = type1;
                        else
                            type2 = list.at(getParent(parent_tip,nt)).type;
                        int type3 = list.at(childs[parent_tip].at(1)).type;


                        if(index_tip < length && (type2 != type1 || type2 != type3))
                        {
                            flag[i] = -1;

                            int parent_tip = getParent(i,nt);
                            while(childs[parent_tip].size()<2)
                           {
                                flag[parent_tip] = -1;
                                parent_tip = getParent(parent_tip,nt);
                                if(parent_tip == 1000000000)
                                    break;
                           }
                        }

                    }

                }

               //NeutronTree structure
               NeuronTree nt_prunned;
               QList <NeuronSWC> listNeuron;
               QHash <int, int>  hashNeuron;
               listNeuron.clear();
               hashNeuron.clear();

               //set node

               NeuronSWC S;
               for (int i=0;i<list.size();i++)
               {
                   if(flag[i] == 1)
                   {
                        NeuronSWC curr = list.at(i);
                        S.n 	= curr.n;
                        S.type 	= curr.type;
                        S.x 	= curr.x;
                        S.y 	= curr.y;
                        S.z 	= curr.z;
                        S.r 	= curr.r;
                        S.pn 	= curr.pn;
                        listNeuron.append(S);
                        hashNeuron.insert(S.n, listNeuron.size()-1);
                   }

              }
               nt_prunned.n = -1;
               nt_prunned.on = true;
               nt_prunned.listNeuron = listNeuron;
               nt_prunned.hashNeuron = hashNeuron;

               if(flag) {delete[] flag; flag = 0;}

               QString fileDefaultName = fileOpenName+QString("_pruned.swc");
               //write new SWC to file
               QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                       fileDefaultName,
                       QObject::tr("Supported file (*.swc)"
                           ";;Neuron structure	(*.swc)"
                           ));
               if (!export_list2file(nt_prunned.listNeuron,fileSaveName,fileOpenName))
               {
                   v3d_msg("fail to write the output swc file.");
                   return;
               }


    }if (menu_name == tr("rotation"))
    {
                QString fileOpenName;
                fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                        "",
                        QObject::tr("Supported file (*.swc *.eswc)"
                            ";;Neuron structure	(*.swc)"
                            ";;Extended neuron structure (*.eswc)"
                            ));
                if(fileOpenName.isEmpty())
                    return;
                double degree = 0;
                NeuronTree nt;
                if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
                {
                     bool ok;
                     nt = readSWC_file(fileOpenName);
                     degree = QInputDialog::getDouble(parent, "Please specify the maximum prunned segment length","segment length:",1,-180,180,0.1,&ok);
                     if (!ok)
                         return;
                }

                double alpha = degree/180.0*3.141592635;
                double xc = 1055, yc = 1701;
                V3DLONG nx = 2111;
                V3DLONG ny = 3403;

                struct PixelPos{double x, y;};


                PixelPos e00, e01, e10, e11; //for the four corners
                e00.x = 0 - xc;
                e00.y = 0 - yc;

                e01.x = 0 - xc;
                e01.y = (ny-1) - yc;

                e10.x = (nx-1) - xc;
                e10.y = 0 - yc;

                e11.x = (nx-1) - xc;
                e11.y = (ny-1) - yc;

                double c00, c01, c10, c11;
                c00 = cos(alpha);
                c01 = sin(alpha);
                c10 = -sin(alpha);
                c11 = cos(alpha);

                PixelPos e00t, e01t, e10t, e11t; //the coordinates of the transformed corners
                e00t.x = c00*e00.x + c01*e00.y;
                e00t.y = c10*e00.x + c11*e00.y;

                e01t.x = c00*e01.x + c01*e01.y;
                e01t.y = c10*e01.x + c11*e01.y;

                e10t.x = c00*e10.x + c01*e10.y;
                e10t.y = c10*e10.x + c11*e10.y;

                e11t.x = c00*e11.x + c01*e11.y;
                e11t.y = c10*e11.x + c11*e11.y;

                double px_min = local_min(local_min(local_min(e00t.x, e01t.x), e10t.x), e11t.x);
                double px_max = local_max(local_max(local_max(e00t.x, e01t.x), e10t.x), e11t.x);
                double py_min = local_min(local_min(local_min(e00t.y, e01t.y), e10t.y), e11t.y);
                double py_max = local_max(local_max(local_max(e00t.y, e01t.y), e10t.y), e11t.y);

                double c00b, c01b, c10b, c11b;
                c00b = cos(-alpha); //c00b=(c00b<my_eps)?0:c00b; c00b=(c00b>1-my_eps)?1:c00b;
                c01b = sin(-alpha); //c01b=(c01b<my_eps)?0:c01b; c01b=(c01b>1-my_eps)?1:c01b;
                c10b = -sin(-alpha);//c10b=(c10b<my_eps)?0:c10b; c10b=(c10b>1-my_eps)?1:c10b;
                c11b = cos(-alpha); //c11b=(c11b<my_eps)?0:c11b; c11b=(c11b>1-my_eps)?1:c11b;


                QList<NeuronSWC> list = nt.listNeuron;

                //NeutronTree structure
                NeuronTree nt_rotated;
                QList <NeuronSWC> listNeuron;
                QHash <int, int>  hashNeuron;
                listNeuron.clear();
                hashNeuron.clear();

                //set node

                NeuronSWC S;
                for (int i=0;i<list.size();i++)
                {
                    NeuronSWC curr = list.at(i);
                    S.n 	= curr.n;
                    S.type 	= curr.type;
                    S.x 	= c00*(curr.x+px_min) + c01*(curr.y+py_min) + xc;
                    S.y 	= c10*(curr.x+px_min) + c11*(curr.y+py_min) + yc;
                    S.z 	= curr.z;
                    S.r 	= curr.r;
                    S.pn 	= curr.pn;
                    listNeuron.append(S);
                    hashNeuron.insert(S.n, listNeuron.size()-1);

                }
                nt_rotated.n = -1;
                nt_rotated.on = true;
                nt_rotated.listNeuron = listNeuron;
                nt_rotated.hashNeuron = hashNeuron;

               QString fileDefaultName = fileOpenName+QString("_rotated.swc");
               //write new SWC to file
               QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                       fileDefaultName,
                       QObject::tr("Supported file (*.swc)"
                           ";;Neuron structure	(*.swc)"
                           ));
               if (!export_list2file(nt_rotated.listNeuron,fileSaveName,fileOpenName))
               {
                   v3d_msg("fail to write the output swc file.");
                   return;
               }

    }else if(menu_name == tr("pruning_group"))
    {
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all swc files "),
                                              QDir::currentPath(),
                                              QFileDialog::ShowDirsOnly);

        QStringList swcList = importSWCFileList_addnumbersort(m_InputfolderName);
        bool ok;
        double length = QInputDialog::getDouble(parent, "Please specify the maximum prunned segment length","segment length:",1,0,2147483647,0.1,&ok);

        for(int i = 0; i < swcList.size(); i++)
        {
            NeuronTree nt;
            QString curPathSWC = swcList.at(i);
            nt = readSWC_file(curPathSWC);

            QVector<QVector<V3DLONG> > childs;


            V3DLONG neuronNum = nt.listNeuron.size();
            childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
            V3DLONG *flag = new V3DLONG[neuronNum];

            for (V3DLONG i=0;i<neuronNum;i++)
            {
                flag[i] = 1;

                V3DLONG par = nt.listNeuron[i].pn;
                if (par<0) continue;
                childs[nt.hashNeuron.value(par)].push_back(i);
            }

            QList<NeuronSWC> list = nt.listNeuron;
            for (int i=0;i<list.size();i++)
            {
                if (childs[i].size()==0)
                {
                    int index_tip = 0;
                    int parent_tip = getParent(i,nt);
                    while(childs[parent_tip].size()<2)
                    {

                        parent_tip = getParent(parent_tip,nt);
                        index_tip++;
                        if(parent_tip == 1000000000)
                            break;
                    }
                    if(index_tip < length)
                    {
                        flag[i] = -1;

                        int parent_tip = getParent(i,nt);
                        while(childs[parent_tip].size()<2)
                        {
                            flag[parent_tip] = -1;
                            parent_tip = getParent(parent_tip,nt);
                            if(parent_tip == 1000000000)
                                break;
                        }
                    }

                }

            }

           //NeutronTree structure
           NeuronTree nt_prunned;
           QList <NeuronSWC> listNeuron;
           QHash <int, int>  hashNeuron;
           listNeuron.clear();
           hashNeuron.clear();

           //set node

           NeuronSWC S;
           for (int i=0;i<list.size();i++)
           {
               if(flag[i] == 1)
               {
                    NeuronSWC curr = list.at(i);
                    S.n 	= curr.n;
                    if(i ==0)
                         S.type = 1;
                    else
                        S.type 	= 3;
                    S.x 	= curr.x;
                    S.y 	= curr.y;
                    S.z 	= curr.z;
                    S.r 	= curr.r;
                    S.pn 	= curr.pn;
                    listNeuron.append(S);
                    hashNeuron.insert(S.n, listNeuron.size()-1);
               }

          }
           nt_prunned.n = -1;
           nt_prunned.on = true;
           nt_prunned.listNeuron = listNeuron;
           nt_prunned.hashNeuron = hashNeuron;

           if(flag) {delete[] flag; flag = 0;}

           QString fileDefaultName = curPathSWC+QString("_prunned.swc");
           export_list2file(nt_prunned.listNeuron,fileDefaultName,curPathSWC);

           nt = nt_prunned;
           neuronNum = nt.listNeuron.size();
           childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
           for (V3DLONG i=0;i<neuronNum;i++)
           {
               V3DLONG par = nt.listNeuron[i].pn;
               if (par<0) continue;
               childs[nt.hashNeuron.value(par)].push_back(i);
           }

           vector<MyMarker*> final_out_swc = readSWC_file(fileDefaultName.toStdString());
           vector<MyMarker*> final_out_swc_updated;
           final_out_swc_updated.push_back(final_out_swc[0]);


           for(int j = 1; j < final_out_swc.size(); j++)
           {
               int flag_prun = 0;
               int par_x = final_out_swc[j]->parent->x;
               int par_y = final_out_swc[j]->parent->y;
               int par_z = final_out_swc[j]->parent->z;
               int par_r = final_out_swc[j]->parent->radius;

               int dis_prun = sqrt(pow2(final_out_swc[j]->x - par_x) + pow2(final_out_swc[j]->y - par_y) + pow2(final_out_swc[j]->z - par_z));
               if( (final_out_swc[j]->radius + par_r - dis_prun)/dis_prun > 0.3)
               {
                   if(childs[j].size() > 0)
                   {
                       for(int jj = 0; jj < childs[j].size(); jj++)
                       final_out_swc[childs[j].at(jj)]->parent = final_out_swc[j]->parent;
                   }
                   flag_prun = 1;
               }

               if(flag_prun == 0)
               {
                  final_out_swc_updated.push_back(final_out_swc[j]);
               }
           }

           saveSWC_file(fileDefaultName.toStdString(), final_out_swc_updated);

        }

    }
    else if(menu_name == tr("aligning"))
    {

        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all swc files "),
                                              QDir::currentPath(),
                                              QFileDialog::ShowDirsOnly);

        QStringList swcList = importSWCFileList_addnumbersort(m_InputfolderName);

        double xoriginal = 5491;
        double yoriginal = 4004;
        double a11, a12, a21,a22,xshift,yshift;

        for(int i = 0; i < swcList.size(); i++)
        {
            QString fileOpenName = swcList.at(i);
            NeuronTree nt = readSWC_file(fileOpenName);

            switch (i)
            {
            case 0: a11 = 0.9999952915847761; a12 = 0.0030686818471570704; a21 = -0.0030686818471570704; a22 = 0.9999952915847761; xshift = 6578.724150979881 - xoriginal,yshift = 5362.7962652663955 - yoriginal; break;
            case 1: a11 = 0.9790086109325223; a12 = 0.20381888950726068; a21 = -0.20381888950726068; a22 = 0.9790086109325223; xshift = 6309.3258765566325 - xoriginal,yshift = 4675.223343270161- yoriginal; break;
            case 2: a11 = 0.9649529336213126; a12 = 0.003885379629013957; a21 = -0.09399915877564446; a22 = 1.109311066828655; xshift = 4451.467218976793 - xoriginal,yshift = 3804.754954241117- yoriginal; break;

            case 3: a11 = 0.9859499304449573; a12 = 0.04889885789093282; a21 = -0.04889885789093282; a22 = 0.9859499304449573; xshift = 3718.445925741472 - xoriginal,yshift = 3279.9928877437733 - yoriginal; break;
            case 4: a11 = 1; a12 = 0; a21 = 0; a22 = 1; xshift = 0; yshift = 0; break;
            case 5: a11 = 0.9724946496210681; a12 = -0.026761134311933332; a21 = 0.026761134311933332; a22 = 0.9724946496210681; xshift = 4193.867823188051 - xoriginal,yshift = 3977.3267690186876- yoriginal; break;

            case 6: a11 = 0.8814191692822646; a12 = 0.1811983087815262; a21 = -0.1811983087815262; a22 = 0.8814191692822646; xshift = 5344.504252853956 - xoriginal,yshift = 2992.4001083908097 - yoriginal; break;
            case 7: a11 = 0.8628208649528964; a12 = 0.2021634804999314; a21 = -0.2021634804999314; a22 = 0.8628208649528964; xshift = 5070.6793466577665 - xoriginal,yshift = 1937.7105765264987- yoriginal; break;
            case 8: a11 = 0.976153679914872; a12 = 0.026526366354767836; a21 = -0.026526366354767836; a22 = 0.976153679914872; xshift = 4103.317414401224 - xoriginal,yshift = 2894.090843709928- yoriginal; break;

            case 9: a11 = 0.9967365324737809; a12 = 0.1106109044593533; a21 = -0.1106109044593533; a22 = 0.9967365324737809; xshift = 4248.728949552501 - xoriginal,yshift = 835.6503035816423 - yoriginal; break;
            case 10: a11 = 0.9832831621170044; a12 = 0.17077744007110596; a21 = -0.17077744007110596; a22 = 0.9832831621170044; xshift = 4687.046610951424 - xoriginal,yshift = 702.4094506502151- yoriginal; break;

            }

            NeuronTree nt_aligned;
            if(i !=4)
            {
                QList <NeuronSWC> listNeuron;
                QHash <int, int>  hashNeuron;
                listNeuron.clear();
                hashNeuron.clear();

                QList<NeuronSWC> list = nt.listNeuron;
                NeuronSWC S;
                for (int i=0;i<list.size();i++)
                {
                    NeuronSWC curr = list.at(i);
                    float x_old = curr.x;
                    float y_old = curr.y;
                    S.x = x_old*a11 + y_old*a21 + xshift;
                    S.y = x_old*a12 + y_old*a22 + yshift;
                    S.z = curr.z;
                    S.n 	= curr.n;
                    S.type 	= curr.type;
                    S.r 	= curr.r;
                    S.pn 	= curr.pn;
                    listNeuron.append(S);
                    hashNeuron.insert(S.n, listNeuron.size()-1);
                }

                nt_aligned.n = -1;
                nt_aligned.on = true;
                nt_aligned.listNeuron = listNeuron;
                nt_aligned.hashNeuron = hashNeuron;
            }
            else
            {
                nt_aligned = nt;
            }


            float min = 2000000;
            float max = -2000000;
            float offsecZ;
            switch (i)
            {
                case 0: offsecZ = -1144; break;
                case 1: offsecZ = -858; break;
                case 2: offsecZ = -572; break;
                case 3: offsecZ = -286; break;
                case 4: offsecZ = 0; break;
                case 5: offsecZ = 286; break;
                case 6: offsecZ = 572; break;
                case 7: offsecZ = 858; break;
                case 8: offsecZ = 1144; break;
                case 9: offsecZ = 1430; break;
                case 10: offsecZ = 1686; break;

            }


            NeuronTree nt_norm;
            QList <NeuronSWC> listNeuron;
            QHash <int, int>  hashNeuron;
            listNeuron.clear();
            hashNeuron.clear();

            QList<NeuronSWC> list = nt_aligned.listNeuron;

            for (int i=0;i<list.size();i++)
            {
                NeuronSWC curr = list.at(i);
                if(curr.z > max ) max = curr.z;
                if(curr.z < min ) min = curr.z;
            }

            NeuronSWC S;
            for (int i=0;i<list.size();i++)
            {
                NeuronSWC curr = list.at(i);
                S.x = curr.x;
                S.y = curr.y;
                S.z = offsecZ + (285 * (curr.z - min) /(max - min));
                S.n 	= curr.n;
                S.type 	= curr.type;
                S.r 	= curr.r;
                S.pn 	= curr.pn;
                listNeuron.append(S);
                hashNeuron.insert(S.n, listNeuron.size()-1);
            }


            nt_norm.n = -1;
            nt_norm.on = true;
            nt_norm.listNeuron = listNeuron;
            nt_norm.hashNeuron = hashNeuron;

            //write new SWC to file
            QString fileSaveName =  fileOpenName+QString("_aligned.swc");

            if (!export_list2file(nt_norm.listNeuron,fileSaveName,fileOpenName))
            {
                v3d_msg("fail to write the output swc file.");
                return;
            }
        }
       }


    else
	{
        v3d_msg(tr("This is a plugin to prun the swc file. "
			"Developed by Zhi Zhou, 2014-05-02"));
	}
}

bool pruning_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, paras, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) paras = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("pruning_nc"))
	{
        cout<<"Welcome to swc retyping plugin"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need input swc file"<<endl;
            return false;
        }

        QString  inswc_file =  infiles[0];
        int k=0;
        int length = (paras.size() >= k+1) ? atoi(paras[k]) : 5;  k++;//0;

        QString  outswc_file;
        if(!outfiles.empty())
            outswc_file = outfiles[0];
        else
            outswc_file = inswc_file + "_pruned.swc";

        cout<<"inswc_file = "<<inswc_file.toStdString().c_str()<<endl;
        cout<<"length = "<<length<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;

        NeuronTree nt;
        nt = readSWC_file(inswc_file);

        QVector<QVector<V3DLONG> > childs;


        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        V3DLONG *flag = new V3DLONG[neuronNum];

        for (V3DLONG i=0;i<neuronNum;i++)
        {
            flag[i] = 1;

            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

        QList<NeuronSWC> list = nt.listNeuron;

        for (int i=0;i<list.size();i++)
        {
            if (childs[i].size()==0)
            {
                int index_tip = 0;
                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
                {
                    parent_tip = getParent(parent_tip,nt);
                    index_tip++;
                    if(parent_tip == 1000000000)
                        break;
                }

                int type1 = list.at(parent_tip).type;
                int type2;
                if(getParent(parent_tip,nt) == 1000000000)
                    type2 = type1;
                else
                    type2 = list.at(getParent(parent_tip,nt)).type;
                int type3 = list.at(childs[parent_tip].at(1)).type;


                if(index_tip < length && (type2 != type1 || type2 != type3))
                {
                    flag[i] = -1;

                    int parent_tip = getParent(i,nt);
                    while(childs[parent_tip].size()<2)
                   {
                        flag[parent_tip] = -1;
                        parent_tip = getParent(parent_tip,nt);
                        if(parent_tip == 1000000000)
                            break;
                   }
                }

            }

        }

       //NeutronTree structure
       NeuronTree nt_prunned;
       QList <NeuronSWC> listNeuron;
       QHash <int, int>  hashNeuron;
       listNeuron.clear();
       hashNeuron.clear();

       //set node

       NeuronSWC S;
       for (int i=0;i<list.size();i++)
       {
           if(flag[i] == 1)
           {
                NeuronSWC curr = list.at(i);
                S.n 	= curr.n;
                S.type 	= curr.type;
                S.x 	= curr.x;
                S.y 	= curr.y;
                S.z 	= curr.z;
                S.r 	= curr.r;
                S.pn 	= curr.pn;
                listNeuron.append(S);
                hashNeuron.insert(S.n, listNeuron.size()-1);
           }

      }
       nt_prunned.n = -1;
       nt_prunned.on = true;
       nt_prunned.listNeuron = listNeuron;
       nt_prunned.hashNeuron = hashNeuron;

       if(flag) {delete[] flag; flag = 0;}

        if (!export_list2file(nt_prunned.listNeuron,outswc_file,inswc_file))
        {
            v3d_msg("fail to write the output swc file.");
            return false;
        }
    }else if (func_name == tr("caculate_distance"))
    {
        cout<<"Welcome to swc caculate_distance plugin"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need input swc file"<<endl;
            return false;
        }

        QString  inswc_file1 =  infiles[0];
        QString  inswc_file2 =  infiles[1];

        NeuronTree nt1 = readSWC_file(inswc_file1);
        NeuronTree nt2 = readSWC_file(inswc_file2);

        QString  outswc_file = outfiles[0];
        V3DLONG neuronNum1 = nt1.listNeuron.size();
        V3DLONG *dis1 = new V3DLONG[neuronNum1];
        V3DLONG *match1 = new V3DLONG[neuronNum1];

        V3DLONG neuronNum2 = nt2.listNeuron.size();
        V3DLONG *dis2 = new V3DLONG[neuronNum2];

        for(int i = 0; i < neuronNum1; i++)
        {
            double x1 = nt1.listNeuron.at(i).x;
            double y1 = nt1.listNeuron.at(i).y;
            double z1 = nt1.listNeuron.at(i).z;
            double dis_min = 10000000;
            for(int j = 0; j < neuronNum2; j++)
            {
                double x2 = nt2.listNeuron.at(j).x;
                double y2 = nt2.listNeuron.at(j).y;
                double z2 = nt2.listNeuron.at(j).z;
                double dis = sqrt(pow2(x1 -x2 ) + pow2(y1 - y2) + pow2(z1 - z2));
                if(dis < dis_min)
                {
                    dis1[i] = j;
                    match1[i] = dis;
                    dis_min = dis;
                }
            }

        }

        for(int j = 0; j < neuronNum2; j++)
        {
            double x2 = nt2.listNeuron.at(j).x;
            double y2 = nt2.listNeuron.at(j).y;
            double z2 = nt2.listNeuron.at(j).z;

            double dis_min = 10000000;
            for(int i = 0; i < neuronNum1; i++)
            {

                double x1 = nt1.listNeuron.at(i).x;
                double y1 = nt1.listNeuron.at(i).y;
                double z1 = nt1.listNeuron.at(i).z;
                double dis = sqrt(pow2(x1 -x2 ) + pow2(y1 - y2) + pow2(z1 - z2));
                if(dis < dis_min)
                {
                    dis2[j] = i;
                    dis_min = dis;

                }
            }
        }

        double dis_totle = 0;
        int d = 0;
        for(int i = 0; i < neuronNum1; i++)
        {
            if(dis2[dis1[i]] == i)
            {
                dis_totle = dis_totle + match1[i];
                d++;
            }

        }

        double final_distance = dis_totle/d;
        double matching_per = (double)d/((neuronNum1 + neuronNum2)/2);
        QFile saveTextFile;
        saveTextFile.setFileName(outswc_file);// add currentScanFile
        if (!saveTextFile.isOpen()){
            if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
                qDebug()<<"unable to save file!";
                return false;}     }
        QTextStream outputStream;
        outputStream.setDevice(&saveTextFile);
        outputStream << "\n";
        outputStream << final_distance << ";" << matching_per;
        saveTextFile.close();
    }else if (func_name == tr("rotation"))
    {
        cout<<"Welcome to swc rotation plugin"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need input swc file"<<endl;
            return false;
        }

        QString  inswc_file =  infiles[0];
        int k=0;
        int degree = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;//0;

        QString  outswc_file;
        if(!outfiles.empty())
            outswc_file = outfiles[0];
        else
            outswc_file = inswc_file + "_rotated.swc";

        cout<<"inswc_file = "<<inswc_file.toStdString().c_str()<<endl;
        cout<<"degree = "<<degree<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;

        NeuronTree nt;
        nt = readSWC_file(inswc_file);

        double alpha = -degree/180.0*3.141592635;
        double xc = 818, yc = 818;
        V3DLONG nx = 1637;
        V3DLONG ny = 1637;

        struct PixelPos{double x, y;};


        PixelPos e00, e01, e10, e11; //for the four corners
        e00.x = 0 - xc;
        e00.y = 0 - yc;

        e01.x = 0 - xc;
        e01.y = (ny-1) - yc;

        e10.x = (nx-1) - xc;
        e10.y = 0 - yc;

        e11.x = (nx-1) - xc;
        e11.y = (ny-1) - yc;

        double c00, c01, c10, c11;
        c00 = cos(alpha);
        c01 = sin(alpha);
        c10 = -sin(alpha);
        c11 = cos(alpha);

        PixelPos e00t, e01t, e10t, e11t; //the coordinates of the transformed corners
        e00t.x = c00*e00.x + c01*e00.y;
        e00t.y = c10*e00.x + c11*e00.y;

        e01t.x = c00*e01.x + c01*e01.y;
        e01t.y = c10*e01.x + c11*e01.y;

        e10t.x = c00*e10.x + c01*e10.y;
        e10t.y = c10*e10.x + c11*e10.y;

        e11t.x = c00*e11.x + c01*e11.y;
        e11t.y = c10*e11.x + c11*e11.y;

        double px_min = local_min(local_min(local_min(e00t.x, e01t.x), e10t.x), e11t.x);
        double px_max = local_max(local_max(local_max(e00t.x, e01t.x), e10t.x), e11t.x);
        double py_min = local_min(local_min(local_min(e00t.y, e01t.y), e10t.y), e11t.y);
        double py_max = local_max(local_max(local_max(e00t.y, e01t.y), e10t.y), e11t.y);

        double c00b, c01b, c10b, c11b;
        c00b = cos(-alpha); //c00b=(c00b<my_eps)?0:c00b; c00b=(c00b>1-my_eps)?1:c00b;
        c01b = sin(-alpha); //c01b=(c01b<my_eps)?0:c01b; c01b=(c01b>1-my_eps)?1:c01b;
        c10b = -sin(-alpha);//c10b=(c10b<my_eps)?0:c10b; c10b=(c10b>1-my_eps)?1:c10b;
        c11b = cos(-alpha); //c11b=(c11b<my_eps)?0:c11b; c11b=(c11b>1-my_eps)?1:c11b;


        QList<NeuronSWC> list = nt.listNeuron;

        //NeutronTree structure
        NeuronTree nt_rotated;
        QList <NeuronSWC> listNeuron;
        QHash <int, int>  hashNeuron;
        listNeuron.clear();
        hashNeuron.clear();

        //set node

        NeuronSWC S;
        for (int i=0;i<list.size();i++)
        {
            NeuronSWC curr = list.at(i);
            S.n 	= curr.n;
            S.type 	= curr.type;
            S.x 	= c00*(curr.x+px_min) + c01*(curr.y+py_min) + xc;
            S.y 	= c10*(curr.x+px_min) + c11*(curr.y+py_min) + yc;
            S.z 	= curr.z;
            S.r 	= curr.r;
            S.pn 	= curr.pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

        }
        nt_rotated.n = -1;
        nt_rotated.on = true;
        nt_rotated.listNeuron = listNeuron;
        nt_rotated.hashNeuron = hashNeuron;

        if (!export_list2file(nt_rotated.listNeuron,outswc_file,inswc_file))
        {
            v3d_msg("fail to write the output swc file.");
            return false;
        }
    }
	else return false;

	return true;
}

