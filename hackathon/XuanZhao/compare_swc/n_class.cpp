#include "n_class.h"
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
                //branchs[i].head_angle.x_angle=x_angle_two_point(points[0],child0);
                //branchs[i].head_angle.z_angle=z_angle_two_point(points[0],child0);

                branchs[i].head_angle.x=child0.x-points[0].x;
                branchs[i].head_angle.y=child0.y-points[0].y;
                branchs[i].head_angle.z=child0.z-points[0].z;

                //double d=sqrt(branchs[i].head_angle.x*branchs[i].head_angle.x+branchs[i].head_angle.y*branchs[i].head_angle.y+branchs[i].head_angle.z*branchs[i].head_angle.z);
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

            //double d=sqrt(branchs[i].head_angle.x*branchs[i].head_angle.x+branchs[i].head_angle.y*branchs[i].head_angle.y+branchs[i].head_angle.z*branchs[i].head_angle.z);
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

                //double d=sqrt(branchs[i].end_angle.x*branchs[i].end_angle.x+branchs[i].end_angle.y*branchs[i].end_angle.y+branchs[i].end_angle.z*branchs[i].end_angle.z);
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

            //double d=sqrt(branchs[i].end_angle.x*branchs[i].end_angle.x+branchs[i].end_angle.y*branchs[i].end_angle.y+branchs[i].end_angle.z*branchs[i].end_angle.z);
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

    int size=branchs.size();

    map<Branch,int> mapbranch;
    for(int i=0;i<size;++i)
    {
        mapbranch[branchs[i]]=i;
    }


    vector<vector<int>> children_b=vector<vector<int>>(size,vector<int>());

    for(int i=0;i<size;++i)
    {
        if(branchs[i].parent==0) continue;
        children_b[mapbranch[*(branchs[i].parent)]].push_back(i);
    }

    vector<int> level0_index;
    this->get_level_index(level0_index,0);
    vector<int> queue_t;
    for(int i=0;i<level0_index.size();++i)
    {
        queue_t.push_back(level0_index[i]);

//        Trunk t;
//        int par_index=level0_index[i];
//        t.branch_index.push_back(par_index);
//        double length=branchs[par_index].length;
//        t.length.push_back(length);
//        t.head_angle=branchs[par_index].head_angle;
//        while(children[par_index].size()!=0)
//        {
//            int child0_index=children[par_index].at(0);
//            int child1_index=children[par_index].at(1);
//            double pc_dot0=branchs[par_index].end_angle*branchs[child0_index].head_angle;
//            double pc_dot1=branchs[par_index].end_angle*branchs[child1_index].head_angle;
//            if(pc_dot0>pc_dot1)
//            {
//                t.branch_index.push_back(child0_index);
//                length+=branchs[child0_index].length;
//                t.length.push_back(length);
//                queue.push_back(child1_index);
//                par_index=child0_index;
//            }
//            else
//            {
//                t.branch_index.push_back(child1_index);
//                length+=branchs[child1_index].length;
//                t.length.push_back(length);
//                queue.push_back(child0_index);
//                par_index=child1_index;
//            }
//        }
//        t.end_angle=branchs[par_index].end_angle;
//        trunks.push_back(t);
    }

    while(!queue_t.empty())
    {
        Trunk t;
        int par_index=queue_t.front();
        queue_t.erase(queue_t.begin());
        t.branch_index.push_back(par_index);
        double length=branchs[par_index].length;
        t.length.push_back(length);
        t.head_angle=branchs[par_index].head_angle;
        while(children_b[par_index].size()!=0)
        {
            int child0_index=children_b[par_index].at(0);
            int child1_index=children_b[par_index].at(1);
            double pc_dot0=branchs[par_index].end_angle*branchs[child0_index].head_angle;
            double pc_dot1=branchs[par_index].end_angle*branchs[child1_index].head_angle;
            if(pc_dot0<pc_dot1)
            {
                t.branch_index.push_back(child0_index);
                length+=branchs[child0_index].length;
                t.length.push_back(length);
                queue_t.push_back(child1_index);
                par_index=child0_index;
            }
            else
            {
                t.branch_index.push_back(child1_index);
                length+=branchs[child1_index].length;
                t.length.push_back(length);
                queue_t.push_back(child0_index);
                par_index=child1_index;
            }
        }
        t.end_angle=branchs[par_index].end_angle;
        trunks.push_back(t);
    }

    for(int i=0;i<trunks.size();++i)
    {
        trunks[i].n=i;
        if(branchs[trunks[i].branch_index[0]].parent==0)
        {
            trunks[i].parent=-1;
            continue;
        }
        int par_index=mapbranch[*branchs[trunks[i].branch_index[0]].parent];
        for(int j=0;j<trunks.size();++j)
        {
            bool stop=false;
            for(int k=0;k<trunks[j].branch_index.size();++k)
            {
                if(par_index==trunks[j].branch_index[k])
                {
                    stop=true;
                    trunks[i].parent=j;
                    break;
                }
            }
            if(stop)
                break;
        }
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





bool Swc_Compare::get_sub_image(QString dir, vector<int> &a_false, vector<int> &b_false,SwcTree &a,SwcTree &b, QString braindir, V3DPluginCallback2 &callback)
{
    int a_size=a_false.size();
    int b_size=b_false.size();
    if(a_size!=b_size)
        return false;


    for(int i=0;i<a_size;++i)
    {
        NeuronTree nttmp_a,nttmp_b;
        vector<NeuronSWC> tmp_a,tmp_b;
//        a.branchs[a_false[i]].get_points_of_branch(tmp_a,a.nt);
//        b.branchs[b_false[i]].get_points_of_branch(tmp_b,b.nt);

        int a_branch_size=a.trunks[a_false[i]].branch_index.size();

        for(int j=0;j<a_branch_size-1;++j)
        {
            int a_branch_index=a.trunks[a_false[i]].branch_index[j];
            a.branchs[a_branch_index].get_points_of_branch(tmp_a,a.nt);
            tmp_a.pop_back();
        }
        int a_branch_index=a.trunks[a_false[i]].branch_index[a_branch_size-1];
        a.branchs[a_branch_index].get_points_of_branch(tmp_a,a.nt);

        int b_branch_size=b.trunks[b_false[i]].branch_index.size();

        for(int j=0;j<b_branch_size-1;++j)
        {
            int b_branch_index=b.trunks[b_false[i]].branch_index[j];
            b.branchs[b_branch_index].get_points_of_branch(tmp_b,b.nt);
            tmp_b.pop_back();
        }
        int b_branch_index=b.trunks[b_false[i]].branch_index[b_branch_size-1];
        b.branchs[b_branch_index].get_points_of_branch(tmp_b,b.nt);

        size_t x0=IN,x1=0,y0=IN,y1=0,z0=IN,z1=0;
        for(int j=0;j<tmp_a.size();++j)
        {
            tmp_a[j].type=3;

            size_t tmpx=(size_t)tmp_a[j].x;
            size_t tmpy=(size_t)tmp_a[j].y;
            size_t tmpz=(size_t)tmp_a[j].z;
            x0=(tmpx>x0)?x0:tmpx;
            x1=(tmpx>x1)?tmpx:x1;
            y0=(tmpy>y0)?y0:tmpy;
            y1=(tmpy>y1)?tmpy:y1;
            z0=(tmpz>z0)?z0:tmpz;
            z1=(tmpz>z1)?tmpz:z1;
        }
        for(int j=0;j<tmp_b.size();++j)
        {
            tmp_b[j].type=2;
            size_t tmpx=(size_t)tmp_b[j].x;
            size_t tmpy=(size_t)tmp_b[j].y;
            size_t tmpz=(size_t)tmp_b[j].z;
            x0=(tmpx>x0)?x0:tmpx;
            x1=(tmpx>x1)?tmpx:x1;
            y0=(tmpy>y0)?y0:tmpy;
            y1=(tmpy>y1)?tmpy:y1;
            z0=(tmpz>z0)?z0:tmpz;
            z1=(tmpz>z1)?tmpz:z1;
        }

        x0-=30;
        x1+=30;
        y0-=30;
        y1+=30;
        z0-=30;
        z1+=30;

        for(int j=0;j<tmp_a.size();++j)
        {
            tmp_a[j].x-=x0;
            tmp_a[j].y-=y0;
            tmp_a[j].z-=z0;
            nttmp_a.listNeuron.push_back(tmp_a[j]);
        }

        for(int j=0;j<tmp_b.size();++j)
        {
            tmp_b[j].x-=x0;
            tmp_b[j].y-=y0;
            tmp_b[j].z-=z0;
            nttmp_b.listNeuron.push_back(tmp_b[j]);
        }

        cout<<"x0: "<<x0<<" x1: "<<x1<<" y0: "<<y0<<" y1: "<<y1<<" z0: "<<z0<<" z1: "<<z1<<endl;

        unsigned char* pdata=0;
        pdata=callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1,y0,y1,z0,z1);

        V3DLONG sz[4]={(x1-x0),(y1-y0),(z1-z0),1};
        int datatype=1;
        QString tif_filename=dir+"/"+QString::number(i,10)+".tif";
        QString a_eswc_filename=dir+"/"+QString::number(i,10)+"manual.eswc";
        QString b_eswc_filename=dir+"/"+QString::number(i,10)+"auto.eswc";

        simple_saveimage_wrapper(callback,tif_filename.toStdString().c_str(),pdata,sz,datatype);
        writeESWC_file(a_eswc_filename,nttmp_a);
        writeESWC_file(b_eswc_filename,nttmp_b);
    }


}

double Swc_Compare::get_distance_two_branch(Branch &a, Branch &b, NeuronTree &nta, NeuronTree &ntb)
{
    vector<NeuronSWC> branch_a,branch_b;
    a.get_points_of_branch(branch_a,nta);
    b.get_points_of_branch(branch_b,ntb);

    double d_btob=0;
    for(int i=0;i<branch_b.size();++i)
    {
        double d_tp=IN;
        for(int j=0;j<branch_a.size()-1;++j)
        {
            double tmp=distance_two_point(branch_b[i],branch_a[j])+distance_two_point(branch_b[i],branch_b[j+1]);
            if(d_tp>tmp)
            {
                d_tp=tmp;
            }
            else
            {
                d_btob+=p_to_line<NeuronSWC,Angle>(branch_b[i],branch_a[j],branch_a[j+1]);
                break;
            }
        }
    }
    return (d_btob/branch_b.size());
}

double Swc_Compare::get_distance_two_branchs(vector<Branch> &a, vector<Branch> &b, NeuronTree &nta, NeuronTree &ntb)
{
    cout<<"in distance..."<<endl;
    vector<NeuronSWC> branchs_a,branchs_b;
    cout<<"a size:"<<a.size()<<endl;
    cout<<"b size:"<<b.size()<<endl;
    for(int i=0;i<(a.size()-1);++i)
    {
        a[i].get_points_of_branch(branchs_a,nta);
        branchs_a.pop_back();
    }

    a[(a.size()-1)].get_points_of_branch(branchs_a,nta);

    cout<<"a finish..."<<endl;

    for(int i=0;i<(b.size()-1);++i)
    {
        b[i].get_points_of_branch(branchs_b,ntb);
        branchs_b.pop_back();
    }
    b[(b.size()-1)].get_points_of_branch(branchs_b,ntb);

    cout<<"b finish..."<<endl;

    double d_bs_to_bs=0;
    for(int i=0;i<branchs_b.size();++i)
    {
        double min_d=IN;
        int min_index;
        for(int j=0;j<branchs_a.size()-1;++j)
        {
            double tmp_d=distance_two_point(branchs_b[i],branchs_a[j])+distance_two_point(branchs_b[i],branchs_a[j+1]);
            if(tmp_d<min_d)
            {
                min_index=j;
                min_d=tmp_d;
            }
        }
        d_bs_to_bs+=p_to_line<NeuronSWC,Angle>(branchs_b[i],branchs_a[min_index],branchs_a[min_index+1]);
    }

    cout<<"d finish..."<<endl;

    return (d_bs_to_bs/branchs_b.size());

}

int Swc_Compare::get_corresponding_branch(Branch &a,vector<int> &b_index,SwcTree &b,vector<bool> &flag_b)
{
    double thres=0.5;
    double max_s=0;
    int index;
    for(int i=0;i<b_index.size();++i)
    {
        if(flag_b[b_index[i]]==true) continue;
        double tmp=a.head_angle*b.branchs[b_index[i]].head_angle;
        if(max_s<tmp)
        {
            max_s=tmp;
            index=b_index[i];
        }
    }
    cout<<"max_s: "<<max_s<<endl;
    if(max_s>thres)
    {
        flag_b[index]=true;
        return index;
    }
    else
        return -1;
}

bool Swc_Compare::compare_two_swc(SwcTree &a, SwcTree &b, vector<int> &a_false, vector<int> &b_false,vector<int> &a_more,vector<int> &b_more,NeuronTree &nta,NeuronTree &ntb)
{


    cout<<"in compare..."<<endl;

    int a_branch_size=a.branchs.size();
    int b_branch_size=b.branchs.size();

    map<Branch,int> map_a,map_b;

    for(int i=0;i<a_branch_size;++i)
    {
        map_a[a.branchs[i]]=i;
    }
    for(int i=0;i<b_branch_size;++i)
    {
        map_b[b.branchs[i]]=i;
    }

    cout<<"end map..."<<endl;

    vector<vector<int>> children_a=vector<vector<int>>(a_branch_size,vector<int>());
    vector<vector<int>> children_b=vector<vector<int>>(b_branch_size,vector<int>());

    for(int i=0;i<a_branch_size;++i)
    {
        if(a.branchs[i].parent==0) continue;
        children_a[map_a[*(a.branchs[i].parent)]].push_back(i);
    }
    for(int i=0;i<b_branch_size;++i)
    {
        if(b.branchs[i].parent==0) continue;
        children_b[map_b[*(b.branchs[i].parent)]].push_back(i);
    }

    cout<<"end children..."<<endl;

    vector<int> a_level0_index,b_level0_index,a_level1_index,b_level1_index;
    a.get_level_index(a_level0_index,0);
    b.get_level_index(b_level0_index,0);
    a.get_level_index(a_level1_index,1);
    b.get_level_index(b_level1_index,1);

    vector<bool> flag_b=vector<bool>(b_branch_size,false);
    vector<bool> flag_a=vector<bool>(a_branch_size,false);

    vector<int> queue;

    for(int i=0;i<b_level0_index.size();++i)
    {

        int b_branch_index=b_level0_index[i];
        cout<<"b_branch_index: "<<b_branch_index<<endl;
        cout<<"start corresponding..."<<i<<endl;
        double d=this->get_distance_for_manual(b.branchs[b_branch_index],nta,ntb);
        if(d<20||b.branchs[b_branch_index].length<15)
        {
            for(int j=0;j<children_b[b_branch_index].size();++j)
            {
                int child_index=children_b[b_branch_index].at(j);

                cout<<"children index: "<<child_index<<endl;
                queue.push_back(child_index);
            }
        }
        else
        {
            b_more.push_back(b_branch_index);
        }
    }
    int count=1;
    while(!queue.empty())
    {

        cout<<"count: "<<count<<endl;
        count++;
        int b_branch_index=queue.front();
        queue.erase(queue.begin());
        //cout<<"start corresponding..."<<i<<endl;
        double d=this->get_distance_for_manual(b.branchs[b_branch_index],nta,ntb);
        if(d<20||b.branchs[b_branch_index].length<15)
        {
            for(int j=0;j<children_b[b_branch_index].size();++j)
            {
                int child_index=children_b[b_branch_index].at(j);

                cout<<"children index: "<<child_index<<endl;
                queue.push_back(child_index);
            }
        }
        else
        {
            b_more.push_back(b_branch_index);
        }
    }

    queue.clear();

    for(int i=0;i<a_level0_index.size();++i)
    {
        int a_branch_index=a_level0_index[i];
        cout<<"start corresponding..."<<i<<endl;
        double d=this->get_distance_for_manual(a.branchs[a_branch_index],ntb,nta);
        if(d<5||a.branchs[a_branch_index].length<10)
        {
            for(int j=0;j<children_a[a_branch_index].size();++j)
            {
                int child_index=children_a[a_branch_index].at(j);
                queue.push_back(child_index);
            }
        }
        else
        {
            a_more.push_back(a_branch_index);
        }
    }

    while(!queue.empty())
    {
        int a_branch_index=queue.front();
        queue.erase(queue.begin());
        double d=this->get_distance_for_manual(a.branchs[a_branch_index],ntb,nta);
        if(d<5||a.branchs[a_branch_index].length<10)
        {
            for(int j=0;j<children_a[a_branch_index].size();++j)
            {
                int child_index=children_a[a_branch_index].at(j);
                queue.push_back(child_index);
            }
        }
        else
        {
            a_more.push_back(a_branch_index);
        }
    }




//    cout<<"in compare..."<<endl;
//    double th=3;
//    int a_size=a.branchs.size();
//    int b_size=b.branchs.size();

//    map<Branch,int> mapa,mapb;
//    for(int i=0;i<a_size;++i)
//    {
//        mapa[a.branchs[i]]=i;
//    }
//    for(int i=0;i<b_size;++i)
//    {
//        mapb[b.branchs[i]]=i;
//    }

//    cout<<"end map..."<<endl;

//    vector<vector<int>> children_a=vector<vector<int>>(a_size,vector<int>());
//    vector<vector<int>> children_b=vector<vector<int>>(b_size,vector<int>());

//    for(int i=0;i<a_size;++i)
//    {
//        if(a.branchs[i].parent==0) continue;
//        children_a[mapa[*(a.branchs[i].parent)]].push_back(i);
//    }
//    for(int i=0;i<b_size;++i)
//    {
//        if(b.branchs[i].parent==0) continue;
//        children_b[mapb[*(b.branchs[i].parent)]].push_back(i);
//    }

//    cout<<"end children..."<<endl;

//    vector<int> a_level0_index,b_level0_index;
//    a.get_level0_index(a_level0_index);
//    b.get_level0_index(b_level0_index);

//    cout<<"a_level0_size:"<<a_level0_index.size()<<endl;
//    cout<<"b_level0_size:"<<b_level0_index.size()<<endl;

//    cout<<"end level0..."<<endl;

//    vector<bool> flag_a=vector<bool>(a_size,false);
//    vector<bool> flag_b=vector<bool>(b_size,false);

//    map<int,int> map_a_b;
//    vector<int> queue;
//    for(int i=0;i<a_level0_index.size();++i)
//    {
//        cout<<"for "<<i<<": "<<endl;
//        int b_index=this->get_corresponding_branch(a.branchs[a_level0_index[i]],b_level0_index,b,flag_b);
//        cout<<"end get branch..."<<endl;
//        if(b_index==-1)
//        {
//            a_more.push_back(a_level0_index[i]);
//        }
//        else
//        {
//            flag_a[a_level0_index[i]]=true;
//            map_a_b[a_level0_index[i]]=b_index;
////            double d=this->get_distance_two_branch(a.branchs[a_level0_index[i]],b.branchs[b_index],nta,ntb);
////            if(d>th)
////            {
////                a_false.push_back(a_level0_index[i]);
////                b_false.push_back(b_index);
////            }
////            else
////            {
////                queue.push_back(a_level0_index[i]);
////            }

//            double l_a=a.branchs[a_level0_index[i]].length;
//            double l_b=b.branchs[b_index].length;
//            double l_a_sum=l_a;
//            double l_b_sum=l_b;

//            while(l_b_sum<l_a_sum)
//            {
//                for(int j=0;j<children_b[b_index].size();++j)
//                {
//                    int b_child_index=children_b[b_index].at(j);
//                    double tmp=b.branchs[b_index].end_angle*b.branchs[b_child_index].head_angle;

//                    if(tmp>0.9)
//                    {
//                        map_a_b[a_level0_index[i]]=b_child_index;
//                    }

//                }
//            }
//        }
//        cout<<"end find..."<<endl;
//    }

//    cout<<"end initialize..."<<endl;

//    for(int i=0;i<b_level0_index.size();++i)
//    {
//        if(flag_b[b_level0_index[i]]==true)
//            continue;
//        else
//        {
//            b_more.push_back(b_level0_index[i]);
//        }

//    }

//    while(!queue.empty())
//    {
//        int a_index=queue.front();
//        queue.erase(queue.begin());

//        int b_index=map_a_b[a_index];
//        vector<int> b_children_indexs;
//        for(int i=0;i<children_b[b_index].size();++i)
//        {
//            b_children_indexs.push_back(children_b[b_index].at(i));
//        }

//        for(int i=0;i<children_a[a_index].size();++i)
//        {
//            int a_child_index=children_a[a_index].at(i);
//            int b_child_index=this->get_corresponding_branch(a.branchs[a_child_index],b_children_indexs,b,flag_b);
//            if(b_child_index==-1)
//            {
//                a_more.push_back(a_child_index);
//            }
//            else
//            {
//                flag_a[a_child_index]=true;
//                map_a_b[a_child_index]=b_child_index;
//                double d=this->get_distance_two_branch(a.branchs[a_child_index],b.branchs[b_child_index],nta,ntb);
//                if(d>th)
//                {
//                    a_false.push_back(a_child_index);
//                    b_false.push_back(b_child_index);
//                }
//                else
//                {
//                    queue.push_back(a_child_index);
//                }
//            }

//        }



//        for(int i=0;i<b_children_indexs.size();++i)
//        {
//            if(flag_b[b_children_indexs[i]]==true)
//                continue;
//            else
//                b_more.push_back(b_children_indexs[i]);
//        }

//    }

//    cout<<"end compare..."<<endl;

    return true;

}

bool Swc_Compare::compare_two_swc(SwcTree &a, SwcTree &b, vector<int> &a_trunk, vector<int> &b_trunk, vector<int> &a_more, vector<int> &b_more, NeuronTree &nta, NeuronTree &ntb, int mode)
{
    cout<<"in compare..."<<endl;
    double th=3;
    int a_size=a.branchs.size();
    int b_size=b.branchs.size();

    map<Branch,int> mapa,mapb;
    for(int i=0;i<a_size;++i)
    {
        mapa[a.branchs[i]]=i;
    }
    for(int i=0;i<b_size;++i)
    {
        mapb[b.branchs[i]]=i;
    }

    cout<<"end map..."<<endl;

    vector<vector<int>> children_a=vector<vector<int>>(a_size,vector<int>());
    vector<vector<int>> children_b=vector<vector<int>>(b_size,vector<int>());

    for(int i=0;i<a_size;++i)
    {
        if(a.branchs[i].parent==0) continue;
        children_a[mapa[*(a.branchs[i].parent)]].push_back(i);
    }
    for(int i=0;i<b_size;++i)
    {
        if(b.branchs[i].parent==0) continue;
        children_b[mapb[*(b.branchs[i].parent)]].push_back(i);
    }

    cout<<"end children..."<<endl;

    map<int,int> map_b_t_a,map_b_t_b;

    for(int i=0;i<a_size;++i)
    {
        for(int j=0;j<a.trunks.size();++j)
        {
            bool stop=false;
            for(int k=0;k<a.trunks[j].branch_index.size();++k)
            {
                if(i==a.trunks[j].branch_index[k])
                {
                    stop=true;
                    map_b_t_a[i]=j;
                    break;
                }
            }
            if(stop)
                break;
        }
    }

    for(int i=0;i<b_size;++i)
    {
        for(int j=0;j<b.trunks.size();++j)
        {
            bool stop=false;
            for(int k=0;k<b.trunks[j].branch_index.size();++k)
            {
                if(i==b.trunks[j].branch_index[k])
                {
                    stop=true;
                    map_b_t_b[i]=j;
                    break;
                }
            }
            if(stop)
                break;
        }
    }

    int a_trunk_size=a.trunks.size();
    int b_trunk_size=b.trunks.size();

    vector<vector<int>> children_trunk_a=vector<vector<int>>(a_trunk_size,vector<int>());
    vector<vector<int>> children_trunk_b=vector<vector<int>>(b_trunk_size,vector<int>());

    for(int i=0;i<a_trunk_size;++i)
    {
        if(a.trunks[i].parent<0) continue;
        children_trunk_a[a.trunks[i].parent].push_back(i);
    }

    for(int i=0;i<b_trunk_size;++i)
    {
        if(b.trunks[i].parent<0) continue;
        children_trunk_b[b.trunks[i].parent].push_back(i);
    }

    vector<int> a_level0_index,b_level0_index;
    a.get_level_index(a_level0_index,0);
    b.get_level_index(b_level0_index,0);

    cout<<"a_level0_size:"<<a_level0_index.size()<<endl;
    cout<<"b_level0_size:"<<b_level0_index.size()<<endl;

    cout<<"end level0..."<<endl;

    map<int,int> map_tt_a_b;

    vector<bool> flag_a=vector<bool>(a_size,false);
    vector<bool> flag_b=vector<bool>(b_size,false);

    vector<int> trunk_queue;

    for(int i=0;i<a_level0_index.size();++i)
    {
        int a_index=a_level0_index[i];
        int b_index=this->get_corresponding_branch(a.branchs[a_index],b_level0_index,b,flag_b);

        if(b_index==-1)
        {
            a_more.push_back(a_index);
            continue;
        }
        int a_t_index=map_b_t_a[a_index];
        int b_t_index=map_b_t_b[b_index];
        map_tt_a_b[a_t_index]=b_t_index;
        trunk_queue.push_back(a_t_index);
    }

    cout<<"end initial trunk_queue..."<<endl;

    for(int i=0;i<b_level0_index.size();++i)
    {
        if(flag_b[b_level0_index[i]]==true)
            continue;
        else
        {
            b_more.push_back(b_level0_index[i]);
        }
    }

    cout<<"end b_more..."<<endl;
    int count=1;


    while(!trunk_queue.empty())
    {
        int a_t_index=trunk_queue.front();
        int b_t_index=map_tt_a_b[a_t_index];
        a_trunk.push_back(a_t_index);
        b_trunk.push_back(b_t_index);
        trunk_queue.erase(trunk_queue.begin());
    }

//    while(!trunk_queue.empty())
//    {
//        cout<<"for: "<<count<<"..."<<endl;

//        int a_t_index=trunk_queue.front();
//        trunk_queue.erase(trunk_queue.begin());
//        int b_t_index=map_tt_a_b[a_t_index];

//        int a_t_branch_size=children_trunk_a[a_t_index].size();
//        int b_t_branch_size=children_trunk_b[b_t_index].size();
//        int a_t_branch_index=0;
//        int b_t_branch_index=0;

//        int incount=0;

//        while((a_t_branch_index<=(a_t_branch_size))&&(b_t_branch_index<=(b_t_branch_size)))
//        {
//            cout<<"incount: "<<incount<<endl;
//            incount++;

//            double d=0;

//            cout<<"length..."<<endl;

//            double length_a=a.trunks[a_t_index].length[a_t_branch_index];
//            double length_b=b.trunks[b_t_index].length[b_t_branch_index];

//            cout<<"000"<<endl;

//            vector<Branch> a_branchs,b_branchs;
//            for(int i=0;i<=a_t_branch_index;++i)
//            {
//                a_branchs.push_back(a.branchs[a.trunks[a_t_index].branch_index[i]]);
//            }
//            cout<<"111"<<endl;
//            b_branchs.push_back(b.branchs[b.trunks[b_t_index].branch_index[b_t_branch_index]]);

//            cout<<"222"<<endl;

//            if(length_a>length_b)
//            {
//                d=this->get_distance_two_branchs(a_branchs,b_branchs,nta,ntb);
//                cout<<"d: "<<d<<endl;
//            }

//            cout<<"branch..."<<endl;

//            if(d>5)
//            {
//                a_trunk.push_back(a_t_index);
//                b_trunk.push_back(b_t_index);
//                break;
//            }
//            cout<<"end d..."<<endl;

//            if((a_t_branch_index==a_t_branch_size&&b_t_branch_index<=b_t_branch_size)||(b_t_branch_index==b_t_branch_size&&a_t_branch_index<=a_t_branch_size))
//            {
//                a_trunk.push_back(a_t_index);
//                b_trunk.push_back(b_t_index);
//                break;
//            }

//            cout<<"end bian..."<<endl;

//            if((a_t_branch_index==a_t_branch_size-1)||(b_t_branch_index==b_t_branch_size-1))
//            {
//                if(a_t_branch_index==a_t_branch_size-1)
//                {
//                    a_t_branch_index++;
//                    continue;
//                }
//                if(b_t_branch_index==b_t_branch_size-1)
//                {
//                    b_t_branch_index++;
//                    continue;
//                }
//            }

//            int a_trunk_child_index=children_trunk_a[a_t_index].at(a_t_branch_index);
//            int b_trunk_child_index=children_trunk_b[b_t_index].at(b_t_branch_index);

//            cout<<"1111"<<endl;




//            if(length_b<length_a*0.8)
//            {
//                //int b_trunk_child_index=children_trunk_b[b_t_index].at(b_t_branch_index);
//                int b_child_index=b.trunks[b_trunk_child_index].branch_index[0];
//                b_more.push_back(b_child_index);
//                b_t_branch_index++;
//                continue;
//            }
//            if(length_a<length_b*0.8)
//            {
//                //int a_trunk_child_index=children_trunk_b[a_t_index].at(a_t_branch_index);
//                int a_child_index=a.trunks[a_trunk_child_index].branch_index[0];
//                a_more.push_back(a_child_index);
//                a_t_branch_index++;
//                continue;
//            }
//            if(length_b>length_a*0.8&&length_a>length_b*0.8)
//            {
//                map_tt_a_b[a_trunk_child_index]=b_trunk_child_index;
//                trunk_queue.push_back(a_trunk_child_index);
//                a_t_branch_index++;
//                b_t_branch_index++;
//            }

//            cout<<"end..."<<endl;
//        }

//        count++;


//    }








    return true;
}

bool Swc_Compare::_get_sub_image(QString filename, vector<NeuronSWC> manual_points, vector<NeuronSWC> auto_points, QString braindir, V3DPluginCallback2 &callback)
{
    cout<<"in sub image"<<endl;

    NeuronTree manual_tree,auto_tree;
    size_t x0=IN,x1=0,y0=IN,y1=0,z0=IN,z1=0;
    for(int i=0;i<manual_points.size();++i)
    {
        manual_points[i].type=2;

        size_t tmpx=(size_t)manual_points[i].x;
        size_t tmpy=(size_t)manual_points[i].y;
        size_t tmpz=(size_t)manual_points[i].z;

        x0=(tmpx>x0)?x0:tmpx;
        x1=(tmpx>x1)?tmpx:x1;
        y0=(tmpy>y0)?y0:tmpy;
        y1=(tmpy>y1)?tmpy:y1;
        z0=(tmpz>z0)?z0:tmpz;
        z1=(tmpz>z1)?tmpz:z1;

    }

    for(int i=0;i<auto_points.size();++i)
    {
        auto_points[i].type=3;

        size_t tmpx=(size_t)auto_points[i].x;
        size_t tmpy=(size_t)auto_points[i].y;
        size_t tmpz=(size_t)auto_points[i].z;

        x0=(tmpx>x0)?x0:tmpx;
        x1=(tmpx>x1)?tmpx:x1;
        y0=(tmpy>y0)?y0:tmpy;
        y1=(tmpy>y1)?tmpy:y1;
        z0=(tmpz>z0)?z0:tmpz;
        z1=(tmpz>z1)?tmpz:z1;

    }

    x0-=30;
    x1+=30;
    y0-=30;
    y1+=30;
    z0-=30;
    z1+=30;

    cout<<"xyz..."<<endl;

    for(int i=0;i<manual_points.size();++i)
    {
        manual_points[i].x-=x0;
        manual_points[i].y-=y0;
        manual_points[i].z-=z0;
        manual_tree.listNeuron.push_back(manual_points[i]);
    }

    for(int i=0;i<auto_points.size();++i)
    {
        auto_points[i].x-=x0;
        auto_points[i].y-=y0;
        auto_points[i].z-=z0;
        auto_tree.listNeuron.push_back(auto_points[i]);
    }

    cout<<"finish tree..."<<endl;

    unsigned char* pdata=0;
    pdata=callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1,y0,y1,z0,z1);

    V3DLONG sz[4]={(x1-x0),(y1-y0),(z1-z0),1};
    int datatype=1;
    QString tif_filename=filename+".tif";
    QString manual_eswc_filename=filename+"false.eswc";
    QString auto_eswc_filename=filename+".eswc";

    simple_saveimage_wrapper(callback,tif_filename.toStdString().c_str(),pdata,sz,datatype);
    writeESWC_file(manual_eswc_filename,manual_tree);
    writeESWC_file(auto_eswc_filename,auto_tree);

    cout<<"finish...."<<endl;

    return true;
}


bool Swc_Compare::get_sub_false_trunk_image(QString dir, vector<int> &manual_false, vector<int> auto_false, SwcTree &manual_t, SwcTree &auto_t, QString braindir, V3DPluginCallback2 &callback)
{
    int manual_size=manual_false.size();
    int auto_size=auto_false.size();
    if(manual_size!=auto_size)
        return false;

    cout<<"in get image..."<<endl;

    map<Branch,int> map_manual,map_auto;
    for(int i=0;i<manual_t.branchs.size();++i)
    {
        map_manual[manual_t.branchs[i]]=i;
    }
    for(int i=0;i<auto_t.branchs.size();++i)
    {
        map_auto[auto_t.branchs[i]]=i;
    }

    cout<<"end map............"<<endl;

    vector<vector<int>> children_manual=vector<vector<int>>(manual_t.branchs.size(),vector<int>());
    vector<vector<int>> children_auto=vector<vector<int>>(auto_t.branchs.size(),vector<int>());

    for(int i=0;i<manual_t.branchs.size();++i)
    {
        if(manual_t.branchs[i].parent==0) continue;
        children_manual[map_manual[*(manual_t.branchs[i].parent)]].push_back(i);
    }
    for(int i=0;i<auto_t.branchs.size();++i)
    {
        if(auto_t.branchs[i].parent==0) continue;
        children_auto[map_auto[*(auto_t.branchs[i].parent)]].push_back(i);
    }

    for(int i=0;i<auto_size;++i)
    {
        cout<<"in for: "<<i<<"...."<<endl;
        int manual_branch_index=manual_false[i];
        int auto_branch_index=auto_false[i];

        vector<Branch> manual_branchs,auto_branchs;

        manual_branchs.push_back(manual_t.branchs[manual_branch_index]);
        while(manual_t.branchs[manual_branch_index].parent!=0)
        {
            int par_branch_index=map_manual[*(manual_t.branchs[manual_branch_index].parent)];
            manual_branchs.push_back(manual_t.branchs[par_branch_index]);
            manual_branch_index=par_branch_index;
        }

        reverse(manual_branchs.begin(),manual_branchs.end());

        auto_branchs.push_back(auto_t.branchs[auto_branch_index]);
        while(auto_t.branchs[auto_branch_index].parent!=0)
        {
            int par_branch_index=map_auto[*(auto_t.branchs[auto_branch_index].parent)];
            auto_branchs.push_back(auto_t.branchs[par_branch_index]);
            auto_branch_index=par_branch_index;
        }

        reverse(auto_branchs.begin(),auto_branchs.end());

        vector<NeuronSWC> manual_points,auto_points;
        manual_t.get_points_of_branchs(manual_branchs,manual_points,manual_t.nt);
        auto_t.get_points_of_branchs(auto_branchs,auto_points,auto_t.nt);

        vector<NeuronSWC> manual_child_points,auto_child_points;

        for(int j=0;j<children_manual[manual_branch_index].size();++j)
        {
            vector<NeuronSWC> tmp;
            int child_index=children_manual[manual_branch_index].at(j);
            manual_t.branchs[child_index].get_points_of_branch(tmp,manual_t.nt);
            tmp.erase(tmp.begin());
            manual_child_points.insert(manual_child_points.end(),tmp.begin(),tmp.end());
        }
        for(int j=0;j<children_auto[auto_branch_index].size();++j)
        {
            vector<NeuronSWC> tmp;
            int child_index=children_auto[auto_branch_index].at(j);
            auto_t.branchs[child_index].get_points_of_branch(tmp,auto_t.nt);
            tmp.erase(tmp.begin());
            auto_child_points.insert(auto_child_points.end(),tmp.begin(),tmp.end());
        }

        manual_points.insert(manual_points.end(),manual_child_points.begin(),manual_child_points.end());
        auto_points.insert(auto_points.end(),auto_child_points.begin(),auto_child_points.end());

        QString filename=dir+"/_"+QString::number(i,10)+"_false_trunk_";
        this->_get_sub_image(filename,manual_points,auto_points,braindir,callback);

    }

    return true;

}

double Swc_Compare::get_distance_for_manual(Branch &auto_branch, NeuronTree &nta, NeuronTree &ntb)
{
    cout<<"in distance..."<<endl;
    vector<NeuronSWC> auto_points;
    auto_branch.get_points_of_branch(auto_points,ntb);
    double d_btotree=0;
    for(int i=0;i<auto_points.size();++i)
    {
        double min_d=IN;
        int min_index;
        for(int j=0;j<nta.listNeuron.size()-1;++j)
        {
            double tmp_d=p_to_line<NeuronSWC,Angle>(auto_points[i],nta.listNeuron[j],nta.listNeuron[j+1]);
            if(tmp_d<min_d)
            {
                min_index=j;
                min_d=tmp_d;
            }
        }
        d_btotree+=p_to_line<NeuronSWC,Angle>(auto_points[i],nta.listNeuron[min_index],nta.listNeuron[min_index+1]);
    }

    return (d_btotree/auto_points.size());
}

bool Swc_Compare::get_sub_image(QString dir, vector<int> &false_index,SwcTree &t,QString braindir,V3DPluginCallback2 &callback,int mode)
{

    int auto_size=false_index.size();

    cout<<"in get image..."<<endl;

    map<Branch,int> map_auto;
    for(int i=0;i<t.branchs.size();++i)
    {
        map_auto[t.branchs[i]]=i;
    }

    cout<<"end map............"<<endl;

    vector<vector<int>> children_auto=vector<vector<int>>(t.branchs.size(),vector<int>());


    for(int i=0;i<t.branchs.size();++i)
    {
        if(t.branchs[i].parent==0) continue;
        children_auto[map_auto[*(t.branchs[i].parent)]].push_back(i);
    }

    for(int i=0;i<auto_size;++i)
    {
        cout<<"in for: "<<i<<"...."<<endl;

        int auto_branch_index=false_index[i];

        vector<NeuronSWC> auto_child_points;
        vector<NeuronSWC> auto_points,manual_points;

        t.branchs[auto_branch_index].get_points_of_branch(manual_points,t.nt);
        auto_points.clear();
        //QString filename=dir+"/_"+QString::number(i,10)+"_false_auto_";




        for(int j=0;j<children_auto[auto_branch_index].size();++j)
        {
            vector<NeuronSWC> tmp;
            int child_index=children_auto[auto_branch_index].at(j);
            t.branchs[child_index].get_points_of_branch(tmp,t.nt);
            tmp.erase(tmp.begin());
            auto_child_points.insert(auto_child_points.end(),tmp.begin(),tmp.end());
        }

        vector<Branch> auto_branchs;

        //auto_branchs.push_back(t.branchs[auto_branch_index]);
        while(t.branchs[auto_branch_index].parent!=0)
        {
            int par_branch_index=map_auto[*(t.branchs[auto_branch_index].parent)];
            auto_branchs.push_back(t.branchs[par_branch_index]);
            auto_branch_index=par_branch_index;
        }

        if(auto_branchs.size()>0)
        {
            reverse(auto_branchs.begin(),auto_branchs.end());


            t.get_points_of_branchs(auto_branchs,auto_points,t.nt);
        }





        auto_points.insert(auto_points.end(),auto_child_points.begin(),auto_child_points.end());
        //manual_points.clear();



        QString filename;
        if(mode==1)
        {
            filename=dir+"/_"+QString::number(i,10)+"_auto_more_";
        }
        if(mode==2)
        {
            filename=dir+"/_"+QString::number(i,10)+"_manual_more_";
        }
        this->_get_sub_image(filename,manual_points,auto_points,braindir,callback);

    }

    return true;
}











