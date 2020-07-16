/* missing_branch_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-01-13 : by LMG
 */
 
#include "v3d_message.h"
#include <vector>
#include <algorithm>
#include "openSWCDialog.h"
#include "missing_branch_detection_plugin.h"
#include "../../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
using namespace std;
Q_EXPORT_PLUGIN2(missing_branch_detection, missing_branch);
 

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

QStringList missing_branch::menulist() const
{
	return QStringList() 
        <<tr("missing_branch_detection")
		<<tr("about");
}

QStringList missing_branch::funclist() const
{
	return QStringList()
        <<tr("missing_branch_detection")
        <<tr("help");
}

void missing_branch::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("missing_branch_detection"))
	{
        if(callback.currentImageWindow())
        {
            v3dhandle curwin = callback.currentImageWindow();
            Image4DSimple* p4DImage = callback.getImage(curwin);

            // Get Image Data
            int nChannel = p4DImage->getCDim();
            ImagePixelType datatype = p4DImage->getDatatype();

            V3DLONG mysz[4];
            mysz[0] = p4DImage->getXDim();
            mysz[1] = p4DImage->getYDim();
            mysz[2] = p4DImage->getZDim();
            mysz[3] = nChannel;
            cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
            unsigned char *data1d_crop=p4DImage->getRawDataAtChannel(nChannel);

            // Get Neuron
            OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
            if (!openDlg->exec())
                return;
            NeuronTree nt = openDlg->nt;
            QList<NeuronSWC> neuron = nt.listNeuron;

            // Run missing branch detection
            LandmarkList candidates;
            ImagePixelType pixtype = datatype;//V3D_UINT16;
            candidates = get_missing_branches(callback,parent,data1d_crop,mysz,datatype,neuron,QString("tree_int.eswc"));

//            QList <ImageMarker> candidates_mk;
//            for(V3DLONG i=0; i<candidates.size(); i++)
//            {
//                ImageMarker candidate;
//                candidate.x = candidates.at(i).x;
//                candidate.y = candidates.at(i).y;
//                candidate.z = candidates.at(i).z;
//                candidates_mk.push_back(candidate);
//            }
            if(callback.setLandmark(curwin,candidates));
            if(load_data(&callback,data1d_crop,candidates,pixtype,mysz,curwin))
            {
                QList <ImageMarker> final_pts,final_far_pts;
                vector <long> final_ptsx,final_ptsy,final_ptsz;
//                vector <struct XYZ> final_pts_xyz;
                mean_shift_fun ms;
                qDebug() << "mean shift fun instantiated";
                for(V3DLONG i=0; i<candidates.size(); i++)
                {
                    vector <float> final_pt;
                    ImageMarker final_pt_mk;
                    //qDebug() << "mean shift not called";

                    //Mean shift
                    ms.pushNewData(data1d_crop,mysz);
                    final_pt = ms.mean_shift_center_mass(V3DLONG(candidates.at(i).z*mysz[0]*mysz[1]+candidates.at(i).y*mysz[0]+candidates.at(i).x),4);
//                    qDebug() << "mean shift called";
                    final_pt_mk.x = final_pt[0]+1;
                    final_pt_mk.y = final_pt[1]+1;
                    final_pt_mk.z = final_pt[2]+1;
//                    qDebug() << final_pt[0] << final_pt_mk.x;
                    if(final_pts.size() == 0) //||
                    {
                        final_ptsx.push_back(long(final_pt[0]));
                        final_ptsy.push_back(long(final_pt[1]));
                        final_ptsz.push_back(long(final_pt[2]));
                        final_pts.push_back(final_pt_mk);
        //                qDebug() << final_pt_mk.z << long(final_pt_mk.z);
                        qDebug() << "Added point:" << final_pts.at(final_pts.size()-1).x << final_pts.at(final_pts.size()-1).y << final_pts.at(final_pts.size()-1).z;

                    }
        //               find(final_ptsx.begin(),final_ptsx.end(),long(final_pt_mk.x))==final_ptsx.end() ||
        //               find(final_ptsy.begin(),final_ptsy.end(),long(final_pt_mk.y))==final_ptsy.end() ||
        //               find(final_ptsz.begin(),final_ptsz.end(),long(final_pt_mk.z))==final_ptsz.end()) &&
                    else if(final_pt_mk.x != final_pts.at(final_pts.size()-1).x && final_pt_mk.y != final_pts.at(final_pts.size()-1).y && final_pt_mk.z != final_pts.at(final_pts.size()-1).z)
        //               final_pt[1] != final_pt[2])
                    {
                        final_ptsx.push_back(long(final_pt[0]));
                        final_ptsy.push_back(long(final_pt[1]));
                        final_ptsz.push_back(long(final_pt[2]));
                        final_pts.push_back(final_pt_mk);
        //                qDebug() << final_pt_mk.z << long(final_pt_mk.z);
                        qDebug() << "Added point:" << final_pts.at(final_pts.size()-1).x << final_pts.at(final_pts.size()-1).y << final_pts.at(final_pts.size()-1).z;
                    }

                    //No mean shift
//                    final_pt_mk.x = candidates.at(i).x;
//                    final_pt_mk.y = candidates.at(i).y;
//                    final_pt_mk.z = candidates.at(i).z;
//                    //qDebug() << final_pt[0] << final_pt_mk.x;
//                    if((final_pts.size() == 0 ||
//                       find(final_ptsx.begin(),final_ptsx.end(),long(final_pt_mk.x))==final_ptsx.end() ||
//                       find(final_ptsy.begin(),final_ptsy.end(),long(final_pt_mk.y))==final_ptsy.end() ||
//                       find(final_ptsz.begin(),final_ptsz.end(),long(final_pt_mk.z))==final_ptsz.end()) &&
//                       final_pt_mk.y != final_pt_mk.z)
//                    {
//                        final_ptsx.push_back(long(final_pt_mk.x));
//                        final_ptsy.push_back(long(final_pt_mk.y));
//                        final_ptsz.push_back(long(final_pt_mk.z));
//                        final_pts.push_back(final_pt_mk);
//                        qDebug() << final_pt_mk.z << long(final_pt_mk.z);
//                        qDebug() << "Added point:" << final_pts.at(final_pts.size()-1).x << final_pts.at(final_pts.size()-1).y << final_pts.at(final_pts.size()-1).z;
//                    }

                    //                    struct XYZ final_pt_xyz = XYZ(final_pt[0],final_pt[1],final_pt[2]);
//                    if(find(final_pts_xyz.begin(),final_pts_xyz.end(),final_pt_xyz) != final_pts_xyz.end()) final_pts_xyz.push_back(final_pt_xyz);
                }

                vector <bool> out;
                for (V3DLONG i=0; i<final_pts.size(); i++)
                {
                    out.push_back(0);
                }
                for (V3DLONG i=0; i<final_pts.size(); i++)
                {
//                    bool out=0;
//                    bool first=1;
                    for(V3DLONG j=0; j<neuron.size(); j++)
                    {
//                        if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(final_pts.at(i).x,final_pts.at(i).y,final_pts.at(i).z))<5) out.at(i) = 1;
//                        if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(final_pts.at(i).x,final_pts.at(i).y,final_pts.at(i).z))>90) out.at(i) = 1;
                    }
                    for (V3DLONG j=i+1; j<final_pts.size(); j++)
                    {
                        if(dist_L2(XYZ(final_pts.at(j).x,final_pts.at(j).y,final_pts.at(j).z),XYZ(final_pts.at(i).x,final_pts.at(i).y,final_pts.at(i).z))<10)
                        {
//                            out.at(j) = 1;
                        }
                    }
                    if(out.at(i)==0)final_far_pts.push_back(final_pts.at(i));
                }
                qDebug() << "Final points size:" << final_far_pts.size();
                writeMarker_file("missing_branch_pts.marker",final_far_pts);
            }
        }
        else v3d_msg(tr("An open 3D image is needed."));
	}
	else
	{
        v3d_msg(tr("This is a plugin for missing branch detection."
            "Developed by LMG, 2020-05-04"));
	}
}

bool missing_branch::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("missing_branch_detection"))
	{
        // Get Image Data
        input_PARA PARA;
        unsigned char* data1d = 0;
        V3DLONG in_sz[4];
        V3DLONG c;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        vector<char*>* outlist = NULL;
        bool hasOutput;
        if (output.size()==0){
            cout<<"No output file specified.\n";
            hasOutput = false;
        }
        else {
            hasOutput = true;
            if (output.size()>1)
            {
                cout<<"You have specified more than 1 output file.\n";
                return false;
            }
            outlist = (vector<char*>*)(output.at(0).p);
        }


        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;

        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
//            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
//            return;
        }

        V3DLONG mysz[4];
        mysz[0] = in_sz[0];
        mysz[1] = in_sz[1];
        mysz[2] = in_sz[2];
        mysz[3] = in_sz[3];
        c = PARA.channel;

        cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
//        unsigned char *data1d_crop=p4DImage->getRawDataAtChannel(nChannel);

        // Get Neuron
        NeuronTree nt;
        QList<NeuronSWC> neuron;

        QString fileOpenName = QString(infiles[1]);
        QString fileSaveName;
        QString fileIntTreeName;
        if (hasOutput)
        {
            cout<<"outfile: "<<outlist->at(0)<<endl;
            fileSaveName = QString(outlist->at(0))+QString(".marker");
            fileIntTreeName = QString(outlist->at(0))+QString("_int.eswc");

        }
        else
        {
            fileSaveName = fileOpenName.left(fileOpenName.length() - 4)+QString("_missing_bp.marker");
            fileIntTreeName = fileOpenName.left(fileOpenName.length() - 4)+QString("_int.eswc");
        }

        if (fileOpenName.endsWith(".swc") || fileOpenName.endsWith(".SWC"))
        {
            neuron = readSWC_file(fileOpenName).listNeuron;
//            markers = readAPO_file(fileOpenName.left(fileOpenName.length() - 4) + QString(".apo"));
        }
        else if (fileOpenName.endsWith(".ano") || fileOpenName.endsWith(".ANO"))
        {
            P_ObjectFileType linker_object;
            if (!loadAnoFile(fileOpenName,linker_object))
            {
                cout<<"Error in reading the linker file."<<endl;
                return false;
            }
            QStringList nameList = linker_object.swc_file_list;
//            QStringList aponameList = linker_object.pointcloud_file_list;
            V3DLONG neuronNum = nameList.size();
            vector<QList<NeuronSWC> > nt_list;
            for (V3DLONG i=0;i<neuronNum;i++)
            {
                QList<NeuronSWC> tmp = readSWC_file(nameList.at(i)).listNeuron;
                nt_list.push_back(tmp);
//                QList<CellAPO> tmp2 = readAPO_file(aponameList.at(i));
//                markers.append(tmp2); //LMG 21-6-2018 not tested for multiple files yet
            }
//            if (!combine_linker(nt_list, neuron))
//            {
//                cout<<"Error in combining neurons."<<endl;
//                return false;
//            }
        }
        else
        {
            cout<<"The reconstruction file type you specified is not supported."<<endl;
            return false;
        }


//        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
//        if (!openDlg->exec())
//            return;
//        NeuronTree nt = openDlg->nt;
//        QList<NeuronSWC> neuron = nt.listNeuron;

        // Run missing branch detection
        LandmarkList candidates;

        ImagePixelType dt = V3D_UINT8;
        if(datatype==1)dt = V3D_UINT8;
        else if(datatype==2)dt = V3D_UINT16;
        else if(datatype==4)dt = V3D_FLOAT32;


        candidates = get_missing_branches(callback,parent,data1d,mysz,dt,neuron,fileIntTreeName);


        QList <ImageMarker> final_pts,final_far_pts;
        vector <long> final_ptsx,final_ptsy,final_ptsz;
//                vector <struct XYZ> final_pts_xyz;
        mean_shift_fun ms;
        qDebug() << "mean shift fun instantiated";
        for(V3DLONG i=0; i<candidates.size(); i++)
        {
            vector <float> final_pt;
            ImageMarker final_pt_mk;
            //qDebug() << "mean shift not called";

            //Mean shift
//            ms.pushNewData(data1d,mysz);
//            final_pt = ms.mean_shift_center_mass(V3DLONG(candidates.at(i).z*mysz[0]*mysz[1]+candidates.at(i).y*mysz[0]+candidates.at(i).x),4);
////            qDebug() << "mean shift called";
//            final_pt_mk.x = final_pt[0]+1;
//            final_pt_mk.y = final_pt[1]+1;
//            final_pt_mk.z = final_pt[2]+1;
////            qDebug() << final_pt[0] << final_pt_mk.x;
//            if(final_pts.size() == 0) //||
//            {
//                final_ptsx.push_back(long(final_pt[0]));
//                final_ptsy.push_back(long(final_pt[1]));
//                final_ptsz.push_back(long(final_pt[2]));
//                final_pts.push_back(final_pt_mk);
////                qDebug() << final_pt_mk.z << long(final_pt_mk.z);
//                qDebug() << "Added point:" << final_pts.at(final_pts.size()-1).x << final_pts.at(final_pts.size()-1).y << final_pts.at(final_pts.size()-1).z;

//            }
////               find(final_ptsx.begin(),final_ptsx.end(),long(final_pt_mk.x))==final_ptsx.end() ||
////               find(final_ptsy.begin(),final_ptsy.end(),long(final_pt_mk.y))==final_ptsy.end() ||
////               find(final_ptsz.begin(),final_ptsz.end(),long(final_pt_mk.z))==final_ptsz.end()) &&
//            else if(final_pt_mk.x != final_pts.at(final_pts.size()-1).x && final_pt_mk.y != final_pts.at(final_pts.size()-1).y && final_pt_mk.z != final_pts.at(final_pts.size()-1).z)
////               final_pt[1] != final_pt[2])
//            {
//                final_ptsx.push_back(long(final_pt[0]));
//                final_ptsy.push_back(long(final_pt[1]));
//                final_ptsz.push_back(long(final_pt[2]));
//                final_pts.push_back(final_pt_mk);
////                qDebug() << final_pt_mk.z << long(final_pt_mk.z);
//                qDebug() << "Added point:" << final_pts.at(final_pts.size()-1).x << final_pts.at(final_pts.size()-1).y << final_pts.at(final_pts.size()-1).z;
//            }

            //No mean shift
            final_pt_mk.x = candidates.at(i).x;
            final_pt_mk.y = candidates.at(i).y;
            final_pt_mk.z = candidates.at(i).z;
            //qDebug() << final_pt[0] << final_pt_mk.x;
            if((final_pts.size() == 0 ||
               find(final_ptsx.begin(),final_ptsx.end(),long(final_pt_mk.x))==final_ptsx.end() ||
               find(final_ptsy.begin(),final_ptsy.end(),long(final_pt_mk.y))==final_ptsy.end() ||
               find(final_ptsz.begin(),final_ptsz.end(),long(final_pt_mk.z))==final_ptsz.end()) &&
               final_pt_mk.y != final_pt_mk.z)
            {
                final_ptsx.push_back(long(final_pt_mk.x));
                final_ptsy.push_back(long(final_pt_mk.y));
                final_ptsz.push_back(long(final_pt_mk.z));
                final_pts.push_back(final_pt_mk);
                qDebug() << final_pt_mk.z << long(final_pt_mk.z);
                qDebug() << "Added point:" << final_pts.at(final_pts.size()-1).x << final_pts.at(final_pts.size()-1).y << final_pts.at(final_pts.size()-1).z;
            }


            //                    struct XYZ final_pt_xyz = XYZ(final_pt[0],final_pt[1],final_pt[2]);
//                    if(find(final_pts_xyz.begin(),final_pts_xyz.end(),final_pt_xyz) != final_pts_xyz.end()) final_pts_xyz.push_back(final_pt_xyz);
        }

        vector <bool> out;
        for (V3DLONG i=0; i<final_pts.size(); i++)
        {
            out.push_back(0);
        }
        for (V3DLONG i=0; i<final_pts.size(); i++)
        {
//                    bool out=0;
//                    bool first=1;
            for(V3DLONG j=0; j<neuron.size(); j++)
            {
//                if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(final_pts.at(i).x,final_pts.at(i).y,final_pts.at(i).z))<8) out.at(i) = 1;
//can be useful for small blocks                if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(final_pts.at(i).x,final_pts.at(i).y,final_pts.at(i).z))>90) out.at(i) = 1;
            }
//            for (V3DLONG j=i+1; j<final_pts.size(); j++)
//            {
//                if(dist_L2(XYZ(final_pts.at(j).x,final_pts.at(j).y,final_pts.at(j).z),XYZ(final_pts.at(i).x,final_pts.at(i).y,final_pts.at(i).z))<10)
//                {
//                    out.at(j) = 1;
//                }
//            }
            if(out.at(i)==0)final_far_pts.push_back(final_pts.at(i));
        }
        qDebug() << "Final points size:" << final_far_pts.size();
        writeMarker_file(fileSaveName,final_far_pts);

	}
	else if (func_name == tr("help"))
	{
        printf("**** Usage of missing branch detection **** \n");
        printf("vaa3d -x missing_branch_detection -f missing_branch_detection -i <inimg_file> <inswc_file> -p <channel> <other parameters> -o <outmarker_file> \n");
        printf("inimg_file       The input image\n");
        printf("inswc_file       The input reconstruction\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("outmarker_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");
    }
	else return false;

	return true;
}

