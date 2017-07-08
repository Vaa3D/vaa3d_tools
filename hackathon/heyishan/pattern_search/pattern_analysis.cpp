#include "pattern_analysis.h"
#include "pre_processing_main.h"
#include "my_sort.h"
#include <qstack.h>
#ifndef VOID
#define VOID 1000000000
#endif
#ifndef getParent(n,nt)
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
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

bool pattern_analysis(const NeuronTree &nt, const NeuronTree &boundary,NeuronTree & consensus, int & boundary_length,V3DPluginCallback2 &callback)
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

    // push points in v_boundary into v_area;
    vector<NeuronTree> v_area(v_boundary.size());

   for(V3DLONG i=0;i<nt.listNeuron.size();i++)
   {
       NeuronSWC curr = nt.listNeuron[i];
       for(int j=0; j<v_boundary.size();j++)
       {           Boundary b=v_boundary[j];
           if(curr.x>b.minx && curr.y>b.miny && curr.z>b.minz &&curr.x<b.maxx && curr.y<b.maxy&&curr.z<b.maxz)
           {
               v_area[j].listNeuron.push_back(curr);
               break;
           }
       }
    }

   // calculate boundary_length of each of v_boundary
   V3DLONG area_num = 0;
   double v_tol_dist = 0;
   cout<<"v_area.size="<<v_area.size()<<endl;
   for(V3DLONG i=0;i<v_area.size();i++)
   {
       cout<<i<<"    area_size="<<v_area[i].listNeuron.size()<<endl;
       V3DLONG endPointNum = 0;
       V3DLONG tol_len = 0;
       if(v_area[i].listNeuron.size()<=1) {cout<<"the number of points within this boundary isn't more than 1"<<endl; continue;}
       NeuronTree area_sorted;
//        if(!SortSWC(v_area[i].listNeuron,area_sorted.listNeuron,VOID,VOID))
//        {
//            cout<<"sort failed"<<endl;
//            return false;
//        }
        V3DLONG root_id=v_area[i].listNeuron[0].n;
        area_sorted = sort(v_area[i], root_id,VOID);
//        area_sorted = sort(v_area[i], VOID, VOID);
//        area_sorted.hashNeuron.clear();
//        for(V3DLONG i=0; i<area_sorted.listNeuron.size();i++){area_sorted.hashNeuron.insert(area_sorted.listNeuron[i].n, i);}

        // get each node's children
        V3DLONG area_size = area_sorted.listNeuron.size();
        QVector<QVector<V3DLONG> > childs;
        childs = QVector< QVector<V3DLONG> >(area_size, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<area_size;i++)
        {
            V3DLONG par = area_sorted.listNeuron[i].pn;
            if (par<0) continue;
            childs[area_sorted.hashNeuron.value(par)].push_back(i);
        }

        // save area for test
        QString savename = QString::number(i+1)+".swc";
        writeSWC_file(savename,area_sorted);

        // using stack marching caculate lenth from each point k to end point;
        cout<<"using stack marching caculate lenth from each point k to end point;"<<endl;
        for(V3DLONG j=0; j<area_size;j++)
        {
            QStack<StackElem> TreeStack;
            char * state = new char[area_size];
            for(V3DLONG k=0;k<area_size;k++)
            {
                if(k==j) state[k]=ALIVE;
                else    state[k]=FAR;
            }
            //StackElem * first_elem = new StackElem(j,0);
            StackElem first_elem(j,0);
            TreeStack.push(first_elem);
            while (!TreeStack.isEmpty())
            {
                StackElem cur_elem = TreeStack.top();
                state[cur_elem.id] = ALIVE;
                V3DLONG cur_elem_pn=getParent(cur_elem.id,area_sorted);
                //cout<<"cur_elem's id="<<cur_elem.id<<"  len="<<cur_elem.len<<"  pn="<<cur_elem_pn<<endl;
                //pop this node
                TreeStack.pop();
                // push its children into stack
                for(V3DLONG k=0; k<childs[cur_elem.id].size(); k++)
                {
                    V3DLONG c=childs[cur_elem.id].at(k);
                    if(state[c]==FAR)
                    {
                        StackElem elem(c,0);
                        elem.len=cur_elem.len + 1;
                        TreeStack.push(elem);
                        state[c]=ALIVE;
                    }
                }
                // push its parent into stack
                if(cur_elem_pn!=VOID && state[cur_elem_pn]==FAR )
                {
                    StackElem elem(cur_elem_pn,0);
                    elem.len=cur_elem.len +1;
                    TreeStack.push(elem);
                    state[cur_elem_pn]=ALIVE;
                }
                // if cur_elem is end point add its len into tol_len and endPointNum plus one
                if(childs[cur_elem.id].size()==0 || cur_elem_pn==VOID)
                {
                    if(cur_elem.len != 0)   //in case cur_elem is end point
                    {
                        endPointNum+=1;
                        tol_len+=cur_elem.len;
                        continue;
                    }
                    else cout<<"This is first point"<<endl;
                }
            }//while
            TreeStack.clear();
            if(state){delete [] state;state=0;}
        }//end j
        //cout<<"tol_len="<<tol_len<<"  endPointNum="<<endPointNum<<endl;
        v_tol_dist = v_tol_dist + tol_len/endPointNum;
        area_num += 1;
        cout<<"v_tol_dist="<<v_tol_dist<<"  area_num="<<area_num<<endl;
        childs.clear();
   }//end i

   boundary_length= int(v_tol_dist/area_num);
   cout<<"boundary_length="<<boundary_length<<endl;

    // pre_processing
   vector<QString> file_list;
   for(int i=0;i<v_area.size();i++)
   {
       if(v_area[i].listNeuron.size()==0)    continue;
       double step_size=2;
       double prune_size = -1; //default case
       double thres = 2;
       cout<<"into prune"<<endl;
       NeuronTree pruned;
       if (!prune_branch(v_area[i], pruned,prune_size))
       {
           fprintf(stderr,"Error in prune_short_branch.\n");
           return false;
       }
       cout<<"into resample"<<endl;
       NeuronTree resampled;
       resampled  = resample(pruned, step_size);
       cout<<"into sort"<<endl;
       NeuronTree sorted;
       if(resampled.listNeuron.size()==0) continue;
       V3DLONG root_id=resampled.listNeuron[0].n;
       sorted = sort (resampled,root_id,thres);
       cout<<"into PCA"<<endl;
       NeuronTree result;
       result = align_axis(sorted);
       //save file
       QString savename = "pattern_bn_" + QString::number(i+1) + ".swc";
       file_list.push_back(savename);
       writeSWC_file(savename,result);
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

  if(file_list.size()>=6)
  {
      consensus = readSWC_file(consensus_result);
  }
  else
  {
      int max_area=0;
      for(int i=0;i<v_area.size();i++)
      {
          if(v_area[i].listNeuron.size()>v_area[max_area].listNeuron.size())  max_area=i;
      }
      cout<<"area of interest is less than 6, the max area is num "<<max_area+1<<" and it's size is "<<v_area[max_area].listNeuron.size()<<endl;
      if(v_area[max_area].listNeuron.size()==0)
      {
          cout<<"no point in the pattern"<<endl;
          return false;
      }
      V3DLONG root_id = v_area[max_area].listNeuron[0].n;
      consensus=sort(v_area[max_area],root_id,VOID);
  }
   return true;
}
