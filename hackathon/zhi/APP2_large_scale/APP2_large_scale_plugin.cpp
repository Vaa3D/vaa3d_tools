/* APP2_large_scale_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-05-12 : by Zhi Zhou
 */

#include "v3d_message.h"
#include <vector>
#include "APP2_large_scale_plugin.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "my_surf_objs.h"


using namespace std;
Q_EXPORT_PLUGIN2(APP2_large_scale, APP2_large_scale);
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

template <class T> T pow2(T a)
{
    return a*a;

}
struct root_node
{
    V3DLONG root_x;
    V3DLONG root_y;
    V3DLONG root_z;
    V3DLONG offset_x;
    V3DLONG offset_y;
    V3DLONG offset_z;
    V3DLONG tc_index;
    V3DLONG ref_index;
    QString tilename;

    struct root_node* next;

};

void autotrace_largeScale(V3DPluginCallback2 &callback, QWidget *parent);
QString getAppPath();

QStringList APP2_large_scale::menulist() const
{
    return QStringList()
        <<tr("trace")
        <<tr("about");
}

QStringList APP2_large_scale::funclist() const
{
    return QStringList()
        <<tr("func1")
        <<tr("func2")
        <<tr("help");
}

void APP2_large_scale::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace"))
    {
        autotrace_largeScale(callback,parent);
    }
    else
    {
        v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by Zhi Zhou, 2014-05-12"));
    }
}

bool APP2_large_scale::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void autotrace_largeScale(V3DPluginCallback2 &callback, QWidget *parent)
{
    APP2largeScaleDialog dialog(callback, parent);

    if (!dialog.image)
        return;


    if(dialog.listLandmarks.count() ==0)
        return;

    if (dialog.exec()!=QDialog::Accepted)
        return;

    QString fileOpenName = dialog.image->getFileName();

    QString tcfile = dialog.tcfilename;
    if(tcfile.isEmpty())
    {
        v3d_msg("Please select the tc file.");
        return;
    }


    int tmpx,tmpy,tmpz;
    LocationSimple tmpLocation(0,0,0);
    tmpLocation = dialog.listLandmarks.at(0);
    tmpLocation.getCoord(tmpx,tmpy,tmpz);

    Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

    if( !vim.y_load(tcfile.toStdString()) )
    {
        printf("Wrong stitching configuration file to be load!\n");
        return;
    }


    int some_index = -1;
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        QString curPath = QFileInfo(tcfile).path();;
        QString curtilename = curPath.append("/").append(QString(vim.lut[ii].fn_img.c_str()));
        if(curtilename == fileOpenName)
        {
            some_index = ii;
            break;
        }
    }

    if(some_index == -1 )
    {
        printf("Can not find the selected image in the tc file!\n");
        return;
    }


    struct root_node *head = new root_node[1];
    struct root_node *walker;

    head->root_x = tmpx;
    head->root_y = tmpy;
    head->root_z = tmpz;
    head->offset_x = 0;
    head->offset_y = 0;
    head->offset_z = 0;

    head->tc_index = some_index;
    head->tilename = fileOpenName;
    head->ref_index = -1;

    head->next = NULL;
    walker = head;

    QElapsedTimer timer1;
    timer1.start();

    QString finalswcfilename = fileOpenName.append("_final.swc");


    while(walker != NULL)
    {
        ImageMarker S;
        QList <ImageMarker> marklist;
        S.x = walker->root_x;
        S.y = walker->root_y;
        S.z = walker->root_z;

        QString eachtileswcfilename = QFileInfo(tcfile).path().append("/").append(QString(vim.lut[walker->tc_index].fn_img.c_str())).append(".swc");
        ifstream ifs_tile(eachtileswcfilename.toStdString().c_str());
        int flag1 = 0;
        if(ifs_tile)
        {
            vector<MyMarker*> tile_out_swc = readSWC_file(eachtileswcfilename.toStdString());
            for(int j = 0; j < tile_out_swc.size(); j++)
            {
                double dis = sqrt(pow2(S.x - tile_out_swc[j]->x) + pow2(S.y - tile_out_swc[j]->y) + pow2(S.z - tile_out_swc[j]->z));
                if(dis < 100)
                {
                    flag1 = 1;
                    break;
                }

            }

        }

        if(flag1)
        {
            walker = walker->next;
            continue;
        }

        marklist.append(S);
        writeMarker_file("root.marker",marklist);
        #if  defined(Q_OS_LINUX)
            QString cmd_APP2 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app2 -i %2 -p 'root.marker' %3 %4 %5 %6 %7 %8 %9").arg(getAppPath().toStdString().c_str()).arg(walker->tilename.toStdString().c_str())
                    .arg(dialog.channel-1).arg(dialog.bkg_thresh).arg(dialog.b_256cube).arg(dialog.b_RadiusFrom2D).arg(dialog.is_gsdt).arg(dialog.is_break_accept).arg(dialog.length_thresh);
            system(qPrintable(cmd_APP2));
        #elif defined(Q_OS_MAC)
            QString cmd_APP2 = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x Vaa3D_Neuron2 -f app2 -i %2 -p 'root.marker' %3 %4 %5 %6 %7 %8 %9").arg(getAppPath().toStdString().c_str()).arg(walker->tilename.toStdString().c_str())
                    .arg(dialog.channel-1).arg(dialog.bkg_thresh).arg(dialog.b_256cube).arg(dialog.b_RadiusFrom2D).arg(dialog.is_gsdt).arg(dialog.is_break_accept).arg(dialog.length_thresh);
            system(qPrintable(cmd_APP2));
        #else
                 v3d_msg("The OS is not Linux or Mac. Do nothing.");
                 return;
        #endif

        if(S.z < 1) S.z = 1;

        QString swcfilename = walker->tilename.append("_x%1_y%2_z%3_app2.swc").arg(S.x-1).arg(S.y-1).arg(S.z-1);
        NeuronTree nt = readSWC_file(swcfilename);
        v3d_msg(swcfilename,0);
        remove("root.marker");

        if(nt.listNeuron.empty())
        {
            remove(swcfilename.toStdString().c_str());
            remove(QFileInfo(tcfile).path().append("/").append(QString(vim.lut[walker->tc_index].fn_img.c_str())).append("_ini.swc").toStdString().c_str());
            walker = walker->next;

            continue;
        }

        QVector<QVector<V3DLONG> > childs;
        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

        QList<NeuronSWC> list = nt.listNeuron;
        int x_shift, y_shift, z_shift;

        struct root_node *walker_inside;
        struct root_node *newNode;
        walker_inside = head;
        while(walker_inside->next != NULL)
        {
             walker_inside = walker_inside->next;
        }

        for (int i=0;i<list.size();i++)
        {

            if (childs[i].size()==0)
            {
                int pa_tip = getParent(i,nt);
                NeuronSWC curr = list.at(getParent(pa_tip,nt));

                if( curr.x < 0.02* dialog.image->getXDim() || curr.x > 0.98 * dialog.image->getXDim() || curr.y < 0.02 * dialog.image->getYDim() || curr.y > 0.98*dialog.image->getYDim())
                {

                    int flag = 0;
                    if(walker->ref_index != -1)
                    {
                        NeuronTree ref_nt = readSWC_file(finalswcfilename);
                        for(int d = 0; d < ref_nt.listNeuron.size();d++)
                        {

                            NeuronSWC ref_curr = ref_nt.listNeuron.at(d);
                            int ref_x = ref_curr.x - walker->offset_x;
                            int ref_y = ref_curr.y - walker->offset_y;
                            int ref_z = ref_curr.z - walker->offset_z;


                            double dis = sqrt(pow2(ref_x - curr.x) + pow2(ref_y - curr.y) + pow2(ref_z - curr.z));
                            if(dis < 30.0)
                            {
                                flag = 1;
                                break;
                            }

                        }
                    }

                    if(flag == 1)
                        continue;

                    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
                    {
                        x_shift = vim.lut[ii].start_pos[0] - vim.lut[ walker->tc_index].start_pos[0];
                        y_shift = vim.lut[ii].start_pos[1] - vim.lut[ walker->tc_index].start_pos[1];
                        z_shift = vim.lut[ii].start_pos[2] - vim.lut[ walker->tc_index].start_pos[2];
                        QString curPath = QFileInfo(tcfile).path();
                        QString curtilename = curPath.append("/").append(QString(vim.lut[ii].fn_img.c_str()));

                        if(x_shift > -dialog.image->getXDim() && x_shift < -0.8*dialog.image->getXDim() && abs(y_shift) < 0.15 * dialog.image->getYDim() &&  ii !=walker->tc_index && curr.x < 0.02* dialog.image->getXDim())
                        {

                            newNode =  new root_node[1];
                            newNode->root_x =  curr.x - x_shift;
                            newNode->root_y = curr.y - y_shift;
                            newNode->root_z = curr.z - z_shift;

                            newNode->offset_x  = walker->offset_x + x_shift;
                            newNode->offset_y  = walker->offset_y + y_shift;
                            newNode->offset_z  = walker->offset_z + z_shift;

                            newNode->tilename = curtilename;
                            newNode->tc_index = ii;
                            newNode->ref_index = walker->tc_index;

                            newNode->next = NULL;
                            walker_inside->next = newNode;
                            walker_inside = walker_inside->next;
                         }
                        else if(x_shift < dialog.image->getXDim() && x_shift > 0.8*dialog.image->getXDim() && abs(y_shift) < 0.15 * dialog.image->getYDim() &&  ii !=walker->tc_index && curr.x > 0.98* dialog.image->getXDim())
                        {

                            newNode =  new root_node[1];
                            newNode->root_x =  curr.x - x_shift;
                            newNode->root_y = curr.y - y_shift;
                            newNode->root_z = curr.z - z_shift ;


                            newNode->offset_x  = walker->offset_x + x_shift;
                            newNode->offset_y  = walker->offset_y + y_shift;
                            newNode->offset_z  = walker->offset_z + z_shift;

                            newNode->tilename = curtilename;
                            newNode->tc_index = ii;
                            newNode->ref_index = walker->tc_index;


                            newNode->next = NULL;
                            walker_inside->next = newNode;
                            walker_inside = walker_inside->next;

                        }
                        else if(y_shift > -dialog.image->getYDim() && y_shift < -0.8*dialog.image->getYDim() && abs(x_shift) < 0.15 * dialog.image->getXDim() &&  ii !=walker->tc_index && curr.y < 0.02* dialog.image->getYDim())
                        {

                            newNode =  new root_node[1];
                            newNode->root_x =  curr.x - x_shift;
                            newNode->root_y = curr.y - y_shift;
                            newNode->root_z = curr.z - z_shift ;


                            newNode->offset_x  = walker->offset_x + x_shift;
                            newNode->offset_y  = walker->offset_y + y_shift;
                            newNode->offset_z  = walker->offset_z + z_shift;

                            newNode->tilename = curtilename;
                            newNode->tc_index = ii;
                            newNode->ref_index = walker->tc_index;



                            newNode->next = NULL;
                            walker_inside->next = newNode;
                            walker_inside = walker_inside->next;

                        }
                        else if(y_shift < dialog.image->getYDim() && y_shift > 0.8*dialog.image->getXDim() && abs(x_shift) < 0.15 * dialog.image->getXDim() &&  ii !=walker->tc_index && curr.y >0.98* dialog.image->getYDim())
                        {
                            newNode =  new root_node[1];
                            newNode->root_x =  curr.x - x_shift;
                            newNode->root_y = curr.y - y_shift;
                            newNode->root_z = curr.z - z_shift ;

                            newNode->offset_x  = walker->offset_x + x_shift;
                            newNode->offset_y  = walker->offset_y + y_shift;
                            newNode->offset_z  = walker->offset_z + z_shift;

                            newNode->tilename = curtilename;
                            newNode->tc_index = ii;
                            newNode->ref_index = walker->tc_index;



                            newNode->next = NULL;
                            walker_inside->next = newNode;
                            walker_inside = walker_inside->next;
                        }

                    }
               }

            }
        }


        vector<MyMarker*> temp_out_swc = readSWC_file(swcfilename.toStdString());
        if(!ifs_tile)
            saveSWC_file(eachtileswcfilename.toStdString(), temp_out_swc);
        else
        {
            vector<MyMarker*> tile_out_swc = readSWC_file(eachtileswcfilename.toStdString());
            for(int j = 0; j < temp_out_swc.size(); j++)
            {
                temp_out_swc[j]->x = temp_out_swc[j]->x;
                temp_out_swc[j]->y = temp_out_swc[j]->y;
                temp_out_swc[j]->z = temp_out_swc[j]->z;
                tile_out_swc.push_back(temp_out_swc[j]);
            }
             saveSWC_file(eachtileswcfilename.toStdString(), tile_out_swc);
        }


        ifstream ifs(finalswcfilename.toStdString().c_str());
        if(!ifs)
           saveSWC_file(finalswcfilename.toStdString(), temp_out_swc);
        else
        {
            vector<MyMarker*> final_out_swc = readSWC_file(finalswcfilename.toStdString());
            vector<MyMarker*> final_out_swc_updated =  final_out_swc;
            for(int j = 0; j < temp_out_swc.size(); j++)
            {
                temp_out_swc[j]->x = temp_out_swc[j]->x + walker->offset_x;
                temp_out_swc[j]->y = temp_out_swc[j]->y + walker->offset_y;
                temp_out_swc[j]->z = temp_out_swc[j]->z + walker->offset_z;
                int flag_prun = 0;
                for(int jj = 0; jj < final_out_swc.size();jj++)
                {
                    int dis_prun = sqrt(pow2(temp_out_swc[j]->x - final_out_swc[jj]->x) + pow2(temp_out_swc[j]->y - final_out_swc[jj]->y) + pow2(temp_out_swc[j]->z - final_out_swc[jj]->z));
                    if( (temp_out_swc[j]->radius + final_out_swc[jj]->radius - dis_prun)/dis_prun > 0.5)
                    {
                        flag_prun = 1;
                        break;
                    }

                }
                if(flag_prun == 0)
                {
                   final_out_swc_updated.push_back(temp_out_swc[j]);
                }
            }
           saveSWC_file(finalswcfilename.toStdString(), final_out_swc_updated);
        }

        remove(swcfilename.toStdString().c_str());
        remove(QFileInfo(tcfile).path().append("/").append(QString(vim.lut[walker->tc_index].fn_img.c_str())).append("_ini.swc").toStdString().c_str());

        walker = walker->next;
    }

    //post-processing

    qint64 etime1 = timer1.elapsed();



    v3d_msg(QString("The tracing uses %1 ms. Now you can drag and drop the generated swc fle [%2] into Vaa3D.").arg(etime1).arg(finalswcfilename));
      return;
}

QString getAppPath()
{
    QString v3dAppPath("~/Work/v3d_external/v3d");
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    v3dAppPath = testPluginsDir.absolutePath();
    return v3dAppPath;
}


