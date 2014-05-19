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
    QString refSWCname;

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
    //struct root_node *temp;

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

    QString finalswcfilename = fileOpenName.append("_final.swc");

    QElapsedTimer timer1;
    timer1.start();

    while(walker != NULL)
    {
        ImageMarker S;
        QList <ImageMarker> marklist;
        S.x = walker->root_x;
        S.y = walker->root_y;
        S.z = walker->root_z;

        marklist.append(S);
        writeMarker_file("root.marker",marklist);
        #if  defined(Q_OS_LINUX)
            QString cmd_APP2 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app2 -i %2 -p 'root.marker' %3 %4 %5 %6 %7 %8 %9").arg(getAppPath().toStdString().c_str()).arg(walker->tilename.toStdString().c_str())
                    .arg(dialog.channel-1).arg(dialog.bkg_thresh).arg(dialog.b_256cube).arg(dialog.b_RadiusFrom2D).arg(dialog.is_gsdt).arg(dialog.is_break_accept).arg(dialog.length_thresh);
            system(qPrintable(cmd_APP2));
        #else
                 v3d_msg("The OS is not Linux. Do nothing.");
                 return;
        #endif

        if(S.z < 1) S.z = 1;

        QString swcfilename = walker->tilename.append("_x%1_y%2_z%3_app2.swc").arg(S.x-1).arg(S.y-1).arg(S.z-1);
        NeuronTree nt = readSWC_file(swcfilename);
        v3d_msg(swcfilename,0);
        remove("root.marker");

        if(nt.listNeuron.empty())
        {
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
                      //  int x_shift_ref = vim.lut[walker->ref_index].start_pos[0] - vim.lut[ walker->tc_index].start_pos[0];
                      //  int y_shift_ref = vim.lut[walker->ref_index].start_pos[1] - vim.lut[ walker->tc_index].start_pos[1];
                     //   int z_shift_ref = vim.lut[walker->ref_index].start_pos[2] - vim.lut[ walker->tc_index].start_pos[2];

                       // NeuronTree ref_nt = readSWC_file(walker->refSWCname);
                        NeuronTree ref_nt = readSWC_file(finalswcfilename);
                        for(int d = 0; d < ref_nt.listNeuron.size();d++)
                        {

                            NeuronSWC ref_curr = ref_nt.listNeuron.at(d);
                          //  int ref_x = ref_curr.x + x_shift_ref;
                          //  int ref_y = ref_curr.y + y_shift_ref;
                          //  int ref_z = ref_curr.z + z_shift_ref;

                            int ref_x = ref_curr.x - walker->offset_x;
                            int ref_y = ref_curr.y - walker->offset_y;
                            int ref_z = ref_curr.z - walker->offset_z;


                            double dis = sqrt(pow((ref_x - curr.x),2.0) + pow((ref_y - curr.y),2.0) + pow((ref_z - curr.z),2.0));
                            if(dis < 20.0)
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
                        QString curPath = QFileInfo(tcfile).path();;
                        QString curtilename = curPath.append("/").append(QString(vim.lut[ii].fn_img.c_str()));

                        if(x_shift > -dialog.image->getXDim() && x_shift < -0.8*dialog.image->getXDim() && abs(y_shift) < 0.15 * dialog.image->getYDim() &&  ii !=walker->tc_index && curr.x < 0.02* dialog.image->getXDim())
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
                            newNode->refSWCname = swcfilename;
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
                            newNode->offset_z  = walker->offset_z + z_shift;;

                            newNode->tilename = curtilename;
                            newNode->tc_index = ii;
                            newNode->refSWCname = swcfilename;
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
                            newNode->refSWCname = swcfilename;
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
                            newNode->refSWCname = swcfilename;
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
        ifstream ifs(finalswcfilename.toStdString().c_str());

        if(!ifs)
           saveSWC_file(finalswcfilename.toStdString(), temp_out_swc);
        else
        {
            vector<MyMarker*> final_out_swc = readSWC_file(finalswcfilename.toStdString());
            vector<MyMarker*> temp_out_swc = readSWC_file(swcfilename.toStdString());

            for(int j = 0; j < temp_out_swc.size(); j++)
            {
                temp_out_swc[j]->x = temp_out_swc[j]->x + walker->offset_x;
                temp_out_swc[j]->y = temp_out_swc[j]->y + walker->offset_y;
                temp_out_swc[j]->z = temp_out_swc[j]->z + walker->offset_z;
                final_out_swc.push_back(temp_out_swc[j]);
            }
           saveSWC_file(finalswcfilename.toStdString(), final_out_swc);

        }



        v3d_msg(swcfilename,0);

        walker = walker->next;
    }
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


