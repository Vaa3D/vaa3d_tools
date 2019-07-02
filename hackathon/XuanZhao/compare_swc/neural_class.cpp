#include "neural_class.h"
#include <algorithm>
#include <map>

#define PI 3.14159266

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
    //r_points.push_back(head_point);
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
    vector<vector<V3DLONG>> children=vector<vector<V3DLONG>>(num_p,vector<V3DLONG>());
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
            //branch.head_point=new NeuronSWC;
            branch.head_point=tmp;
            NeuronSWC child=nt.listNeuron[children[nt.hashNeuron.value(branch.head_point.n)][i]];
            branch.length+=distance_two_point(tmp,child);
            while(children[nt.hashNeuron.value(child.n)].size()==1)
            {
                NeuronSWC par=child;
                child=nt.listNeuron[children[nt.hashNeuron.value(par.n)][0]];
                branch.length+=distance_two_point(par,child);
            }
            if(children[nt.hashNeuron.value(child.n)].size()>=2)
            {
                queue.push_back(child);
            }
            //branch.end_point=new NeuronSWC;
            branch.end_point=child;
            branch.distance=branch.get_distance();
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
                branchs[i].head_angle.x_angle=x_angle_two_point(points[0],child0);
                branchs[i].head_angle.z_angle=z_angle_two_point(points[0],child0);
                break;
            }
            par0=child0;
        }
        if(length<=5)
        {
            branchs[i].head_angle.x_angle=x_angle_two_point(points[0],par0);
            branchs[i].head_angle.z_angle=z_angle_two_point(points[0],par0);
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
                branchs[i].end_angle.x_angle=x_angle_two_point(par1,points[0]);
                branchs[i].end_angle.z_angle=z_angle_two_point(par1,points[0]);
                break;
            }
            child1=par1;
        }
        if(length<=5)
        {
            branchs[i].end_angle.x_angle=x_angle_two_point(child1,points[0]);
            branchs[i].end_angle.z_angle=z_angle_two_point(child1,points[0]);
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
                    //branchs[i].parent=new Branch;
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

bool SwcTree::branchs_to_nt()
{
    NeuronTree nt_tmp;
    NeuronSWC tmp;
    for(V3DLONG i=0;i<nt.listNeuron.size();++i)
    {
        if(nt.listNeuron[i].parent<0)
        {
            tmp=nt.listNeuron[i];
            break;
        }
    }
    nt_tmp.listNeuron.push_back(tmp);
    for(int i=0;i<branchs.size();++i)
    {
        vector<NeuronSWC> points;
        points.clear();
        branchs[i].get_points_of_branch(points,nt);
        for(int j=1;j<points.size();++j)
        {
            nt_tmp.listNeuron.push_back(points[j]);
        }
    }
    nt.deepCopy(nt_tmp);
    nt_tmp.listNeuron.clear();
    return true;
}

bool SwcTree::cut_cross()
{
    V3DLONG num_p=nt.listNeuron.size();
    vector<vector<V3DLONG>> children=vector<vector<V3DLONG>>(num_p,vector<V3DLONG>());
    for(V3DLONG i=0;i<num_p;++i)
    {
        V3DLONG par=nt.listNeuron[i].parent;
        if(par<0)
        {
            continue;
        }
        children[nt.hashNeuron.value(par)].push_back(i);
    }

    cout<<"initial delete_branch......"<<endl;
    int b_num=branchs.size();
    vector<int> delete_branch;

    for(int i=0;i<b_num;++i)
    {
        if(branchs[i].parent!=0
                &&children[nt.hashNeuron.value(branchs[i].end_point.n)].size()>=2
                &&branchs[i].length<20)
        {
            double x_angle_sum=0,z_angle_sum=0;
            int count=0;
            for(int j=0;j<b_num;++j)
            {
                //double x_angle=0,z_angle=0;
                NeuronSWC p1,p2,p;
                if(branchs[i].end_point==branchs[j].head_point)
                {
                    vector<NeuronSWC> points;
                    branchs[j].get_points_of_branch(points,nt);
                    double length=0;
                    NeuronSWC par0=points[0];
                    for(int k=1;k<points.size();++k)
                    {
                        NeuronSWC child0=points[k];
                        length+=distance_two_point(par0,child0);
                        if(length>30)
                        {
                            /*
                            x_angle=x_angle_two_point(points[0],child0);
                            z_angle=z_angle_two_point(points[0],child0);
                            break;
                            */
                            p=child0;
                            break;
                        }
                        par0=child0;
                    }
                    if(length<=30)
                    {
                        //x_angle=x_angle_two_point(points[0],par0);
                        //z_angle=z_angle_two_point(points[0],par0);
                        p=par0;
                    }
                    count++;
                }
                if(count==1)
                {
                    //x_angle_sum+=x_angle;
                    //z_angle_sum+=z_angle;
                    p1=p;
                }
                if(count==2)
                {
                    //x_angle_sum-=x_angle;
                    //z_angle_sum-=z_angle;
                    //x_angle_sum=abs(x_angle_sum);
                    //z_angle_sum=abs(z_angle_sum);
                    p2=p;
                    x_angle_sum=angle_three_point(branchs[i].end_point,p1,p2);
                    break;
                }
            }
            if(x_angle_sum>2.5/*&&z_angle_sum>3*/)
            {
                delete_branch.push_back(i);
            }
        }
    }

    cout<<"cross......."<<endl;
    NeuronTree nttmp;
    for(int i=0;i<delete_branch.size();++i)
    {
        cout<<delete_branch[i]<<endl;
        vector<NeuronSWC> a;
        branchs[delete_branch[i]].get_points_of_branch(a,nt);
        for(int j=0;j<a.size();++j)
        {
            a[j].type=2;
            nttmp.listNeuron.push_back(a[j]);
        }
    }
    writeESWC_file("D://2.eswc",nttmp);
    cout<<"cross end........."<<endl;

    cout<<"initial delete_queue......."<<endl;

    vector<int> delete_queue;
    for(int i=0;i<delete_branch.size();++i)
    {
        if(children[nt.hashNeuron.value(branchs[delete_branch[i]].end_point.n)].size()>=2)
        {
            delete_queue.push_back(delete_branch[i]);
        }
    }

    while(!delete_queue.empty())
    {
        int delete_index=delete_queue.front();
        delete_queue.erase(delete_queue.begin());
        for(int i=0;i<branchs.size();++i)
        {
            if(branchs[delete_index].end_point==branchs[i].head_point)
            {
                delete_branch.push_back(i);
                if(children[nt.hashNeuron.value(branchs[i].end_point.n)].size()>=2)
                {
                    delete_queue.push_back(i);
                }
            }
        }
    }

    cout<<"delete children...."<<endl;


    sort(delete_branch.begin(),delete_branch.end());
    vector<int>::iterator iter_r=unique(delete_branch.begin(),delete_branch.end());
    delete_branch.erase(iter_r,delete_branch.end());
    sort(delete_branch.begin(),delete_branch.end(),Comp);

    cout<<"deleting......"<<endl;

    for(int i=0;i<delete_branch.size();++i)
    {
        cout<<delete_branch[i]<<endl;
        vector<Branch>::iterator iter=branchs.begin()+delete_branch[i];
        branchs.erase(iter);
    }

    return true;

}


bool SwcTree::find_big_turn()
{
    NeuronTree nttmp;

    for(int i=0;i<branchs.size();++i)
    {
        vector<NeuronSWC> r_points;
        r_points.clear();
        branchs[i].get_r_points_of_branch(r_points,nt);

        int num_p=r_points.size();
        cout<<"num_p:"<<num_p<<endl;
        //NeuronSWC child=r_points[0];
        vector<double> angles=vector<double>(num_p-2,0);
        double max_angle=0;

        for(int j=1;j<num_p-1;++j)
        {
            //NeuronSWC par=r_points[j];
            //NeuronSWC p_par=r_points[j+1];
            angles[j-1]=PI-angle_three_point(r_points[j],r_points[j-1],r_points[j+1]);

            max_angle=(max_angle>angles[j-1])?max_angle:angles[j-1];

            //cout<<"index: "<<j<<" "<<angles[j-1]<<endl;

            if(angles[j-1]>1)
            {
                for(int k=0;k<j+1;++k)
                {
                    nttmp.listNeuron.push_back(r_points[k]);
                }
            }


            //child=par;
        }


        cout<<i<<":"<<"max_angle: "<<max_angle<<endl;
        cout<<endl;

    }

    for(int i=0;i<nttmp.listNeuron.size();++i)
    {
        nttmp.listNeuron[i].type=2;
    }

    writeESWC_file("D://3.eswc",nttmp);



    return true;
}











