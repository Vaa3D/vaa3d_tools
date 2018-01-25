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

bool trace_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1)
    {
        cout<<"Need at least one input and one output filename specified\n";
        return false;
    }

    //parsing input
    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    int k=6;
    float m = 3;
    double distthresh = 15;
    bool removeNoise=true;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            maxAngle = atof(paras->at(0));
            cout<<"threshold(angle): "<<maxAngle<<endl;

            if (paras->size() >= 2)
            {
                k = atoi(paras->at(1));
                cout<<"k(nn): "<<k<<endl;

                if (paras->size() >= 3)
                {
                    m = atof(paras->at(2));
                    cout<<"dist(p2lc): "<<m<<endl;

                    if (paras->size() >= 4)
                    {
                        distthresh = double(atof(paras->at(3)));
                        cout<<"dist: "<<distthresh<<endl;

                        if (paras->size() >= 5)
                        {
                            removeNoise = atoi(paras->at(4))==0?false:true;
                            cout<<"remove noise: "<<removeNoise<<endl;
                        }
                    }
                }
            }
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
    pointcloud.tracing(QString(inlist->at(0)), QString(outlist->at(0)), k, maxAngle, m, distthresh, removeNoise);

    //
    return true;
}

bool neutrace_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1)
    {
        cout<<"Need at least one input and one output filename specified\n";
        return false;
    }

    //parsing input
    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    int k=6;
    float m = 3;
    double distthresh = 15;
    bool removeNoise = true;
    float adjintthresh = 0.5;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            maxAngle = atof(paras->at(0));
            cout<<"threshold(angle): "<<maxAngle<<endl;

            if (paras->size() >= 2)
            {
                k = atoi(paras->at(1));
                cout<<"k(nn): "<<k<<endl;

                if (paras->size() >= 3)
                {
                    m = atof(paras->at(2));
                    cout<<"dist(p2ls): "<<m<<endl;

                    if (paras->size() >= 4)
                    {
                        distthresh = double(atof(paras->at(3)));
                        cout<<"dist: "<<distthresh<<endl;

                        if (paras->size() >= 5)
                        {
                            removeNoise = atoi(paras->at(4))==0?false:true;
                            cout<<"remove noise: "<<removeNoise<<endl;

                            if (paras->size() >= 6)
                            {
                                adjintthresh = atof(paras->at(5));
                                cout<<"adjintthresh: "<<adjintthresh<<endl;
                            }
                        }
                    }
                }
            }
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

    //
    QString fnimage = QString(inlist->at(0));
    QString fnpointcloud = fnimage.left(fnimage.lastIndexOf(".")).append(".apo");

    //
    if(fnimage.toUpper().endsWith(".V3DRAW") || fnimage.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(fnimage.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }

        long sx = p4dImage->getXDim();
        long sy = p4dImage->getYDim();
        long sz = p4dImage->getZDim();
        long size = sx*sy*sz;
        unsigned char *p = p4dImage->getRawData();
        unsigned char *puint8 = NULL;

        //
        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            // convert 16-bit to 8-bit
            if(p4dImage->getDatatype()==V3D_UINT16)
            {
                unsigned short *p1d = (unsigned short *)(p4dImage->getRawData());

                float maxval = 0;
                for(long i=0; i<size; i++)
                {
                    if(maxval<p1d[i])
                    {
                        maxval = p1d[i];
                    }
                }

                maxval = 255 / maxval;

                y_new1dp<unsigned char, long>(puint8, size);
                for(long i=0; i<size; i++)
                {
                    puint8[i] = maxval * p1d[i];
                }

                p = puint8;
            }
            else
            {
                cout<<"datatype is not supported\n";
                return false;
            }
        }

        //
        NCPointCloud pc;

        QStringList files;
        for (int i=1;i<inlist->size();i++)
        {
            files.push_back(QString(inlist->at(i)));
        }

        //
        pc.getPointCloud(files);
        pc.delDuplicatedPoints();
        sortByRadiusIntensity<unsigned char>(p, sx, sy, sz, pc, adjintthresh);
        pc.savePointCloud(fnpointcloud);

        // load
        NCPointCloud pointcloud;
        pointcloud.tracing(fnpointcloud, QString(outlist->at(0)), k, maxAngle, m, distthresh, removeNoise, p, sx, sy, sz);
    }

    //
    return true;
}

bool translate_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1)
    {
        cout<<"Need at least one input and one output filename specified\n";
        return false;
    }

    //parsing input
    long z = 0;
    long y = 0;
    long x = 0;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            z = atoi(paras->at(0));
            cout<<"translate (z): "<<z<<endl;
            if (paras->size() >= 2)
            {
                y = atoi(paras->at(1));
                cout<<"translate (y): "<<z<<endl;
                if (paras->size() >= 3)
                {
                    x = atoi(paras->at(2));
                    cout<<"translate (x): "<<z<<endl;
                }
            }
        }
        else
        {
            cerr<<"Invalid parameters"<<endl;
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

    //
    QString fninput = QString(inlist->at(0));
    QString fnoutput = QString(outlist->at(0));

    //
    NCPointCloud pc;
    if(fninput.toUpper().endsWith(".APO"))
    {
        QList <CellAPO> inputPoints = readAPO_file(fninput);

        long n = inputPoints.size();

        //
        for(long i=0; i<n; i++)
        {
            CellAPO cell = inputPoints[i];

            //
            Point p;

            p.n = i+1; // # assigned
            p.x = cell.x + x;
            p.y = cell.y + y;
            p.z = cell.z + z;
            p.radius = 0.5*cell.volsize;
            p.parents.push_back(-1);

            pc.points.push_back(p);
        }
    }

    if(fninput.toUpper().endsWith(".SWC"))
    {
        QStringList files;
        files.push_back(fninput);

        //
        pc.getPointCloud(files);

        for(long i=0; i<pc.points.size(); i++)
        {
            pc.points[i].x += x;
            pc.points[i].y += y;
            pc.points[i].z += z;
        }
    }

    //
    if(fnoutput.toUpper().endsWith(".APO"))
    {
        pc.savePointCloud(fnoutput);
    }

    if(fnoutput.toUpper().endsWith(".SWC"))
    {
        pc.saveNeuronTree(pc,fnoutput);
    }

    //
    return true;
}

bool dfsconnect_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0)
    {
        cout<<"Need at least one input filename specified\n";
        return false;
    }

    //parsing input
    long distthresh = 5;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            distthresh = atoi(paras->at(0));
            cout<<"distthresh: "<<distthresh<<endl;
        }
        else
        {
            cerr<<"Invalid parameters"<<endl;
            return false;
        }
    }

    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //
    QString fninput = QString(inlist->at(0));

    // assemble fragments into trees
    QList<NeuronSWC> neuron, result;
    NeuronTree nt = readSWC_file(fninput);
    neuron = nt.listNeuron;
    QString fileDefaultName;
    if (sortswc::SortSWC<long>(neuron, result, VOID, distthresh))
    {
        fileDefaultName = fninput+QString("_sorted.swc");
        //write new SWC to file
        if (!sortswc::export_list2file<long>(result,fileDefaultName,fninput))
        {
            cout<<"fail to write the output result"<<endl;
            return -1;
        }
    }

    //
    QStringList files;
    files << fileDefaultName;

    NCPointCloud pc;
    pc.getPointCloud(files);

    vector<LineSegment> lines = separate(pc);
    pc = combinelines(lines);

    pc.saveNeuronTree(pc, fninput + QString("_cleaned.swc"));

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
    

    // output .apo file (point cloud)
    QString outfileName;
    outfileName = QString(outlist->at(0));

    //
    pointcloud.savePointCloud(outfileName.left(outfileName.lastIndexOf(".")).append("_wodup.apo"));

    // sort
    NCPointCloud pcsorted;
    pcsorted.ksort(pointcloud, 10);
    
    pcsorted.savePointCloud(outfileName);
    
    //
    return true;
}

bool mergelines_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    // load linetraced.swc output a mergedlines.swc
    if(input.size()==0 || output.size() != 1)
    {
        cout<<"Please specify one input and one output\n";
        return false;
    }

    //parsing input
    float maxAngle = 0.942;
    if (input.size()==2)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            maxAngle = atof(paras->at(0));
            cout<<"max angle: "<<maxAngle<<endl;
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

    NCPointCloud pc;
    pc.getPointCloud(files);

    // merge
    pc.mergeLines(maxAngle);

    // save
    pc.saveNeuronTree(pc, QString(outlist->at(0)));

    //
    return true;
}

bool samplepc_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        return false;
    }

    //parsing input
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
    vector<char*> *infiles, *outfiles;
    infiles =  (vector<char*> *)(input.at(0).p);
    if (infiles->size()<1)
    {
        cerr<<"Invalid input"<<endl;
        return false;
    }

    //
    QString inputfile = QString(infiles->at(0));
    if(output.size() >= 1)
        outfiles = (vector<char*> *)output.at(0).p;
    QString outputfile = QString(outfiles->at(0));

    //
    NCPointCloud pointcloud;
    pointcloud.sample(inputfile, outputfile);

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
    int searchingradius = 64;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
            searchingradius = atoi(paras->at(0));
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
        //distanceTransformL2(p, p4dImage->getRawData(), p4dImage->getXDim(), p4dImage->getYDim(), p4dImage->getZDim());

        //        float threshold;
        //        estimateIntensityThreshold(p4dImage->getRawData(), volsz, threshold);

        //        unsigned char *p1dImg = p4dImage->getRawData();

        //        for(long i=0; i<volsz; i++)
        //        {
        //            if(p1dImg[i]>threshold)
        //                p[i] = p1dImg[i];
        //        }

        adaptiveThreshold(p, p4dImage->getRawData(), p4dImage->getXDim(), p4dImage->getYDim(), p4dImage->getZDim(), searchingradius);

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

bool crop_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        return false;
    }

    //parsing input
    long xstep = 512;
    long ystep = 512;
    long zstep = 512;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
            xstep = atoi(paras->at(0));

            if (paras->size() >= 2)
            {
                ystep = atoi(paras->at(1));

                if (paras->size() >= 3)
                {
                    zstep = atoi(paras->at(2));
                }
            }
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

    long n = 0;

    //
    QString filename = QString(inlist->at(0));


    if(filename.toUpper().endsWith(".TIF") || filename.toUpper().endsWith(".V3DRAW"))
    {
        cout<<"loading image ...\n";
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(filename.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }
        cout<<"... image is loaded\n";

        //
        unsigned char *pImg = p4dImage->getRawData();
        long sx = p4dImage->getXDim(), sy = p4dImage->getYDim(), sz = p4dImage->getZDim();
        long szblock = xstep*ystep*zstep;

        for(long z=0; z<sz; z+=zstep)
        {
            long zs = z;
            long ze = z+zstep-1;
            if(ze>sz-1) ze=sz-1;

            cout<<"z ... "<<zs<<" "<<ze<<endl;

            for(long y=0; y<sy; y+=ystep)
            {
                long ys = y;
                long ye = y+ystep-1;
                if(ye>sy-1) ye=sy-1;

                cout<<"y ... "<<ys<<" "<<ye<<endl;

                for(long x=0; x<sx; x+=xstep)
                {
                    long xs = x;
                    long xe = x+xstep-1;
                    if(xe>sx-1) xe=sx-1;

                    cout<<"x ... "<<xs<<" "<<xe<<endl;

                    //
                    unsigned char *p=NULL;
                    long bsx = xe-xs+1;
                    long bsy = ye-ys+1;
                    long bsz = ze-zs+1;
                    szblock=bsx*bsy*bsz;
                    y_new1dp<unsigned char, long>(p, szblock);

                    cout<<"new a pointer for a block \n";

                    //
                    n++;
                    QString outFileName = filename.left(filename.lastIndexOf(".")).append(QString("_block%1.tif").arg(n));

                    cout<<"output block image name "<<outFileName.toStdString()<<endl;

                    //
                    for(long k=zs; k<ze; k++)
                    {
                        //cout<<"k "<<k<<endl;
                        long ofk = (k-zs)*bsx*bsy;
                        long ofz = k*sx*sy;
                        for(long j=ys; j<ye; j++)
                        {
                            //cout<<"j "<<j<<endl;
                            long ofj = ofk + (j-ys)*bsx;
                            long ofy = ofz + j*sx;
                            for(long i=xs; i<xe; i++)
                            {
                                p[ofj + i-xs] = pImg[ofy + i];
                            }
                        }
                    }

                    cout<<"done with block image copy\n";

                    //
                    Image4DSimple p4dimg;

                    p4dimg.setDatatype(V3D_UINT8);
                    p4dimg.setXDim(bsx);
                    p4dimg.setYDim(bsy);
                    p4dimg.setZDim(bsz);
                    p4dimg.setCDim(1);
                    p4dimg.setTDim(1);

                    cout<<"set pointer to image4dsimple class"<<endl;

                    p4dimg.setNewRawDataPointer(p);

                    cout<<"save block image to "<<outFileName.toStdString()<<endl;

                    p4dimg.saveImage(const_cast<char *>(outFileName.toStdString().c_str()));

                    cout<<"block image is saved\n";

                    //
                    //y_del1dp<unsigned char>(p);

                }
            }
        }
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

bool convertUShort2UByte_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        cout<<"usage: -i input -o output\n";
        return false;
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<1)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //
    QString fn_in = QString(inlist->at(0));
    QString fn_out = fn_in.left(fn_in.lastIndexOf(".")).append("_ubyte.tif");

    //
    if(fn_in.toUpper().endsWith(".V3DRAW") || fn_in.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(fn_in.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }

        V3DLONG dimx = p4dImage->getXDim();
        V3DLONG dimy = p4dImage->getYDim();
        V3DLONG dimz = p4dImage->getZDim();
        long size = dimx*dimy*dimz;
        unsigned char *p1dImg = p4dImage->getRawData();
        unsigned char *puint8 = NULL;

        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            // convert 16-bit to 8-bit
            if(p4dImage->getDatatype()==V3D_UINT16)
            {
                unsigned short *p1d = (unsigned short *)(p4dImage->getRawData());

                float maxval = 0;
                for(long i=0; i<size; i++)
                {
                    if(maxval<p1d[i])
                    {
                        maxval = p1d[i];
                    }
                }

                maxval = 255 / maxval;

                y_new1dp<unsigned char, long>(puint8, size);
                for(long i=0; i<size; i++)
                {
                    puint8[i] = maxval * p1d[i];
                }

                //
                p4dImage->setDatatype(V3D_UINT8);
                p4dImage->setNewRawDataPointer(puint8);
                p4dImage->saveImage(const_cast<char *>(fn_out.toStdString().c_str()));

            }
            else
            {
                cout<<"datatype is not supported\n";
                return false;
            }
        }
    }
    else
    {
        cout<<"Image format is not supported\n";
        return false;
    }

    //
    return true;
}

bool findpeaks_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    // anisotropic filter
    // distance transform
    // find the 26-neighborhood peaks as the points for neuron reconstruction

    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        cout<<"usage: -i input -o output -p intensity_thresh_percent radius_thresh_percent nn_sort nn_lm outformat\n";
        return false;
    }

    //parsing input
    float athresh = 0.75;
    float aradius = 0.75;
    int nn = 10;
    int plateau = 1;
    int format = 0;
    int skipfiltering = 0;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
            athresh = atof(paras->at(0));
            cout<<"threshold relax parameter (0.75 by default): "<<athresh<<endl;

            if (paras->size() >= 2)
            {
                aradius = atof(paras->at(1));
                cout<<"radius relax parameter (0.75 by default): "<<aradius<<endl;

                if (paras->size() >= 3)
                {
                    nn = atoi(paras->at(2));
                    cout<<"nn (neareast neighbor for sort): "<<nn<<endl;

                    if (paras->size() >= 4)
                    {
                        plateau = atoi(paras->at(3));
                        cout<<"plateau (neighbors for local maxima): "<<plateau<<endl;

                        if (paras->size() >= 5)
                        {
                            format = atoi(paras->at(4));
                            cout<<"point cloud format (0:apo, 1:marker): "<<format<<endl;

                            if (paras->size() >= 6)
                            {
                                skipfiltering = atoi(paras->at(5));
                                cout<<"skip aniostropic filtering (0:skip, 1:no): "<<skipfiltering<<endl;
                            }
                        }
                    }
                }
            }
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

    if(!skipfiltering)
    {
        if(filename.toUpper().endsWith(".TIF"))
        {
            runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
        }
        else
        {
            cout<<"Current only support TIFF image as input file\n";
            return -1;
        }
    }
    else
    {
        fnITKfiltered = filename;
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

        V3DLONG dimx = p4dImage->getXDim();
        V3DLONG dimy = p4dImage->getYDim();
        V3DLONG dimz = p4dImage->getZDim();
        long volsz = p4dImage->getTotalUnitNumberPerChannel();
        long size = dimx*dimy*dimz;
        unsigned char *p1dImg = p4dImage->getRawData();
        unsigned char *puint8 = NULL;

        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            // convert 16-bit to 8-bit
            if(p4dImage->getDatatype()==V3D_UINT16)
            {
                unsigned short *p1d = (unsigned short *)(p4dImage->getRawData());

                float maxval = 0;
                for(long i=0; i<size; i++)
                {
                    if(maxval<p1d[i])
                    {
                        maxval = p1d[i];
                    }
                }

                maxval = 255 / maxval;

                y_new1dp<unsigned char, long>(puint8, size);
                for(long i=0; i<size; i++)
                {
                    puint8[i] = maxval * p1d[i];
                }

                p1dImg = puint8;
            }
            else
            {
                cout<<"datatype is not supported\n";
                return false;
            }
        }

        // local maxima (peaks)
        V3DLONG i,j,k, idx, nn;

        // estimate the radius with distance transform
        float *dt=NULL;
        y_new1dp<float, long>(dt, volsz);

        //
        distanceTransformL2<float>(dt, p1dImg, dimx, dimy, dimz);

        // estimate threshold
        float threshold;
        //estimateIntensityThreshold(p1dImg, volsz, threshold, 1);
        estimateIntensityThreshold(p1dImg, volsz, threshold);
        threshold *= athresh;
        cout<<"threshold ... "<<threshold<<endl;

        //
        long ofz = dimx*dimy;
        long ofy = dimx;
        long ofx = 1;

        long offset[26] = {ofx, -ofx, ofy, -ofy, ofz, -ofz,
                           ofx+ofy, ofx-ofy, ofx+ofz, ofx-ofz, ofy+ofz, ofy-ofz, -ofx+ofy, -ofx-ofy, -ofx+ofz, -ofx-ofz, -ofy+ofz, -ofy-ofz,
                           ofx+ofy+ofz, -ofx+ofy+ofz, ofx-ofy+ofz, -ofx-ofy+ofz, ofx+ofy-ofz, -ofx+ofy-ofz, ofx-ofy-ofz, -ofx-ofy-ofz};


        //
        cout<<"find peaks in a image "<<dimx<<" x "<<dimy<<" x "<<dimz<<endl;

        //
        for(k=1; k<dimz-1; k++)
        {
            for(j=1; j<dimy-1; j++)
            {
                for(i=1; i<dimx-1; i++)
                {
                    //
                    idx = k*ofz + j*ofy + i;

                    float val = p1dImg[idx];
                    if(val<threshold)
                        continue;

                    Point p;
                    p.radius = aradius*dt[idx];
                    bool skip = false;
                    int n = 0;
                    for(nn=0; nn<26; nn++)
                    {
                        if(p1dImg[idx + offset[nn] ]>val)
                        {
                            if(++n > plateau)
                            {
                                skip = true;
                                break;
                            }
                        }
                    }

                    if(skip)
                    {
                        continue;
                    }

                    //
                    p.x = i;
                    p.y = j;
                    p.z = k;

                    //
                    pointcloud.shift(dt, dimx, dimy, dimz, max(3.0f, p.radius), p);

                    if(p.radius<1.5)
                    {
                        continue;
                    }

                    //
                    pointcloud.append(p);
                }
            }
        }
        //
        y_del1dp<float>(dt);

        //
        pointcloud.delDuplicatedPoints();
    }
    else
    {
        cout<<"Please input an image file (.v3draw/.tif)\n";
        return -1;
    }

    //
    NCPointCloud pcsorted;

    if(format==0)
    {
        pcsorted.ksort(pointcloud, nn);
        pcsorted.savePointCloud(cnvtPoints, format);
    }
    else if(format==1)
    {
        pointcloud.sortbyradius();
        pointcloud.savePointCloud(cnvtPoints, format);
    }


    // step 4. lines constructed
    //    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    //    int k=6;
    //    float m = 8;

    //    NCPointCloud lines;
    //    lines.connectPoints2Lines(cnvtPoints, linesTraced, k, maxAngle, m);

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
        p2.bkg_thresh = 72; // 55
        p2.length_thresh = 10; // 15
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
//        p2.is_gsdt = true;
//        p2.is_coverage_prune = true;
//        p2.is_break_accept = true;
//        p2.bkg_thresh = 10;
//        p2.length_thresh = 5;
//        p2.cnn_type = 2;
//        p2.channel = 0;
//        p2.SR_ratio = 3.0/9.9;
//        p2.b_256cube = false;
//        p2.b_RadiusFrom2D = true;
//        p2.b_resample = 1;
//        p2.b_intensity = 0;
//        p2.b_brightfiled = 0;
//        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.
        
//        p2.p4dImage = p4dImage;
//        p2.xc0 = p2.yc0 = p2.zc0 = 0;
//        p2.xc1 = p2.p4dImage->getXDim()-1;
//        p2.yc1 = p2.p4dImage->getYDim()-1;
//        p2.zc1 = p2.p4dImage->getZDim()-1;
        
//        p2.outswc_file = neuronTraced2;
//        proc_app2(callback, p2, versionStr);
        
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
//    files.push_back(neuronTraced2);
    
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
    
    // compare single projection to 3 projections
    bool zprojonly = true;
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

    // 1st parameter k=0
    QString model_file = paras.empty() ? "" : paras[k]; if(model_file == "NULL") model_file = ""; k++;
    if(model_file.isEmpty())
    {
        cerr<<"Need a model_file"<<endl;
        return false;
    }
    
    // 2nd parameter k=1
    QString trained_file = paras.empty() ? "" : paras[k]; if(trained_file == "NULL") trained_file = ""; k++;
    if(trained_file.isEmpty())
    {
        cerr<<"Need a trained_file"<<endl;
        return false;
    }
    
    // 3rd parameter k=2
    QString mean_file = paras.empty() ? "" : paras[k]; if(mean_file == "NULL") mean_file = ""; k++;
    if(mean_file.isEmpty())
    {
        cerr<<"Need a mean_file"<<endl;
        return false;
    }

    // 4th parameter k=3
    if(paras.size() >= k+1)
        zprojonly = atoi(paras.at(k))==0?false:true;
    k++;

    cout<<"z proj only "<<zprojonly<<endl;
    
    // 5th parameter k=4
    int Sxy = (paras.size() >= k+1) ? atoi(paras[k]):10;k++;

    // 6th parameter k=5
    int Ws = (paras.size() >= k+1) ? atoi(paras[k]):512;k++;


    
    //
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
    unsigned char *data1d_zmip=0, *data1d_ymip=0, *data1d_xmip=0;
    
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

        if(zprojonly)
        {
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
        else
        {
            // z proj
            V3DLONG pagesz_mip = N*M;
            try {data1d_zmip = new unsigned char [pagesz_mip];}
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
                            data1d_zmip[iy*N + ix] = data1d[offsetk + offsetj + ix];
                            max_mip = data1d[offsetk + offsetj + ix];
                        }
                    }
                }
            }

            // y proj
            pagesz_mip = N*M*P;
            try {data1d_ymip = new unsigned char [pagesz_mip];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {
                    int max_mip = 0;
                    for(V3DLONG iy = 0; iy < M; iy++)
                    {
                        V3DLONG offsetj = iy*N;
                        if(data1d[offsetk + offsetj + ix] >= max_mip)
                        {
                            data1d_ymip[offsetk + ix] = data1d[offsetk + offsetj + ix];
                            max_mip = data1d[offsetk + offsetj + ix];
                        }
                    }
                }
            }

            // x proj
            pagesz_mip = N*M*P;
            try {data1d_xmip = new unsigned char [pagesz_mip];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                for(V3DLONG iy = 0; iy < M; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    int max_mip = 0;
                    for(V3DLONG ix = 0; ix < N; ix++)
                    {
                        if(data1d[offsetk + offsetj + ix] >= max_mip)
                        {
                            data1d_xmip[offsetk + iy] = data1d[offsetk + offsetj + ix];
                            max_mip = data1d[offsetk + offsetj + ix];
                        }
                    }
                }
            }
        }
    }
    
    std::vector<std::vector<float> > detection_results;
    LandmarkList marklist_2D, marklist_2Dz, marklist_2Dy, marklist_2Dx;
    Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());
    
    //
    unsigned int numOfThreads = 8; // default value for number of theads
    omp_set_num_threads(numOfThreads);
    
    V3DLONG iy, yb, ye;
    V3DLONG ix, xb, xe, blockpagesz;
    V3DLONG i, d, iiy, offsetj, iix;
    std::vector<float> det_output;
    LocationSimple LS;
    
    if(zprojonly)
    {
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
    }
    else
    {

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

                // z proj
                i = 0;
                for(iiy = yb; iiy < ye+1; iiy++)
                {
                    offsetj = iiy*N;
                    for(iix = xb; iix < xe+1; iix++)
                    {
                        blockarea[i] = data1d_zmip[offsetj + iix];
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
                            marklist_2Dz.push_back(LS);
                        }
                        d++;
                    }
                }
                if(blockarea) {delete []blockarea; blockarea =0;}
            }
        }

        for(iy = 0; iy < P; iy = iy+Ws)
        {

            yb = iy;
            ye = iy+Ws-1; if(ye>=P-1) ye = P-1;

            unsigned char *blockarea=0;

            for(ix = 0; ix < N; ix = ix+Ws)
            {
                xb = ix;
                xe = ix+Ws-1;
                if(xe>=N-1) xe = N-1;

                blockpagesz = (xe-xb+1)*(ye-yb+1)*1;
                blockarea = new unsigned char [blockpagesz];

                // y proj
                i = 0;
                for(iiy = yb; iiy < ye+1; iiy++)
                {
                    offsetj = iiy*N;
                    for(iix = xb; iix < xe+1; iix++)
                    {
                        blockarea[i] = data1d_ymip[iiy*M*N + iix];
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
                            LS.y = 1;
                            LS.z = iiy;
                            marklist_2Dy.push_back(LS);
                        }
                        d++;
                    }
                }
                if(blockarea) {delete []blockarea; blockarea =0;}
            }
        }

        for(iy = 0; iy < P; iy = iy+Ws)
        {

            yb = iy;
            ye = iy+Ws-1; if(ye>=P-1) ye = P-1;

            unsigned char *blockarea=0;

            for(ix = 0; ix < M; ix = ix+Ws)
            {
                xb = ix;
                xe = ix+Ws-1;
                if(xe>=M-1) xe = M-1;

                blockpagesz = (xe-xb+1)*(ye-yb+1)*1;
                blockarea = new unsigned char [blockpagesz];


                // x proj
                i = 0;
                for(iiy = yb; iiy < ye+1; iiy++)
                {
                    offsetj = iiy*N;
                    for(iix = xb; iix < xe+1; iix++)
                    {
                        blockarea[i] = data1d_xmip[iiy*M*N + iix];
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
                            LS.x = 1;
                            LS.y = iix;
                            LS.z = iiy;
                            marklist_2Dx.push_back(LS);
                        }
                        d++;
                    }
                }
            }
            if(blockarea) {delete []blockarea; blockarea =0;}
        }
    }

    //mean shift
    mean_shift_fun fun_obj, fun_objz, fun_objy, fun_objx;
    LandmarkList marklist_2D_shifted, marklist_2D_shiftedz, marklist_2D_shiftedy, marklist_2D_shiftedx;
    vector<V3DLONG> poss_landmark;
    vector<float> mass_center;
    double windowradius = Sxy+5;
    V3DLONG sz_img[4];

    if(zprojonly)
    {
        sz_img[0] = N; sz_img[1] = M; sz_img[2] = 1; sz_img[3] = 1;
        fun_obj.pushNewData<unsigned char>((unsigned char*)data1d_mip, sz_img);
        poss_landmark=landMarkList2poss(marklist_2D, sz_img[0], sz_img[0]*sz_img[1]);

        for (V3DLONG j=0;j<poss_landmark.size();j++)
        {
            mass_center=fun_obj.mean_shift_center_mass(poss_landmark[j],windowradius);
            LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
            marklist_2D_shifted.append(tmp);
        }
    }
    else
    {
        // z proj
        sz_img[0] = N; sz_img[1] = M; sz_img[2] = 1; sz_img[3] = 1;
        fun_objz.pushNewData<unsigned char>((unsigned char*)data1d_zmip, sz_img);
        poss_landmark=landMarkList2poss(marklist_2Dz, sz_img[0], sz_img[0]*sz_img[1]);

        for (V3DLONG j=0;j<poss_landmark.size();j++)
        {
            mass_center=fun_objz.mean_shift_center_mass(poss_landmark[j],windowradius);
            LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
            marklist_2D_shiftedz.append(tmp);
        }

        // y proj
        sz_img[0] = N; sz_img[1] = 1; sz_img[2] = P; sz_img[3] = 1;
        fun_objy.pushNewData<unsigned char>((unsigned char*)data1d_ymip, sz_img);
        poss_landmark=landMarkList2poss(marklist_2Dy, sz_img[0], sz_img[0]*sz_img[1]);

        for (V3DLONG j=0;j<poss_landmark.size();j++)
        {
            mass_center=fun_objy.mean_shift_center_mass(poss_landmark[j],windowradius);
            LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
            marklist_2D_shiftedy.append(tmp);
        }

        // x proj
        sz_img[0] = 1; sz_img[1] = M; sz_img[2] = P; sz_img[3] = 1;
        fun_objx.pushNewData<unsigned char>((unsigned char*)data1d_xmip, sz_img);
        poss_landmark=landMarkList2poss(marklist_2Dx, sz_img[0], sz_img[0]*sz_img[1]);

        for (V3DLONG j=0;j<poss_landmark.size();j++)
        {
            mass_center=fun_objx.mean_shift_center_mass(poss_landmark[j],windowradius);
            LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
            marklist_2D_shiftedx.append(tmp);
        }
    }

    //
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

    if(zprojonly)
    {
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
    }
    else
    {
        // z proj
        for(V3DLONG i = 0; i < marklist_2D_shiftedz.size(); i++)
        {
            V3DLONG ix = marklist_2D_shiftedz.at(i).x;
            V3DLONG iy = marklist_2D_shiftedz.at(i).y;
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

        // y proj
        for(V3DLONG i = 0; i < marklist_2D_shiftedy.size(); i++)
        {
            V3DLONG ix = marklist_2D_shiftedy.at(i).x;
            V3DLONG iz = marklist_2D_shiftedy.at(i).z;
            double I_max = 0;
            V3DLONG iy;
            for(V3DLONG j = 0; j < M; j++)
            {
                if(data1d[iz*M*N + j*N + ix] >= I_max)
                {
                    I_max = data1d[iz*M*N + j*N + ix];
                    iy = j;
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

        // x proj
        for(V3DLONG i = 0; i < marklist_2D_shiftedx.size(); i++)
        {
            V3DLONG iz = marklist_2D_shiftedx.at(i).z;
            V3DLONG iy = marklist_2D_shiftedx.at(i).y;
            double I_max = 0;
            V3DLONG ix;
            for(V3DLONG j = 0; j < N; j++)
            {
                if(data1d[iz*M*N + iy*N + j] >= I_max)
                {
                    I_max = data1d[iz*M*N + iy*N + j];
                    ix = j;
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
    }

    // delete false detections
    QList <ImageMarker> marklist_3D_pruned = batch_deletion(data1d,classifier,marklist_3D,N,M,P);

    //    cout<<"test 01 "<<marklist_3D.size()<<" "<<marklist_3D_pruned.size()<<endl;

    //    //
    //    LandmarkList marklist_3D_pruned1,marklist_3D_pruned_shifted;
    //    for(long i=0; i<marklist_3D_pruned.size(); i++)
    //    {
    //        ImageMarker m = marklist_3D_pruned[i];
    //        LocationSimple s;
    //        s.x = m.x;
    //        s.y = m.y;
    //        s.z = m.z;

    //        if(s.x<0 || s.y<0 || s.z<0 || s.x>N-2 || s.y>M-2 || s.z>P-2)
    //        {
    //            cout<<"odd"<<endl;
    //        }
    //        else
    //        {
    //            marklist_3D_pruned1.append(s);
    //        }
    //    }

    //    cout<<"test 02\n";

    //    sz_img[0] = N; sz_img[1] = M; sz_img[2] = P; sz_img[3] = 1;
    //    fun_objx.pushNewData<unsigned char>((unsigned char*)data1d, sz_img);

    //    cout<<"test 021\n";

    //    poss_landmark=landMarkList2poss(marklist_3D_pruned1, sz_img[0], sz_img[0]*sz_img[1]);

    //    cout<<"test 022 "<<poss_landmark.size()<<endl;

    //    for (V3DLONG j=0;j<poss_landmark.size();j++)
    //    {
    //        mass_center=fun_objx.mean_shift_center_mass(poss_landmark[j],windowradius);
    //        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
    //        marklist_3D_pruned_shifted.append(tmp);
    //    }

    //    cout<<"test 03\n";

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
    //pcsorted.ksort(pointcloud, 10);
    //pcsorted.savePointCloud(cnvtPoints);

    pointcloud.sortbyradius();
    pointcloud.savePointCloud(cnvtPoints);

    // step 3. lines constructed
    float maxAngle = 0.942; // threshold 60 degree (120 degree)
    int knn=6;
    float m = 8;

    //NCPointCloud lines;
    //lines.connectPoints2Lines(cnvtPoints, linesTraced, knn, maxAngle, m);

    // step 4.

    //
    return true;
}

// trace array tomography data
bool attrace_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file and a marker file\n";
        return false;
    }

    //parsing input
    float athresh = 0.75;
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
            athresh = atof(paras->at(0));
            cout<<"threshold relax parameter (0.75 by default): "<<athresh<<endl;
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<2)
    {
        cerr<<"You must specify input tiff and marker files"<<endl;
        return false;
    }

    //
    QString fnimage = QString(inlist->at(0));
    QString fnmarker = QString(inlist->at(1));
    QString markers4trace = fnimage.left(fnimage.lastIndexOf(".")).append(".marker");
    QString markersIntrace = fnimage.left(fnimage.lastIndexOf(".")).append("_used4trace.marker");

    //
    if(fnimage.toUpper().endsWith(".V3DRAW") || fnimage.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(fnimage.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }

        long sx = p4dImage->getXDim();
        long sy = p4dImage->getYDim();
        long sz = p4dImage->getZDim();
        long size = sx*sy*sz;
        unsigned char *p = p4dImage->getRawData();
        unsigned char *puint8 = NULL;

        //
        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            // convert 16-bit to 8-bit
            if(p4dImage->getDatatype()==V3D_UINT16)
            {
                unsigned short *p1d = (unsigned short *)(p4dImage->getRawData());

                float maxval = 0;
                for(long i=0; i<size; i++)
                {
                    if(maxval<p1d[i])
                    {
                        maxval = p1d[i];
                    }
                }

                maxval = 255 / maxval;

                y_new1dp<unsigned char, long>(puint8, size);
                for(long i=0; i<size; i++)
                {
                    puint8[i] = maxval * p1d[i];
                }

                p = puint8;
            }
            else
            {
                cout<<"datatype is not supported\n";
                return false;
            }
        }

        //
        float thresh;
        estimateIntensityThreshold(p, size, thresh);
        thresh *= athresh;
        cout<<"threshold ... "<<thresh<<endl;

        //
        LandmarkList landmarks;
        QList<ImageMarker> markers = readMarker_file(fnmarker);
        NCPointCloud pc, pcsave;
        for(size_t i=0; i<markers.size(); i++)
        {
            ImageMarker marker = markers[i];
            Point point;
            long z = long(marker.z);

            if(z>sz-1) z=sz-1;

            long y = long(marker.y);

            if(y>sy-1) y=sy-1;

            long x = long(marker.x);

            if(x>sx-1) x=sx-1;


            long xpre = x-1, xnext = x+1;

            if(xpre<0) xpre=0;
            if(xnext>sx-1) xnext = sx-1;

            long ypre = y-1, ynext = y+1;

            if(ypre<0) ypre=0;
            if(ynext>sy-1) ynext = sy-1;

            long zpre = z-1, znext = z+1;

            if(zpre<0) zpre=0;
            if(znext>sz-1) znext = sz-1;

            float maxval = 0;

            for(long zz=zpre; zz<=znext; zz++)
            {
                for(long yy=ypre; yy<=ynext; yy++)
                {
                    for(long xx=xpre; xx<=xnext; xx++)
                    {
                        if(float(p[zz*sx*sy + yy*sx + xx]) > maxval)
                        {
                            maxval = float(p[zz*sx*sy + yy*sx + xx]);
                            x = xx;
                            y = yy;
                            z = zz;
                        }
                    }
                }
            }

            point.x = x;
            point.y = y;
            point.z = z;
            point.val = maxval;

            if( point.val > thresh)
            {
                //
                pc.points.push_back(point);
            }
        }
        //
        sort(pc.points.begin(), pc.points.end(), [](const Point& a, const Point& b) -> bool
        {
            return a.val*a.radius > b.val*b.radius;
        });
        pc.savePointCloud(markers4trace, 1);

        for(size_t i=0; i<pc.points.size(); i++)
        {
            Point point = pc.points[i];
            landmarks.push_back(LocationSimple(point.x, point.y, point.z));
        }
        cout<<"tracing with "<<landmarks.size()<<" markers "<<endl;

        // app2
        vector<NeuronTree> tracedFilaments;
        float distthresh = 10;
        long iter = 0;
        for(size_t i=0; i<pc.points.size(); i++)
        {
            //
            Point point = pc.points[i];

            //
            bool skip = false;
            if(i>0)
            {
                if(tracedFilaments.size()>0)
                {
                    for(size_t j=0; j<tracedFilaments.size(); j++)
                    {
                        NeuronTree nt = tracedFilaments[j];

                        if(nt.listNeuron.size()>0)
                        {
                            for(size_t k=0; k<nt.listNeuron.size(); k++)
                            {
                                Point q;
                                q.x = nt.listNeuron[k].x;
                                q.y = nt.listNeuron[k].y;
                                q.z = nt.listNeuron[k].z;
                                q.radius = nt.listNeuron[k].r;

                                if(pc.distance(point, q)< max(distthresh, q.radius))
                                {
                                    skip = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if(skip)
            {
                cout<<"skip point #"<<i<<endl;
                continue;
            }

            //
            QString segmenttraced = fnimage.left(fnimage.lastIndexOf(".")).append(QString("_autotraced%1.swc").arg(++iter));
            LandmarkList landmark;
            landmark.push_back(LocationSimple(point.x, point.y, point.z));

            //
            PARA_APP2 p2;
            QString versionStr = "v0.001";

            //
            p2.is_gsdt = true;
            p2.is_coverage_prune = true;
            p2.is_break_accept = true;
            p2.bkg_thresh = thresh;
            p2.length_thresh = 10;
            p2.cnn_type = 2;
            p2.channel = 0;
            p2.SR_ratio = 3.0/9.9;
            p2.b_256cube = false;
            p2.b_RadiusFrom2D = true;
            p2.b_resample = 0;
            p2.b_intensity = 0;
            p2.b_brightfiled = 0;
            p2.b_menu = 0;
            p2.landmarks = landmark;

            p2.p4dImage = p4dImage;
            p2.xc0 = p2.yc0 = p2.zc0 = 0;
            p2.xc1 = sx-1;
            p2.yc1 = sy-1;
            p2.zc1 = sz-1;

            p2.outswc_file = segmenttraced;
            proc_app2(callback, p2, versionStr);

            //
            NeuronTree nt = readSWC_file(segmenttraced);
            if(nt.listNeuron.size()>0)
            {
                float swcdistthresh = 10;

                if(tracedFilaments.size()>0)
                {
                    bool add = true;
                    for(size_t j=0; j<tracedFilaments.size(); j++)
                    {
                        if(pc.meandistance(nt, tracedFilaments[j])<swcdistthresh)
                        {
                            add = false;
                            break;
                        }
                    }

                    if(add)
                    {
                        tracedFilaments.push_back(nt);
                        pcsave.points.push_back(point);
                    }
                }
                else
                {
                    tracedFilaments.push_back(nt);
                    pcsave.points.push_back(point);
                }
            }

        }

        //
        pcsave.savePointCloud(markersIntrace, 1);

        //
        long offset = 0;
        QString outputswc = fnimage.left(fnimage.lastIndexOf(".")).append(QString("_autotraced.swc"));
        NeuronTree autotraced;
        if(tracedFilaments.size()>0)
        {
            for(size_t j=0; j<tracedFilaments.size(); j++)
            {
                NeuronTree nt = tracedFilaments[j];

                if(nt.listNeuron.size()>0)
                {
                    for(size_t k=0; k<nt.listNeuron.size(); k++)
                    {
                        nt.listNeuron[k].n += offset;

                        if(nt.listNeuron[k].pn > 0)
                        {
                            nt.listNeuron[k].pn += offset;
                        }

                        autotraced.listNeuron.push_back(nt.listNeuron[k]);
                    }

                    offset += nt.listNeuron.size() + 1;
                }
            }
        }
        writeSWC_file(outputswc,autotraced);

        //
        y_del1dp<unsigned char>(puint8);
    }
    else
    {
        cout<<"Please input an image file (.v3draw/.tif)\n";
        return -1;
    }

    //
    return true;
}

//
bool maskmeandev_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file and a swc file\n";
        return false;
    }

    //parsing input
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
    if (inlist->size()<2)
    {
        cerr<<"You must specify input tiff and marker files"<<endl;
        return false;
    }

    //
    QString fnimage = QString(inlist->at(0));
    QString fnswc = QString(inlist->at(1));
    QString outFileName = fnimage.left(fnimage.lastIndexOf(".")).append("_mask.tif");

    //
    if(fnimage.toUpper().endsWith(".V3DRAW") || fnimage.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(fnimage.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image!\n";
            return false;
        }

        long sx = p4dImage->getXDim();
        long sy = p4dImage->getYDim();
        long sz = p4dImage->getZDim();
        long size = sx*sy*sz;
        unsigned char *p = p4dImage->getRawData();
        unsigned char *puint8 = NULL;

        //
        if(p4dImage->getDatatype()!=V3D_UINT8)
        {
            // convert 16-bit to 8-bit
            if(p4dImage->getDatatype()==V3D_UINT16)
            {
                unsigned short *p1d = (unsigned short *)(p4dImage->getRawData());

                float maxval = 0;
                for(long i=0; i<size; i++)
                {
                    if(maxval<p1d[i])
                    {
                        maxval = p1d[i];
                    }
                }

                maxval = 255 / maxval;

                y_new1dp<unsigned char, long>(puint8, size);
                for(long i=0; i<size; i++)
                {
                    puint8[i] = maxval * p1d[i];
                }

                p = puint8;
            }
            else
            {
                cout<<"datatype is not supported\n";
                return false;
            }
        }

        //
        NCPointCloud pc;
        vector<LineSegment> lines;
        if(fnswc.toUpper().endsWith(".SWC"))
        {
            NeuronTree nt = readSWC_file(fnswc);
            pc.addPointFromNeuronTree(nt);
            lines = separate(pc);
        }
        long n = lines.size();

        //
        unsigned char *dt=NULL;
        y_new1dp<unsigned char, long>(dt, size);

        distanceTransformL2(dt, p, sx, sy, sz);

        //
        double *meanr = NULL;
        y_new1dp<double, long>(meanr, n);
        for(long ii=0; ii<n; ii++)
        {
            meanr[ii] = 0;
        }

        V3DLONG insz[4];
        insz[0] = sx;
        insz[1] = sy;
        insz[2] = sz;
        insz[3] = 1;

        long nstep=3;

        for(long i=0; i<n; i++)
        {
            for(long j=0; j<lines[i].points.size(); j++)
            {
                Point pi = lines[i].points[j];

                double maxval = 0;

                long x = pi.x;
                long y = pi.y;
                long z = pi.z;

                for(long zz=z-nstep; zz<=z+nstep; zz++)
                {
                    if(zz<0 || zz>sz-1)
                        continue;

                    long ofz = zz*sx*sy;
                    for(long yy=y-nstep; yy<=y+nstep; yy++)
                    {
                        if(yy<0 || yy>sy-1)
                            continue;

                        long ofy = ofz + yy*sx;
                        for(long xx=x-nstep; xx<=x+nstep; xx++)
                        {
                            if(xx<0 || xx>sx-1)
                                continue;

                            if(dt[ofy+xx]>maxval)
                                maxval = dt[ofy+xx];
                        }
                    }

                }

                meanr[i] += maxval;

                lines[i].points[j].radius = maxval; // update radius with distance transform
                //meanr[i] += estimateRadius<unsigned char>(p, insz, pi.x, pi.y, pi.z, 10);
            }
        }

        //
        unsigned char *mask = NULL;
        y_new1dp<unsigned char, long>(mask, size);

        //
        for(long i=0; i<n; i++)
        {
            points2maskimage<unsigned char>(mask, lines[i], sx, sy, sz, i+1);
        }

        for(long i=0; i<n; i++)
        {
            meanr[i] /= lines[i].points.size();
        }

        y_del1dp<unsigned char>(dt);

        //
        double *meanval = NULL, *stddev = NULL, *snr = NULL;
        y_new1dp<double, long>(meanval, n);
        y_new1dp<double, long>(stddev, n);
        y_new1dp<double, long>(snr, n);
        long *nvox = NULL;
        y_new1dp<long, long>(nvox, n);

        for(long ii=0; ii<n; ii++)
        {
            meanval[ii] = 0;
            stddev[ii] = 0;
            snr[ii] = 0;
            nvox[ii] = 0;
        }

        for(long i=0; i<size; i++)
        {
            for(long ii=0; ii<n; ii++)
            {
                if(mask[i]==ii+1)
                {
                    meanval[ii] += p[i];
                    nvox[ii]++;
                }
            }
        }

        for(long ii=0; ii<n; ii++)
        {
            meanval[ii] /= nvox[ii];
            cout<<"i "<<ii+1<<" mean "<<meanval[ii]<<endl;
        }

        //
        for(long i=0; i<size; i++)
        {
            for(long ii=0; ii<n; ii++)
            {
                if(mask[i]==ii+1)
                {
                    stddev[ii] += (p[i] - meanval[ii])*(p[i] - meanval[ii]);
                }
            }
        }

        if(n>1)
        {
            for(long ii=0; ii<n; ii++)
            {
                stddev[ii] = sqrt(stddev[ii]/(n-1));
                cout<<"i "<<ii+1<<" standard deviation "<<stddev[ii]<<endl;

                if(stddev[ii])
                {
                    snr[ii] = meanval[ii] / stddev[ii];
                    cout<<"snr "<<snr[ii]<<endl;
                }
            }
        }

        //
        double mean = 0, sd = 0;

        if(n>1)
        {
            // mean radius
            for(long ii=0; ii<n; ii++)
            {
                mean += meanr[ii];
            }

            mean /= n;

            for(long ii=0; ii<n; ii++)
            {
                sd += (snr[ii] - mean)*(snr[ii] - mean);
            }

            sd = sqrt(sd / (n-1));

            cout<<endl<<"mean "<<mean<<" stddev "<<sd<<endl;

            // mean val
            mean = 0;
            sd = 0;
            for(long ii=0; ii<n; ii++)
            {
                mean += meanval[ii];
            }

            mean /= n;

            for(long ii=0; ii<n; ii++)
            {
                sd += (snr[ii] - mean)*(snr[ii] - mean);
            }

            sd = sqrt(sd / (n-1));

            cout<<endl<<"mean "<<mean<<" stddev "<<sd<<endl;


            // SNR
            mean = 0;
            sd = 0;
            for(long ii=0; ii<n; ii++)
            {
                mean += snr[ii];
            }

            mean /= n;

            for(long ii=0; ii<n; ii++)
            {
                sd += (snr[ii] - mean)*(snr[ii] - mean);
            }

            sd = sqrt(sd / (n-1));

            cout<<endl<<"mean "<<mean<<" stddev "<<sd<<endl;
        }

        //
        y_del1dp<long>(nvox);
        y_del1dp<double>(meanr);
        y_del1dp<double>(snr);
        y_del1dp<double>(meanval);
        y_del1dp<double>(stddev);
        y_del1dp<unsigned char>(mask);

        //
        p4dImage->setData(mask, p4dImage->getXDim(), p4dImage->getYDim(), p4dImage->getZDim(), 1, V3D_UINT8);
        p4dImage->saveImage(const_cast<char *>(outFileName.toStdString().c_str()));
    }

    //
    return true;
}

bool swcsplit_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file and a swc file\n";
        return false;
    }

    //parsing input
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
        cerr<<"You must input a swc file"<<endl;
        return false;
    }

    //
    QString fnswc = QString(inlist->at(0));

    //
    NCPointCloud pc;
    vector<LineSegment> lines;
    if(fnswc.toUpper().endsWith(".SWC"))
    {
        NeuronTree nt = readSWC_file(fnswc);
        pc.addPointFromNeuronTree(nt);
        lines = separate(pc);
    }
    long n = lines.size();

    //
    for(long i=0; i<n; i++)
    {
        QString separateswc = fnswc.left(fnswc.lastIndexOf(".")).append(QString("_part%1.swc").arg(i));
        pc.saveNeuronTree(lines[i], separateswc);
    }

    //
    return true;
}

bool rmisolatepoints_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file and a swc file\n";
        return false;
    }

    //parsing input
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
        cerr<<"You must input a swc file"<<endl;
        return false;
    }

    //
    QString fnswc = QString(inlist->at(0));
    QString fnout = fnswc.left(fnswc.lastIndexOf(".")).append("_woisolated.swc");

    //
    NCPointCloud pc;
    if(fnswc.toUpper().endsWith(".SWC"))
    {
        NeuronTree nt = readSWC_file(fnswc);
        pc.addPointFromNeuronTree(nt);
        pc.isolatedPoints();
        pc.saveNeuronTree(pc,fnout);
    }

    //
    return true;
}

bool checkImage_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1)
    {
        cout<<"please input a TIFF file\n";
        return false;
    }

    //parsing input
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
        cerr<<"Invalid input"<<endl;
        return false;
    }

    //
    QString filename = QString(inlist->at(0));

    if(filename.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(filename.toStdString().c_str()) );
        if (!p4dImage || !p4dImage->valid())
        {
            cout<<"fail to load image "<<filename.toStdString()<<endl;
            return false;
        }
        else
        {
            cout<<"load image "<<filename.toStdString()<<" "<<p4dImage->getXDim()<<" x "<<p4dImage->getYDim()<<endl;
        }
    }
    else
    {
        cout<<"Current only support TIFF image as input file\n";
        return false;
    }

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
    QGroupBox *input_panel = new QGroupBox("BigNeuron-based Neuron Construction:");
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
    if(!QFile(fnITKfiltered).exists())
    {
        if(filename.toUpper().endsWith(".TIF") || filename.toUpper().endsWith(".TIFF"))
        {
            runGPUGradientAnisotropicDiffusionImageFilter<unsigned char, unsigned char, 3>(filename.toStdString(), fnITKfiltered.toStdString());
        }
        else
        {
            cout<<"Current only support TIFF image as input file\n";
            return;
        }
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
