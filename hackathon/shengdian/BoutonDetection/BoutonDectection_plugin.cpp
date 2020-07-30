/* BoutonDectection_plugin.cpp
 * designed by shengdian
 * 2020-7-29 : by SD-Jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "BoutonDectection_plugin.h"

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
//        if(infiles.size() != 2 && infiles.size() != 3)
//        {
//            cerr<<"Invalid input"<<endl;
//            return false;
//        }
        string inimg_file = infiles[0];
        string inswc_file = infiles[1];
//        string out_path=outfiles[0];
//        double bkg_thresh = (inparas.size() >= 1) ? atof(inparas[0]) : -1;
        cout<<"inimg_file = "<<inimg_file<<endl;
        cout<<"inswc_file = "<<inswc_file<<endl;
//        cout<<"outswc_file = "<<out_path<<endl;
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        getBoutonInTerafly(callback,inimg_file,nt);
        string outswc_file = inswc_file + ".densityResult.swc";
        writeSWC_file(QString::fromStdString(outswc_file),nt);
	}
    else if (func_name == tr("BoutonDection_crop"))
    {
        string inimg_file = infiles[0];
        string inapo_file = infiles[1];
        string out_path=outfiles[0];
        //filter and crop
        getBoutonFilter(callback,inimg_file,inapo_file,apolist,out_path);
//        string outswc_file = inswc_file + ".densityfilter.swc";
//        writeSWC_file(QString::fromStdString(outswc_file),nt);
    }
	else if (func_name == tr("help"))
    {
        printHelp();
	}
	else return false;

	return true;
}
void getBoutonFilter(V3DPluginCallback2 &callback, string imgPath, string apofilename,string outpath)
{
    cout<<"Welcome into bouton detection: filter part"<<endl;
//    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
//    V3DLONG siz = nt.listNeuron.size();
//    //find all the tips
//    QHash <int, int>  hashNeuron;
//    hashNeuron.clear();
//    for (V3DLONG i=0;i<siz;i++)
//    {
//        NeuronSWC s = nt.listNeuron[i];
//        hashNeuron.insert(s.n,0);
//    }
//    for (V3DLONG i=0;i<siz;i++)
//    {
//        NeuronSWC s = nt.listNeuron[i];
//        if(s.pn!=-1)
//        {
//            //change hash value
//            hashNeuron.value(s.pn)+=1;

//        }
//    }
//    for (V3DLONG i=0;i<siz;i++)
//    {
//        NeuronSWC s = nt.listNeuron[i];
//        if(hashNeuron.value(s.n)==0)
//        {
//            //this is a tip point
//            cout<<"tip:"<<s.n<<endl;
//        }
//    }
    QList <CellAPO> apolist = readAPO_file(QString::fromStdString(apofilename));
    QFileInfo APOinfo(QString::fromStdString(apofilename));
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
        unsigned char * im_cropped = 0;
        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
//        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,yb,ye+1,zb,ze+1);

        QString tmpstr = "";
        tmpstr.append("_x").append(QString("%1").arg(s.x);
        tmpstr.append("_y").append(QString("%1").arg(s.y);
        tmpstr.append("_z").append(QString("%1").arg(s.z);

        QString default_name = APOinfo.baseName()+"Bouton"+tmpstr+".tif";
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
