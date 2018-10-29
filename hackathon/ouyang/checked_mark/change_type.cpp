# include "change_type.h"
#include <v3d_interface.h>
#include "v3d_message.h"
#include <vector>
#include <iostream>
#include <algorithm>
//#include <PMain.h>

using namespace std;

forbackup savetheblock;
void checked_mark_type(V3DPluginCallback2 &callback, QWidget *parent)
{
    savetheblock.backup.clear();
    savetheblock.backupelse.clear();
    savetheblock.current_block_all_else.clear();
    savetheblock.current_block_all.clear();
    const Image4DSimple *curr_block=callback.getImageTeraFly();
    NeuronTree nt=callback.getSWCTeraFly();
    int swcnum=nt.listNeuron.size();
    //v3d_msg(QString("There are totally [%1] swc nodes in your neuron!").arg(swcnum));
    NeuronTree result;
    QList<NeuronSWC> neuron = nt.listNeuron;
    //for (int i=0;i<100;i++)
    //{
     //   cout<<" x:"<<neuron.at(i).x<<" y:"<<neuron.at(i).y<<" z:"<<neuron.at(i).z<<endl;
      //  cout<<" n:"<<neuron.at(i).n<<" pn:"<<neuron.at(i).pn<<endl;
    // }
 if(!neuron.empty())
 {
     QList<NeuronSWC> tochange;
     vector<V3DLONG> allnumofbackup;
     if (!savetheblock.backup.empty())
    {
        for (int i=0;i<savetheblock.backup.size();i++)
         {
            allnumofbackup.push_back(savetheblock.backup.at(i).n);
         }
    }
    vector<double>allx,ally,allz;
    for (int i=0;i<neuron.size();i++)
    {
       allx.push_back(neuron.at(i).x);
       ally.push_back(neuron.at(i).y);
       allz.push_back(neuron.at(i).z);
    }
    if(curr_block)
        {
           double x_ie,y_ie,z_ie;
           double x_ae,y_ae,z_ae,x_e,y_e,z_e;
           int swc_num_of_block=0;
           int detect_have_type0=0;
           x_ie=curr_block->getOriginX();
           x_e=curr_block->getRezX();
           x_ae=x_e+x_ie-1;
           y_ie=curr_block->getOriginY();
           y_e=curr_block->getRezY();
           y_ae=y_e+y_ie-1;
           z_ie=curr_block->getOriginZ();
           z_e=curr_block->getRezZ();
           z_ae=x_e+z_ie-1;
           for (int i=0;i<neuron.size();i++)
           {
               if(x_ie==1 && y_ie==1)
               {
                  v3d_msg("You don't need to change all the annotations to type 0!");
                  return;
               }
               NeuronSWC s;
               s.x=neuron.at(i).x;
               s.y=neuron.at(i).y;
               s.z=neuron.at(i).z;
               s.type=neuron.at(i).type;
               s.radius=neuron.at(i).radius;
               s.pn=neuron.at(i).pn;
               s.n=neuron.at(i).n;
               if((x_ie<=allx.at(i)) && (allx.at(i)<=x_ae) && (y_ie<=ally.at(i)) && (ally.at(i)<=y_ae) && (z_ie<=allz.at(i)) && (allz.at(i)<=z_ae))
                {
                   swc_num_of_block+=1;
                   savetheblock.current_block_all.append(s);
                   if(s.type!=0)
                   {

                       //savetheblock.backup.append(s);
                       savetheblock.all_current_x.push_back(neuron.at(i).x);
                       savetheblock.all_current_y.push_back(neuron.at(i).y);
                       savetheblock.all_current_z.push_back(neuron.at(i).z);
                       savetheblock.all_current_type.push_back(neuron.at(i).type);
                   }
                   if(s.type==0)
                   {
                       detect_have_type0+=1;
                   }
                }
               else
                {
                   if(savetheblock.all_current_xtype0.empty())
                   {
                       savetheblock.current_block_all_else.append(s);
                   }
                   else
                   {
                       if((find(savetheblock.all_current_xtype0.begin(), savetheblock.all_current_xtype0.end(),s.x) !=savetheblock.all_current_xtype0.end()) && (find(savetheblock.all_current_ytype0.begin(), savetheblock.all_current_ytype0.end(),s.y)!=savetheblock.all_current_ytype0.end()))
                       {
                          s.type=0;
                          savetheblock.current_block_all_else.append(s);
                       }
                       else //((find(savetheblock.all_current_xtype0.begin(), savetheblock.all_current_xtype0.end(),s.x) ==savetheblock.all_current_xtype0.end()) && (find(savetheblock.all_current_ytype0.begin(), savetheblock.all_current_ytype0.end(),s.y)==savetheblock.all_current_ytype0.end()))
                       {
                          savetheblock.current_block_all_else.append(s);
                       }
                   }

                }
           }
           if((swc_num_of_block==swcnum) && (detect_have_type0>1))
           {
               v3d_msg("There are some annotations with the check type(type=0) in this neuron,please change them to other type firstly!");
               return;
           }
           //v3d_msg(QString("backup size[%1]!").arg(savetheblock.backup.size()));
           //v3d_msg(QString("backupelse size[%1]!").arg(savetheblock.backupelse.size()));
           for (int i=0;i<savetheblock.current_block_all.size();i++)
           {
               NeuronSWC s;
               s.x=savetheblock.current_block_all.at(i).x;
               s.y=savetheblock.current_block_all.at(i).y;
               s.z=savetheblock.current_block_all.at(i).z;
               s.type=0;
               s.radius=savetheblock.current_block_all.at(i).radius;
               s.pn=savetheblock.current_block_all.at(i).pn;
               s.n=savetheblock.current_block_all.at(i).n;
               tochange.append(s);
               savetheblock.all_current_xtype0.push_back(s.x);
               savetheblock.all_current_ytype0.push_back(s.y);
               savetheblock.all_current_ztype0.push_back(s.z);
           }
           for (int i=0;i<savetheblock.current_block_all_else.size();i++)
           {
               tochange.append(savetheblock.current_block_all_else.at(i));
           }
           QHash <int, int>  hashNeuron;
           for(V3DLONG j=0; j<tochange.size();j++)
           {
              hashNeuron.insert(tochange[j].n, j);
           }
           //v3d_msg(QString("There are totally [%1] swc nodes in your neuron of current block!").arg(swc_num_of_block));
           result.listNeuron=tochange;
           //v3d_msg(QString("tochange size[%1]!").arg(tochange.size()));
           result.hashNeuron=hashNeuron;
           callback.setSWCTeraFly(result);          
        }
   }
   else
   {
      v3d_msg("You have no annotations in your current window,please load an ano. file firstly!");
      return;
   }
}
void return_checked(V3DPluginCallback2 &callback, QWidget *parent)
{
    //QList<NeuronSWC> toreturn=savetheblock.backupelse;
    //QList<NeuronSWC> toreturnbackup=savetheblock.backup;
    if(savetheblock.all_current_x.empty())
    {
        v3d_msg("There are nothing you need to put back in terafly!");
        return;
    }
    QList<NeuronSWC> current_to_return,currentelse_to_return;
    NeuronTree nt=callback.getSWCTeraFly();
    QList<NeuronSWC> neuron = nt.listNeuron;
    //const Image4DSimple *curr_block=callback.getImageTeraFly();
    NeuronTree result1;
    for (int i=0;i<neuron.size();i++)
    {
        NeuronSWC s;
        s.x=neuron.at(i).x;
        s.y=neuron.at(i).y;
        s.z=neuron.at(i).z;
        s.type=neuron.at(i).type;
        s.radius=neuron.at(i).radius;
        s.pn=neuron.at(i).pn;
        s.n=neuron.at(i).n;
      if(s.type==0)
      {
         for (int j=0;j<savetheblock.all_current_x.size();j++)
           {
              if ((s.x==savetheblock.all_current_x.at(j)) && (s.y==savetheblock.all_current_y.at(j)) && (s.z==savetheblock.all_current_z.at(j)))
             {
                s.type=savetheblock.all_current_type.at(j);
                current_to_return.append(s);
                break;
             }
           }
       }
      else {current_to_return.append(s);}
    }
    /*vector<double>allx,ally,allz;
    for (int i=0;i<neuron.size();i++)
    {
       allx.push_back(neuron.at(i).x);
       ally.push_back(neuron.at(i).y);
       allz.push_back(neuron.at(i).z);
    }
    if(curr_block)
      {
        double x_ie,y_ie,z_ie;
        double x_ae,y_ae,z_ae,x_e,y_e,z_e;
        x_ie=curr_block->getOriginX();
        x_e=curr_block->getRezX();
        x_ae=x_e+x_ie-1;
        y_ie=curr_block->getOriginY();
        y_e=curr_block->getRezY();
        y_ae=y_e+y_ie-1;
        z_ie=curr_block->getOriginZ();
        z_e=curr_block->getRezZ();
        z_ae=x_e+z_ie-1;
        for (int i=0;i<neuron.size();i++)
        {
            NeuronSWC s;
            s.x=neuron.at(i).x;
            s.y=neuron.at(i).y;
            s.z=neuron.at(i).z;
            s.type=neuron.at(i).type;
            s.radius=neuron.at(i).radius;
            s.pn=neuron.at(i).pn;
            s.n=neuron.at(i).n;
            if((x_ie<=allx.at(i)) && (allx.at(i)<=x_ae) && (y_ie<=ally.at(i)) && (ally.at(i)<=y_ae) && (z_ie<=allz.at(i)) && (allz.at(i)<=z_ae))
            {
                if(s.type==0)
                {
                    for (int j=0;j<savetheblock.all_current_x.size();j++)
                    {
                       if ((s.x==savetheblock.all_current_x.at(j)) && (s.y==savetheblock.all_current_y.at(j)) && (s.z==savetheblock.all_current_z.at(j)))
                       {
                           s.type=savetheblock.all_current_type.at(j);
                           current_to_return.append(s);
                           break;
                       }
                    }
                }
                else
                {
                  current_to_return.append(s);
                }
            }
            else
            {
                if(s.type==0)
                {
                    for (int j=0;j<savetheblock.all_current_x.size();j++)
                    {
                       if ((s.x==savetheblock.all_current_x.at(j)) && (s.y==savetheblock.all_current_y.at(j)) && (s.z==savetheblock.all_current_z.at(j)))
                       {
                           s.type=savetheblock.all_current_type.at(j);
                           currentelse_to_return.append(s);
                       }
                    }
                }
                else
                {
                  currentelse_to_return.append(s);
                }
            }
        }
        for(int i=0;i<current_to_return.size();i++)
        {
           currentelse_to_return.append(current_to_return.at(i));
        }*/
        QHash <int, int>  hashNeuron;
        for(V3DLONG j=0; j<current_to_return.size();j++)
        {
           hashNeuron.insert(current_to_return[j].n, j);
        }
        result1.listNeuron=current_to_return;
        result1.hashNeuron=hashNeuron;
        //v3d_msg(QString("There are totally [%1] swc nodes you wanna to return back!").arg(current_to_return.size()));
        callback.setSWCTeraFly(result1);
        //v3d_msg("All the annotations you have chenged have been put back to terafly!");
 }




