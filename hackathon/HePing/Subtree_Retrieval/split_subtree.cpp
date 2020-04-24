#include"split_subtree.h"
#include<v3d_interface.h>
#include<algorithm>
#define MAX_PATH 120
using namespace std;
bool find_all_files(QString datadir,vector<QString> &swcfiles){
    const size_t FILE_TYPE=2;
    string support_format[FILE_TYPE]={".swc",".eswc"};
    string dir=datadir.toStdString();
    //qDebug()<<dir.c_str();
    const char* lppath=dir.c_str();
    char szFind[MAX_PATH];
    WIN32_FIND_DATAA FindFileData;
    strcpy(szFind,lppath);
    strcat(szFind,"/*.*");//该目录下所有文件
    //创建搜索句柄
    HANDLE Hfind=FindFirstFileA(szFind,&FindFileData);

    if(INVALID_HANDLE_VALUE==Hfind){
        qDebug()<<"path is error!";
    }
    do{
        if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){//不是目录
            //获取文件名
            string lower_name(FindFileData.cFileName);
            if(support_format[0]==lower_name.substr(lower_name.length()-support_format[0].length())||
                    support_format[1]==lower_name.substr(lower_name.length()-support_format[1].length())){
                swcfiles.push_back(FindFileData.cFileName);
            }else
                continue;
        }else{//目录，继续往内部搜索

        }



    }while(FindNextFileA(Hfind,&FindFileData));
    FindClose(Hfind);//关闭搜索句柄

    return true;
}

bool SplitSubtree(NeuronTree nt,vector<bool> &select,QString para,int level,vector<Subtree> &subtrees){
    //对每一个swc文件进行分割,参数指定subtree的深度，返回所有的subtree的集合
    V3DLONG size = nt.listNeuron.size();
    NeuronSWC soma;
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(size,vector<V3DLONG>());//segment的所有孩子节点index
    for(V3DLONG i=0;i<size;i++){
        V3DLONG par=nt.listNeuron[i].parent;
        if(par<0){
            soma=nt.listNeuron[i];
            continue;
        }
        if(select[i]==false)//该结点没有被划分为某个子树部分
        children[nt.hashNeuron.value(par)].push_back(i);
    }


    multimap<V3DLONG,int> index_order;
    multimap<V3DLONG,int> birfu_order;
    index_order.clear();
    birfu_order.clear();
    //找到叶子节点，并分配给它向心顺序
    for(V3DLONG j=0;j<size;j++){
        if(children[j].size()==0&&select[j]==false){//叶子节点,并且不是已存在subtree的结点
            index_order.insert(pair<V3DLONG,int>(j,0));//叶子节点的order为0
        }
    }
    qDebug()<<"leaf size:"<<index_order.size();
    //向上遍历所有分叉点，并赋给它们order
    multimap<V3DLONG,int>::iterator iter;
    for(iter=index_order.begin();iter!=index_order.end();iter++){

        V3DLONG index=iter->first;//该节点的index,是否正确？？？
        qDebug()<<"leaf index:"<<index;
        V3DLONG parent_index=nt.listNeuron[index].parent;//该节点的父节点
        //这是后面对subtree进行标记时存在的问题
        if(parent_index==-1||select[nt.hashNeuron.value(parent_index)]==true)continue;//防止下标越界，如果该叶节点父亲是之前subtree的中的分叉点，则不应该作为叶子节点
        qDebug()<<parent_index;
        //出现从叶子节点往上遍历无法停止，也没有到达parent_index==-1的情况？？？？
        int order=0;
        while(1){//解决
        while(children[nt.hashNeuron.value(parent_index)].size()==1){
            parent_index=nt.listNeuron[nt.hashNeuron.value(parent_index)].parent;
            if(parent_index==-1||select[nt.hashNeuron.value(parent_index)]==true)break;
        }
        if(children[nt.hashNeuron.value(parent_index)].size()>=2){
            order++;
            qDebug()<<"---"<<parent_index;
            qDebug()<<order;

            if(birfu_order.count(parent_index)>=1)//出现过
             {
                if(order<=birfu_order.find(parent_index)->second)//order小于之前的赋值则跳过，不需要向上因为其他的父节点也会比它大
                    break;
                else
                { birfu_order.find(parent_index)->second=order;
                  //parent_index=nt.listNeuron[nt.hashNeuron.value(parent_index)].parent;
                }
            }

            else{//没出现过该分叉点

                birfu_order.insert(pair<V3DLONG,int>(parent_index,order));//内部分叉点的order,左右子树最大的因此要注意更新

            }

            parent_index=nt.listNeuron[nt.hashNeuron.value(parent_index)].parent;
            qDebug()<<"--"<<parent_index;



        }
        if(parent_index==-1)break;//防止死循环
        if(select[nt.hashNeuron.value(parent_index)]==true)break;//如果父节点被访问过，则停止向上

        }

    }
    qDebug()<<"birfucation num:"<<birfu_order.size();
    //根据结点的order和paramater进行分割

    int depth=para.toInt();//子树的深度
    //qDebug()<<"depth:"<<depth;
    multimap<V3DLONG,int> roots;//保存所有子树的根结点的id
    multimap<V3DLONG,int>::iterator it;
    for(it=birfu_order.begin();it!=birfu_order.end();it++){
        //访问所有分叉点，凡是order%depth=0的作为一个子树的根
        //剩下未满足条件的root如何处理，一般都是以soma为root的子树，针对每个从soma出来的分支单独split
        //qDebug()<<"birfu order:"<<it->second;
        if(it->second==depth){//一层层往内部进行分割，为什么效果和之前没啥区别
            qDebug()<<"root index:"<<it->first;
            //root的order需修改
            roots.insert(pair<V3DLONG,int>(it->first,level));//每一个subtree的根节点和它们的order
        }
    }

    //根据subtree的root分割出所有的subtree,这里有问题？？
    multimap<V3DLONG,int>::iterator it1;
    vector<V3DLONG> queue;
    qDebug()<<"subtree num:"<<roots.size();
    if(roots.size()==0)return false;
    for(it1=roots.begin();it1!=roots.end();it1++){
        //遍历所有的结点
        V3DLONG index=it1->first;
        Subtree tree;
        tree.order=it1->second;//需修改-----------
        qDebug()<<"tree order:"<<it1->second;
        qDebug()<<"tree root index:"<<index;
        //特殊情况到soma点，会导致很多子树分配到一个树中,这种特殊情况不知是否应该特殊处理
//        if(nt.listNeuron[nt.hashNeuron.value(index)].parent==-1){
//            //soma点
//            for(int j=0;j<children[nt.hashNeuron.value(index)].size();j++){
//                queue.push_back(children[nt.hashNeuron.value(index)][j]);
//            }
//            continue;
//        }
//        else{
            //将根结点放入
            queue.push_back(index);
//        }

        while(!queue.empty()){
            index=queue.front();
            queue.erase(queue.begin());


            //否应该修改为从叶节点逐渐向上找，直到到达一个root点，从root点作为叶子节点开始重新计算order
            //从根节点开始往下寻找它的子树中的结点
            tree.listNeuron.push_back(nt.listNeuron[nt.hashNeuron.value(index)]);
            //有待查验是否正确标记root
            select[nt.hashNeuron.value(index)]=true;
            qDebug()<<"child size:"<<children[nt.hashNeuron.value(index)].size();

            for(int i=0;i<children[nt.hashNeuron.value(index)].size();i++){
                V3DLONG index1=nt.listNeuron[children[nt.hashNeuron.value(index)][i]].n;//开始遍历它的孩子
                qDebug()<<"**"<<index1;
                while(children[nt.hashNeuron.value(index1)].size()==1){
                    //qDebug()<<index;
                    tree.listNeuron.push_back(nt.listNeuron[nt.hashNeuron.value(index1)]);
                    select[nt.hashNeuron.value(index1)]=true;
                    //selectid.push_back(index);
                    index1=nt.listNeuron[children[nt.hashNeuron.value(index1)][0]].n;
                }
                //少了分叉点，已解决，但是结果似乎还是有点问题，似乎不全是深度为4的？？？？
                if(children[nt.hashNeuron.value(index1)].size()>=2){//判断是否到下一个root
                    qDebug()<<"child 2:"<<index1;
                    if(roots.find(index1)==roots.end()){//不是下一个subtree的root,则继续往下
                        tree.listNeuron.push_back(nt.listNeuron[nt.hashNeuron.value(index1)]);
                        select[nt.hashNeuron.value(index1)]=true;
                        //selectid.push_back(index);
                        qDebug()<<nt.listNeuron[children[nt.hashNeuron.value(index1)][0]].n<<nt.listNeuron[children[nt.hashNeuron.value(index1)][1]].n;
                        queue.push_back(nt.listNeuron[children[nt.hashNeuron.value(index1)][0]].n);
                        queue.push_back(nt.listNeuron[children[nt.hashNeuron.value(index1)][1]].n);
                    }
                    else{//是下一个subtree的继续其他分支是否也到达，若都到达则subtree形成
                        tree.listNeuron.push_back(nt.listNeuron[nt.hashNeuron.value(index1)]);
                        select[nt.hashNeuron.value(index1)]=true;
                        continue;

                    }
                }
                //如果index的孩子为0，到达叶子节点也应该被放入tree中------
                else if(children[nt.hashNeuron.value(index1)].size()==0){
                    qDebug()<<index1;
                    tree.listNeuron.push_back(nt.listNeuron[nt.hashNeuron.value(index1)]);
                    select[nt.hashNeuron.value(index1)]=true;

                }

            }



    }
    qDebug()<<"subtree size:"<<tree.listNeuron.size();
    if(tree.listNeuron.size()!=0)
    subtrees.push_back(tree);
    //更新swc，用于下一次split_subtree,将selectid中结点删除，同时要注意subtree的root不要删除

    }

    return true;
}
