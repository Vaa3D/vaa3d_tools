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

struct root_node
{
    V3DLONG root_x;
    V3DLONG root_y;
    V3DLONG root_z;
    V3DLONG offset_x;
    V3DLONG offset_y;
    V3DLONG offset_z;
    V3DLONG tc_index;
    int     direction; //0 for all, 1 for left, 2 for right, 3 for up, 4 for down
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

    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);
    QString fileOpenName = callback.getImageName(curwin);

    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    LandmarkList listLandmarks = callback.getLandmark(curwin);
    if(listLandmarks.count() ==0)
    {
        v3d_msg("No markers in the current image, please double check.");
        return;
    }

    int tmpx,tmpy,tmpz;
    LocationSimple tmpLocation(0,0,0);
    tmpLocation = listLandmarks.at(0);
    tmpLocation.getCoord(tmpx,tmpy,tmpz);


    QString tcfile;
    tcfile = QFileDialog::getOpenFileName(0, QObject::tr("Open TC File"),
            "",
            QObject::tr("Supported file (*.tc)"
                ));
    if(tcfile.isEmpty())
        return;

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
    head->direction = 0;

    head->next = NULL;
    walker = head;

    QString finalswcfilename = fileOpenName.append("_final.swc");


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
            QString cmd_APP2 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app2 -i %2 -p 'root.marker' 0 40 0 0 0 20").arg(getAppPath().toStdString().c_str()).arg(walker->tilename.toStdString().c_str());
            system(qPrintable(cmd_APP2));
        #else
                 v3d_msg("The OS is not Linux. Do nothing.");
                 return;
        #endif
;

        QString swcfilename = walker->tilename.append("_x%1_y%2_z%3_app2.swc").arg(S.x-1).arg(S.y-1).arg(S.z-1);
        NeuronTree nt = readSWC_file(swcfilename);
        v3d_msg(swcfilename,0);
        remove("root.marker");

        if(nt.listNeuron.empty())
        {
            walker = walker->next;
            continue;
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
            NeuronSWC curr = list.at(i);
            if (childs[i].size()==0)
            {
                if( curr.x < 20 || curr.x > 1004 || curr.y < 20 || curr.y > 1004)
                {

                    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
                    {
                        x_shift = vim.lut[ii].start_pos[0] - vim.lut[ walker->tc_index].start_pos[0];
                        y_shift = vim.lut[ii].start_pos[1] - vim.lut[ walker->tc_index].start_pos[1];
                        z_shift = vim.lut[ii].start_pos[2] - vim.lut[ walker->tc_index].start_pos[2];
                        QString curPath = QFileInfo(tcfile).path();;
                        QString curtilename = curPath.append("/").append(QString(vim.lut[ii].fn_img.c_str()));

                        if(x_shift > -1024 && x_shift < -800 && abs(y_shift) < 20 &&  ii !=walker->tc_index && curr.x < 20 && (walker->direction == 0 || walker->direction !=2))
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
                            newNode->direction = 1;

                            newNode->next = NULL;
                            walker_inside->next = newNode;
                            walker_inside = walker_inside->next;
                         }
                        else if(x_shift < 1024 && x_shift > 800 && abs(y_shift) < 20 &&  ii !=walker->tc_index && curr.x > 1004 && (walker->direction == 0 || walker->direction !=1))
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
                            newNode->direction = 2;

                            newNode->next = NULL;
                            walker_inside->next = newNode;
                            walker_inside = walker_inside->next;

                        }
                        else if(y_shift > -1024 && y_shift < -800 && abs(x_shift) < 20 &&  ii !=walker->tc_index && curr.y < 20 && (walker->direction == 0 || walker->direction !=4))
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
                            newNode->direction = 3;

                            newNode->next = NULL;
                            walker_inside->next = newNode;
                            walker_inside = walker_inside->next;

                        }
                        else if(y_shift < 1024 && y_shift > 800 && abs(x_shift) < 20 &&  ii !=walker->tc_index && curr.y >1004 && (walker->direction == 0 || walker->direction !=3))
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
                            newNode->direction = 4;

                            newNode->next = NULL;
                            walker_inside->next = newNode;
                            walker_inside = walker_inside->next;
                        }

                    }
               }

            }
        }

        walker = walker->next;
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(finalswcfilename));
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


