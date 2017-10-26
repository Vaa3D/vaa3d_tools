/* neuronrecon_func.cpp
 * a plugin to construct neuron tree(s) from detected signals
 * 09/11/2017 : by Yang Yu
 */

//
#include "img.h"
#include "neuronrecon_func.h"
#include "neuronrecon.h"
#include "zhidl/classification.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"

#if  defined(Q_OS_LINUX)
    #include <omp.h>
#endif

//
const QString title = QObject::tr("Neuron Tree(s) Construction");

int lineconstruct_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if (ControlPanel::m_controlpanel)
    {
        ControlPanel::m_controlpanel->show();
        return 0;
    }
    else
    {
        ControlPanel* p = new ControlPanel(callback, parent, 1);
        if (p)	p->show();
    }

    //
    return 0;
}

int localmaxima_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if (ControlPanel::m_controlpanel)
    {
        ControlPanel::m_controlpanel->show();
        return 0;
    }
    else
    {
        ControlPanel* p = new ControlPanel(callback, parent, 2);
        if (p)	p->show();
    }

    //
    return 0;
}

int bigneuron_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if (ControlPanel::m_controlpanel)
    {
        ControlPanel::m_controlpanel->show();
        return 0;
    }
    else
    {
        ControlPanel* p = new ControlPanel(callback, parent, 3);
        if (p)	p->show();
    }

    //
    return 0;
}

int deeplearning_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if (ControlPanel::m_controlpanel)
    {
        ControlPanel::m_controlpanel->show();
        return 0;
    }
    else
    {
        ControlPanel* p = new ControlPanel(callback, parent, 4);
        if (p)	p->show();
    }

    //
    return 0;
}

bool convertTrees2Pointcloud_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1) return false;

    //parsing input
    char * paras = NULL;
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //parsing output
    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    // load multiple traced neurons (trees saved as .swc)
    NCPointCloud pointcloud;

    QStringList files;
    for (int i=0;i<inlist->size();i++)
    {
        files.push_back(QString(inlist->at(i)));
    }

    pointcloud.getPointCloud(files);

    // output .apo file (point cloud)
    QString outfileName;
    outfileName = QString(outlist->at(0));

    pointcloud.savePointCloud(outfileName);

    //
    return true;
}

bool samplingtree_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1) return false;

    //parsing input
    char * paras = NULL;
    float sampleratio = 2;
    if (input.size()==2)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            sampleratio = atof(paras->at(0));
            cout<<"sampleratio: "<<sampleratio<<endl;
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //parsing output
    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    // load
    QStringList files;
    for (int i=0;i<inlist->size();i++)
    {
        files.push_back(QString(inlist->at(i)));
    }

    QString filename = files.at(0);

    if(filename.toUpper().endsWith(".SWC"))
    {
        NeuronTree nt = readSWC_file(filename);

        for(V3DLONG i=0; i<nt.listNeuron.size(); i++)
        {
            nt.listNeuron[i].x *= sampleratio;
            nt.listNeuron[i].y *= sampleratio;
            nt.listNeuron[i].z *= sampleratio;
            nt.listNeuron[i].r *= sampleratio;
        }

        // output
        QString outfileName;
        outfileName = QString(outlist->at(0));

        writeSWC_file(outfileName, nt);
    }

    //
    return true;
}

bool finetunepoints_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1) return false;

    //parsing input
    char * paras = NULL;
    float radius = 5;
    if (input.size()==2)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            radius = atof(paras->at(0));
            cout<<"radius: "<<radius<<endl;
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //parsing output
    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    // load
    QList <CellAPO> pc;
    V3DLONG *szimg = 0;
    int datatype_img = 0;
    unsigned char* p1dImg = NULL;
    V3DLONG dimx, dimy, dimz;
    for (int i=0;i<inlist->size();i++)
    {
        QString filename = QString(inlist->at(i));

        if(filename.toUpper().endsWith(".APO"))
        {
            pc = readAPO_file(filename);
        }
        else if(filename.toUpper().endsWith(".V3DRAW"))
        {
            Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(filename.toStdString().c_str()) );
            if (!p4dImage || !p4dImage->valid())
            {
                cout<<"fail to load image!\n";
                return false;
            }

            if(p4dImage->getDatatype()!=V3D_UINT8)
            {
                cout<<"Not supported!\n";
                return false;
            }

            p1dImg = p4dImage->getRawData();
            dimx = p4dImage->getXDim();
            dimy = p4dImage->getYDim();
            dimz = p4dImage->getZDim();
        }
    }

    // fine tuning with mean-shift
    float distance = 2; // converge
    NCPointCloud pointcloud;
    for(V3DLONG i=0; i<pc.size(); i++)
    {
        cout<<"before fine tuning ... "<<pc[i].x <<" "<<pc[i].y <<" "<<pc[i].z <<" "<<pc[i].volsize<<endl;

        Point p;

        p.setLocation(pc[i].x, pc[i].y, pc[i].z);
        p.setRadius(pc[i].volsize/2);

        // mean shift estimate location
        float errorDist = INFINITY;
        double weightx = 0, weighty = 0, weightz = 0;
        double sumval = 0;
        Point q;

        while(errorDist > distance)
        {
            q.x = p.x;
            q.y = p.y;
            q.z = p.z;

            for(V3DLONG z = p.z - radius; z <= p.z + radius; z++)
            {
                if(z<0 || z>=dimz)
                    continue;

                V3DLONG ofz = z*dimx*dimy;

                for(V3DLONG y = p.y - radius; y <= p.y + radius; y++)
                {
                    if(y<0 || y>=dimy)
                        continue;

                    V3DLONG ofy = ofz + y*dimx;

                    for(V3DLONG x = p.x - radius; x <= p.x + radius; x++)
                    {
                        if(x<0 || x>=dimx)
                            continue;

                        double val = p1dImg[ofy + x];

                        // center of mass
                        weightx += x*val;
                        weighty += y*val;
                        weightz += z*val;

                        sumval += val;
                    }

                }
            }

            // center of mass
            if(sumval)
            {
                p.x = weightx / sumval;
                p.y = weighty / sumval;
                p.z = weightz / sumval;
            }
            else
            {
                cout<<"No signals\n";
                break;
            }

            errorDist = pointcloud.distance(p,q);

        } // while

        pc[i].x = p.x;
        pc[i].y = p.y;    //
        return true;
        pc[i].z = p.z;

        // mean-shift estimate the radius
        float r = p.radius;
        float thresh = 15;
        float ratio = 0.01;
        float step = 1;

        V3DLONG sx = 0, sy = 0, sz = 0, count = 0;

        // init
        for(V3DLONG z = p.z - r; z <= p.z + r; z++)
        {
            if(z<0 || z>=dimz)
                continue;

            sz++;

            V3DLONG ofz = z*dimx*dimy;

            for(V3DLONG y = p.y - r; y <= p.y + r; y++)
            {
                if(y<0 || y>=dimy)
                    continue;

                sy++;

                V3DLONG ofy = ofz + y*dimx;

                for(V3DLONG x = p.x - r; x <= p.x + r; x++)
                {
                    if(x<0 || x>=dimx)
                        continue;

                    sx++;

                    if(p1dImg[ofy + x] > thresh)
                    {
                        count++;
                    }
                }
            }
        }

        if(count/sx/sy/sz < ratio)
            step = -1;


        while(count/sx/sy/sz < ratio && r>1)
        {
            count = 0;
            sx = 0;
            sy = 0;
            sz = 0;

            r += step;

            for(V3DLONG z = p.z - r; z <= p.z + r; z++)
            {
                if(z<0 || z>=dimz)
                    continue;

                sz++;

                V3DLONG ofz = z*dimx*dimy;

                for(V3DLONG y = p.y - r; y <= p.y + r; y++)
                {
                    if(y<0 || y>=dimy)
                        continue;

                    sy++;

                    V3DLONG ofy = ofz + y*dimx;

                    for(V3DLONG x = p.x - r; x <= p.x + r; x++)
                    {
                        if(x<0 || x>=dimx)
                            continue;

                        sx++;

                        if(p1dImg[ofy + x] > thresh)
                        {
                            count++;
                        }
                    }
                }
            }

            cout<<"count "<<count<<" : "<<sx*sy*sz<< endl;

        }

        pc[i].volsize = 2*r;

        cout<<"after fine tuning ... "<<pc[i].x <<" "<<pc[i].y <<" "<<pc[i].z <<" "<<pc[i].volsize<<endl;
    }

    // output
    QString outfileName;
    outfileName = QString(outlist->at(0));

    writeAPO_file(outfileName,pc);

    //
    return true;
}

bool getbranchpoints_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a swc\n";
        return false;
    }

    //parsing input
    char * paras = NULL;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    // load
    QString filename = QString(inlist->at(0));

    qDebug()<<"filename: "<<filename;

    NCPointCloud pointcloud;
    pointcloud.getBranchPoints(filename);

    // output
    if(output.size()>0)
    {
        vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
        if (outlist->size()>1)
        {
            cerr << "You cannot specify more than 1 output files"<<endl;
            return false;
        }

        //QString outfileName;
        //outfileName = QString(outlist->at(0));

        //writeSWC_file(outfileName, nt);
    }

    //
    return true;
}

//
bool processpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    // 1. read TIFF image
    // 2. itk anisotropic filtering TIFF
    // 3. app2 tracing SWC
    // 4. convert swc to a point cloud APO
    // 5. optimal construction SWC

    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        return false;
    }

    //parsing input
    char * paras = NULL;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<1)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    // processing

    // step 1.
    QString filename = QString(inlist->at(0));
    QString fnITKfiltered = filename.left(filename.lastIndexOf(".")).append("_anisotropicFiltered.tif");
    QString neuronTraced = filename.left(filename.lastIndexOf(".")).append("_traced.swc");
    QString cnvtPoints = filename.left(filename.lastIndexOf(".")).append("_pointcloud.apo");

    if(filename.toUpper().endsWith(".TIF"))
    {
        runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
    }
    else
    {
        cout<<"Current only support TIFF image as input file\n";
        return -1;
    }

    // step 2.
    if(fnITKfiltered.toUpper().endsWith(".V3DRAW") || fnITKfiltered.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(fnITKfiltered.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }

        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            cout<<"Not supported!\n";
            return false;
        }

        PARA_APP2 p2;
        QString versionStr = "v0.001";

        p2.is_gsdt = false;
        p2.is_coverage_prune = true;
        p2.is_break_accept = true;
        p2.bkg_thresh = 15;
        p2.length_thresh = 15;
        p2.cnn_type = 2;
        p2.channel = 0;
        p2.SR_ratio = 3.0/9.9;
        p2.b_256cube = false;
        p2.b_RadiusFrom2D = true;
        p2.b_resample = 1;
        p2.b_intensity = 0;
        p2.b_brightfiled = 0;
        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p2.p4dImage = p4dImage;
        p2.xc0 = p2.yc0 = p2.zc0 = 0;
        p2.xc1 = p2.p4dImage->getXDim()-1;
        p2.yc1 = p2.p4dImage->getYDim()-1;
        p2.zc1 = p2.p4dImage->getZDim()-1;

        p2.outswc_file = neuronTraced;
        proc_app2(callback, p2, versionStr);

    }
    else
    {
        cout<<"Please input an image file (.v3draw/.tif)\n";
        return -1;
    }

    // step 3. load multiple traced neurons (trees saved as .swc)
    NCPointCloud pointcloud;

    QStringList files;
    //    for (int i=0;i<inlist->size();i++)
    //    {
    //        files.push_back(QString(inlist->at(i)));
    //    }

    files.push_back(neuronTraced);

    //
    pointcloud.getPointCloud(files);
    pointcloud.savePointCloud(cnvtPoints);

    // step 4.

    //
    return true;
}

bool getStatisticsTracedNeurons_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    // load swc and save statistics to a plain text file
    //
    if(input.size()==0 || output.size() != 1) return false;

    //parsing input
    char * paras = NULL;
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //parsing output
    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    // load multiple traced neurons (trees saved as .swc)
    LineSegment line;

    QStringList files;
    for (int i=0;i<inlist->size();i++)
    {
        files.push_back(QString(inlist->at(i)));
    }

    line.getPointCloud(files);
    line.getMeanDev();

    // output .apo file (point cloud)
    QString outfileName;
    outfileName = QString(outlist->at(0));

    line.save(outfileName);

    //
    return true;
}

bool connectpointstolines_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1) return false;

    //parsing input
    char * paras = NULL;
    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    int k=6;
    float m = 3;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            maxAngle = atof(paras->at(0));
            cout<<"threshold(angle): "<<maxAngle<<endl;

            k = atoi(paras->at(1));
            cout<<"k(nn): "<<k<<endl;

            m = atof(paras->at(2));
            cout<<"dist(p2lc): "<<m<<endl;
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //parsing output
    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    // load
    NCPointCloud pointcloud;
    pointcloud.connectPoints2Lines(QString(inlist->at(0)), QString(outlist->at(0)), k, maxAngle, m);

    //
    return true;
}

bool anisotropicimagefilter_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        return false;
    }

    //parsing input
    char * paras = NULL;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<1)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //
    QString filename = QString(inlist->at(0));
    QString fnITKfiltered = filename.left(filename.lastIndexOf(".")).append("_anisotropicFiltered.tif");

    //
    if(filename.toUpper().endsWith(".TIF"))
    {
        runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
    }
    else
    {
        cout<<"Current only support TIFF image as input file\n";
        return false;
    }

    //
    return true;
}

bool sort_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1)
    {
        return false;
    }

    //parsing input
    char * paras = NULL;
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //parsing output
    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    // load multiple traced neurons (trees saved as .swc)
    NCPointCloud pointcloud;

    QStringList files;
    for (int i=0;i<inlist->size();i++)
    {
        files.push_back(QString(inlist->at(i)));
    }

    pointcloud.getPointCloud(files);

    // delete duplicated points
    pointcloud.delDuplicatedPoints();

    // sort
    NCPointCloud pcsorted;
    pcsorted.ksort(pointcloud, 10);

    // output .apo file (point cloud)
    QString outfileName;
    outfileName = QString(outlist->at(0));

    pcsorted.savePointCloud(outfileName);

    //
    return true;
}

bool test_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        return false;
    }

    //parsing input
    char * paras = NULL;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<1)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    // processing

    // step 1.
    QString filename = QString(inlist->at(0));
    QString outFileName = filename.left(filename.lastIndexOf(".")).append("_test.tif");

    if(filename.toUpper().endsWith(".TIF") || filename.toUpper().endsWith(".V3DRAW"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(filename.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }

        //
        unsigned char *p=NULL;

        int volsz = p4dImage->getTotalUnitNumberPerChannel();

        try
        {
            p = new unsigned char [volsz];
        }
        catch(...)
        {
            cout<<"fail to alloc memory for out image\n";
            return false;
        }

        // test
        //adaptiveThreshold(p, p4dImage->getRawData(), p4dImage->getXDim(), p4dImage->getYDim(), p4dImage->getZDim(), 3);
        distanceTransformL2(p, p4dImage->getRawData(), p4dImage->getXDim(), p4dImage->getYDim(), p4dImage->getZDim());

        //
        p4dImage->setData(p, p4dImage->getXDim(), p4dImage->getYDim(), p4dImage->getZDim(), 1, p4dImage->getDatatype());
        p4dImage->saveImage(const_cast<char *>(outFileName.toStdString().c_str()));
    }
    else
    {
        cout<<"Current only support TIFF/V3DRAW image as input file\n";
        return -1;
    }

    //
    return true;
}

bool lmpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        return false;
    }

    //parsing input
    char * paras = NULL;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<1)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    // processing

    // step 1.
    QString filename = QString(inlist->at(0));
    QString fnITKfiltered = filename.left(filename.lastIndexOf(".")).append("_anisotropicFiltered.tif");
    QString cnvtPoints = filename.left(filename.lastIndexOf(".")).append("_pointcloud.apo");
    QString linesTraced = filename.left(filename.lastIndexOf(".")).append("_linestraced.swc");

    if(filename.toUpper().endsWith(".TIF"))
    {
        runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
    }
    else
    {
        cout<<"Current only support TIFF image as input file\n";
        return -1;
    }

    // step 2.
    NCPointCloud pointcloud;

    //
    if(fnITKfiltered.toUpper().endsWith(".V3DRAW") || fnITKfiltered.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(fnITKfiltered.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }

        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            cout<<"Not supported!\n";
            return false;
        }

        // local maxima
        V3DLONG nstep = 16; // searching window's radius

        V3DLONG i,j,k, idx;
        V3DLONG ii, jj, kk, ofkk, ofjj;
        V3DLONG xb, xe, yb, ye, zb, ze;

        float lmax;

        unsigned char *p1dImg = p4dImage->getRawData();
        V3DLONG dimx = p4dImage->getXDim();
        V3DLONG dimy = p4dImage->getYDim();
        V3DLONG dimz = p4dImage->getZDim();
        long volsz = p4dImage->getTotalUnitNumberPerChannel();

        // estimate the radius with distance transform
        unsigned char *dt=NULL;
        try
        {
            dt = new unsigned char [volsz];
        }
        catch(...)
        {
            cout<<"fail to alloc memory for out image\n";
            return false;
        }
        distanceTransformL2(dt, p1dImg, dimx, dimy, dimz);

        // estimate threshold
        float threshold;
        estimateIntensityThreshold(p1dImg, volsz, threshold);

        //
        for(k=0; k<dimz; k+=nstep)
        {
            for(j=0; j<dimy; j+=nstep)
            {
                for(i=0; i<dimx; i+=nstep)
                {
                    //
                    xb = i - nstep;
                    xe = i + nstep;

                    if(xb<0)
                        xb = 0;
                    if(xe>dimx)
                        xe = dimx;

                    yb = j - nstep;
                    ye = j + nstep;

                    if(yb<0)
                        yb = 0;
                    if(ye>dimy)
                        ye = dimy;

                    zb = k - nstep;
                    ze = k + nstep;

                    if(zb<0)
                        zb = 0;
                    if(ze>dimz)
                        ze = dimz;

                    lmax = threshold;
                    Point p;
                    bool found = false;
                    for(kk=zb; kk<ze; kk++)
                    {
                        ofkk = kk*dimx*dimy;
                        for(jj=yb; jj<ye; jj++)
                        {
                            ofjj = ofkk + jj*dimx;
                            for(ii=xb; ii<xe; ii++)
                            {
                                idx = ofjj + ii;

                                if(p1dImg[idx]>lmax)
                                {
                                    found = true;

                                    lmax = p1dImg[idx];

                                    p.x = ii;
                                    p.y = jj;
                                    p.z = kk;
                                }
                            }
                        }
                    }

                    if(found)
                    {
                        pointcloud.points.push_back(p);
                    }
                }
            }
        }

        //
        pointcloud.delDuplicatedPoints();

        // add radius
        for(int i=0; i<pointcloud.points.size(); i++)
        {
            long idx = pointcloud.points[i].z*dimy*dimx + pointcloud.points[i].y*dimx + pointcloud.points[i].x;

            pointcloud.points[i].radius = dt[idx];
        }
    }
    else
    {
        cout<<"Please input an image file (.v3draw/.tif)\n";
        return -1;
    }

    //
    NCPointCloud pcsorted;
    pcsorted.ksort(pointcloud, 10);
    pcsorted.savePointCloud(cnvtPoints);

    // step 4. lines constructed
    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    int k=6;
    float m = 8;

    NCPointCloud lines;
    lines.connectPoints2Lines(cnvtPoints, linesTraced, k, maxAngle, m);

    // step 5. neuron tree(s) traced


    //
    return true;
}

// bigneuron-based methods to detect signals
bool bnpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        return false;
    }

    //parsing input
    char * paras = NULL;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<1)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    // processing

    // step 1.
    QString filename = QString(inlist->at(0));
    QString fnITKfiltered = filename.left(filename.lastIndexOf(".")).append("_anisotropicFiltered.tif");

    QString neuronTraced1 = filename.left(filename.lastIndexOf(".")).append("_traced1.swc");
    QString neuronTraced2 = filename.left(filename.lastIndexOf(".")).append("_traced2.swc");

    QString cnvtPoints = filename.left(filename.lastIndexOf(".")).append("_pointcloud.apo");
    QString linesTraced = filename.left(filename.lastIndexOf(".")).append("_linestraced.swc");

    if(filename.toUpper().endsWith(".TIF"))
    {
        runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
    }
    else
    {
        cout<<"Current only support TIFF image as input file\n";
        return -1;
    }

    // step 2.
    if(fnITKfiltered.toUpper().endsWith(".V3DRAW") || fnITKfiltered.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(fnITKfiltered.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }

        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            cout<<"Not supported!\n";
            return false;
        }

        // method #1
        PARA_APP2 p2;
        QString versionStr = "v0.001";

        // method #1 parameters set #1
        p2.is_gsdt = true;
        p2.is_coverage_prune = true;
        p2.is_break_accept = true;
        p2.bkg_thresh = 55;
        p2.length_thresh = 15;
        p2.cnn_type = 2;
        p2.channel = 0;
        p2.SR_ratio = 3.0/9.9;
        p2.b_256cube = false;
        p2.b_RadiusFrom2D = true;
        p2.b_resample = 1;
        p2.b_intensity = 0;
        p2.b_brightfiled = 0;
        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p2.p4dImage = p4dImage;
        p2.xc0 = p2.yc0 = p2.zc0 = 0;
        p2.xc1 = p2.p4dImage->getXDim()-1;
        p2.yc1 = p2.p4dImage->getYDim()-1;
        p2.zc1 = p2.p4dImage->getZDim()-1;

        p2.outswc_file = neuronTraced1;
        proc_app2(callback, p2, versionStr);

        // method #1 parameters set #2
        p2.is_gsdt = true;
        p2.is_coverage_prune = true;
        p2.is_break_accept = true;
        p2.bkg_thresh = 10;
        p2.length_thresh = 5;
        p2.cnn_type = 2;
        p2.channel = 0;
        p2.SR_ratio = 3.0/9.9;
        p2.b_256cube = false;
        p2.b_RadiusFrom2D = true;
        p2.b_resample = 1;
        p2.b_intensity = 0;
        p2.b_brightfiled = 0;
        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p2.p4dImage = p4dImage;
        p2.xc0 = p2.yc0 = p2.zc0 = 0;
        p2.xc1 = p2.p4dImage->getXDim()-1;
        p2.yc1 = p2.p4dImage->getYDim()-1;
        p2.zc1 = p2.p4dImage->getZDim()-1;

        p2.outswc_file = neuronTraced2;
        proc_app2(callback, p2, versionStr);bool lmpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);

        // method2 ...

    }
    else
    {
        cout<<"Please input an image file (.v3draw/.tif)\n";
        return -1;
    }

    // step 3. load multiple traced neurons (trees saved as .swc)
    NCPointCloud pointcloud;

    QStringList files;
    files.push_back(neuronTraced1);
    files.push_back(neuronTraced2);

    //
    pointcloud.getPointCloud(files);
    pointcloud.savePointCloud(cnvtPoints);

    // step 4. lines constructed
    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    int k=6;
    float m = 8;

    NCPointCloud lines;
    lines.connectPoints2Lines(cnvtPoints, linesTraced, k, maxAngle, m);

    // step 5. neuron tree(s) traced


    //
    return true;
}

// deep-learning methods to detect signals
bool dlpipeline_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    // pipeline:
    // 1. use dl trained classifier to detect signals
    // 2. connect detected points into lines
    // 3. assemble into (a) tree(s)

    cout<<"step 1. deep-learning methods to detect signals ...\n";

    // Zhi's 3D Axon Detection
    // inputs: image, model, deployed, mean
    vector<char*> infiles, paras, outfiles;

    if(input.size() >= 1)
    {
        infiles = *((vector<char*> *)input.at(0).p);
    }
    else
    {
        cout<<"please input an image\n";
        return false;
    }

    if(input.size() >= 2)
    {
        paras = *((vector<char*> *)input.at(1).p);
    }
    else
    {
        cout<<"please input classifier, mean, ...\n";
        return false;
    }

    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.empty())
    {
        cerr<<"Need input image file"<<endl;
        return false;
    }

    QString  inimg_file =  infiles[0];
    int k=0;
    QString model_file = paras.empty() ? "" : paras[k]; if(model_file == "NULL") model_file = ""; k++;
    if(model_file.isEmpty())
    {
        cerr<<"Need a model_file"<<endl;
        return false;
    }

    QString trained_file = paras.empty() ? "" : paras[k]; if(trained_file == "NULL") trained_file = ""; k++;
    if(trained_file.isEmpty())
    {
        cerr<<"Need a trained_file"<<endl;
        return false;
    }

    QString mean_file = paras.empty() ? "" : paras[k]; if(mean_file == "NULL") mean_file = ""; k++;
    if(mean_file.isEmpty())
    {
        cerr<<"Need a mean_file"<<endl;
        return false;
    }

    //
    int Sxy = (paras.size() >= k+1) ? atoi(paras[k]):10;k++;
    int Ws = (paras.size() >= k+1) ? atoi(paras[k]):512;k++;

    QString mip_file = (paras.size() >= k+1) ? paras[k]:""; if(mip_file == "NULL") mip_file = "";
    bool mip_flag = false;
    if(!mip_file.isEmpty())
    {
        mip_flag = true;
    }

    //
    cout<<"inputs ...\n";
    cout<<"inimg_file = "<<inimg_file.toStdString().c_str()<<endl;
    cout<<"model_file = "<<model_file.toStdString().c_str()<<endl;
    cout<<"trained_file = "<<trained_file.toStdString().c_str()<<endl;
    cout<<"mean_file = "<<mean_file.toStdString().c_str()<<endl;
    cout<<"sample_size = "<<Sxy<<endl;
    cout<<"image_size = "<<Ws<<endl;
    cout<<"mip_file = "<<mip_flag<<endl;
    cout<<"...\n";

    // check file exist
    if (!QFile(inimg_file).exists())
    {
        cout<<"Cannot find the input image file.\n";
        return false;
    }
    if (!QFile(model_file).exists())
    {
        cout<<"Cannot find the model image file.\n";
        return false;
    }
    if (!QFile(trained_file).exists())
    {
        cout<<"Cannot find the trained image file.\n";
        return false;
    }
    if (!QFile(mean_file).exists())
    {
        cout<<"Cannot find the mean image file.\n";
        return false;
    }

    //
    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    unsigned char *data1d_mip=0;

    //unsigned char *data1d_zmip=0, *data1d_ymip=0, *data1d_xmip=0;

    int datatype;
    V3DLONG N,M,P;
    if(mip_flag)
    {
        V3DLONG in_mip_sz[4];
        if(!simple_loadimage_wrapper(callback, mip_file.toStdString().c_str(), data1d_mip, in_mip_sz, datatype))
        {
            cerr<<"load image "<<mip_file.toStdString().c_str()<<" error!"<<endl;
            return false;
        }
        N = in_mip_sz[0];
        M = in_mip_sz[1];
    }
    else
    {
        if(!simple_loadimage_wrapper(callback, inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            cerr<<"load image "<<inimg_file.toStdString().c_str()<<" error!"<<endl;
            return false;
        }

        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];

        V3DLONG pagesz_mip = in_sz[0]*in_sz[1];
        try {data1d_mip = new unsigned char [pagesz_mip];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                int max_mip = 0;
                for(V3DLONG iz = 0; iz < P; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    if(data1d[offsetk + offsetj + ix] >= max_mip)
                    {
                        data1d_mip[iy*N + ix] = data1d[offsetk + offsetj + ix];
                        max_mip = data1d[offsetk + offsetj + ix];
                    }
                }
            }
        }
    }

    std::vector<std::vector<float> > detection_results;
    LandmarkList marklist_2D;
    Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());

    //
    unsigned int numOfThreads = 8; // default value for number of theads
    omp_set_num_threads(numOfThreads);

    V3DLONG iy, yb, ye;
    V3DLONG ix, xb, xe, blockpagesz;
    V3DLONG i, d, iiy, offsetj, iix;
    std::vector<float> det_output;
    LocationSimple LS;

//#pragma omp parallel for default(shared) private(ix, iy, xb, xe, yb, ye, blockpagesz, i, d, iiy, iix, offsetj, detection_results, det_output, LS)
    for(iy = 0; iy < M; iy = iy+Ws)
    {

        yb = iy;
        ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

        unsigned char *blockarea=0;

        for(ix = 0; ix < N; ix = ix+Ws)
        {
            xb = ix;
            xe = ix+Ws-1;
            if(xe>=N-1) xe = N-1;

            blockpagesz = (xe-xb+1)*(ye-yb+1)*1;
            blockarea = new unsigned char [blockpagesz];


            i = 0;
            for(iiy = yb; iiy < ye+1; iiy++)
            {
                offsetj = iiy*N;
                for(iix = xb; iix < xe+1; iix++)
                {
                    blockarea[i] = data1d_mip[offsetj + iix];
                    i++;
                }
            }

            detection_results = batch_detection(blockarea,classifier,xe-xb+1,ye-yb+1,1,Sxy);

            d = 0;
            for(iiy = yb+Sxy; iiy < ye+1; iiy = iiy+Sxy)
            {
                for(iix = xb+Sxy; iix < xe+1; iix = iix+Sxy)
                {
                    det_output = detection_results[d];
                    if(det_output.at(1) > det_output.at(0))
                    {
                        LS.x = iix;
                        LS.y = iiy;
                        LS.z = 1;
                        marklist_2D.push_back(LS);
                    }
                    d++;
                }
            }
            if(blockarea) {delete []blockarea; blockarea =0;}
        }
    }

    //mean shift
    mean_shift_fun fun_obj;
    LandmarkList marklist_2D_shifted;
    vector<V3DLONG> poss_landmark;
    vector<float> mass_center;
    double windowradius = Sxy+5;

    V3DLONG sz_img[4];
    sz_img[0] = N; sz_img[1] = M; sz_img[2] = 1; sz_img[3] = 1;
    fun_obj.pushNewData<unsigned char>((unsigned char*)data1d_mip, sz_img);
    poss_landmark=landMarkList2poss(marklist_2D, sz_img[0], sz_img[0]*sz_img[1]);

    for (V3DLONG j=0;j<poss_landmark.size();j++)
    {
        mass_center=fun_obj.mean_shift_center_mass(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        marklist_2D_shifted.append(tmp);
    }

    QList <ImageMarker> marklist_3D;
    ImageMarker S;
    NeuronTree nt;
    QList <NeuronSWC> & listNeuron = nt.listNeuron;

    if(!data1d)
    {
        if(!simple_loadimage_wrapper(callback, inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            cerr<<"load image "<<inimg_file.toStdString().c_str()<<" error!"<<endl;
            return false;
        }
    }

    if(mip_flag)    P = in_sz[2];

    for(V3DLONG i = 0; i < marklist_2D_shifted.size(); i++)
    {
        V3DLONG ix = marklist_2D_shifted.at(i).x;
        V3DLONG iy = marklist_2D_shifted.at(i).y;
        double I_max = 0;
        V3DLONG iz;
        for(V3DLONG j = 0; j < P; j++)
        {
            if(data1d[j*M*N + iy*N + ix] >= I_max)
            {
                I_max = data1d[j*M*N + iy*N + ix];
                iz = j;
            }
        }
        S.x = ix;
        S.y = iy;
        S.z = iz;
        S.color.r = 255;
        S.color.g = 0;
        S.color.b = 0;
        marklist_3D.append(S);
    }

    // delete false detections
    QList <ImageMarker> marklist_3D_pruned = batch_deletion(data1d,classifier,marklist_3D,N,M,P);

    // estimate radius
    for(V3DLONG i = 0; i < marklist_3D_pruned.size(); i++)
    {
        V3DLONG ix = marklist_3D_pruned.at(i).x;
        V3DLONG iy = marklist_3D_pruned.at(i).y;
        V3DLONG iz = marklist_3D_pruned.at(i).z;

        NeuronSWC n;
        n.x = ix-1;
        n.y = iy-1;
        n.z = iz-1;
        n.n = i+1;
        n.type = 2;
        n.r = estimateRadius<unsigned char>(data1d, in_sz, n.x, n.y, n.z, 10);
        n.pn = -1;
        listNeuron << n;
    }
    if(data1d) {delete []data1d; data1d = 0;}

    QString  swc_dl_detected = inimg_file.left(inimg_file.lastIndexOf(".")).append("_axon_3D.swc");
    writeSWC_file(swc_dl_detected,nt);

    // step 2. load multiple traced neurons (trees saved as .swc)
    QString cnvtPoints = swc_dl_detected.left(swc_dl_detected.lastIndexOf(".")).append("_pointcloud.apo");
    QString linesTraced = swc_dl_detected.left(swc_dl_detected.lastIndexOf(".")).append("_linestraced.swc");

    NCPointCloud pointcloud, pcsorted;

    QStringList files;
    files.push_back(swc_dl_detected);

    //
    pointcloud.getPointCloud(files);
    pointcloud.delDuplicatedPoints();
    pcsorted.ksort(pointcloud, 10);
    pcsorted.savePointCloud(cnvtPoints);

    // step 3. lines constructed
    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    int knn=6;
    float m = 8;

    NCPointCloud lines;
    lines.connectPoints2Lines(cnvtPoints, linesTraced, knn, maxAngle, m);

    // step 4.

    //
    return true;
}

//
void printHelp()
{
    cout<<"\n Optimal Constructing Neuron Trees: \n"<<endl;
}

//
ControlPanel::ControlPanel(V3DPluginCallback2 &_v3d, QWidget *parent, int func) :
    QDialog(parent), m_v3d(_v3d)
{
    if(func==1)
    {
        lineconstruct(_v3d, parent);
    }
    else if(func==2)
    {
        localmaxima(_v3d, parent);
    }
    else if(func==3)
    {
        bigneuron(_v3d, parent);
    }
    else if(func==4)
    {
        deeplearning(_v3d, parent);
    }
    else
    {
        cout<<"input is not supported.\n";
    }
}

ControlPanel::~ControlPanel()
{
    m_controlpanel = 0;
}

void ControlPanel::lineconstruct(V3DPluginCallback2 &_v3d, QWidget *parent)
{
    m_controlpanel = this;

    //
    m_le_filename = new QLineEdit(QObject::tr(" .apo"));
    QPushButton *pPushButton_start = new QPushButton(QObject::tr("construct"));
    QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
    QPushButton *pPushButton_openFileDlg = new QPushButton(QObject::tr("..."));

    pknn = new QSpinBox();
    pthresh = new QSpinBox();
    pdist = new QSpinBox();

    pknn->setMaximum(100); pknn->setMinimum(1); pknn->setValue(5);
    pthresh->setMaximum(180); pthresh->setMinimum(0); pthresh->setValue(120);
    pdist->setMaximum(100); pdist->setMinimum(1); pdist->setValue(5);

    //
    QGroupBox *input_panel = new QGroupBox("Line Construction:");
    input_panel->setStyle(new QWindowsStyle());
    QGridLayout *constructLayout = new QGridLayout();
    constructLayout->addWidget(new QLabel(QObject::tr("Input a point cloud:")),1,1);
    constructLayout->addWidget(m_le_filename,2,1,1,2);
    constructLayout->addWidget(pPushButton_openFileDlg,2,3,1,1);
    input_panel->setLayout(constructLayout);

    QGroupBox *control_panel = new QGroupBox("Parameters:");
    control_panel->setStyle(new QWindowsStyle());
    QGridLayout *controlLayout = new QGridLayout();
    controlLayout->addWidget(new QLabel(QObject::tr("k(nn):")),1,1,1,1);
    controlLayout->addWidget(pknn,1,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("nearest point(s)")),1,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("thresh(angle):")),2,1,1,1);
    controlLayout->addWidget(pthresh,2,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("degree")),2,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("dist(p2ls):")),3,1,1,1);
    controlLayout->addWidget(pdist,3,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("X point's radius")),3,3,1,1);
    control_panel->setLayout(controlLayout);

    QWidget* container = new QWidget();
    QGridLayout* bottomBar = new QGridLayout();
    bottomBar->addWidget(pPushButton_start,1,2);
    bottomBar->addWidget(pPushButton_close,1,1);
    container->setLayout(bottomBar);

    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->addWidget(input_panel);
    pGridLayout->addWidget(control_panel);
    pGridLayout->addWidget(container);

    setLayout(pGridLayout);
    setWindowTitle(QString("Construct points into line segments"));

    connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start1()));
    connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(_slot_close()));
    connect(pPushButton_openFileDlg, SIGNAL(clicked()), this, SLOT(_slots_openFile()));
}

void ControlPanel::localmaxima(V3DPluginCallback2 &_v3d, QWidget *parent)
{
    //
    m_controlpanel = this;

    //
    m_le_filename = new QLineEdit(QObject::tr(" .tif"));

    //
    v3dhandle curwin = m_v3d.currentImageWindow();
    if(curwin)
    {
        m_le_filename->setText(m_v3d.getImageName(curwin));
    }

    QPushButton *pPushButton_start = new QPushButton(QObject::tr("construct"));
    QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
    QPushButton *pPushButton_openFileDlg = new QPushButton(QObject::tr("..."));

    pWin = new QSpinBox();
    pWin->setMaximum(100); pWin->setMinimum(1); pWin->setValue(16);

    pknn = new QSpinBox();
    pthresh = new QSpinBox();
    pdist = new QSpinBox();

    pknn->setMaximum(100); pknn->setMinimum(1); pknn->setValue(5);
    pthresh->setMaximum(180); pthresh->setMinimum(0); pthresh->setValue(120);
    pdist->setMaximum(100); pdist->setMinimum(1); pdist->setValue(5);

    //
    QGroupBox *input_panel = new QGroupBox("Local Maxima Neuron Construction:");
    input_panel->setStyle(new QWindowsStyle());
    QGridLayout *constructLayout = new QGridLayout();
    constructLayout->addWidget(new QLabel(QObject::tr("Input:")),1,1);
    constructLayout->addWidget(m_le_filename,2,1,1,2);
    constructLayout->addWidget(pPushButton_openFileDlg,2,3,1,1);
    input_panel->setLayout(constructLayout);

    QGroupBox *control_panel = new QGroupBox("Parameters:");
    control_panel->setStyle(new QWindowsStyle());
    QGridLayout *controlLayout = new QGridLayout();
    controlLayout->addWidget(new QLabel(QObject::tr("searching window radius:")),1,1,1,1);
    controlLayout->addWidget(pWin,1,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("voxel(s)")),1,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("k(nn):")),2,1,1,1);
    controlLayout->addWidget(pknn,2,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("nearest point(s)")),2,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("thresh(angle):")),3,1,1,1);
    controlLayout->addWidget(pthresh,3,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("degree")),3,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("dist(p2ls):")),4,1,1,1);
    controlLayout->addWidget(pdist,4,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("X point's radius")),4,3,1,1);
    control_panel->setLayout(controlLayout);

    QWidget* container = new QWidget();
    QGridLayout* bottomBar = new QGridLayout();
    bottomBar->addWidget(pPushButton_start,1,2);
    bottomBar->addWidget(pPushButton_close,1,1);
    container->setLayout(bottomBar);

    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->addWidget(input_panel);
    pGridLayout->addWidget(control_panel);
    pGridLayout->addWidget(container);

    setLayout(pGridLayout);
    setWindowTitle(QString("Construct points into line segments"));

    connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start2()));
    connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(_slot_close()));
    connect(pPushButton_openFileDlg, SIGNAL(clicked()), this, SLOT(_slots_openFile()));
}

void ControlPanel::bigneuron(V3DPluginCallback2 &_v3d, QWidget *parent)
{
    //
    m_controlpanel = this;

    //
    m_le_filename = new QLineEdit(QObject::tr(" .tif"));

    //
    v3dhandle curwin = m_v3d.currentImageWindow();
    if(curwin)
    {
        m_le_filename->setText(m_v3d.getImageName(curwin));
    }

    QPushButton *pPushButton_start = new QPushButton(QObject::tr("construct"));
    QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
    QPushButton *pPushButton_openFileDlg = new QPushButton(QObject::tr("..."));

    pknn = new QSpinBox();
    pthresh = new QSpinBox();
    pdist = new QSpinBox();

    pknn->setMaximum(100); pknn->setMinimum(1); pknn->setValue(5);
    pthresh->setMaximum(180); pthresh->setMinimum(0); pthresh->setValue(120);
    pdist->setMaximum(100); pdist->setMinimum(1); pdist->setValue(5);

    //
    QGroupBox *input_panel = new QGroupBox("Local Maxima Neuron Construction:");
    input_panel->setStyle(new QWindowsStyle());
    QGridLayout *constructLayout = new QGridLayout();
    constructLayout->addWidget(new QLabel(QObject::tr("Input:")),1,1);
    constructLayout->addWidget(m_le_filename,2,1,1,2);
    constructLayout->addWidget(pPushButton_openFileDlg,2,3,1,1);
    input_panel->setLayout(constructLayout);

    QGroupBox *control_panel = new QGroupBox("Parameters:");
    control_panel->setStyle(new QWindowsStyle());
    QGridLayout *controlLayout = new QGridLayout();
    controlLayout->addWidget(new QLabel(QObject::tr("k(nn):")),1,1,1,1);
    controlLayout->addWidget(pknn,1,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("nearest point(s)")),1,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("thresh(angle):")),2,1,1,1);
    controlLayout->addWidget(pthresh,2,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("degree")),2,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("dist(p2ls):")),3,1,1,1);
    controlLayout->addWidget(pdist,3,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("X point's radius")),3,3,1,1);
    control_panel->setLayout(controlLayout);

    QWidget* container = new QWidget();
    QGridLayout* bottomBar = new QGridLayout();
    bottomBar->addWidget(pPushButton_start,1,2);
    bottomBar->addWidget(pPushButton_close,1,1);
    container->setLayout(bottomBar);

    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->addWidget(input_panel);
    pGridLayout->addWidget(control_panel);
    pGridLayout->addWidget(container);

    setLayout(pGridLayout);
    setWindowTitle(QString("Construct points into line segments"));

    connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start3()));
    connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(_slot_close()));
    connect(pPushButton_openFileDlg, SIGNAL(clicked()), this, SLOT(_slots_openFile()));
}

void ControlPanel::deeplearning(V3DPluginCallback2 &_v3d, QWidget *parent)
{
    //
    m_controlpanel = this;

    //
    m_le_filename = new QLineEdit(QObject::tr(" .tif"));

    //
    v3dhandle curwin = m_v3d.currentImageWindow();
    if(curwin)
    {
        m_le_filename->setText(m_v3d.getImageName(curwin));
    }

    QPushButton *pPushButton_start = new QPushButton(QObject::tr("construct"));
    QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
    QPushButton *pPushButton_openFileDlg = new QPushButton(QObject::tr("..."));

    pknn = new QSpinBox();
    pthresh = new QSpinBox();
    pdist = new QSpinBox();

    pknn->setMaximum(100); pknn->setMinimum(1); pknn->setValue(5);
    pthresh->setMaximum(180); pthresh->setMinimum(0); pthresh->setValue(120);
    pdist->setMaximum(100); pdist->setMinimum(1); pdist->setValue(5);

    //
    QGroupBox *input_panel = new QGroupBox("Local Maxima Neuron Construction:");
    input_panel->setStyle(new QWindowsStyle());
    QGridLayout *constructLayout = new QGridLayout();
    constructLayout->addWidget(new QLabel(QObject::tr("Input:")),1,1);
    constructLayout->addWidget(m_le_filename,2,1,1,2);
    constructLayout->addWidget(pPushButton_openFileDlg,2,3,1,1);
    input_panel->setLayout(constructLayout);

    QGroupBox *control_panel = new QGroupBox("Parameters:");
    control_panel->setStyle(new QWindowsStyle());
    QGridLayout *controlLayout = new QGridLayout();
    controlLayout->addWidget(new QLabel(QObject::tr("k(nn):")),1,1,1,1);
    controlLayout->addWidget(pknn,1,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("nearest point(s)")),1,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("thresh(angle):")),2,1,1,1);
    controlLayout->addWidget(pthresh,2,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("degree")),2,3,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("dist(p2ls):")),3,1,1,1);
    controlLayout->addWidget(pdist,3,2,1,1);
    controlLayout->addWidget(new QLabel(QObject::tr("X point's radius")),3,3,1,1);
    control_panel->setLayout(controlLayout);

    QWidget* container = new QWidget();
    QGridLayout* bottomBar = new QGridLayout();
    bottomBar->addWidget(pPushButton_start,1,2);
    bottomBar->addWidget(pPushButton_close,1,1);
    container->setLayout(bottomBar);

    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->addWidget(input_panel);
    pGridLayout->addWidget(control_panel);
    pGridLayout->addWidget(container);

    setLayout(pGridLayout);
    setWindowTitle(QString("Construct points into line segments"));

    connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start4()));
    connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(_slot_close()));
    connect(pPushButton_openFileDlg, SIGNAL(clicked()), this, SLOT(_slots_openFile()));
}

void ControlPanel::_slot_close()
{
    if (m_controlpanel)
    {
        delete m_controlpanel;
        m_controlpanel=0;
    }
}

void ControlPanel::_slot_start1()
{
    //
    QString pointcloud_filename = m_le_filename->text();
    if (!QFile(pointcloud_filename).exists())
    {
        v3d_msg("Cannot find your point cloud file.");
        return;
    }

    //
    int k = pknn->value() + 1;
    float maxAngle = 3.14f - (pthresh->value()/180.0f*3.14159f);
    float m = pdist->value();

    //
    NCPointCloud pointcloud;
    pointcloud.connectPoints2Lines(pointcloud_filename, NULL, k, maxAngle, m);

    //
    NeuronTree nt;

    for(V3DLONG i=0; i<pointcloud.points.size(); i++)
    {
        NeuronSWC S;
        S.n = pointcloud.points[i].n;
        S.type = 3;
        S.x = pointcloud.points[i].x;
        S.y= pointcloud.points[i].y;
        S.z = pointcloud.points[i].z;
        S.r = pointcloud.points[i].radius;
        S.pn = pointcloud.points[i].parents[0];

        nt.listNeuron.append(S);
        nt.hashNeuron.insert(S.n, nt.listNeuron.size()-1);
    }

    // need to reset the color to zero of display with color (using the types)
    nt.color.r = 0;
    nt.color.g = 0;
    nt.color.b = 0;
    nt.color.a = 0;

    //
    QList <V3dR_MainWindow *> list_3dviewer = m_v3d.getListAll3DViewers();
    V3dR_MainWindow * new3DWindow = NULL;
    bool b_found = false;
    QString title = QString("construct");
    if (list_3dviewer.size() < 1)
    {
        new3DWindow = m_v3d.createEmpty3DViewer();
        if (!new3DWindow)
        {
            v3d_msg(QString("Failed to open an empty window!"));
            return;
        }
    }
    else
    {
        for(int j= 0; j < list_3dviewer.size(); j++)
        {
            if(m_v3d.getImageName(list_3dviewer[j]).contains(title))
            {
                b_found = true;
                new3DWindow = list_3dviewer[j];
                break;
            }
        }

        if(!b_found)
        {
            new3DWindow = m_v3d.createEmpty3DViewer();
            if(!new3DWindow)
            {
                v3d_msg(QString("Failed to open an empty window!"));
                return;
            }
        }
    }

    //
    QList<NeuronTree> * new_treeList = m_v3d.getHandleNeuronTrees_Any3DViewer (new3DWindow);
    new_treeList->clear();
    new_treeList->push_back(nt);

    //
    m_v3d.setWindowDataTitle(new3DWindow, title);
    m_v3d.update_3DViewer(new3DWindow);
    m_v3d.update_NeuronBoundingBox(new3DWindow);
}

void ControlPanel::_slot_start2()
{
    //
    QString filename = m_le_filename->text();
    if (!QFile(filename).exists())
    {
        v3d_msg("Cannot find your image file.");
        return;
    }

    //
    int k = pknn->value() + 1;
    float maxAngle = 3.14f - (pthresh->value()/180.0f*3.14159f);
    float m = pdist->value();

    // temporary files
    QString fnITKfiltered = filename.left(filename.lastIndexOf(".")).append("_anisotropicFiltered.tif");
    QString cnvtPoints = filename.left(filename.lastIndexOf(".")).append("_pointcloud.apo");
    QString linesTraced = filename.left(filename.lastIndexOf(".")).append("_linestraced.swc");

    //
    if(filename.toUpper().endsWith(".TIF") || filename.toUpper().endsWith(".TIFF"))
    {
        runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
    }
    else
    {
        cout<<"Current only support TIFF image as input file\n";
        return;
    }

    //
    NCPointCloud pointcloud;

    //
    if(fnITKfiltered.toUpper().endsWith(".V3DRAW") || fnITKfiltered.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = m_v3d.loadImage( const_cast<char *>(fnITKfiltered.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return;
        }

        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            cout<<"Not supported!\n";
            return;
        }

        // local maxima
        V3DLONG nstep = pWin->value(); // searching window's radius

        V3DLONG i,j,k, idx;
        V3DLONG ii, jj, kk, ofkk, ofjj;
        V3DLONG xb, xe, yb, ye, zb, ze;

        float lmax;

        unsigned char *p1dImg = p4dImage->getRawData();
        V3DLONG dimx = p4dImage->getXDim();
        V3DLONG dimy = p4dImage->getYDim();
        V3DLONG dimz = p4dImage->getZDim();
        long volsz = p4dImage->getTotalUnitNumberPerChannel();

        // estimate the radius with distance transform
        unsigned char *dt=NULL;
        try
        {
            dt = new unsigned char [volsz];
        }
        catch(...)
        {
            cout<<"fail to alloc memory for out image\n";
            return;
        }
        distanceTransformL2(dt, p1dImg, dimx, dimy, dimz);

        // estimate threshold
        float threshold;
        estimateIntensityThreshold(p1dImg, volsz, threshold);

        //
        for(k=0; k<dimz; k+=nstep)
        {
            for(j=0; j<dimy; j+=nstep)
            {
                for(i=0; i<dimx; i+=nstep)
                {
                    //
                    xb = i - nstep;
                    xe = i + nstep;

                    if(xb<0)
                        xb = 0;
                    if(xe>dimx)
                        xe = dimx;

                    yb = j - nstep;
                    ye = j + nstep;

                    if(yb<0)
                        yb = 0;
                    if(ye>dimy)
                        ye = dimy;

                    zb = k - nstep;
                    ze = k + nstep;

                    if(zb<0)
                        zb = 0;
                    if(ze>dimz)
                        ze = dimz;

                    lmax = threshold;
                    Point p;
                    bool found = false;
                    for(kk=zb; kk<ze; kk++)
                    {
                        ofkk = kk*dimx*dimy;
                        for(jj=yb; jj<ye; jj++)
                        {
                            ofjj = ofkk + jj*dimx;
                            for(ii=xb; ii<xe; ii++)
                            {
                                idx = ofjj + ii;

                                if(p1dImg[idx]>lmax)
                                {
                                    found = true;

                                    lmax = p1dImg[idx];

                                    p.x = ii;
                                    p.y = jj;
                                    p.z = kk;
                                }
                            }
                        }
                    }

                    if(found)
                    {
                        pointcloud.points.push_back(p);
                    }
                }
            }
        }

        //
        pointcloud.delDuplicatedPoints();

        // add radius
        for(int i=0; i<pointcloud.points.size(); i++)
        {
            long idx = pointcloud.points[i].z*dimy*dimx + pointcloud.points[i].y*dimx + pointcloud.points[i].x;

            pointcloud.points[i].radius = dt[idx];
        }

        //
        y_del1dp<unsigned char>(dt);
    }
    else
    {
        cout<<"Please input an image file (.v3draw/.tif)\n";
        return;
    }

    //
    NCPointCloud pcsorted;
    pcsorted.ksort(pointcloud, 10);
    pcsorted.savePointCloud(cnvtPoints);

    // construct lines
    NCPointCloud lines;
    lines.connectPoints2Lines(cnvtPoints, linesTraced, k, maxAngle, m);

    //
    NeuronTree nt;

    for(V3DLONG i=0; i<lines.points.size(); i++)
    {
        NeuronSWC S;
        S.n = lines.points[i].n;
        S.type = 3;
        S.x = lines.points[i].x;
        S.y = lines.points[i].y;
        S.z = lines.points[i].z;
        S.r = lines.points[i].radius;
        S.pn = lines.points[i].parents[0];

        nt.listNeuron.append(S);
        nt.hashNeuron.insert(S.n, nt.listNeuron.size()-1);
    }

    // need to reset the color to zero of display with color (using the types)
    nt.color.r = 0;
    nt.color.g = 0;
    nt.color.b = 0;
    nt.color.a = 0;

    //
    QList <V3dR_MainWindow *> list_3dviewer = m_v3d.getListAll3DViewers();
    V3dR_MainWindow * new3DWindow = NULL;
    bool b_found = false;
    QString title = QString("construct");
    if (list_3dviewer.size() < 1)
    {
        new3DWindow = m_v3d.createEmpty3DViewer();
        if (!new3DWindow)
        {
            v3d_msg(QString("Failed to open an empty window!"));
            return;
        }
    }
    else
    {
        for(int j= 0; j < list_3dviewer.size(); j++)
        {
            if(m_v3d.getImageName(list_3dviewer[j]).contains(title))
            {
                b_found = true;
                new3DWindow = list_3dviewer[j];
                break;
            }
        }

        if(!b_found)
        {
            new3DWindow = m_v3d.createEmpty3DViewer();
            if(!new3DWindow)
            {
                v3d_msg(QString("Failed to open an empty window!"));
                return;
            }
        }
    }

    //
    QList<NeuronTree> * new_treeList = m_v3d.getHandleNeuronTrees_Any3DViewer (new3DWindow);
    new_treeList->clear();
    new_treeList->push_back(nt);

    //
    m_v3d.setWindowDataTitle(new3DWindow, title);
    m_v3d.update_3DViewer(new3DWindow);
    m_v3d.update_NeuronBoundingBox(new3DWindow);
}

void ControlPanel::_slot_start3()
{
    //
    QString filename = m_le_filename->text();
    if (!QFile(filename).exists())
    {
        v3d_msg("Cannot find your image file.");
        return;
    }

    //
    int k = pknn->value() + 1;
    float maxAngle = 3.14f - (pthresh->value()/180.0f*3.14159f);
    float m = pdist->value();

    // temporary files
    QString fnITKfiltered = filename.left(filename.lastIndexOf(".")).append("_anisotropicFiltered.tif");
    QString neuronTraced1 = filename.left(filename.lastIndexOf(".")).append("_traced1.swc");
    QString neuronTraced2 = filename.left(filename.lastIndexOf(".")).append("_traced2.swc");
    QString cnvtPoints = filename.left(filename.lastIndexOf(".")).append("_pointcloud.apo");
    QString linesTraced = filename.left(filename.lastIndexOf(".")).append("_linestraced.swc");

    //
    if(filename.toUpper().endsWith(".TIF") || filename.toUpper().endsWith(".TIFF"))
    {
        runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
    }
    else
    {
        cout<<"Current only support TIFF image as input file\n";
        return;
    }

    //
    if(fnITKfiltered.toUpper().endsWith(".V3DRAW") || fnITKfiltered.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = m_v3d.loadImage( const_cast<char *>(fnITKfiltered.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return;
        }

        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            cout<<"Not supported!\n";
            return;
        }

        // method #1
        PARA_APP2 p2;
        QString versionStr = "v0.001";

        // method #1 parameters set #1
        p2.is_gsdt = true;
        p2.is_coverage_prune = true;
        p2.is_break_accept = true;
        p2.bkg_thresh = 55;
        p2.length_thresh = 15;
        p2.cnn_type = 2;
        p2.channel = 0;
        p2.SR_ratio = 3.0/9.9;
        p2.b_256cube = false;
        p2.b_RadiusFrom2D = true;
        p2.b_resample = 1;
        p2.b_intensity = 0;
        p2.b_brightfiled = 0;
        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p2.p4dImage = p4dImage;
        p2.xc0 = p2.yc0 = p2.zc0 = 0;
        p2.xc1 = p2.p4dImage->getXDim()-1;
        p2.yc1 = p2.p4dImage->getYDim()-1;
        p2.zc1 = p2.p4dImage->getZDim()-1;

        p2.outswc_file = neuronTraced1;
        proc_app2(m_v3d, p2, versionStr);

        // method #1 parameters set #2
        p2.is_gsdt = true;
        p2.is_coverage_prune = true;
        p2.is_break_accept = true;
        p2.bkg_thresh = 10;
        p2.length_thresh = 5;
        p2.cnn_type = 2;
        p2.channel = 0;
        p2.SR_ratio = 3.0/9.9;
        p2.b_256cube = false;
        p2.b_RadiusFrom2D = true;
        p2.b_resample = 1;
        p2.b_intensity = 0;
        p2.b_brightfiled = 0;
        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p2.p4dImage = p4dImage;
        p2.xc0 = p2.yc0 = p2.zc0 = 0;
        p2.xc1 = p2.p4dImage->getXDim()-1;
        p2.yc1 = p2.p4dImage->getYDim()-1;
        p2.zc1 = p2.p4dImage->getZDim()-1;

        p2.outswc_file = neuronTraced2;
        proc_app2(m_v3d, p2, versionStr);

        // method2 ...
    }
    else
    {
        cout<<"Please input an image file (.v3draw/.tif)\n";
        return;
    }

    // step 3. load multiple traced neurons (trees saved as .swc)
    NCPointCloud pointcloud;

    QStringList files;
    files.push_back(neuronTraced1);
    files.push_back(neuronTraced2);

    //
    pointcloud.getPointCloud(files);
    pointcloud.savePointCloud(cnvtPoints);

    // construct lines
    NCPointCloud lines;
    lines.connectPoints2Lines(cnvtPoints, linesTraced, k, maxAngle, m);

    //
    NeuronTree nt;

    for(V3DLONG i=0; i<lines.points.size(); i++)
    {
        NeuronSWC S;
        S.n = lines.points[i].n;
        S.type = 3;
        S.x = lines.points[i].x;
        S.y = lines.points[i].y;
        S.z = lines.points[i].z;
        S.r = lines.points[i].radius;
        S.pn = lines.points[i].parents[0];

        nt.listNeuron.append(S);
        nt.hashNeuron.insert(S.n, nt.listNeuron.size()-1);
    }

    // need to reset the color to zero of display with color (using the types)
    nt.color.r = 0;
    nt.color.g = 0;
    nt.color.b = 0;
    nt.color.a = 0;

    //
    QList <V3dR_MainWindow *> list_3dviewer = m_v3d.getListAll3DViewers();
    V3dR_MainWindow * new3DWindow = NULL;
    bool b_found = false;
    QString title = QString("construct");
    if (list_3dviewer.size() < 1)
    {
        new3DWindow = m_v3d.createEmpty3DViewer();
        if (!new3DWindow)
        {
            v3d_msg(QString("Failed to open an empty window!"));
            return;
        }
    }
    else
    {
        for(int j= 0; j < list_3dviewer.size(); j++)
        {
            if(m_v3d.getImageName(list_3dviewer[j]).contains(title))
            {
                b_found = true;
                new3DWindow = list_3dviewer[j];
                break;
            }
        }

        if(!b_found)
        {
            new3DWindow = m_v3d.createEmpty3DViewer();
            if(!new3DWindow)
            {
                v3d_msg(QString("Failed to open an empty window!"));
                return;
            }
        }
    }

    //
    QList<NeuronTree> * new_treeList = m_v3d.getHandleNeuronTrees_Any3DViewer (new3DWindow);
    new_treeList->clear();
    new_treeList->push_back(nt);

    //
    m_v3d.setWindowDataTitle(new3DWindow, title);
    m_v3d.update_3DViewer(new3DWindow);
    m_v3d.update_NeuronBoundingBox(new3DWindow);
}

void ControlPanel::_slot_start4()
{
    //
    QString filename = m_le_filename->text();
    if (!QFile(filename).exists())
    {
        v3d_msg("Cannot find your image file.");
        return;
    }

    QString model_file = m_le_model->text();
    if(model_file.isEmpty() || !QFile(model_file).exists())
    {
        v3d_msg("Need a model_file");
        return;
    }

    QString trained_file = m_le_train->text();
    if(trained_file.isEmpty() || !QFile(trained_file).exists())
    {
        v3d_msg("Need a trained_file");
        return;
    }

    QString mean_file = m_le_mean->text();
    if(mean_file.isEmpty() || !QFile(mean_file).exists())
    {
        v3d_msg("Need a mean_file");
        return;
    }

    //
    int Sxy = pStep->value();
    int Ws = pWin->value();

    //
    int k = pknn->value() + 1;
    float maxAngle = 3.14f - (pthresh->value()/180.0f*3.14159f);
    float m = pdist->value();

    // temporary files
    QString fnITKfiltered = filename.left(filename.lastIndexOf(".")).append("_anisotropicFiltered.tif");
    QString cnvtPoints = filename.left(filename.lastIndexOf(".")).append("_pointcloud.apo");
    QString linesTraced = filename.left(filename.lastIndexOf(".")).append("_linestraced.swc");

    //
    if(filename.toUpper().endsWith(".TIF") || filename.toUpper().endsWith(".TIFF"))
    {
        runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
    }
    else
    {
        cout<<"Current only support TIFF image as input file\n";
        return;
    }

    //
    NCPointCloud pointcloud;

    //
    if(fnITKfiltered.toUpper().endsWith(".V3DRAW") || fnITKfiltered.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = m_v3d.loadImage( const_cast<char *>(fnITKfiltered.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return;
        }

        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            cout<<"Not supported!\n";
            return;
        }

        // local maxima
        V3DLONG nstep = 16; // searching window's radius

        V3DLONG i,j,k, idx;
        V3DLONG ii, jj, kk, ofkk, ofjj;
        V3DLONG xb, xe, yb, ye, zb, ze;

        float lmax;

        unsigned char *p1dImg = p4dImage->getRawData();
        V3DLONG dimx = p4dImage->getXDim();
        V3DLONG dimy = p4dImage->getYDim();
        V3DLONG dimz = p4dImage->getZDim();
        long volsz = p4dImage->getTotalUnitNumberPerChannel();

        // estimate the radius with distance transform
        unsigned char *dt=NULL;
        try
        {
            dt = new unsigned char [volsz];
        }
        catch(...)
        {
            cout<<"fail to alloc memory for out image\n";
            return;
        }
        distanceTransformL2(dt, p1dImg, dimx, dimy, dimz);

        // estimate threshold
        float threshold;
        estimateIntensityThreshold(p1dImg, volsz, threshold);

        //
        for(k=0; k<dimz; k+=nstep)
        {
            for(j=0; j<dimy; j+=nstep)
            {
                for(i=0; i<dimx; i+=nstep)
                {
                    //
                    xb = i - nstep;
                    xe = i + nstep;

                    if(xb<0)
                        xb = 0;
                    if(xe>dimx)
                        xe = dimx;

                    yb = j - nstep;
                    ye = j + nstep;

                    if(yb<0)
                        yb = 0;
                    if(ye>dimy)
                        ye = dimy;

                    zb = k - nstep;
                    ze = k + nstep;

                    if(zb<0)
                        zb = 0;
                    if(ze>dimz)
                        ze = dimz;

                    lmax = threshold;
                    Point p;
                    bool found = false;
                    for(kk=zb; kk<ze; kk++)
                    {
                        ofkk = kk*dimx*dimy;
                        for(jj=yb; jj<ye; jj++)
                        {
                            ofjj = ofkk + jj*dimx;
                            for(ii=xb; ii<xe; ii++)
                            {
                                idx = ofjj + ii;

                                if(p1dImg[idx]>lmax)
                                {
                                    found = true;

                                    lmax = p1dImg[idx];

                                    p.x = ii;
                                    p.y = jj;
                                    p.z = kk;
                                }
                            }
                        }
                    }

                    if(found)
                    {
                        pointcloud.points.push_back(p);
                    }
                }
            }
        }

        //
        pointcloud.delDuplicatedPoints();

        // add radius
        for(int i=0; i<pointcloud.points.size(); i++)
        {
            long idx = pointcloud.points[i].z*dimy*dimx + pointcloud.points[i].y*dimx + pointcloud.points[i].x;

            pointcloud.points[i].radius = dt[idx];
        }

        //
        y_del1dp<unsigned char>(dt);
    }
    else
    {
        cout<<"Please input an image file (.v3draw/.tif)\n";
        return;
    }

    //
    NCPointCloud pcsorted;
    pcsorted.ksort(pointcloud, 10);
    pcsorted.savePointCloud(cnvtPoints);

    // construct lines
    NCPointCloud lines;
    lines.connectPoints2Lines(cnvtPoints, linesTraced, k, maxAngle, m);

    //
    NeuronTree nt;

    for(V3DLONG i=0; i<lines.points.size(); i++)
    {
        NeuronSWC S;
        S.n = lines.points[i].n;
        S.type = 3;
        S.x = lines.points[i].x;
        S.y = lines.points[i].y;
        S.z = lines.points[i].z;
        S.r = lines.points[i].radius;
        S.pn = lines.points[i].parents[0];

        nt.listNeuron.append(S);
        nt.hashNeuron.insert(S.n, nt.listNeuron.size()-1);
    }

    // need to reset the color to zero of display with color (using the types)
    nt.color.r = 0;
    nt.color.g = 0;
    nt.color.b = 0;
    nt.color.a = 0;

    //
    QList <V3dR_MainWindow *> list_3dviewer = m_v3d.getListAll3DViewers();
    V3dR_MainWindow * new3DWindow = NULL;
    bool b_found = false;
    QString title = QString("construct");
    if (list_3dviewer.size() < 1)
    {
        new3DWindow = m_v3d.createEmpty3DViewer();
        if (!new3DWindow)
        {
            v3d_msg(QString("Failed to open an empty window!"));
            return;
        }
    }
    else
    {
        for(int j= 0; j < list_3dviewer.size(); j++)
        {
            if(m_v3d.getImageName(list_3dviewer[j]).contains(title))
            {
                b_found = true;
                new3DWindow = list_3dviewer[j];
                break;
            }
        }

        if(!b_found)
        {
            new3DWindow = m_v3d.createEmpty3DViewer();
            if(!new3DWindow)
            {
                v3d_msg(QString("Failed to open an empty window!"));
                return;
            }
        }
    }

    //
    QList<NeuronTree> * new_treeList = m_v3d.getHandleNeuronTrees_Any3DViewer (new3DWindow);
    new_treeList->clear();
    new_treeList->push_back(nt);

    //
    m_v3d.setWindowDataTitle(new3DWindow, title);
    m_v3d.update_3DViewer(new3DWindow);
    m_v3d.update_NeuronBoundingBox(new3DWindow);
}

void ControlPanel::_slots_openFile()
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose a file (.apo):"));
    if(!fileOpenName.isEmpty())
    {
        m_le_filename->setText(fileOpenName);
    }
}
