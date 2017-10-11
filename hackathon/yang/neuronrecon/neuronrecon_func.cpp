/* neuronrecon_func.cpp
 * a plugin to construct neuron tree(s) from detected signals
 * 09/11/2017 : by Yang Yu
 */

//
#include "neuronrecon_func.h"
#include "neuronrecon.h"

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
        ControlPanel* p = new ControlPanel(callback, parent);
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

//
void printHelp()
{
    cout<<"\n Optimal Constructing Neuron Trees: \n"<<endl;
}

//
ControlPanel::ControlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
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

    connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
    connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(_slot_close()));
    connect(pPushButton_openFileDlg, SIGNAL(clicked()), this, SLOT(_slots_openFile()));
}

ControlPanel::~ControlPanel()
{
    m_controlpanel = 0;
}

void ControlPanel::_slot_close()
{
    if (m_controlpanel)
    {
        delete m_controlpanel;
        m_controlpanel=0;
    }
}
void ControlPanel::_slot_start()
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
    QString title = QString("line construction");
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
