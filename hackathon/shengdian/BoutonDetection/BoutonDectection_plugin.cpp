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
		<<tr("menu1")
		<<tr("about");
}

QStringList BoutonDectectionPlugin::funclist() const
{
	return QStringList()
        <<tr("BoutonDection_terafly")
          <<tr("BoutonDection_crop")
            <<tr("BoutonDection_filter")
		<<tr("help");
}

void BoutonDectectionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
        v3d_msg(tr("Only do func, no menu"
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
//        string out_path=outfiles[0];
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        getBoutonInTerafly(callback,inimg_file,nt);
        string outswc_file = inswc_file + ".densityResult.swc";
        writeSWC_file(QString::fromStdString(outswc_file),nt);
	}
    else if (func_name == tr("BoutonDection_filter"))
    {
        string inimg_file = infiles[0];
        string inswc_file = infiles[1];
        string out_path=outfiles[0];
        int threshold=(inparas.size()>=1)?atoi(inparas[0]):30;
        int crop_block_size=(inparas.size()>=2)?atoi(inparas[1]):32;
        QList <CellAPO> apolist=getBouton(inswc_file,threshold);
        string apo_file_path = inswc_file + ".bouton.apo";
        writeAPO_file(QString::fromStdString(apo_file_path),apolist);
        getBoutonBlock(callback,inimg_file,inswc_file,apolist,out_path,crop_block_size);
//        getBoutonFilter(callback,inimg_file,inapo_file,apolist,out_path);
    }
    else if (func_name == tr("BoutonDection_crop"))
    {
        printHelp();
        string inimg_file = infiles[0];
        string inapo_file = infiles[1];
        string out_path=outfiles[0];
        //filter and crop
//        getBoutonBlock(callback,inimg_file,inapo_file,apolist,out_path);
    }
	else if (func_name == tr("help"))
    {
        printHelp();
	}
	else return false;

	return true;
}
QList <CellAPO> getBouton(string swcfile,int threshold)
{
    cout<<"Welcome into bouton detection: filter part"<<endl;
    NeuronTree nt = readSWC_file(QString::fromStdString(swcfile));
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = nt.listNeuron.size();
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

    float thisnode_intensity,thisnodep_intensity;
    float intensity_change=0;
//    int threshold=40;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];


        if(s.parent>0)
        {
            long pid=hashNeuron.value(s.parent);
            hashchild[pid]=hashchild[pid]+1;
            if(s.type==2)
            {
                thisnode_intensity=s.radius;

                NeuronSWC s_pp=listNeuron[pid];
                thisnodep_intensity=s_pp.radius;
                intensity_change=thisnodep_intensity-thisnode_intensity;
                if(intensity_change>threshold)
                {
                    cout<<"bouton: index="<<s_pp.n<<endl;
                    boutonnodelist[pid]=1;
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
            cout<<"tip: index="<<s.n<<endl;
            if(s.parent>0&&s.type==2)
            {
                long pid=hashNeuron.value(s.parent);
                thisnode_intensity=s.radius;

                NeuronSWC s_pp=listNeuron[pid];
                thisnodep_intensity=s_pp.radius;
                intensity_change=thisnode_intensity-thisnodep_intensity;
                if(intensity_change>threshold)
                {
                    cout<<"tip bouton: index="<<s_pp.n<<endl;
                    boutonnodelist[pid]=1;
                }
            }
        }
    }
    //for all the bouton nodes
    QList <CellAPO> apolist;apolist.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        if(boutonnodelist[i]==1)
        {
//            cout<<"this is a bouton:"<<endl;
            NeuronSWC s = listNeuron[i];
            CellAPO apo;
            apo.x=s.x;
            apo.y=s.y;
            apo.z=s.z;
            apo.volsize=100;
            apo.color.r=255;
            apo.color.g=0;
            apo.color.b=0;
            apolist.push_back(apo);
        }
    }
    return apolist;
}

void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath, string swcfile,QList <CellAPO> apolist,string outpath,int block_size)
{
    cout<<"Welcome into bouton detection: crop part"<<endl;

//    QList <CellAPO> apolist = readAPO_file(QString::fromStdString(apofilename));
    QFileInfo APOinfo(QString::fromStdString(swcfile));
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
//        int block_size=16;
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
//        V3DLONG *sz;
//        sz=in_sz;

        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
//        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,yb,ye+1,zb,ze+1);

        QString tmpstr = "";
        tmpstr.append("_x").append(QString("%1").arg(s.x));
        tmpstr.append("_y").append(QString("%1").arg(s.y));
        tmpstr.append("_z").append(QString("%1").arg(s.z));
        QString default_name = APOinfo.baseName()+"_Bouton"+tmpstr+".tif";
        QStringList namelist=APOinfo.baseName().split("_");
        QString save_path = QString::fromStdString(outpath);
        if(namelist.size()>1)
        {
            QString brainid=namelist[0];
            save_path = QString::fromStdString(outpath)+"/"+brainid;
        }
        QDir path(save_path);
        if(!path.exists())
        {
            path.mkpath(save_path);
        }
        QString save_path_img =save_path+"/"+default_name;
        cout<<"save path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}

void getBoutonInTerafly(V3DPluginCallback2 &callback, string imgPath, NeuronTree& nt)
{
    cout<<"Welcome into bouton detection: intensity part"<<endl;
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
//    QHash<int,int>& hN = nt.hashNeuron;
//    SwcTree t;
//    t.initialize(nt);
    //load terafly img
    V3DLONG siz = nt.listNeuron.size();
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].radius=1;
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
        if(s.type==2 && s.radius==1)
        {
            //get a block
            int start_x,start_y,start_z,end_x,end_y,end_z;
            int block_size=512;
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
//            NeuronSWC tmp;
//            tmp.x = listNeuron.at(i).x;
//            tmp.y = listNeuron.at(i).y;
//            tmp.z = listNeuron.at(i).z;
//            tmp.n = listNeuron.at(i).n;
//            tmp.parent = listNeuron.at(i).parent;
//            tmp.type = listNeuron.at(i).type;
//            tmp.radius =inimg1d[tmp.z * sz01 + tmp.y * sz0 + tmp.x];
//            cout<<"radius:"<<tmp.radius<<endl;
//            listNeuron.replace(i,tmp);
            //version two
            //for all the node in this block, update its radius with its img intensity
            for(V3DLONG i2=0;i2<siz;i2++)
            {
                NeuronSWC s_block = listNeuron[i2];
                //skip the updated nodes
                if(s_block.type != 2 || s_block.radius !=1)
                    continue;
                //start from here
                int thisx,thisy,thisz;
                thisx=s_block.x;
                thisy=s_block.y;
                thisz=s_block.z;
                if(thisx>start_x && thisx<end_x
                        && thisy>start_y && thisy<end_y
                        && thisz>start_z && thisz<end_z)
                  {
                    thisx-=start_x;
                    thisy-=start_y;
                    thisz-=start_z;
                    listNeuron[i2].radius=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
//                    cout<<"radius:"<<listNeuron[i2].radius<<endl;
                  }
            }
            if(inimg1d) {delete []inimg1d; inimg1d=0;}
        }
    }

    //next step: save to swc file
//    for (V3DLONG i=0;i<siz;i++)
//    {
//        NeuronSWC tmp;
//        tmp.x = listNeuron.at(i).x;
//        tmp.y = listNeuron.at(i).y;
//        tmp.z = listNeuron.at(i).z;
//        tmp.n = listNeuron.at(i).n;
//        tmp.parent = listNeuron.at(i).parent;
//        tmp.type = listNeuron.at(i).type;
//        tmp.radius = listNeuron.at(i).radius;

//        listNeuron.replace(i,tmp);
//    }
    //release pointer
    if(in_zz) {delete []in_zz; in_zz=0;}

}

void printHelp()
{
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_terafly -i <input_image_terafly> <input_swc> -o <output_folder>";
}
