/* make_segments_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-4-15 : by Yongzhang
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <iostream>
#include <map>
#include <stack>
#include "my_surf_objs.h"
#include "swc_utils.h"

#include "make_segments_plugin.h"
Q_EXPORT_PLUGIN2(make_segments, make_segments_Plugin);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

bool swc_to_segments(vector<MyMarker*> & inmarkers, vector<NeuronSegment*> &tree,input_PARA &PARA);
 
QStringList make_segments_Plugin::menulist() const
{
	return QStringList() 
        <<tr("swc_to_segments")
		<<tr("about");
}

QStringList make_segments_Plugin::funclist() const
{
	return QStringList()
        <<tr("swc_to_segments")
		<<tr("help");
}

void make_segments_Plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("APP2_swc_to_segments"))
	{
        bool bmenu = true;
        input_PARA PARA;

        QString APP2_path = PARA.inimg_file + "_APP2_ported.swc";
        vector<MyMarker *> APP2 = readSWC_file(APP2_path.toStdString());
        vector<NeuronSegment*> tree;
        cout << "over" << endl;

        swc_to_segments(APP2,tree,PARA);

	}
    else if (menu_name == tr("snake_swc_to_segments"))
    {
        bool bmenu = true;
        input_PARA PARA;

        QString snake_path = PARA.inimg_file + "_snake_tracing.swc";
        vector<MyMarker*> snake = readSWC_file(snake_path.toStdString());
        vector<NeuronSegment*> tree;

        swc_to_segments(snake,tree,PARA);

    }
    else if (menu_name == tr("neuTube_swc_to_segments"))
    {
        bool bmenu = true;
        input_PARA PARA;

        QString neuTube_path = PARA.inimg_file + "_neutube.swc";
        vector<MyMarker*> neuTube = readSWC_file(neuTube_path.toStdString());
        vector<NeuronSegment*> tree;

        swc_to_segments(neuTube,tree,PARA);

    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by Yongzhang, 2018-4-15"));
	}
}

bool make_segments_Plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    bool bmenu = false;
    input_PARA PARA;

    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

    if(infiles.empty())
    {
        fprintf (stderr, "Need input image. \n");
        return false;
    }
    else
        PARA.inimg_file = infiles[0];
    int k=0;
    PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;

    //make APP2/snake/neutube
    cout<<"Example: vaa3d -x APP2_ported -f tracing_func -i <inimg_file> -o APP2.eswc -p <channel> <other parameters>\n"<<endl;
    V3DPluginArgItem arg;
    V3DPluginArgList inputs;
    V3DPluginArgList outputs;
    QString result = ".swc";

    arg.type = "random";
    vector<char*> arg_inputs;
    QString files_name = PARA.inimg_file;
    string fileName_Qstring(files_name.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());

    arg_inputs.push_back(fileName_string);
    arg.p = (void *) & arg_inputs;
    inputs<< arg;

    arg.type = "random";
    vector<char*> arg_para;
    arg_para.push_back("1");
    arg.p = (void *) & arg_para;
    inputs << arg;

    arg.type = "random";vector<char*> arg_outputs;
    string fileName_Qstring2(result.toStdString());char* fileName_string2 =  new char[fileName_Qstring2.length() + 1]; strcpy(fileName_string2, fileName_Qstring2.c_str());
    arg_outputs.push_back(fileName_string2);
    arg.p = (void *) & arg_outputs;
    outputs<< arg;

    if (func_name == tr("APP2_swc_to_segments"))
    {
        QString full_plugin_name = "APP2_ported";
        QString func_name = "tracing_func";
        callback.callPluginFunc(full_plugin_name,func_name,inputs,outputs);

        QString APP2_path = PARA.inimg_file + "_APP2_ported.swc";
        string name = APP2_path.toStdString();
        vector<MyMarker*> APP2 = readSWC_file(name);
        vector<NeuronSegment*> tree;
        cout << "over" << endl;

        swc_to_segments(APP2,tree,PARA);

    }

    else if (func_name == tr("snake_swc_to_segments"))
    {
        QString full_plugin_name = "snake_tracing";
        QString func_name = "snake_trace";
        callback.callPluginFunc(full_plugin_name,func_name,inputs,outputs);

        QString snake_path = PARA.inimg_file + "_snake_tracing.swc";
        string name = snake_path.toStdString();
        vector<MyMarker*> snake = readSWC_file(name);
        vector<NeuronSegment*> tree;
        cout << "over" << endl;

        swc_to_segments(snake,tree,PARA);

    }
    else if (func_name == tr("neuTube_swc_to_segments"))
    {
        QString full_plugin_name = "neuTube";
        QString func_name = "neutube_trace";
        callback.callPluginFunc(full_plugin_name,func_name,inputs,outputs);

        QString neuTube_path = PARA.inimg_file + "_neutube.swc";
        string name = neuTube_path.toStdString();
        vector<MyMarker*> neuTube = readSWC_file(name);
        vector<NeuronSegment*> tree;
        cout << "over" << endl;

        swc_to_segments(neuTube,tree,PARA);

    }
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


        printf("**** Usage of make_segments tracing **** \n");
        printf("vaa3d -x make_segments -f APP2_swc_to_segments -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

    }
    else return false;

    return true;
}

// convert to post_order tree
bool swc_to_segments(vector<MyMarker*> & inmarkers, vector<NeuronSegment*> &tree,input_PARA &PARA)
{
    map<MyMarker *, int>  childs_num;
    getLeaf_markers(inmarkers, childs_num);

    map<MyMarker*, NeuronSegment*> marker_seg_map;
    MyMarker * root_marker;

    for (int i=0;i<inmarkers.size();i++)
    {
        MyMarker * marker = inmarkers[i];
        if (!marker || !marker->parent || marker->parent==marker || childs_num[marker] != 1) //tip, branch or root points
        {
            if ( !marker->parent || marker->parent==marker)
                root_marker = marker;
            NeuronSegment * seg = new NeuronSegment;
            marker_seg_map[marker] = seg;
        }
    }
    for (map<MyMarker*, NeuronSegment*>::iterator it = marker_seg_map.begin(); it != marker_seg_map.end(); it++)
    {
        MyMarker * marker = (*it).first;
        MyMarker * p = marker;
        do
        {
            marker_seg_map[marker]->markers.push_back(p);
            p = p->parent;
        }
        while (p && marker_seg_map.find(p)==marker_seg_map.end());
        if (!p) continue;
        NeuronSegment * seg_par = marker_seg_map[p];
        seg_par->child_list.push_back(marker_seg_map[marker]);
    }

    stack<NeuronSegment*> seg_stack;
    map<NeuronSegment*, bool> map_visit;
    for (map<MyMarker*, NeuronSegment*>::iterator it = marker_seg_map.begin(); it != marker_seg_map.end(); it++)
        map_visit[(*it).second] = false;
    seg_stack.push(marker_seg_map[root_marker]);
    while (!seg_stack.empty())
    {
        NeuronSegment * cur = seg_stack.top();
        bool all_childs_visited = true;
        for (int i=0;i<cur->child_list.size();i++)
        {
            if (!map_visit[cur->child_list[i]])
            {
                seg_stack.push(cur->child_list[i]);
                all_childs_visited = false;
            }
        }
        if (all_childs_visited)
        {
            map_visit[cur] = true;
            tree.push_back(cur);
            seg_stack.pop();
        }
    }
    cout<<"tree seg num = "<<tree.size()<<endl;

    QList<NeuronSWC> seg_result;
    int swc_id = 1;
    for(int i = 0; i < tree.size(); i++)
    {
        for(int j=0; j< tree[i]->markers.size();j++)
        {
            NeuronSWC cur;
            cur.n = swc_id;
            cur.x = tree[i]->markers[j]->x;
            cur.y = tree[i]->markers[j]->y;
            cur.z = tree[i]->markers[j]->z;
            cur.type=tree[i]->markers[j]->type;
            cur.radius=tree[i]->markers[j]->radius;
            if(j==0)
                cur.pn =-1;
            else
                cur.pn =swc_id-1;
            seg_result.push_back(cur);
            swc_id++;

        }

    }
    NeuronTree seg_nt;
    seg_nt.listNeuron = seg_result;

    QString filename = PARA.inimg_file + "_segments.swc";
    //QString filename = "segments.swc";
    writeSWC_file(filename,seg_nt);

    return true;
}

