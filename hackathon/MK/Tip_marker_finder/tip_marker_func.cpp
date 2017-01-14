#include "../../zhi/IVSCC_sort_swc/openSWCDialog.h"
#include <vector>
#include "Tip_marker_finder_plugin.h"

#include <v3d_interface.h>
#include "v3d_message.h"
#include "tip_marker_func.h"
#include <vector>
#include <iostream>
#include "../../../released_plugins/v3d_plugins/gsdt/common_dialog.h"

using namespace std;

const QString title = QObject::tr("Border Marker Finder");

bool SpecDialog(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString fileOpenName;
    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return true;
    fileOpenName = openDlg->file_name;

    NeuronTree nt = openDlg->nt;

    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    vector<string> items;
    items.push_back("Total number of z slices in the image stack");
    items.push_back("Specify the bottom plane number of the top section to look into: ");
    items.push_back("Specify the top plane number of the bottom section to look into: ");
    CommonDialog dialog(items);
    dialog.setWindowTitle(title);
    if (dialog.exec()!=QDialog::Accepted)
        return 0;

    int top=NULL, bottom=NULL, num=NULL;
    dialog.get_num("Total number of z slices in the image stack", num);
    dialog.get_num("Specify the bottom plane number of the top section to look into: ", top);
    dialog.get_num("Specify the top plane number of the bottom section to look into: ", bottom);
    if (num == NULL || top == NULL || bottom == NULL)
    {
        v3d_msg("Parameter(s) not specified. Failed to generate marker file.");
        return 0;
    }

    QList<NeuronSWC> list = nt.listNeuron;
    QList<ImageMarker> bifur_marker;

    int z_coord;
    for (int i=0;i<list.size();i++)
    {
        ImageMarker t;
        if (childs[i].size() == 0)
        {
            if (nt.listNeuron.at(i).type == 3 || nt.listNeuron.at(i).type == 4)
            {
                z_coord = int(nt.listNeuron.at(i).z);
                if ((z_coord >= bottom) || (z_coord <= top))
                {
                    t.x = nt.listNeuron.at(i).x;
                    t.y = nt.listNeuron.at(i).y;
                    t.z = nt.listNeuron.at(i).z;
                    t.type = nt.listNeuron.at(i).type;
                    bifur_marker.append(t);
                    printf("%d\n", nt.listNeuron.at(i).type);
                }
            }
        }
    }

    QString fileSaveName =fileOpenName;
    QString markerfileName = fileSaveName+QString("_TipMarker.marker");
    writeMarker_file(markerfileName, bifur_marker);
    QString FinishMsg = QString("A marker file [") + markerfileName + QString("] has been generated.");
    v3d_msg(FinishMsg);


    return true;
}

bool SpecDialog_all(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString fileOpenName;
    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return true;
    fileOpenName = openDlg->file_name;

    NeuronTree nt = openDlg->nt;

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

    int z_coord;
    for (int i=0;i<list.size();i++)
    {
        ImageMarker t;
        if (childs[i].size() == 0)
        {
                    t.x = nt.listNeuron.at(i).x;
                    t.y = nt.listNeuron.at(i).y;
                    t.z = nt.listNeuron.at(i).z;
                    t.type = nt.listNeuron.at(i).type;
                    bifur_marker.append(t);
        }
    }

    QString fileSaveName =fileOpenName;
    QString markerfileName = fileSaveName+QString("_TipMarker_all.marker");
    writeMarker_file(markerfileName, bifur_marker);
    QString FinishMsg = QString("A marker file [") + markerfileName + QString("] has been generated.");
    v3d_msg(FinishMsg);
    return true;
}




