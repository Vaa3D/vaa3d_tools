#include "n_class.h"

bool Comp(const int &a,const int &b)
{
    return a>b;
}

bool Branch::get_r_points_of_branch(vector<NeuronSWC> &r_points, NeuronTree &nt)
{
    NeuronSWC tmp=end_point;
    r_points.push_back(end_point);
    while(tmp.n!=head_point.n)
    {
        tmp=nt.listNeuron[nt.hashNeuron.value(tmp.parent)];
        r_points.push_back(tmp);
    }
    return true;
}

bool Branch::get_points_of_branch(vector<NeuronSWC> &points, NeuronTree &nt)
{
    vector<NeuronSWC> r_points;
    this->get_r_points_of_branch(r_points,nt);
    while(!r_points.empty())
    {
        NeuronSWC tmp=r_points.back();
        r_points.pop_back();
        points.push_back(tmp);
    }

    return true;
}

bool SwcTree::initialize(NeuronTree t)
{
    nt.deepCopy(t);

    NeuronSWC ori;

    V3DLONG num_p=nt.listNeuron.size();
    vector<vector<V3DLONG> > children=vector<vector<V3DLONG> >(num_p,vector<V3DLONG>());
    for(V3DLONG i=0;i<num_p;++i)
    {
        V3DLONG par=nt.listNeuron[i].parent;
        if(par<0)
        {
            ori=nt.listNeuron[i];
            continue;
        }
        children[nt.hashNeuron.value(par)].push_back(i);
    }

    vector<NeuronSWC> queue;
    queue.push_back(ori);

    //initial head_point,end_point,distance,length
    cout<<"initial head_point,end_point,distance,length"<<endl;
    while(!queue.empty())
    {
        NeuronSWC tmp=queue.front();
        queue.erase(queue.begin());
        for(int i=0;i<children[nt.hashNeuron.value(tmp.n)].size();++i)
        {
            Branch branch;
            branch.head_point=tmp;
            NeuronSWC child=nt.listNeuron[children[nt.hashNeuron.value(branch.head_point.n)][i]];
            branch.length+=distance_two_point(tmp,child);
            Angle near_point_angle = Angle(child.x-tmp.x,child.y-tmp.y,child.z-tmp.z);
            near_point_angle.norm_angle();
            double sum_angle = 0;
            while(children[nt.hashNeuron.value(child.n)].size()==1)
            {
                NeuronSWC par=child;
                child=nt.listNeuron[children[nt.hashNeuron.value(par.n)][0]];
                branch.length+=distance_two_point(par,child);
                Angle next_point_angle = Angle(child.x-par.x,child.y-par.y,child.z-par.z);
                next_point_angle.norm_angle();
                sum_angle += acos(near_point_angle*next_point_angle);
            }
            if(children[nt.hashNeuron.value(child.n)].size()>=1)
            {
                queue.push_back(child);
            }
            branch.end_point=child;
            branch.distance=branch.get_distance();
            branch.sum_angle = sum_angle;
            branchs.push_back(branch);
        }
    }

    //initial head_angle,end_angle
    cout<<"initial head_angle,end_angle"<<endl;
    for(int i=0;i<branchs.size();++i)
    {
        vector<NeuronSWC> points;
        branchs[i].get_points_of_branch(points,nt);
        double length=0;
        NeuronSWC par0=points[0];
        for(int j=1;j<points.size();++j)
        {
            NeuronSWC child0=points[j];
            length+=distance_two_point(par0,child0);
            if(length>5)
            {
                branchs[i].head_angle.x=child0.x-points[0].x;
                branchs[i].head_angle.y=child0.y-points[0].y;
                branchs[i].head_angle.z=child0.z-points[0].z;

                double d=norm_v(branchs[i].head_angle);

                branchs[i].head_angle.x/=d;
                branchs[i].head_angle.y/=d;
                branchs[i].head_angle.z/=d;

                break;
            }
            par0=child0;
        }
        if(length<=5)
        {
            branchs[i].head_angle.x=par0.x-points[0].x;
            branchs[i].head_angle.y=par0.y-points[0].y;
            branchs[i].head_angle.z=par0.z-points[0].z;

            double d=norm_v(branchs[i].head_angle);

            branchs[i].head_angle.x/=d;
            branchs[i].head_angle.y/=d;
            branchs[i].head_angle.z/=d;
        }

        length=0;
        points.clear();
        branchs[i].get_r_points_of_branch(points,nt);
        NeuronSWC child1=points[0];
        for(int j=1;j<points.size();++j)
        {
            NeuronSWC par1=points[j];
            length+=distance_two_point(par1,child1);
            if(length>5)
            {
                branchs[i].end_angle.x=par1.x-points[0].x;
                branchs[i].end_angle.y=par1.y-points[0].y;
                branchs[i].end_angle.z=par1.z-points[0].z;

                double d=norm_v(branchs[i].end_angle);

                branchs[i].end_angle.x/=d;
                branchs[i].end_angle.y/=d;
                branchs[i].end_angle.z/=d;

                break;
            }
            child1=par1;
        }
        if(length<=5)
        {
            branchs[i].end_angle.x=child1.x-points[0].x;
            branchs[i].end_angle.y=child1.y-points[0].y;
            branchs[i].end_angle.z=child1.z-points[0].z;

            double d=norm_v(branchs[i].end_angle);

            branchs[i].end_angle.x/=d;
            branchs[i].end_angle.y/=d;
            branchs[i].end_angle.z/=d;
        }
    }

    //initial parent
    cout<<"initial parent"<<endl;
    for(int i=0;i<branchs.size();++i)
    {
        if(branchs[i].head_point.parent<0)
        {
            branchs[i].parent=0;
        }
        else
        {
            for(int j=0;j<branchs.size();++j)
            {
                if(branchs[i].head_point==branchs[j].end_point)
                {
                    branchs[i].parent=&branchs[j];
                }
            }
        }
    }

    //initial level
    for(int i=0;i<branchs.size();++i)
    {
        Branch* tmp;
        tmp=&branchs[i];
        int level=0;
        while(tmp->parent!=0)
        {
            level++;
            tmp=tmp->parent;
        }
        branchs[i].level=level;
    }

    return true;
}

bool SwcTree::get_level_index(vector<int> &level_index,int level)
{
    for(int i=0;i<branchs.size();++i)
    {
        if(branchs[i].level==level)
        {
            level_index.push_back(i);
        }
    }
    return true;
}


bool SwcTree::get_points_of_branchs(vector<Branch> &b,vector<NeuronSWC> &points,NeuronTree &ntb)
{
    for(int i=0;i<(b.size()-1);++i)
    {
        b[i].get_points_of_branch(points,ntb);
        points.pop_back();
    }
    b[(b.size()-1)].get_points_of_branch(points,ntb);
    return true;
}


int SwcTree::get_max_level()
{
    int max_level;
    for(int i=0;i<branchs.size();++i)
    {
        max_level=(max_level>branchs[i].level)?max_level:branchs[i].level;
    }
    return max_level;
}

bool SwcTree::get_bifurcation_image(QString dir, int resolutionX, int resolutionY, int resolutionZ, bool all, QString braindir, V3DPluginCallback2 &callback)
{
    int num_b = branchs.size();

    map<Branch,int> mapbranch;
    for(int i=0; i<num_b ;++i)
    {
        mapbranch[branchs[i]] = i;
    }

    vector<vector<int> > branch_children=vector<vector<int> >(num_b,vector<int>());

    for(int i=0; i<num_b; ++i)
    {
        if(branchs[i].parent==0) continue;
        branch_children[mapbranch[*(branchs[i].parent)]].push_back(i);
    }

    vector<int> queue;
    get_level_index(queue,0);

    int num = 0;

    while(!queue.empty()){
        int bi = queue.front();//branch_index
        queue.erase(queue.begin());
        if(branch_children[bi].size()>0){
            int level = branchs[bi].level;
            num++;
            if(all || branchs[bi].level>2){
                QString filename = dir+"/b_level"+QString::number(level)
                        +"_x"+QString::number(branchs[bi].end_point.x)
                        +"_y"+QString::number(branchs[bi].end_point.y)
                        +"_z"+QString::number(branchs[bi].end_point.z)+QString::number(num)+".tif";
                get3DImageBasedPoint(filename.toStdString().c_str(),branchs[bi].end_point,resolutionX,resolutionY,resolutionZ,braindir,callback);
            }
            for(int i=0; i<branch_children[bi].size(); ++i){
                queue.push_back(branch_children[bi][i]);
            }
        }
    }

    return true;

}

bool SwcTree::get_un_bifurcation_image(QString dir, int resolutionX, int resolutionY, int resolutionZ, bool all, QString braindir, V3DPluginCallback2 &callback)
{
    const int distance_to_bifurcation = 20;
    int num_b = branchs.size();
    int num = 0;
    for(int i=0; i<num_b; ++i){
        if(all||branchs[i].length<200){
            if(branchs[i].length>2*distance_to_bifurcation){
                vector<NeuronSWC> points;
                branchs[i].get_points_of_branch(points,nt);
                int level = branchs[i].level;
                int num_p = points.size();
                if(num_p>10){
                    int k;
                    if(all)
                        k = 1;
                    else
                        k = 3;
                    for(int j=5; j<num_p-5; j+=k){
                        if(distance_two_point(points[j],points[0])>distance_to_bifurcation
                                && distance_two_point(points[j],points[num_p-1])>distance_to_bifurcation)
                        {
                            num++;
                            QString filename = dir+"/level"+QString::number(level)+"_j"+QString::number(j)+"_"
                                    +QString::number(num)+"_x"+QString::number(points[j].x)
                                    +"_y"+QString::number(points[j].y)+"_z"+QString::number(points[j].z)+".tif";
                            get3DImageBasedPoint(filename.toStdString().c_str(),points[j],resolutionX,resolutionY,resolutionZ,braindir,callback);
                        }
                    }
                }
            }
        }
    }
    return true;
}


