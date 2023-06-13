#include "ExternalFunction.h"


bool SortSWC(QList<NeuronSWC> &neurons, QList<NeuronSWC> &result, long newrootid, double thres)
{
    if(neurons.size()==0){
        return(false);
    }
    QList<V3DLONG> nlist;
    for(int i=0; i<neurons.size(); i++){
        nlist.append(neurons.at(i).n);
    }
    //create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
    QHash<V3DLONG, NeuronSWC> LUT_newid_to_node;
    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons, LUT_newid_to_node);
    auto keys=LUT.keys();
    std::sort(keys.begin(),keys.end());
    //create a new id list to give every different neuron a new id
    auto idlist=LUT.values();
    std::unique(idlist.begin(),idlist.end());
    std::sort(idlist.begin(),idlist.end());
    V3DLONG siz = idlist.size();
    // create a vector to keep neighbors of each node
    QVector< QVector<V3DLONG> > neighbors = get_neighbors(neurons, LUT);
    // Find the new id of the new root
    V3DLONG root = 0;
    if (newrootid==VOID_VALUE)  // If unspecified, use the 1st root as new root.
    {
        for (V3DLONG i=0;i<neurons.size();i++)
            if (neurons.at(i).pn==-1){
                root = idlist.indexOf(LUT.value(neurons.at(i).n));
                qDebug()<<"value of root"<<root;
                break;
            }
    }
    else{
        root = idlist.indexOf(LUT.value(newrootid));
        if (LUT.keys().indexOf(newrootid)==-1)
        {
            v3d_msg("The new root id you have chosen does not exist in the SWC file.");
            return(false);
        }
    }
    //Major steps
    //do a DFS for the the matrix and re-allocate ids for all the nodes
    QList<V3DLONG> neworder;
    QList<V3DLONG> cur_neworder;
    QList<V3DLONG> component_id;
    for(int i=0; i<siz; i++){
        component_id.append(0);
    }
    V3DLONG sorted_size = 0;
    int cur_group = 1;
    // Begin with the new root node and
    // generate the 1st sorted tree.
    cur_neworder= DFS(neighbors, root, siz);
    qDebug()<<QString("cur_neworder_first=%1").arg(cur_neworder.size());
    sorted_size += cur_neworder.size();
    neworder.append(cur_neworder);
    for(int i=0; i<cur_neworder.size(); i++){
        component_id[cur_neworder.at(i)] = cur_group;
    }
    // Continue to sort the rest of the tree
    while (sorted_size <siz)
    {
        V3DLONG new_root;
        cur_group++;
        for (V3DLONG iter=0;iter<siz;iter++)
        {
            if (!neworder.contains(iter))
            {
                new_root = iter;
                break;
            }
        }
        cur_neworder = DFS(neighbors, new_root, siz);
        qDebug()<<QString("cur_neworder_second=%1").arg(cur_neworder.size());
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id[cur_neworder.at(i)] = cur_group;
        }
    }
    QList<V3DLONG> output_newroot_list;
    if(thres>=0){  // If distance threshold > 0: make new connections
        output_newroot_list.append(root);
        while(cur_group>1)
        {
            double min = VOID_VALUE;
            double dist2 = 0;
            int mingroup = 1;
            // Find the closest pair of nodes between group 1 and the rest.
            V3DLONG m1,m2;
            for (V3DLONG ii=0;ii<siz;ii++)
            {
                if (component_id[ii]==1)
                {
                    for (V3DLONG jj=0;jj<siz;jj++)
                        if (component_id[jj]!=1)
                        {
                            dist2 = computeDist2(LUT_newid_to_node.value(ii),
                                                 LUT_newid_to_node.value(jj));
                            if (dist2<min)
                            {
                                min = dist2;
                                mingroup = component_id[jj];
                                m1 = ii;
                                m2 = jj;
                            }
                        }
                }
            }
            for (V3DLONG i=0;i<siz;i++)
            {
                if (component_id[i]==mingroup)
                {
                    component_id[i] = 1;
                }
            }
            if (min<=thres)
            {
                if(!neighbors.at(m1).contains(m2)){neighbors[m1].push_back(m2);}
                if(!neighbors.at(m2).contains(m1)){neighbors[m2].push_back(m1);}
            }
            else{  // set component the node closest to group 1 is root
                output_newroot_list.append(m2);
            }
            cur_group--;
        }
    }
    else{
        int tp_group = 0;
        for(int i=0; i<siz; i++){
            if(component_id.at(i) != tp_group){
                output_newroot_list.append(neworder.at(i));
                tp_group = component_id.at(i);
            }
        }
    }
    // DFS sort of the neuronlist after new connections
    for (int i=0;i<siz;i++)
    {
        component_id[i] = 0;
        neworder[i]= VOID_VALUE;
    }
    component_id.clear();
    neworder.clear();
    sorted_size = 0;
    cur_group = 1;

    V3DLONG offset=0;
    for(V3DLONG i=0; i<output_newroot_list.size(); i++)
    {
        V3DLONG new_root = output_newroot_list.at(i);
        V3DLONG cnt = 0;
        // Sort current component;
        cur_neworder= DFS(neighbors, new_root, siz);
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id.append(cur_group);
        }
        NeuronSWC S;
        S = LUT_newid_to_node.value(new_root);
        S.n = offset+1;
        S.pn = -1;
        result.append(S);
        cnt++;
        for (V3DLONG ii=offset+1;ii<(sorted_size);ii++)
        {
            for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
            {
                V3DLONG cid = neworder[ii];
                V3DLONG pid = neworder[jj];
                // If there's an edge between the two nodes, then pid is the parent of cid
                if (pid!=VOID_VALUE && cid!=VOID_VALUE && neighbors.at(pid).contains(cid))
                {
                        NeuronSWC S;
                        S = LUT_newid_to_node.value(cid);
                        S.n = ii+1;
                        S.pn = jj+1;

                        result.append(S);
                        cnt++;
                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
                }
            }
        }
        offset += cnt;
    }

    if ((sorted_size)<siz) {
        return false;
    }
    // free space.
    neighbors.clear();
    return(true);
}

QHash<long, long> getUniqueLUT(QList<NeuronSWC> &neurons, QHash<long, NeuronSWC> &LUT_newid_to_node)
{
    QHash<V3DLONG,V3DLONG> LUT;
    V3DLONG cur_id=0;
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        V3DLONG j=0;
        for (j=0;j<i;j++) // Check whether this node is a duplicated with the previous ones
        {
            if (neurons.at(i).x==neurons.at(j).x
                    && neurons.at(i).y==neurons.at(j).y
                    && neurons.at(i).z==neurons.at(j).z)
            {
                break;
            }
        }
        if(i==j){  // not a duplicate
            LUT.insert(neurons.at(i).n, cur_id);
            LUT_newid_to_node.insert(cur_id, neurons.at(j));
            cur_id++;
        }
        else{  // is a duplicate
            LUT.insert(neurons.at(i).n, LUT.value(neurons.at(j).n));
        }
    }
    return (LUT);
}

QVector<QVector<long> > get_neighbors(QList<NeuronSWC> &neurons, const QHash<long, long> &LUT)
{
    // generate neighbor lists for each node, using new ids.
    // LUT (look-up table): old name -> new ids
    // ids are the line numbers
    // names are the node names (neurons.name)
    auto idlist=LUT.values();
    std::unique(idlist.begin(),idlist.end());
    int siz = idlist.size();
    QList<int> nlist;
    for(V3DLONG i=0; i<neurons.size(); i++){nlist.append(neurons.at(i).n);}
    QVector< QVector<V3DLONG> > neighbors = QVector< QVector<V3DLONG> >(siz, QVector<V3DLONG>() );
    int mt=neurons.size()/5;
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        // Find parent node
        if(i%mt==1||i==neurons.size()-1)
            qDebug()<<i<<"/"<<neurons.size();
        int pid_old = nlist.lastIndexOf(neurons.at(i).pn);
        if(pid_old<0){
            continue;  // Skip root nodes
        }
        else{
            int pname_old = neurons.at(pid_old).n;
            int cname_old = neurons.at(i).n;
            int pid_new = LUT.value(pname_old);
            int cid_new = LUT.value(cname_old);
            if((pid_new>=siz) || (cid_new>=siz)){
                v3d_msg(QString("Out of range [0, %1]: pid:%2; cid:%3").arg(siz).arg(pid_new).arg(cid_new));
            }
            // add a new neighbor for the child node
            if(!neighbors.at(cid_new).contains(pid_new)){
                neighbors[cid_new].push_back(pid_new);
            }
            // add a new neighbor for the parent node
            if(!neighbors.at(pid_new).contains(cid_new)){
                neighbors[pid_new].push_back(cid_new);
            }
        }
    }
    return neighbors;
}

QList<long> DFS(QVector<QVector<long> > neighbors, long newrootid, long siz)
{
    // siz: size of the whole neuronlist
    // The neuronlist may include multiple components
    // A component is a connected tree
    // Sorted components: other components that have already been sorted.
    // Current component: the component where newroot resides. We will sort it and append it to the sorted components

    // sorted_size: size of sorted components
    // neworder: new order of the sored components
    // *group: id of the current component

    QList<V3DLONG> neworder;

    // DFS to sort current component;

    // Initialization
    QStack<int> pstack;
    QList<int> visited;
    for(int i=0;i<siz; i++){visited.append(0);}
    visited[newrootid]=1;
    pstack.push(newrootid);
    neworder.append(newrootid);

    // Tree traverse
    bool is_push;
    int pid;
    while(!pstack.isEmpty()){
        is_push = false;
        pid = pstack.top();
        // whether exist unvisited neighbors of pid
        // if yes, push neighbor to stack;
        QVector<V3DLONG>::iterator it;
        QVector<V3DLONG> cur_neighbors = neighbors.at(pid);
        for(it=cur_neighbors.begin(); it!=cur_neighbors.end(); ++it)
        {
            if(visited.at(*it)==0)
            {
                pstack.push(*it);
                is_push=true;
                visited[*it]=1;
                neworder.append(*it);
                break;
            }
        }
        if(!is_push){
            pstack.pop();
        }
    }
    return neworder;
}

double computeDist2(const NeuronSWC &s1, const NeuronSWC &s2)
{
    double xx = s1.x-s2.x;
    double yy = s1.y-s2.y;
    double zz = s1.z-s2.z;
    return sqrt(xx*xx+yy*yy+zz*zz);
}

bool export_list2file(QList<NeuronSWC> &lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin sort_neuron_swc"<<"\n";
    myfile<<"# source file(s): "<<fileOpenName<<"\n";
    myfile<<"# id,type,x,y,z,r,pid"<<"\n";
    for (V3DLONG i=0;i<lN.size();i++)
    {
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << " ";
        if(fileSaveName.endsWith(".eswc",Qt::CaseInsensitive))
        {
            myfile << lN.at(i).seg_id << " " << lN.at(i).level << " " << lN.at(i).creatmode << " " << lN.at(i).timestamp << " " << (long)lN.at(i).tfresindex;
        }
        myfile << "\n";
    }
    file.close();
    return true;
}

bool combine_list2file(QList<NeuronSWC> &lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::Text|QIODevice::Append))
        return false;
    QTextStream myfile(&file);
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    return true;
}


bool rescale_to_0_255_and_copy(unsigned char *inimg, long datalen, double &minn, double &maxx, unsigned char *outimg)
{
    if (!inimg || !outimg || datalen<=0) return false;

    minn = maxx = inimg[0];
    for (V3DLONG i=0;i<datalen;i++)
    {
        unsigned char v = inimg[i];
        if (v<minn) minn=v;
        else if (v>maxx) maxx=v;
    }

    if (minn!=maxx)
    {
        double s = 255.0/(maxx-minn);

        for (V3DLONG i=0;i<datalen;i++)
        {
            outimg[i] = (unsigned char)(s*(inimg[i]-minn));
        }
    }
    else
    {
        for (V3DLONG i=0;i<datalen;i++)
        {
            outimg[i] = 0;
        }
    }

    return true;
}

char *qstring2char(QString text)
{
    char *result=new char[text.length()+1];
    result[text.length()]='\0';
    strcpy(result,text.toStdString().c_str());
    return result;
}


Delivery PackImage(Image4DSimple *img, QString IP, QString Port,QString filepath,QString name,QString method,bool sendimg,bool quality)
{
    Delivery delivery;
    if(!img){
        return delivery;
    }
    if(!sendimg)
        delivery.imgdata=nullptr;
    else
        delivery.imgdata=img->getRawData();
    delivery.datalen=img->getTotalBytes();
    delivery.xdim=img->getXDim();
    delivery.ydim=img->getYDim();
    delivery.zdim=img->getZDim();
    delivery.cdim=img->getCDim();
    delivery.IP=IP;
    delivery.Port=Port;
    delivery.filepath=filepath;
    delivery.name=name;
    delivery.method=method;
    if(quality){
        ImageQuality iq(img);
        delivery.quality=iq.getQuality()+",";
    }
    return delivery;
}

bool mean_and_std(unsigned char *data, V3DLONG n, double & ave, double & sdev)
{
    if (!data || n<=0)
      return false;

    int j;
    double ep=0.0,s,p;

    if (n <= 1)
    {
      //printf("len must be at least 2 in mean_and_std\n");
      ave = data[0];
      sdev = (double)0;
      return true; //do nothing
    }

    s=0.0;
    for (j=0;j<n;j++) s += data[j];
    double ave_double=(double)(s/n); //use ave_double for the best accuracy

    double var=0.0;
    for (j=0;j<n;j++) {
        s=data[j]-(ave_double);
        var += (p=s*s);
    }
    var=(var-ep*ep/n)/(n-1);
    sdev=(double)(sqrt(var));
    ave=(double)ave_double; //use ave_double for the best accuracy

    return true;
}
