/* BoutonDectection_plugin.cpp
 * designed by shengdian
 * 2020-7-29 : by SD-Jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "BoutonDectection_plugin.h"
#include <QHash>

using namespace std;
Q_EXPORT_PLUGIN2(BoutonDectection, BoutonDectectionPlugin);
 
QStringList BoutonDectectionPlugin::menulist() const
{
	return QStringList() 
        <<tr("BoutonDection_Img")
		<<tr("about");
}

QStringList BoutonDectectionPlugin::funclist() const
{
	return QStringList()
        <<tr("BoutonDection_terafly")
        <<tr("BoutonDection_Img")
        <<tr("BoutonDection_filter")
		<<tr("help");
}

void BoutonDectectionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("BoutonDection_Img"))
	{
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        V3DLONG  in_sz[4];
        unsigned char* inimg1d = p4DImage->getRawData();

        in_sz[0] = p4DImage->getXDim();
        in_sz[1] = p4DImage->getYDim();
        in_sz[2] = p4DImage->getZDim();
        in_sz[3] = p4DImage->getCDim();

        cout<<"Img size,x="<<in_sz[0]<<",y="<<in_sz[1]<<",z="<<in_sz[2]<<endl;
        long sz01 = in_sz[0] * in_sz[1];
        long sz0 = in_sz[0];
        NeuronTree nt =callback.getSWC(curwin);
        QList<NeuronSWC>& listNeuron =  nt.listNeuron;

        V3DLONG siz = nt.listNeuron.size();
        cout<<"SWC size:"<<siz<<endl;
        int max_intensity=1;
        QHash <int, int>  hashNeuron;
        hashNeuron.clear();
        for (V3DLONG i=0;i<siz;i++)
        {
            listNeuron[i].radius=1;
            hashNeuron.insert(listNeuron[i].n,i);
        }

        QDialog * dialog = new QDialog();
        QLineEdit * savePath_box = new QLineEdit("");
        QLineEdit * thresh_box = new QLineEdit("18");
        QCheckBox * useNeighborArea_checkbox = new QCheckBox("Use Neighbor Area");
        useNeighborArea_checkbox->setChecked(true);
        {
            QGridLayout * layout = new QGridLayout;
            layout->addWidget(new QLabel("Save path"), 0, 0, 1, 1);
            layout->addWidget(savePath_box, 0, 1, 1, 5);
            layout->addWidget(new QLabel("Threshold"), 1, 0, 1, 1);
            layout->addWidget(thresh_box, 1, 1, 1, 5);
            layout->addWidget(useNeighborArea_checkbox, 2, 0, 1, 6);
            QPushButton * ok = new QPushButton("Ok");
            QPushButton * cancel = new QPushButton("Cancel");
            ok->setDefault(true);
            layout->addWidget(ok, 5, 0, 1, 3);
            layout->addWidget(cancel, 5, 3, 1, 3);
            dialog->setLayout(layout);
            connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        }
        if(dialog->exec() != QDialog::Accepted) return;
        string save_path = savePath_box->text().toStdString();
        bool useNeighborArea = useNeighborArea_checkbox->isChecked();
        int threshold = atof(thresh_box->text().toStdString().c_str());
        QString savepath=QString::fromStdString(save_path);
        QDir path(savepath);
        if(!path.exists())
        {
            path.mkpath(savepath);
        }

        for(V3DLONG i=0;i<siz;i++)
        {
            NeuronSWC s = listNeuron[i];
            if(listNeuron[i].radius==1)
            {

                int thisx,thisy,thisz;
                thisx=s.x;
                thisy=s.y;
                thisz=s.z;

                if(useNeighborArea)
                {
                    listNeuron[i].radius=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                    NeuronSWC out=nodeIntensity(inimg1d,listNeuron[i].x,listNeuron[i].y,listNeuron[i].z,in_sz,1);
                    if(listNeuron[i].radius<out.radius)
                    {
                        cout<<"update intensity:old="<<listNeuron[i].radius<<";New="<<out.radius<<endl;
                        cout<<"Distance changes:x="<<(listNeuron[i].x-out.x)<<";y="<<(listNeuron[i].y-out.y)<<";z="<<(listNeuron[i].z-out.z)<<endl;
                        listNeuron[i].radius=out.radius;
                        listNeuron[i].x=out.x;
                        listNeuron[i].y=out.y;
                        listNeuron[i].z=out.z;
                    }
                }
                else
                {
                    listNeuron[i].radius=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                    float childIntensity=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
//                    listNeuron[i].radius=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                    //get their parent node intensity

                    if(s.parent>0)
                    {
                        long pid=hashNeuron.value(s.parent);
                        NeuronSWC sp=listNeuron[pid];
                        //consider the distance of child-parent node
                        float distanceThre=5;
                        float child_parent_distance=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
                        if(child_parent_distance<distanceThre*distanceThre)
                        {

                            NeuronSWC sm;
                            sm.x=(sp.x-s.x)/2+s.x;
                            sm.y=(sp.y-s.y)/2+s.y;
                            sm.z=(sp.z-s.z)/2+s.z;
                            float parentIntensity=inimg1d[int(sp.z) * sz01 + int(sp.y) * sz0 + int(sp.x)];
                            float middleIntensity=inimg1d[int(sm.z) * sz01 + int(sm.y) * sz0 + int(sm.x)];
                            //this may need another threshold
                            int updateThre=10;
                            if((middleIntensity-childIntensity>updateThre)&&(middleIntensity-parentIntensity>updateThre))
                            {
                                cout<<"update child intensity:old = "<<childIntensity<<";New="<<middleIntensity<<". this node id:"<<s.n<<endl;

                                //                    update child node Intensity
                                listNeuron[i].radius=middleIntensity;
                            }
                        }
                    }

                }
                if(max_intensity<listNeuron[i].radius)
                    max_intensity=listNeuron[i].radius;
            }
        }
        QString outswc_file =savepath+"/"+"IntensityResult_original.swc";
        writeSWC_file(outswc_file,nt);
        QList <CellAPO> apolist=getBouton(nt,threshold,1);
        QString apo_file_path = savepath +"/"+ "bouton.apo";
        writeAPO_file(apo_file_path,apolist);
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
        //set back the result
        for(V3DLONG i=0;i<siz;i++)
        {
            listNeuron[i].radius/=max_intensity;
            listNeuron[i].radius*=10;
        }
        QString outswc_file_scale =savepath+"/"+"IntensityResult_scale.swc";
        writeSWC_file(outswc_file_scale,nt);
        callback.setSWC(curwin,nt);
	}
	else
	{
        v3d_msg(tr("Email: shengdianjiang@seu.edu.cn"
			"Developed by SD-Jiang, 2020-7-29"));
	}
}


bool BoutonDectectionPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    if (func_name == tr("BoutonDection_terafly"))
	{
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        string inswc_file = infiles[1];
        int allnode=0;
        if(inparas.size()>=1)
        {
            allnode=atoi(inparas[0]);
        }
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        getBoutonInTerafly(callback,inimg_file,nt,allnode);
        string outswc_file = inswc_file + "_IntensityResult.swc";
        writeSWC_file(QString::fromStdString(outswc_file),nt);
	}
    else if (func_name == tr("BoutonDection_filter"))
    {
        string inswc_file,inimg_file;
        cout<<"size of the input file is "<<infiles.size()<<endl;
        cout<<"size of the inpara file is "<<inparas.size()<<endl;
        cout<<"size of the output file is "<<outfiles.size()<<endl;
        if(infiles.size()==1)
        {
            inswc_file = infiles[0];
        }
        else if(infiles.size()==2)
        {
            inimg_file = infiles[0];
            inswc_file = infiles[1];
        }
        else
        {
            printHelp();
            return false;
        }
        int threshold=20;
        int allnode=0;
//        cout<<threshold<<" is ."<<endl;
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
//        cout<<nt.listNeuron.size()<<" size"<<endl;
        if(inparas.size()>=1)
        {
            threshold=atoi(inparas[0]);
        }
        else if(inparas.size()>=2)
        {
            allnode=atoi(inparas[1]);
        }

        QList <CellAPO> apolist=getBouton(nt,threshold,allnode);
        string apo_file_path = inswc_file + "_bouton.apo";
        writeAPO_file(QString::fromStdString(apo_file_path),apolist);
        if(outfiles.size()==1)
        {
            int crop_block_size=(inparas.size()>=3)?atoi(inparas[2]):32;
            string out_path=outfiles[0];
            getBoutonBlock(callback,inimg_file,apolist,out_path,crop_block_size);
        }
        else if(outfiles.size()>1)
            printHelp();return false;

    }
    else if (func_name == tr("BoutonDection_Img"))
    {
        /*Input1: img block path; Input2: SWC;*/
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        string inswc_file = infiles[1];

        int threshold=(inparas.size()>=1)?atoi(inparas[0]):20;
        int useNeighborArea=(inparas.size()>=2)?atoi(inparas[1]):0;
        int allnode=(inparas.size()>=3)?atoi(inparas[2]):0;
        //read img
        unsigned char * inimg1d = 0;
        V3DLONG in_sz[4];
        int datatype;
        if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return false;
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        getBoutonInImg(callback,inimg1d,in_sz,nt,useNeighborArea);
        string outswc_file = inswc_file + "_IntensityResult.swc";
        writeSWC_file(QString::fromStdString(outswc_file),nt);
        QList <CellAPO> apolist=getBouton(nt,threshold,allnode);
        string apo_file_path = inswc_file + "_bouton.apo";
        writeAPO_file(QString::fromStdString(apo_file_path),apolist);
        //scale the result
        int max_intensity=1;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            if(max_intensity<nt.listNeuron[i].radius)
                max_intensity=nt.listNeuron[i].radius;

        }
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            nt.listNeuron[i].radius/=max_intensity;
            nt.listNeuron[i].radius*=10;
        }
        QString outswc_file_scale =QString::fromStdString(inswc_file)+"_IntensityResult_scale.swc";
        writeSWC_file(outswc_file_scale,nt);

    }
	else if (func_name == tr("help"))
    {
        /* Version one,20200801
        1.Basic idea: get the intensity value of the reconstruction nodes and consider big intensity changes among child-parent nodes as bouton.
        2.If threshold of the intensity changes is low, there will be more bouton results.
        3.Input swc need to be refined, if the reconstructed nodes don't place at the center of the signal, the performance will be extremly ugly.
        (Maybe consider to add refinement in future version)
        Future:
        1. For testing purpose:
               1)crop img blocks and swc_in_block,in this way, the performance is easy to compute and visualize;
               2)(Vertical)move reconstruction nodes to signal, which is so-called refinement;
               3)(Horizontal)move reconstruction nodes to a local maxmimal intensity point, need to define the surrounding region, and may also need a purning afterwards.
               4) The threshold parameter is a place to improve the overall performance. (maybe the avearge of the intensity of listneuron)
        */
        printHelp();
	}
	else return false;

	return true;
}
/*Given a NeuronTree and a swc node in this NeuronTree, return std intensity of surrounding area (thre_size)*/
QHash<V3DLONG,int> getIntensityStd(NeuronTree nt,int thre_size)
{
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash<V3DLONG,int> id_threshold;//key is line id and value is std
    id_threshold.clear();
    QList<float> surIntensity;
//    long processedNode[siz];
    if(nt.listNeuron.size()==0)
    {
        cout<<"Can't read neuronTree and input error"<<endl;
        return id_threshold;
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC thisnode=listNeuron[i];
        //get a surrounding block nodelist
        float blockIntensityAll=thisnode.radius;
        V3DLONG blockNodeNum=1;
        surIntensity.clear();
        for (V3DLONG j=0;j<siz;j++)
        {
            NeuronSWC surnode=listNeuron[j];
            if(abs(surnode.x-thisnode.x)<thre_size&&
                    abs(surnode.y-thisnode.y)<thre_size&&
                    abs(surnode.z-thisnode.z)<thre_size)
            {
                blockIntensityAll+=surnode.radius;
                surIntensity.push_back(surnode.radius);
                blockNodeNum++;
            }
        }
        float blockIntensityAvearage=blockIntensityAll/blockNodeNum;

        //standard
        float varIntensity=0;
        for (V3DLONG j=0;j<blockNodeNum;j++)
        {
            varIntensity+=(surIntensity.at(j)-blockIntensityAvearage)*(surIntensity.at(j)-blockIntensityAvearage);
        }
        varIntensity/=blockNodeNum;
        float stdIntensity=sqrt(varIntensity);
        if(stdIntensity<0)
        {
            cout<<"varIntensity="<<varIntensity<<endl;
            cout<<"Avearage="<<blockIntensityAvearage<<";index="<<i<<endl;
            cout<<"blockNode Num="<<blockNodeNum<<endl;
        }
        if(blockIntensityAvearage>120)
        {
            id_threshold.insert(i,stdIntensity*3);
            cout<<"Avearage="<<blockIntensityAvearage<<";index="<<i<<endl;
        }
        else
            id_threshold.insert(i,stdIntensity);
//        id_threshold.insert(i,stdIntensity);
    }
    return id_threshold;
}

void getBoutonInImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronTree& nt,int useNeighborArea)
{
    cout<<"Welcome into bouton detection: intensity part"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash <int, int>  hashNeuron;
    hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].radius=1;
        hashNeuron.insert(listNeuron[i].n,i);
    }

    cout<<"Img size,x="<<in_sz[0]<<",y="<<in_sz[1]<<",z="<<in_sz[2]<<endl;
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];

    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and radius=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(s.type==2 && s.radius==1)
        {
            int thisx,thisy,thisz;
            thisx=s.x;
            thisy=s.y;
            thisz=s.z;
            if(useNeighborArea!=0)
            {
                listNeuron[i].radius=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                NeuronSWC out=nodeIntensity(inimg1d,listNeuron[i].x,listNeuron[i].y,listNeuron[i].z,in_sz,1);
                if(listNeuron[i].radius<out.radius)
                {
                    cout<<"update intensity:old="<<listNeuron[i].radius<<";New="<<out.radius<<endl;
                    cout<<"Distance changes:x="<<(listNeuron[i].x-out.x)<<";y="<<(listNeuron[i].y-out.y)<<";z="<<(listNeuron[i].z-out.z)<<endl;
                    listNeuron[i].radius=out.radius;
                    listNeuron[i].x=out.x;
                    listNeuron[i].y=out.y;
                    listNeuron[i].z=out.z;
                }
            }
            else
            {
                float childIntensity=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                listNeuron[i].radius=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                //get their parent node intensity

                if(s.parent>0)
                {
                    long pid=hashNeuron.value(s.parent);
                    NeuronSWC sp=listNeuron[pid];
                    //consider the distance of child-parent node
                    float distanceThre=5;
                    float child_parent_distance=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
                    if(child_parent_distance<distanceThre*distanceThre)
                    {

                        NeuronSWC sm;
                        sm.x=(sp.x-s.x)/2+s.x;
                        sm.y=(sp.y-s.y)/2+s.y;
                        sm.z=(sp.z-s.z)/2+s.z;
                        float parentIntensity=inimg1d[int(sp.z) * sz01 + int(sp.y) * sz0 + int(sp.x)];
                        float middleIntensity=inimg1d[int(sm.z) * sz01 + int(sm.y) * sz0 + int(sm.x)];
                        //this may need another threshold
                        int updateThre=10;
                        if((middleIntensity-childIntensity>updateThre)&&(middleIntensity-parentIntensity>updateThre))
                        {
                            cout<<"update child intensity:old = "<<childIntensity<<";New="<<middleIntensity<<";this node id:"<<s.n<<endl;

                            //                    update child node Intensity
                            listNeuron[i].radius=middleIntensity;
                        }
                    }
                }
            }
//            cout<<"radius:"<<listNeuron[i].radius<<endl;
        }
    }
    //release pointer
     if(inimg1d) {delete []inimg1d; inimg1d=0;}
}

QList <CellAPO> getBouton(NeuronTree nt, int threshold,int allnode)
{
    cout<<"Welcome into bouton detection: filter part"<<endl;

    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
    QHash<V3DLONG,int> id_threshold;
    id_threshold.clear();
    id_threshold=getIntensityStd(nt,128);//key is line id and value is std
    QHash <int, int>  hashNeuron,hashchild;
    hashNeuron.clear();
    hashchild.clear();
    long boutonnodelist[siz];

    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        boutonnodelist[i]=0;
        hashNeuron.insert(s.n,i);
        hashchild.insert(i,0);
    }

    float Ip1,Ip2,Ip3,Ip4;
    float intensity_change=0;
//    int thre_low=4*threshold/5;
    /*Two level threshold seperate the region into three intervals:
     *p1 maybe is a P-site or maybe not. if p1 is a P-site, it should be known by its child.
     *So consider the situation below, p1 is not a P-site, which means it has a low intensity:
     *p1,p2,p3,p4(p1->p2->p3->p4)
     *R1(~,Ip1),R2(Ip1,threshold),R3(threshold,~)
     *0. p2 in R1; PASS this point
     *1. p2 in R2: look at P3
     *1.1. p3 in R1; use thre_low,
     *     if  Ip2-Ip3 >thre_low, this is P-site-low.
     *     if  Ip2-Ip3 >threshold, this is P-site-high.
     *1.2. p3 in R2;
     *      look at P4 (maybe p2-p3 line direction have the local maximal P-site)
     *              p4 in R1; P-site should be in p2 or p3. if we store this point, maybe we can further improve performance
     *              p4 in R2; pass this point
     *              p4 in R3; p4 =P-site-high
     *1.3. p3 in R3; p3 =P-site-high
     *2. p2 in R3; p2=P-site-high
    */
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];

        if(id_threshold.contains(i))
        {
            threshold=id_threshold.value(i);
        }
        else
        {
            cout<<"can not find the key of "<<i<<endl;
            threshold=20;
        }
        if(allnode==0 && s.type!=2)
            continue;
        if(s.parent>0)
        {
            long p2_id=hashNeuron.value(s.parent);
            hashchild[p2_id]=hashchild[p2_id]+1;
            if(s.radius>1)
            {
                Ip1=s.radius;
                NeuronSWC p2=listNeuron[p2_id];
                Ip2=p2.radius;
                intensity_change=Ip2-Ip1;
                if(intensity_change>=threshold)
                {
                    cout<<"(internal p2, p2>p1)bouton: index="<<p2.n<<endl;
                    cout<<"threshold intensity is "<<threshold<<endl;
                    boutonnodelist[p2_id]=Ip2;
                }
                else if(intensity_change>0&&intensity_change<threshold)
                {
                    //1.1
                    if(p2.parent<0)
                        continue;
                    long p3_id=hashNeuron.value(p2.parent);
                    hashchild[p3_id]=hashchild[p3_id]+1;
                    NeuronSWC p3=listNeuron[p3_id];
                    Ip3=p3.radius;
                    int p3change=Ip3-Ip1;
                    if(p3change>=threshold)
                    {
                        cout<<"(internal p3, 0<p2-p1<thre && p3-p1 > thre)bouton: index="<<p3.n<<endl;
                        cout<<"threshold intensity is "<<threshold<<endl;
                        boutonnodelist[p3_id]=Ip3;
                    }
                    else if(p3change<0)
                    {
                        if(Ip2-Ip3>=threshold)
                        {
                            cout<<"(internal p3, 0<p2-p1<thre && p2-p3 > thre && p3<p1 )bouton: index="<<p2.n<<endl;
                            cout<<"threshold intensity is "<<threshold<<endl;
                            boutonnodelist[p2_id]=Ip2;
                        }
                        else
                            continue;
                    }
                    else
                    {
                        //0<p2-p1<thre && 0<p3-p1<thre
                        if(p3.parent<0)
                        {
                            continue;
                        }
                        long p4_id=hashNeuron.value(p3.parent);
                        hashchild[p4_id]=hashchild[p4_id]+1;
                        NeuronSWC p4=listNeuron[p4_id];
                        Ip4=p4.radius;
                        if(Ip4-Ip1>=threshold)
                        {
                            cout<<"(internal p4, p4-p1>thre )bouton: index="<<p4.n<<endl;
                            cout<<"threshold intensity is "<<threshold<<endl;
                            boutonnodelist[p4_id]=Ip4;
                        }
                        else
                        {
                            //0<p2-p1<thre && 0<p3-p1<thre && 0<p4-p1<thre
                            continue;
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
        }
        else
            hashchild[hashNeuron.value(s.n)]=hashchild[hashNeuron.value(s.n)]+1;
    }
    //find all the tips
    QHash<int, int>::const_iterator hcit;
    for(hcit=hashchild.begin();hcit!=hashchild.end();hcit++)
    {
        if(hcit.value()==0)
        {

            int childid=hcit.key();
            NeuronSWC s = listNeuron[childid];
            if(allnode==0)
            {
                if(s.parent>0)
                {
                    long pid=hashNeuron.value(s.parent);
                    Ip1=s.radius;
                    NeuronSWC s_pp=listNeuron[pid];
                    if(id_threshold.contains(pid))
                        threshold=id_threshold.value(pid);
                    else
                        threshold=20;
                    Ip2=s_pp.radius;
                    intensity_change=Ip1-Ip2;
                    if(intensity_change>threshold)
                    {
                        cout<<"tip bouton: index="<<s.n<<endl;
                        boutonnodelist[s.n-1]=Ip1;
                    }
                }
            }
            else
            {
                if(s.parent>0&&s.type==2)
                {
                    long pid=hashNeuron.value(s.parent);
                    Ip1=s.radius;
                    NeuronSWC s_pp=listNeuron[pid];

                    Ip2=s_pp.radius;
                    intensity_change=Ip1-Ip2;
                    if(id_threshold.contains(pid))
                        threshold=id_threshold.value(pid);
                    else
                        threshold=20;
//                    threshold=id_threshold.value(pid);
                    if(intensity_change>threshold)
                    {
                        cout<<"tip bouton: index="<<s.n<<endl;
                        boutonnodelist[s.n-1]=Ip1;
                    }
                }
            }

        }
    }
    //for all the P-sites nodes
    QList <CellAPO> apolist;apolist.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        if(boutonnodelist[i]==0)
            continue;
        NeuronSWC s = listNeuron[i];
        //parent node and grandparent node should not be bouton

        if(s.parent>0)
        {
            long pid=hashNeuron.value(s.parent);
            NeuronSWC s_p=listNeuron[pid];

            if(boutonnodelist[pid]>0)
                continue;
            if(s_p.parent>0)
            {
                long ppid=hashNeuron.value(s_p.parent);
//                NeuronSWC s_pp=listNeuron[ppid];
                if(boutonnodelist[ppid]>0)
                    continue;
            }
        }
        threshold=id_threshold.value(i);
        CellAPO apo;
        apo.x=s.x+1;
        apo.y=s.y+1;
        apo.z=s.z+1;
        apo.intensity=threshold;
        apo.volsize=s.radius;
        apo.color.r=0;
        apo.color.g=0;
        apo.color.b=255;
        apolist.push_back(apo);
    }
    return apolist;
}
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int block_size)
{
    cout<<"Welcome into bouton detection: crop part"<<endl;
    cout<<"get bouton apo file and crop img out"<<endl;
    V3DLONG siz = apolist.size();
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    for(V3DLONG i=0;i<siz;i++)
    {
        CellAPO s = apolist[i];
        int start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - block_size; if(start_x<0) start_x = 0;
        end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
        start_y =s.y - block_size;if(start_y<0) start_y = 0;
        end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
        start_z = s.z - block_size;if(start_z<0) start_z = 0;
        end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] = end_x - start_x;
        in_sz[1] = end_y - start_y;
        in_sz[2] = end_z - start_z;
        in_sz[3]=1;
        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
        QString tmpstr = "";
        tmpstr.append("_x").append(QString("%1").arg(s.x));
        tmpstr.append("_y").append(QString("%1").arg(s.y));
        tmpstr.append("_z").append(QString("%1").arg(s.z));
        QString default_name = "Bouton"+tmpstr+".tif";
        QString default_name_apo="Bouton"+tmpstr+".apo";
        QString save_path = QString::fromStdString(outpath);
        QDir path(save_path);
        if(!path.exists())
        {
            path.mkpath(save_path);
        }
        QString save_path_img =save_path+"/"+default_name;
        QString save_path_apo=save_path+"/"+default_name_apo;
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);
        //save apo file
        QList <CellAPO> apoboutonlist;apoboutonlist.clear();
        CellAPO tmp;
        tmp.x=block_size;
        tmp.y=block_size;
        tmp.z=block_size;
        tmp.volsize=100;
        tmp.color.r=0;
        tmp.color.g=0;
        tmp.color.b=255;
        apoboutonlist.push_back(tmp);
        writeAPO_file(save_path_apo,apoboutonlist);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}

void getBoutonInTerafly(V3DPluginCallback2 &callback, string imgPath, NeuronTree& nt,int allnode)
{
    cout<<"Welcome into bouton detection: intensity part"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = nt.listNeuron.size();
    QHash <int, int>  hashNeuron;
    hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].radius=1;
        hashNeuron.insert(listNeuron[i].n,i);
    }
    //only keep axon part
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and radius=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(allnode>0)
        {
            if(s.radius==1)
            {
                //get a block
                int start_x,start_y,start_z,end_x,end_y,end_z;
                int block_size=64;
                start_x = s.x - block_size; if(start_x<0) start_x = 0;
                end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
                start_y =s.y - block_size;if(start_y<0) start_y = 0;
                end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
                start_z = s.z - block_size;if(start_z<0) start_z = 0;
                end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

                V3DLONG *in_sz = new V3DLONG[4];
                in_sz[0] = end_x - start_x;
                in_sz[1] = end_y - start_y;
                in_sz[2] = end_z - start_z;
                V3DLONG *sz;
                sz=in_sz;
                long sz0 = sz[0];
                long sz01 = sz[0] * sz[1];
                unsigned char * inimg1d = 0;
                inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
    //            //version one
                int thisx,thisy,thisz;
                thisx=s.x-start_x;
                thisy=s.y-start_y;
                thisz=s.z-start_z;

                listNeuron[i].radius=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
                //refine the intensity to local maximal
                float childIntensity=listNeuron[i].radius;
                //get their parent node intensity

                if(s.parent>0)
                {
                    long pid=hashNeuron.value(s.parent);
                    NeuronSWC sp=listNeuron[pid];
                    //consider the distance of child-parent node
                    float distanceThre=5;
                    float child_parent_distance=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
                    if(child_parent_distance<distanceThre*distanceThre)
                    {

                        NeuronSWC sm;
                        sm.x=(sp.x-s.x)/2+s.x;
                        sm.y=(sp.y-s.y)/2+s.y;
                        sm.z=(sp.z-s.z)/2+s.z;
                        int thisxm,thisym,thiszm;
                        thisxm=sm.x-start_x;
                        thisym=sm.y-start_y;
                        thiszm=sm.z-start_z;
                        int thisxp,thisyp,thiszp;
                        thisxp=sp.x-start_x;
                        thisyp=sp.y-start_y;
                        thiszp=sp.z-start_z;
                        float parentIntensity=inimg1d[int(thiszp) * sz01 + int(thisyp) * sz0 + int(thisxp)];
                        float middleIntensity=inimg1d[int(thiszm) * sz01 + int(thisym) * sz0 + int(thisxm)];
                        //this may need another threshold
                        int updateThre=10;
                        if((middleIntensity-childIntensity>updateThre)&&(middleIntensity-parentIntensity>updateThre))
                        {
                            cout<<"update child intensity:old = "<<childIntensity<<";New="<<middleIntensity<<endl;
                            cout<<"this node id:"<<s.n;
                            //                    update child node Intensity
                            listNeuron[i].radius=middleIntensity;
                        }
                    }
                }

                if(inimg1d) {delete []inimg1d; inimg1d=0;}
            }
        }
        else
        {
            if(s.type==2 && s.radius==1)
            {
                //get a block
                int start_x,start_y,start_z,end_x,end_y,end_z;
                int block_size=32;
                start_x = s.x - block_size; if(start_x<0) start_x = 0;
                end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
                start_y =s.y - block_size;if(start_y<0) start_y = 0;
                end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
                start_z = s.z - block_size;if(start_z<0) start_z = 0;
                end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

                V3DLONG *in_sz = new V3DLONG[4];
                in_sz[0] = end_x - start_x;
                in_sz[1] = end_y - start_y;
                in_sz[2] = end_z - start_z;
                V3DLONG *sz;
                sz=in_sz;
                long sz0 = sz[0];
                long sz01 = sz[0] * sz[1];
                unsigned char * inimg1d = 0;
                inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
    //            //version one
                int thisx,thisy,thisz;
                thisx=s.x-start_x;
                thisy=s.y-start_y;
                thisz=s.z-start_z;

                listNeuron[i].radius=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
//                cout<<"radius:"<<listNeuron[i].radius<<endl;
                //refine the intensity to local maximal
                float childIntensity=listNeuron[i].radius;
                //get their parent node intensity

                if(s.parent>0)
                {
                    long pid=hashNeuron.value(s.parent);
                    NeuronSWC sp=listNeuron[pid];
                    //consider the distance of child-parent node
                    float distanceThre=5;
                    float child_parent_distance=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
                    if(child_parent_distance<distanceThre*distanceThre)
                    {

                        NeuronSWC sm;
                        sm.x=(sp.x-s.x)/2+s.x;
                        sm.y=(sp.y-s.y)/2+s.y;
                        sm.z=(sp.z-s.z)/2+s.z;
                        int thisxm,thisym,thiszm;
                        thisxm=sm.x-start_x;
                        thisym=sm.y-start_y;
                        thiszm=sm.z-start_z;
                        int thisxp,thisyp,thiszp;
                        thisxp=sp.x-start_x;
                        thisyp=sp.y-start_y;
                        thiszp=sp.z-start_z;
                        float parentIntensity=inimg1d[int(thiszp) * sz01 + int(thisyp) * sz0 + int(thisxp)];
                        float middleIntensity=inimg1d[int(thiszm) * sz01 + int(thisym) * sz0 + int(thisxm)];
                        //this may need another threshold
                        int updateThre=10;
                        if((middleIntensity-childIntensity>updateThre)&&(middleIntensity-parentIntensity>updateThre))
                        {
                            cout<<"update child intensity:old = "<<childIntensity<<";New="<<middleIntensity<<endl;
                            cout<<"this node id:"<<s.n;
                            //                    update child node Intensity
                            listNeuron[i].radius=middleIntensity;
                        }
                    }
                }
                if(inimg1d) {delete []inimg1d; inimg1d=0;}
            }
        }

    }
    //release pointer
    if(in_zz) {delete []in_zz; in_zz=0;}

}
NeuronSWC nodeIntensity(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size)
{
    //return the intensity of the input node
//    int outRadius=0;
    NeuronSWC out;
    out.radius=0;
    float thisx,thisy,thisz;
    out.x=thisx=nodex;
    out.y=thisy=nodey;
    out.z=thisz=nodez;

    V3DLONG sz01 = sz[0] * sz[1];
    for(float iz=((thisz-neighbor_size)>=0)?(thisz-neighbor_size):0;(iz<thisz+neighbor_size+1)&&(iz<sz[2]);iz=iz+0.2)
    {

        for( float iy=((thisy-neighbor_size)>=0)?(thisy-neighbor_size):0;(iy<thisy+neighbor_size-1)&&(iy<sz[1]);iy=iy+0.2)
        {
            for(float ix=((thisx-neighbor_size)>=0)?(thisx-neighbor_size):0;(ix<thisx+neighbor_size-1)&&(ix<sz[0]);ix=ix+0.2)
            {
                long pos = iz*sz01 + iy * sz[0] + ix;
                int thisRadius=inimg1d[pos];
                if(thisRadius>out.radius)
                {
                    out.radius=thisRadius;
                    out.x=ix;
                    out.y=iy;
                    out.z=iz;
                }
            }
        }
    }
    return out;
}
void printHelp()
{
    qDebug()<<"Function for terafly";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_terafly -i <input_image_terafly> <input_swc>";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_terafly -i <input_image_terafly> <input_swc> -p <all node is used: 0 or 1>";
    qDebug()<<"Use one: get bouton apo file and crop img out";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_filter -i <input_image_terafly> <input_swc> -o <out_path> -p <threshold> <allnode> <crop_size>";
    qDebug()<<"Use two: only get bouton apo file.";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_filter -i <input_image_terafly> <input_swc>";
    qDebug()<<"Function for Img block";
    qDebug()<<"Usage: get bouton in using inputImg and inputSWC";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_Img -i <input_image> <input_swc> -p <threshold> <useNeighborArea: 0 or 1>";
}
