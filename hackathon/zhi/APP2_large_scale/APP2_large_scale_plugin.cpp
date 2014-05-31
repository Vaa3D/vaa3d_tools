/* APP2_large_scale_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-05-12 : by Zhi Zhou
 */

#include "v3d_message.h"
#include <vector>
#include "APP2_large_scale_plugin.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "my_surf_objs.h"
#include "stackutil.h"



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
    V3DLONG tc_index;
    V3DLONG ref_index;

    V3DLONG start[3];
    V3DLONG end[3];

    QString tilename;

    struct root_node* next;

};

// group images blending function
template <class SDATATYPE>
int region_groupfusing(SDATATYPE *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, unsigned char *relative1d,
                       V3DLONG vx, V3DLONG vy, V3DLONG vz, V3DLONG vc, V3DLONG rx, V3DLONG ry, V3DLONG rz, V3DLONG rc,
                       V3DLONG tile2vi_zs, V3DLONG tile2vi_ys, V3DLONG tile2vi_xs,
                       V3DLONG z_start, V3DLONG z_end, V3DLONG y_start, V3DLONG y_end, V3DLONG x_start, V3DLONG x_end, V3DLONG *start)
{

    //
    SDATATYPE *prelative = (SDATATYPE *)relative1d;

    if(x_end<x_start || y_end<y_start || z_end<z_start)
        return false;

    // update virtual image pVImg
    V3DLONG offset_volume_v = vx*vy*vz;
    V3DLONG offset_volume_r = rx*ry*rz;

    V3DLONG offset_pagesz_v = vx*vy;
    V3DLONG offset_pagesz_r = rx*ry;

    for(V3DLONG c=0; c<rc; c++)
    {
        V3DLONG o_c = c*offset_volume_v;
        V3DLONG o_r_c = c*offset_volume_r;
        for(V3DLONG k=z_start; k<z_end; k++)
        {
            V3DLONG o_k = o_c + (k-start[2])*offset_pagesz_v;
            V3DLONG o_r_k = o_r_c + (k-tile2vi_zs)*offset_pagesz_r;

            for(V3DLONG j=y_start; j<y_end; j++)
            {
                V3DLONG o_j = o_k + (j-start[1])*vx;
                V3DLONG o_r_j = o_r_k + (j-tile2vi_ys)*rx;

                for(V3DLONG i=x_start; i<x_end; i++)
                {
                    V3DLONG idx = o_j + i-start[0];
                    V3DLONG idx_r = o_r_j + (i-tile2vi_xs);

                    if(pVImg[idx])
                    {
                        pVImg[idx] = 0.5*(pVImg[idx] + prelative[idx_r]); // Avg. Intensity
                    }
                    else
                    {
                        pVImg[idx] = prelative[idx_r];
                    }
                }
            }
        }
    }

    return true;
}


void autotrace_largeScale(V3DPluginCallback2 &callback, QWidget *parent);
void save_region(V3DPluginCallback2 &callback, V3DLONG *start, V3DLONG *end, QString &tcfile,QString &region_name,
                 Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim);
NeuronTree swc_pruning(NeuronTree nt, double length);

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName,list<string> & infostring);

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


    int soma_index = -1;
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        QString curPath = QFileInfo(tcfile).path();
        QString curtilename = curPath.append("/").append(QString(vim.lut[ii].fn_img.c_str()));
        if(curtilename == fileOpenName)
        {
            soma_index = ii;
            break;
        }
    }

    if(soma_index == -1 )
    {
        printf("Can not find the selected image in the tc file!\n");
        return;
    }

    V3DLONG start[3], end[3];

    start[0] = vim.lut[soma_index].start_pos[0] - vim.min_vim[0] + tmpx - int(dialog.block_size/2);
    start[1] = vim.lut[soma_index].start_pos[1] - vim.min_vim[1] + tmpy - int(dialog.block_size/2);
    start[2] = 0;

    end[0] = start[0] + dialog.block_size - 1;
    end[1] = start[1] + dialog.block_size - 1;
    end[2] = vim.sz[2];


    struct root_node *head = new root_node[1];
    struct root_node *walker;

    QString startingpos="", tmps;
    tmps.setNum(start[0]).prepend("x"); startingpos += tmps;
    tmps.setNum(start[1]).prepend("_y"); startingpos += tmps;
    QString region_name = startingpos + ".raw";

    head->root_x = int(dialog.block_size/2);
    head->root_y = int(dialog.block_size/2);
    head->root_z = vim.lut[soma_index].start_pos[2] - vim.min_vim[2] + tmpz ;

    head->tc_index = soma_index;
    head->tilename = QFileInfo(tcfile).path().append("/").append(QString(region_name));
    head->ref_index = -1;

    head->start[0] = start[0];
    head->start[1] = start[1];
    head->start[2] = start[2];
    head->end[0] = end[0];
    head->end[1] = end[1];
    head->end[2] = end[2];


    head->next = NULL;
    walker = head;

    QElapsedTimer timer1;
    timer1.start();

    QString finalswcfilename = QFileInfo(tcfile).path().append("/").append("APP2_largescale.swc");

    int swc_type = 1;
    while(walker != NULL)
    {
        ImageMarker S;
        QList <ImageMarker> marklist;
        S.x = walker->root_x;
        S.y = walker->root_y;
        S.z = walker->root_z;

        marklist.append(S);
        writeMarker_file("root.marker",marklist);

        ifstream ifs_image(walker->tilename.toStdString().c_str());
        if(!ifs_image)
         save_region(callback,walker->start,walker->end,tcfile,walker->tilename,vim);

        QString eachtileswcfilename = walker->tilename + ".swc";
        ifstream ifs_tile(eachtileswcfilename.toStdString().c_str());
        int flag1 = 0;
        if(ifs_tile)
        {
            vector<MyMarker*> tile_out_swc = readSWC_file(eachtileswcfilename.toStdString());
            for(int j = 0; j < tile_out_swc.size(); j++)
            {
                double dis = sqrt(pow2(S.x - tile_out_swc[j]->x) + pow2(S.y - tile_out_swc[j]->y) + pow2(S.z - tile_out_swc[j]->z));
                if(dis < 30)
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

        QString swcfilename = walker->tilename + QString("_x%1_y%2_z%3_app2.swc").arg(S.x-1).arg(S.y-1).arg(S.z-1);
        NeuronTree nt = readSWC_file(swcfilename);
        //v3d_msg(swcfilename,0);
        remove("root.marker");

        if(nt.listNeuron.empty())
        {
            remove(swcfilename.toStdString().c_str());
            remove(walker->tilename.append("_ini.swc").toStdString().c_str());
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
                NeuronSWC curr = list.at(pa_tip);

                if( curr.x < 0.02* dialog.block_size || curr.x > 0.98 * dialog.block_size || curr.y < 0.02 * dialog.block_size || curr.y > 0.98*dialog.block_size)
                {

                    int flag = 0;
                    if(walker->ref_index != -1)
                    {
                        NeuronTree ref_nt = readSWC_file(finalswcfilename);
                        int shift_x = curr.x + walker->start[0];;
                        int shift_y = curr.y + walker->start[1];;
                        int shift_z = curr.z + walker->start[2];

                        for(int d = 0; d < ref_nt.listNeuron.size();d++)
                        {

                            NeuronSWC ref_curr = ref_nt.listNeuron.at(d);
                            double dis = sqrt(pow2(ref_curr.x - shift_x) + pow2(ref_curr.y - shift_y) + pow2(ref_curr.z - shift_z));
                            if(dis < 30.0)
                            {
                                flag = 1;
                                break;
                            }

                        }
                    }

                    if(flag == 1)
                        continue;

                    if(curr.x < 0.02* dialog.block_size)
                    {

                        newNode =  new root_node[1];
                        newNode->root_x =  dialog.block_size * 0.9 + curr.x;
                        newNode->root_y = curr.y;
                        newNode->root_z = curr.z;

                        newNode->start[0] = walker->start[0] - dialog.block_size * 0.9;
                        newNode->start[1] = walker->start[1];
                        newNode->start[2] = walker->start[2];
                        newNode->end[0] = newNode->start[0] + dialog.block_size - 1;;
                        newNode->end[1] = walker->end[1];
                        newNode->end[2] = walker->end[2];

                        QString startingpos="", tmps;
                        tmps.setNum(newNode->start[0]).prepend("x"); startingpos += tmps;
                        tmps.setNum(newNode->start[1]).prepend("_y"); startingpos += tmps;
                        QString region_name = startingpos + ".raw";

                        newNode->tilename = QFileInfo(tcfile).path().append("/").append(QString(region_name));
                        newNode->ref_index = walker->tc_index;

                        newNode->next = NULL;
                        walker_inside->next = newNode;
                        walker_inside = walker_inside->next;
                    }
                    else if(curr.x > 0.98 * dialog.block_size)
                    {
                        newNode =  new root_node[1];
                        newNode->root_x =  curr.x - dialog.block_size * 0.9;
                        newNode->root_y = curr.y;
                        newNode->root_z = curr.z;

                        newNode->start[0] = walker->start[0] + dialog.block_size * 0.9 + 1;
                        newNode->start[1] = walker->start[1];
                        newNode->start[2] = walker->start[2];
                        newNode->end[0] = newNode->start[0] + dialog.block_size - 1;;
                        newNode->end[1] = walker->end[1];
                        newNode->end[2] = walker->end[2];

                        QString startingpos="", tmps;
                        tmps.setNum(newNode->start[0]).prepend("x"); startingpos += tmps;
                        tmps.setNum(newNode->start[1]).prepend("_y"); startingpos += tmps;
                        QString region_name = startingpos + ".raw";

                        newNode->tilename = QFileInfo(tcfile).path().append("/").append(QString(region_name));
                        newNode->ref_index = walker->tc_index;

                        newNode->next = NULL;
                        walker_inside->next = newNode;
                        walker_inside = walker_inside->next;
                    }
                    else if(curr.y < 0.02* dialog.block_size)
                    {

                        newNode =  new root_node[1];
                        newNode->root_x = curr.x;
                        newNode->root_y = dialog.block_size * 0.9 + curr.y;
                        newNode->root_z = curr.z;

                        newNode->start[0] = walker->start[0];
                        newNode->start[1] = walker->start[1] - dialog.block_size * 0.9;
                        newNode->start[2] = walker->start[2];
                        newNode->end[0] = walker->end[0];
                        newNode->end[1] = newNode->start[1]+ dialog.block_size - 1;
                        newNode->end[2] = walker->end[2];

                        QString startingpos="", tmps;
                        tmps.setNum(newNode->start[0]).prepend("x"); startingpos += tmps;
                        tmps.setNum(newNode->start[1]).prepend("_y"); startingpos += tmps;
                        QString region_name = startingpos + ".raw";

                        newNode->tilename = QFileInfo(tcfile).path().append("/").append(QString(region_name));
                        newNode->ref_index = walker->tc_index;

                        newNode->next = NULL;
                        walker_inside->next = newNode;
                        walker_inside = walker_inside->next;
                    }
                    else if(curr.y > 0.98 * dialog.block_size)
                    {
                        newNode =  new root_node[1];
                        newNode->root_x =  curr.x;
                        newNode->root_y = curr.y - dialog.block_size * 0.9;
                        newNode->root_z = curr.z;

                        newNode->start[0] = walker->start[0];
                        newNode->start[1] = walker->start[1]  + dialog.block_size * 0.9 + 1;
                        newNode->start[2] = walker->start[2];
                        newNode->end[0] = walker->end[0];
                        newNode->end[1] = newNode->start[1] + dialog.block_size - 1;
                        newNode->end[2] = walker->end[2];

                        QString startingpos="", tmps;
                        tmps.setNum(newNode->start[0]).prepend("x"); startingpos += tmps;
                        tmps.setNum(newNode->start[1]).prepend("_y"); startingpos += tmps;
                        QString region_name = startingpos + ".raw";

                        newNode->tilename = QFileInfo(tcfile).path().append("/").append(QString(region_name));
                        newNode->ref_index = walker->tc_index;

                        newNode->next = NULL;
                        walker_inside->next = newNode;
                        walker_inside = walker_inside->next;
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
        if(walker->ref_index == -1)
        {
            if(ifs)
               remove(finalswcfilename.toStdString().c_str());
            for(int j = 0; j < temp_out_swc.size(); j++)
            {
                temp_out_swc[j]->x = temp_out_swc[j]->x + walker->start[0];
                temp_out_swc[j]->y = temp_out_swc[j]->y + walker->start[1];
                temp_out_swc[j]->z = temp_out_swc[j]->z + walker->start[2];
                temp_out_swc[j]->type = swc_type;
            }

           saveSWC_file(finalswcfilename.toStdString(), temp_out_swc);
        }
        else
        {
            vector<MyMarker*> final_out_swc = readSWC_file(finalswcfilename.toStdString());
            vector<MyMarker*> final_out_swc_updated =  final_out_swc;
            for(int j = 0; j < temp_out_swc.size(); j++)
            {
                temp_out_swc[j]->x = temp_out_swc[j]->x + walker->start[0];
                temp_out_swc[j]->y = temp_out_swc[j]->y + walker->start[1];
                temp_out_swc[j]->z = temp_out_swc[j]->z + walker->start[2];
                temp_out_swc[j]->type = swc_type;
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
         remove(walker->tilename.append("_ini.swc").toStdString().c_str());

        walker = walker->next;
        swc_type++;
    }

    //post-processing

     qint64 etime1 = timer1.elapsed();

     list<string> infostring;
     string tmpstr; QString qtstr;
     tmpstr =  qPrintable( qtstr.prepend("##Vaa3D-Neuron-APP2 for Large-scale ")); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.channel).prepend("#channel = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);

     tmpstr =  qPrintable( qtstr.setNum(dialog.length_thresh).prepend("#length_thresh = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.SR_ratio).prepend("#SR_ratio = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.is_gsdt).prepend("#is_gsdt = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.is_break_accept).prepend("#is_gap = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.cnn_type).prepend("#cnn_type = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.b_RadiusFrom2D).prepend("#b_radiusFrom2D = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(dialog.block_size).prepend("#block_size = ") ); infostring.push_back(tmpstr);
     tmpstr =  qPrintable( qtstr.setNum(etime1).prepend("#neuron preprocessing time (milliseconds) = ") ); infostring.push_back(tmpstr);

     V3DPluginArgItem arg;
     V3DPluginArgList input_sort;
     V3DPluginArgList output;

     arg.type = "random";std::vector<char*> arg_input_sort;
     std:: string fileName_Qstring(finalswcfilename.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
     arg_input_sort.push_back(fileName_string);
     arg.p = (void *) & arg_input_sort; input_sort<< arg;
     arg.type = "random";std::vector<char*> arg_sort_para;arg.p = (void *) & arg_sort_para; input_sort << arg;
     QString full_plugin_name_sort = "sort_neuron_swc";
     QString func_name_sort = "sort_swc";
     arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(fileName_string); arg.p = (void *) & arg_output; output<< arg;
     callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

     NeuronTree nt_sort = readSWC_file(finalswcfilename);
     NeuronTree nt_pruned = swc_pruning(nt_sort,5.0);

     export_list2file(nt_pruned.listNeuron, finalswcfilename, infostring);

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

void save_region(V3DPluginCallback2 &callback, V3DLONG *start, V3DLONG *end, QString &tcfile, QString &region_name,
                 Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim)
{

    //virtual image
    V3DLONG vx, vy, vz, vc;

    vx = end[0] - start[0] + 1;
    vy = end[1] - start[1] + 1;
    vz = end[2] - start[2] + 1;
    vc = vim.sz[3];

    V3DLONG pagesz_vim = vx*vy*vz*vc;

    // flu bird algorithm
    bitset<3> lut_ss, lut_se, lut_es, lut_ee;

    //
    V3DLONG x_s = start[0] + vim.min_vim[0];
    V3DLONG y_s = start[1] + vim.min_vim[1];
    V3DLONG z_s = start[2] + vim.min_vim[2];

    V3DLONG x_e = end[0] + vim.min_vim[0];
    V3DLONG y_e = end[1] + vim.min_vim[1];
    V3DLONG z_e = end[2] + vim.min_vim[2];

    bool flag_init = true;
    unsigned char *pVImg_UINT8 = NULL;

    QString curFilePath = QFileInfo(tcfile).path();
    curFilePath.append("/");

    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        // init
        lut_ss.reset();
        lut_se.reset();
        lut_es.reset();
        lut_ee.reset();

        //
        if(x_s < vim.lut[ii].start_pos[0]) lut_ss[1] = 1; // r  0 l
        if(y_s < vim.lut[ii].start_pos[1]) lut_ss[0] = 1; // d  0 u
        if(z_s < vim.lut[ii].start_pos[2]) lut_ss[2] = 1; // b  0 f

        if(x_e < vim.lut[ii].start_pos[0]) lut_se[1] = 1; // r  0 l
        if(y_e < vim.lut[ii].start_pos[1]) lut_se[0] = 1; // d  0 u
        if(z_e < vim.lut[ii].start_pos[2]) lut_se[2] = 1; // b  0 f

        if(x_s < vim.lut[ii].end_pos[0]) lut_es[1] = 1; // r  0 l
        if(y_s < vim.lut[ii].end_pos[1]) lut_es[0] = 1; // d  0 u
        if(z_s < vim.lut[ii].end_pos[2]) lut_es[2] = 1; // b  0 f

        if(x_e < vim.lut[ii].end_pos[0]) lut_ee[1] = 1; // r  0 l
        if(y_e < vim.lut[ii].end_pos[1]) lut_ee[0] = 1; // d  0 u
        if(z_e < vim.lut[ii].end_pos[2]) lut_ee[2] = 1; // b  0 f

        // copy data
        if( (!lut_ss.any() && lut_ee.any()) || (lut_es.any() && !lut_ee.any()) || (lut_ss.any() && !lut_se.any()) )
        {
            //
            cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;

            // loading relative image files
            V3DLONG *sz_relative = 0;
            int datatype_relative = 0;
            unsigned char* relative1d = 0;

            QString curPath = curFilePath;

            string fn = curPath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();

            qDebug()<<"testing..."<<curFilePath<< fn.c_str();


            if(flag_init)
            {
                    try
                    {
                        pVImg_UINT8 = new unsigned char [pagesz_vim];
                    }
                    catch (...)
                    {
                        printf("Fail to allocate memory.\n");
                        return;
                    }

                    // init
                    memset(pVImg_UINT8, 0, pagesz_vim*sizeof(unsigned char));

                    flag_init = false;

            }

            if (loadImage(const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_relative)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                continue;
            }
            V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];


            //
            V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
            V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
            V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
            V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
            V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
            V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];

            V3DLONG x_start = (start[0] > tile2vi_xs) ? start[0] : tile2vi_xs;
            V3DLONG x_end = (end[0] < tile2vi_xe) ? end[0] : tile2vi_xe;
            V3DLONG y_start = (start[1] > tile2vi_ys) ? start[1] : tile2vi_ys;
            V3DLONG y_end = (end[1] < tile2vi_ye) ? end[1] : tile2vi_ye;
            V3DLONG z_start = (start[2] > tile2vi_zs) ? start[2] : tile2vi_zs;
            V3DLONG z_end = (end[2] < tile2vi_ze) ? end[2] : tile2vi_ze;

            x_end++;
            y_end++;
            z_end++;

            //
            cout << x_start << " " << x_end << " " << y_start << " " << y_end << " " << z_start << " " << z_end << endl;
            region_groupfusing<unsigned char>(pVImg_UINT8, vim, relative1d,
                                                  vx, vy, vz, vc, rx, ry, rz, rc,
                                                  tile2vi_zs, tile2vi_ys, tile2vi_xs,
                                                  z_start, z_end, y_start, y_end, x_start, x_end, start);

            //de-alloc
            if(relative1d) {delete []relative1d; relative1d=0;}
            if(sz_relative) {delete []sz_relative; sz_relative=0;}
        }

    }

    V3DLONG in_sz[4];
    in_sz[0] = vx;
    in_sz[1] = vy;
    in_sz[2] = vz;
    in_sz[3] = vc;

    simple_saveimage_wrapper(callback, region_name.toStdString().c_str(),  (unsigned char *)pVImg_UINT8, in_sz, V3D_UINT8);
    if(pVImg_UINT8) {delete []pVImg_UINT8; pVImg_UINT8=0;}

    return;
}

NeuronTree swc_pruning(NeuronTree nt, double length)
{
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
            }
            if(index_tip < length)
            {
                flag[i] = -1;

                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
                {
                    flag[parent_tip] = -1;
                    parent_tip = getParent(parent_tip,nt);
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
   return nt_prunned;
}

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName,list<string> & infostring)
{
    ofstream ofs(fileSaveName.toStdString().c_str());

    if(ofs.fail())
    {
        cout<<"open swc file error"<<endl;
        return false;
    }
    ofs<<"#name "<<fileSaveName.toStdString()<<endl;
    ofs<<"#comment "<<endl;

    list<string>::iterator it;
    for (it=infostring.begin();it!=infostring.end(); it++)
        ofs<< *it <<endl;

    ofs<<"##n,type,x,y,z,radius,parent"<<endl;


    for (V3DLONG i=0;i<lN.size();i++)
        ofs << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    ofs.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;

}
