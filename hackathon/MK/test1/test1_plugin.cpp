/* test1_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-11-8 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "test1_plugin.h"
#include "../../zhi/IVSCC_sort_swc/openSWCDialog.h"
#include "../../zhi/IVSCC_radius_estimation/IVSCC_radius_estimation_plugin.h"


using namespace std;
Q_EXPORT_PLUGIN2(test, TestPlugin)

QStringList TestPlugin::menulist() const
{
    return QStringList()
        <<tr("percentage")
        <<tr("about");
}

QStringList TestPlugin::funclist() const
{
    return QStringList()
        <<tr("TipMarker_swc")
        <<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("percentage"))
    {
        QString fileOpenName;
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;
        fileOpenName = openDlg->file_name;
        double percentage = 0;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            bool ok;
            nt = openDlg->nt;
            percentage = QInputDialog::getDouble(parent, "Please specify the resampling step length","Percentage:",1,0,2147483647,0.1,&ok);
            if (!ok)
                return;
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
        QList<ImageMarker> bifur_marker;

        double zMax = nt.listNeuron.at(0).z;
        for (int i=1; i<list.size(); i++)
        {
            if (nt.listNeuron.at(i).z > zMax)
            {
                zMax = nt.listNeuron.at(i).z;
            }
        }
        double percentile = (1-percentage/100)*zMax;

        for (int i=0;i<list.size();i++)
        {
            ImageMarker t;
            if (childs[i].size() == 0)
            {
                if (nt.listNeuron.at(i).z >= percentile)
                {
                    t.x = nt.listNeuron.at(i).x;
                    t.y = nt.listNeuron.at(i).y;
                    t.z = nt.listNeuron.at(i).z;
                    bifur_marker.append(t);
                }
            }
        }
        QString fileSaveName =fileOpenName;
        QString markerfileName = fileSaveName+QString("_TipMarker.marker");
        writeMarker_file(markerfileName, bifur_marker);

    }
    else
    {
        v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by MK, 2016-11-8"));
    }
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("func1"))
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

