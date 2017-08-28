#include "pattern_analysis.h"
#include "pre_processing_main.h"
#include "my_sort.h"
#include <qstack.h>
#include<qset.h>
#include<qqueue.h>
#ifndef VOID
#define VOID 1000000000
#endif
#ifndef getParent(n,nt)
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#endif
#ifndef mhd(a)
#define mhd(a) (fabs(a.x)+fabs(a.y)+fabs(a.z))
#endif
struct Boundary
{
    float minx;
    float miny;
    float minz;
    float maxx;
    float maxy;
    float maxz;
};

// pt need be sorted and count node from 0
void create_AM(vector<vector<V3DLONG> >&AM, V3DLONG siz, const NeuronTree & pt)
{
    // initialize adjance matrix
    v3d_msg("0");
    for(V3DLONG i=0; i<siz; i++)
    {
        AM[i].resize(siz);
    }
    v3d_msg("0.5");
    for(V3DLONG i=0; i<siz; i++)
    {
        for(V3DLONG j=0; j<siz; j++)
            AM[i][j] = 0;
    }
    v3d_msg("1");
    // creat AM
    for(int i=0; i<siz ; i++)
    {
        NeuronSWC cur = pt.listNeuron[i];
        int p = cur.pn;
        if( p < 0) continue;
        AM[i][p-1] = AM[p-1][i] = 1;
    }
    v3d_msg("2");
    return;
}
bool cal_degree(vector<int> & degree, V3DLONG siz, const NeuronTree & pt, const QVector<QVector<V3DLONG> > &childs)
{
    for(int i=0; i<siz; i++)
    {
        NeuronSWC cur = pt.listNeuron[i];
        if(cur.pn < 0)
            degree[i] = childs[i].size();
        else
            degree[i] = childs[i].size() + 1;
    }
    return true;
}

bool pattern_analysis(const NeuronTree &nt, const NeuronTree &boundary,vector<NeuronTree> & pt_list, vector<int> & pt_lens,V3DPluginCallback2 &callback)
{
    V3DLONG boundary_size = boundary.listNeuron.size();
    V3DLONG nt_size= nt.listNeuron.size();
    if(boundary_size==0 || nt_size==0)
    {
        v3d_msg("Input is empty, please retry");
        return false;
    }

    // find min max boundary of each area of interest
    Boundary temp;
    vector<Boundary> v_boundary;
    for(V3DLONG i=0;i<boundary_size;i++)
    {

       if(boundary.listNeuron[i].pn<0)
       {
            temp.minx=VOID; temp.miny=VOID;temp.minz=VOID;
            temp.maxx=0;temp.maxy=0;temp.maxz=0;
        }
       temp.minx=min(boundary.listNeuron[i].x,temp.minx);
       temp.miny=min(boundary.listNeuron[i].y,temp.miny);
       temp.minz=min(boundary.listNeuron[i].z,temp.minz);
       temp.maxx=max(boundary.listNeuron[i].x,temp.maxx);
       temp.maxy=max(boundary.listNeuron[i].y,temp.maxy);
       temp.maxz=max(boundary.listNeuron[i].z,temp.maxz);

       if(i==boundary_size-2)
       {
           temp.minx=min(boundary.listNeuron[i+1].x,temp.minx);
           temp.miny=min(boundary.listNeuron[i+1].y,temp.miny);
           temp.minz=min(boundary.listNeuron[i+1].z,temp.minz);
           temp.maxx=max(boundary.listNeuron[i+1].x,temp.maxx);
           temp.maxy=max(boundary.listNeuron[i+1].y,temp.maxy);
           temp.maxz=max(boundary.listNeuron[i+1].z,temp.maxz);
           v_boundary.push_back(temp);
           i++;
       }
       else if(boundary.listNeuron[i+1].pn==-1)
       {
           v_boundary.push_back(temp);
       }
    }
    vector<Boundary> v_large_boundary(v_boundary.size());
    for(int i=0; i<v_boundary.size(); i++)
    {
        Boundary cur = v_boundary[i];
        Boundary large_boundary;
        large_boundary.maxx = cur.maxx + (cur.maxx - cur.minx) * 0.5;
        large_boundary.minx = cur.minx - (cur.maxx - cur.minx) * 0.5;
        large_boundary.maxy = cur.maxy + (cur.maxy - cur.miny) * 0.5;
        large_boundary.miny = cur.miny - (cur.maxy - cur.miny) * 0.5;
        large_boundary.maxz = cur.maxz + (cur.maxz - cur.minz) * 0.5;
        large_boundary.minz = cur.minz - (cur.maxz - cur.minz) * 0.5;
        v_large_boundary[i]=large_boundary;
        cout<<"boundary:"<<endl;
        cout<<cur.maxx<<"   "<<cur.maxy<<"  "<<cur.maxz<<endl;
        cout<<"large_boundary:"<<endl;
        cout<<large_boundary.maxx<<"   "<<large_boundary.maxy<<"  "<<large_boundary.maxz<<endl;
    }
    // push points from nt into v_large_area;
    vector<NeuronTree> v_large_area(v_large_boundary.size());
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++)
    {
        NeuronSWC curr = nt.listNeuron[i];
        for(int j=0; j<v_large_boundary.size(); j++)
        {
            Boundary b = v_large_boundary[j];
            if(curr.x>b.minx && curr.y>b.miny && curr.z>b.minz &&curr.x<b.maxx && curr.y<b.maxy&&curr.z<b.maxz)
                v_large_area[j].listNeuron.push_back(curr);
        }
    }
    // push points from v_large_area into v_area;
    cout<<"v_boundary size="<<v_boundary.size()<<endl;
    //vector<NeuronTree> v_area(v_boundary.size());
    vector<NeuronTree> v_area(v_large_boundary.size());
    vector<NeuronTree> v_area2; //storing the max subtree in v_area
   for(int i=0;i<v_large_area.size();i++)
   {
       NeuronTree cur_tree;
       for(V3DLONG j=0; j<v_large_area[i].listNeuron.size(); j++)
       {
           NeuronSWC curr = v_large_area[i].listNeuron[j];
           Boundary b = v_boundary[i];
           if(curr.x>b.minx && curr.y>b.miny && curr.z>b.minz &&curr.x<b.maxx && curr.y<b.maxy&&curr.z<b.maxz)
               cur_tree.listNeuron.push_back(curr);
       }
       v_area[i] = cur_tree;
    }
   cout<<v_boundary.size()<<endl;
   cout<<v_large_boundary.size()<<endl;
    cout<< v_large_area[0].listNeuron.size()<<endl;
   // calculate boundary_length of each v_area
   V3DLONG area_num = 0;
   double v_tol_dist = 0;
   vector<int> v_area_len;
   cout<<"v_area.size="<<v_area.size()<<endl;
   for(V3DLONG i=0;i<v_area.size();i++)
   {
       cout<<i<<"    area_size="<<v_area[i].listNeuron.size()<<endl;
       V3DLONG endPointNum = 0;
       V3DLONG tol_len = 0;
       if(v_area[i].listNeuron.size()<=1) {cout<<"the number of points within this boundary isn't more than 1"<<endl; continue;}
       NeuronTree area_sorted;
       double sort_thres = 0;
       area_sorted.listNeuron.clear();
       area_sorted.hashNeuron.clear();
       V3DLONG root_id=v_area[i].listNeuron[0].n;
       area_sorted = sort(v_area[i], root_id,sort_thres);
       // select max subtree
       V3DLONG tot_len = area_sorted.listNeuron.size();
       V3DLONG sublen = 0;
       V3DLONG max_sublen = 0;
       V3DLONG max_id = 0;
       for(V3DLONG j=0; j<tot_len; j++)
       {
           NeuronSWC cur = area_sorted.listNeuron[j];
           if(cur.pn == -1)
               sublen = 0;
           sublen += 1;
           if(sublen > max_sublen)
           {
               max_sublen = sublen;
               max_id = j;
           }
       }
       NeuronTree area_sub; //sub means the biggest connected subtree in area
       for(V3DLONG j=max_id - max_sublen + 1; j<= max_id; j++)
       {
           area_sub.listNeuron.push_back(area_sorted.listNeuron[j]);
       }
       area_sub = sort(area_sub,VOID,VOID);
       area_sub.hashNeuron.clear();
       for(V3DLONG j=0; j<area_sub.listNeuron.size();j++){area_sub.hashNeuron.insert(area_sub.listNeuron[j].n, j);}

       QString saveold="pattern_old.swc";
        writeSWC_file(saveold,area_sorted);

        // get each node's children
        V3DLONG area_size = area_sub.listNeuron.size();
        QVector<QVector<V3DLONG> > childs;
        childs = QVector< QVector<V3DLONG> >(area_size, QVector<V3DLONG>() );
        for (V3DLONG j=0;j<area_size;j++)
        {
            V3DLONG par = area_sub.listNeuron[j].pn;
            if (par<0) continue;
            childs[area_sub.hashNeuron.value(par)].push_back(j);
        }
        if(0)
        {
//       old method for calculate area length
//       {
//        // using stack marching caculate lenth from each point k to end point;
//        cout<<"using stack marching caculate lenth from each point k to end point;"<<endl;
//        for(V3DLONG j=0; j<area_size;j++)
//        {
//            QStack<StackElem> TreeStack;
//            char * state = new char[area_size];
//            for(V3DLONG k=0;k<area_size;k++)
//            {
//                if(k==j) state[k]=ALIVE;
//                else    state[k]=FAR;
//            }
//            StackElem first_elem(j,0);
//            TreeStack.push(first_elem);
//            while (!TreeStack.isEmpty())
//            {
//                StackElem cur_elem = TreeStack.top();
//                state[cur_elem.id] = ALIVE;
//                V3DLONG cur_elem_pn=getParent(cur_elem.id,area_sub);
//                //pop this node
//                TreeStack.pop();
//                // push its children into stack
//                for(V3DLONG k=0; k<childs[cur_elem.id].size(); k++)
//                {
//                    V3DLONG c=childs[cur_elem.id].at(k);
//                    if(state[c]==FAR)
//                    {
//                        StackElem elem(c,0);
//                        elem.len=cur_elem.len + 1;
//                        TreeStack.push(elem);
//                        state[c]=ALIVE;
//                    }
//                }
//                // push its parent into stack
//                if(cur_elem_pn!=VOID && state[cur_elem_pn]==FAR )
//                {
//                    StackElem elem(cur_elem_pn,0);
//                    elem.len=cur_elem.len +1;
//                    TreeStack.push(elem);
//                    state[cur_elem_pn]=ALIVE;
//                }
//                // if cur_elem is end point add its len into tol_len and endPointNum plus one
//                if(childs[cur_elem.id].size()==0 || cur_elem_pn==VOID)
//                {
//                    if(cur_elem.len != 0)   //in case cur_elem is end point
//                    {
//                        endPointNum+=1;
//                        tol_len+=cur_elem.len;
//                        continue;
//                    }
//                    else endPointNum+=1;
//                }
//            }//while
//            TreeStack.clear();
//            if(state){delete [] state;state=0;}
//        }//end j

//        cout<<"tol_len="<<tol_len<<"  endPointNum="<<endPointNum<<endl;
//        v_tol_dist = v_tol_dist + tol_len/endPointNum;
//        area_num += 1;
//        int area_len = tol_len/endPointNum;
//        v_area_len.push_back(area_len);
//       }
        }
        //using BFS find the center of graph
        cout<<"using BFS the center of graph"<<endl;
        // creat  Adjacency Matrix of Graph

        vector<vector<V3DLONG> >AM(area_size);
        vector<int> degree(area_size);
        vector<int> level(area_size);
        int maxlevel = 0;
        QSet <int> c;
        QQueue <int> q;
        create_AM(AM, area_size, area_sub);
        cal_degree(degree,area_size,area_sub,childs);

        // Start from leaves
        for(int j=0; j<area_size; j++)
        {
            level[j] = 0;
            if(degree[j] == 1)
                q.enqueue(j);
        }
        while (!q.isEmpty())
        {
            int v = q.dequeue();
            // remove leaf and try to add its parent
            for(int j=0; j<area_size; j++)
            {
                if(AM[v][j])
                {
                    degree[j] --;
                    if(degree[j] == 1)
                    {
                        q.enqueue(j);
                        level[j] = level[v] + 1;
                        maxlevel = (maxlevel>=level[j]) ? maxlevel : level[j];
                    }
                }
            }// end j
        }//end while
        // put center node into c
        double c_x, c_y, c_z;
        for (int j=0; j<area_size; j++)
            if(level[j] == maxlevel)
            {
                c.insert(j);
                c_x = area_sub.listNeuron[j].x;
                c_y = area_sub.listNeuron[j].y;
                c_z = area_sub.listNeuron[j].z;
            }
        int diam = 2*maxlevel + c.size() - 1;
        int rad = (diam + 1) / 2;
        v_area_len.push_back(rad);
        area_num+=1;
        cout<<"rad = "<<rad<<endl;

        cout<<"v_tol_dist="<<v_tol_dist<<"  area_num="<<area_num<<endl;
        childs.clear();
        q.clear();

        // find the nearest point from c int the large_area
        double min_dist=VOID;
        V3DLONG nearest_ind;
        for(int j=0; j<v_large_area[i].listNeuron.size(); j++)
        {
            NeuronSWC cur = v_large_area[i].listNeuron[j];
            double cur_dist_square = (cur.x - c_x)*(cur.x - c_x) + (cur.y - c_y)*(cur.y - c_y) + (cur.z - c_z)*(cur.z - c_z);
            if(cur_dist_square < min_dist)
            {
                min_dist = cur_dist_square;
                nearest_ind = j;
            }
        }

        // get each node's children in large_area
        NeuronTree large_area = v_large_area[i];
        V3DLONG lg_area_size = large_area.listNeuron.size();
        large_area.hashNeuron.clear();
        for(V3DLONG j=0; j<large_area.listNeuron.size();j++){large_area.hashNeuron.insert(large_area.listNeuron[j].n, j);}
        QVector<QVector<V3DLONG> > lg_childs;
        lg_childs = QVector< QVector<V3DLONG> >(lg_area_size, QVector<V3DLONG>() );
        for (V3DLONG j=0;j<lg_area_size;j++)
        {
            V3DLONG par = large_area.listNeuron[j].pn;
            if (par<0) continue;
            lg_childs[large_area.hashNeuron.value(par)].push_back(j);
        }

        // find the pattern in large_area and push it into v_area2
        NeuronTree pattern;
        NeuronSWC cur_point = large_area.listNeuron[nearest_ind];
        pattern.listNeuron.push_back(cur_point);
        QStack<StackElem> TreeStack;
        char * state = new char[lg_area_size];
        memset(state,FAR,lg_area_size);
        state[nearest_ind]=ALIVE;
        StackElem first_elem(nearest_ind,0);
        TreeStack.push(first_elem);
        while(!TreeStack.isEmpty())
        {
            StackElem cur_elem = TreeStack.top();
            TreeStack.pop();
            V3DLONG cur_elem_pn = getParent(cur_elem.id,large_area);
            if(cur_elem.len>=rad) continue;
            else
            {
                //push its children into stack and pattern
                for(V3DLONG j=0; j<lg_childs[cur_elem.id].size(); j++)
                {
                    V3DLONG c = lg_childs[cur_elem.id].at(j);
                    if(state[c]==FAR)
                    {
                        StackElem elem(c,0);
                        elem.len = cur_elem.len +1;
                        TreeStack.push(elem);
                        NeuronSWC p = large_area.listNeuron[elem.id];
                        pattern.listNeuron.push_back(p);
                        state[c]=ALIVE;
                    }
                } //end j
                // push its parent into stack,points and pattern
                if(cur_elem_pn!=VOID && state[cur_elem_pn]==FAR)
                {
                    StackElem elem(cur_elem_pn,0);
                    elem.len = cur_elem.len + 1;
                    TreeStack.push(elem);
                    NeuronSWC p = large_area.listNeuron[elem.id];
                    pattern.listNeuron.push_back(p);
                    state[cur_elem_pn] = ALIVE;
                }
            } //else
        }//while

        // save area for test
        QString savename = "pattern_"+ QString::number(i+1)+".swc";
        writeSWC_file(savename,pattern);
        v_area2.push_back(pattern);

        pattern.listNeuron.clear();
        pattern.hashNeuron.clear();
        TreeStack.clear();
        if(state){delete [] state;state=0;}
   }//end i

   int pt_ave_len= int(v_tol_dist/area_num);
   cout<<"pt_ave_len="<<pt_ave_len<<endl;

   if(v_area2.size()>=10) //could not perform well yet
   {
       // pre_processing
       vector<QString> file_list;
       for(int i=0;i<v_area2.size();i++)
       {
            if(v_area2[i].listNeuron.size()==0)    continue;
            double step_size=2;
            double prune_size = -1; //default case
            double thres = 2;
            cout<<"into prune"<<endl;
            NeuronTree pruned;
            if (!prune_branch(v_area2[i], pruned,prune_size))
            {
                fprintf(stderr,"Error in prune_short_branch.\n");
                return false;
            }
            cout<<"into resample"<<endl;
            NeuronTree resampled;
            resampled  = resample(pruned, step_size);
            cout<<"into PCA"<<endl;
            NeuronTree pca;
            if(resampled.listNeuron.size()>=10)  pca = align_axis(resampled);
            else pca=resampled;
            cout<<"into sort"<<endl;
            NeuronTree sorted;
            double dist0=VOID;
            if(pca.listNeuron.size()<1) continue;
//            V3DLONG ind0=pca.listNeuron[0].n;
//            for(V3DLONG j=0;j<pca.listNeuron.size();j++)
//            {
//                NeuronSWC p=pca.listNeuron[j];
//                if(dist0>mhd(p)) {dist0=mhd(p);ind0=j;}
//            }
//            V3DLONG root_id=pca.listNeuron[ind0].n;
            V3DLONG root_id=pca.listNeuron[0].n;
            sorted = sort (pca,root_id,VOID);

            //save file
            QString savename = "pattern_bn_" + QString::number(i+1) + ".swc";
            file_list.push_back(savename);
            writeSWC_file(savename,sorted);
        }

       // make_consensus
       cout<<"Example: v3d -x consensus_swc -f consensus_swc -i myfolder/*.swc -o consensus.eswc -p 3 5 0\n"<<endl;
       if(file_list.size()==0) {cout<<"no list_bn file"<<endl;return false;}
       V3DPluginArgItem arg;
       V3DPluginArgList input_consensus;
       V3DPluginArgList output_consensus;
       QString consensus_result = "consensus.swc";
       arg.type = "random";vector<char*> arg_input_consensus;
       for(int i=0;i<file_list.size();i++)
       {
           QString files_name = file_list[i];
           string fileName_Qstring(files_name.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
           arg_input_consensus.push_back(fileName_string);
       }
       arg.p = (void *) & arg_input_consensus;
       input_consensus<< arg;

       arg.type = "random";vector<char*> arg_consensus_para; arg_consensus_para.push_back("3");arg_consensus_para.push_back("5");arg_consensus_para.push_back("0");
       arg.p = (void *) & arg_consensus_para; input_consensus << arg;
       arg.type = "random";vector<char*> arg_output;
       string fileName_Qstring2(consensus_result.toStdString());char* fileName_string2 =  new char[fileName_Qstring2.length() + 1]; strcpy(fileName_string2, fileName_Qstring2.c_str());
       arg_output.push_back(fileName_string2); arg.p = (void *) & arg_output; output_consensus<< arg;
       QString full_plugin_name_consensus = "consensus_swc";
       QString func_name_consensus = "consensus_swc";
       callback.callPluginFunc(full_plugin_name_consensus,func_name_consensus,input_consensus,output_consensus);

      NeuronTree consensus = readSWC_file(consensus_result);
      pt_list.push_back(consensus);
      pt_lens.push_back(pt_ave_len);
  }
  else
  {
       for(int i=0;i<v_area2.size();i++)
       {
           if(v_area2[i].listNeuron.size()>1)
           {
               NeuronTree sorted;
               double dist0=VOID;
               V3DLONG ind0=v_area2[i].listNeuron[0].n;
               for(V3DLONG j=0;j<v_area2[i].listNeuron.size();j++)
               {
                   NeuronSWC p=v_area2[i].listNeuron[j];
                   if(dist0>mhd(p)) {dist0=mhd(p);ind0=j;}
               }
               V3DLONG root_id=v_area2[i].listNeuron[ind0].n;
               sorted = sort (v_area2[i],root_id,VOID);
               pt_list.push_back(sorted);
//               QString sav = "pattern_aaa"+ QString::number(i+1)+".swc";
//               writeSWC_file(sav,sorted);
           }
       }
       pt_lens=v_area_len;
//      int max_area=0;
//      for(int i=0;i<v_area.size();i++)
//      {
//          if(v_area[i].listNeuron.size()>v_area[max_area].listNeuron.size())  max_area=i;
//      }
//      cout<<"area of interest is less than 6, the max area is num "<<max_area+1<<" and it's size is "<<v_area[max_area].listNeuron.size()<<endl;
//      if(v_area[max_area].listNeuron.size()==0)
//      {
//          cout<<"no point in the pattern"<<endl;
//          return false;
//      }
//      V3DLONG root_id = v_area[max_area].listNeuron[0].n;
//      consensus=sort(v_area[max_area],root_id,VOID);
  }
   return true;
}
