/* flythrough_plugin.cpp
 * a plugin to create a fly-through virtual reality
 * 11/20/2018 : by Yang Yu
 */

#include "flythrough_plugin.h"
#include "flythrough_func.h"
#include "neuron_format_converter.h"

Q_EXPORT_PLUGIN2(flythrough, FlyThroughPlugin);

QStringList FlyThroughPlugin::menulist() const
{
    return QStringList()
            <<tr("flythrough")
           <<tr("about");
}

QStringList FlyThroughPlugin::funclist() const
{
    return QStringList()
            <<tr("flythrough")
           <<tr("help");
}

void FlyThroughPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if(menu_name == tr("flythrough"))
    {
        // inputs
        // swc (sampled preferred)
        // data dir
        // size x,y,z

        QString winname = QString("fly through");

        QString swcFilePath, dataDirPath;
        V3DLONG bsx, bsy, bsz;

        FlyThroughDialog dialog(callback, parent);
        if(dialog.exec()!=QDialog::Accepted)
            return;

        dialog.update();

        dataDirPath = dialog.fn_img;
        swcFilePath = dialog.fn_swc;

        bsx = dialog.bsx;
        bsy = dialog.bsy;
        bsz = dialog.bsz;

        qDebug()<<"swcFilePath ... "<<swcFilePath;
        qDebug()<<"dataDirPath ... "<<dataDirPath;
        qDebug()<<"size ... "<<bsx<<bsy<<bsz;

        // step 1. swc into fragments (stacking)
        V_NeuronSWC_list neulist;
        if(swcFilePath.toUpper().endsWith(".SWC") || swcFilePath.toUpper().endsWith(".ESWC"))
        {
            NeuronTree nt = readSWC_file(swcFilePath);
            neulist = NeuronTree__2__V_NeuronSWC_list(&nt);

            cout<<"neuron segments "<<neulist.seg.size()<<endl;
        }
        else
        {
            cout<<"Need input a neuron reconstruction in swc/eswc format"<<endl;
            return;
        }


        // step 2. for each fragment: (p switch bwteen play and pause)

//        for(V3DLONG i=0; i<neulist.nsegs(); i++)
//        {
//            cout<<i<<": "<<neulist.seg[i].row.size()<<endl;
//        }

        V3DLONG i = 31; // for test
        V_NeuronSWC neuseg = neulist.seg[i];

        //
        NeuronTree fragment;
        fragment.name=winname;
        fragment.listNeuron.clear();
        fragment.hashNeuron.clear();
        fragment.color.r = fragment.color.g = fragment.color.b = fragment.color.a = 0;

        float x_offset = bsx/2 - neuseg.row[0].x;
        float y_offset = bsy/2 - neuseg.row[0].y;
        float z_offset = bsz/2 - neuseg.row[0].z;

        PointCloud *pc = new PointCloud;
        for(V3DLONG j=0; j<neuseg.row.size(); j++)
        {
            Point p(neuseg.row[j].x, neuseg.row[j].y, neuseg.row[j].z);
            pc->push_back(p);

            //
            NeuronSWC S;
            S.n = neuseg.row[j].n;
            S.type = neuseg.row[j].type;
            S.x = neuseg.row[j].x+x_offset-1;
            S.y = neuseg.row[j].y+y_offset-1;
            S.z = neuseg.row[j].z+z_offset-1;
            S.r = neuseg.row[j].r;
            S.pn = neuseg.row[j].parent;
            fragment.listNeuron.append(S);
            fragment.hashNeuron.insert(S.n, fragment.listNeuron.size()-1);
        }

        //
        DataFlow df(pc, dataDirPath.toStdString(), bsx, bsy, bsz);

        // demo
        for(V3DLONG k=0; k<pc->size(); k++)
        {
            cout<<"test ... "<<k<<" "<<pc->at(k).blocks.size()<<endl;

            // update swc visualization offset
            if(k>0)
            {
                x_offset = bsx/2 - pc->at(k).x;
                y_offset = bsy/2 - pc->at(k).y;
                z_offset = bsz/2 - pc->at(k).z;

                for(V3DLONG j=0; j<neuseg.row.size(); j++)
                {
                    //
                    fragment.listNeuron[j].x = neuseg.row[j].x+x_offset-1;
                    fragment.listNeuron[j].y = neuseg.row[j].y+y_offset-1;
                    fragment.listNeuron[j].z = neuseg.row[j].z+z_offset-1;
                }
            }

            // update centered image stack
            unsigned char *pCropImage = pc->at(k).data(df.bytesPerVoxel, df.tree); // will replace tree to lrucache later

            cout<<"test pointer ... 1 "<<(void*)pCropImage<<endl;
            // cout<<"test "<<((unsigned short*)pCropImage)[0]<<endl;

            unsigned char *p;
            try
            {
                V3DLONG size = pc->at(k).getSize();

                p = new unsigned char [size];
                //memcpy(p, pCropImage, size);
                pc->at(k).release();
                memset(p, 0, size);
            }
            catch(...)
            {
                cout<<"failed in allocate memory and copy the cropped image"<<endl;
                return;
            }

            cout<<"test pointer ... 1.2 "<<(void*)p<<endl;
            cout<<"test "<<((unsigned short*)p)[0]<<endl;

            // view
            Image4DSimple *tmpimg = new Image4DSimple();
            tmpimg->setData(p,bsx,bsy,bsz,1,V3D_UINT16);

            cout<<"test ... "<<pc->at(k).getVoxels()<<" == "<<bsx*bsy*bsz<<endl;

//            int maxval = 0;
//            unsigned short * pTest = (unsigned short *)(p);
//            for(int i=0; i<32*32*32; i++)
//            {
//                if(pTest[i]>maxval)
//                    maxval = pTest[i];
//            }
//            cout<<"max val ... ... "<<maxval<<endl;

            cout<<"test pointer ... 2 "<<(void*)p<<endl;
            cout<<"test "<<((unsigned short*)p)[0]<<endl;

            //
            v3dhandleList allWindowList = callback.getImageWindowList();
            v3dhandle localwin = 0;
            for (V3DLONG i=0;i<allWindowList.size();i++)
            {
                if(callback.getImageName(allWindowList.at(i)).contains(winname)){
                    localwin = allWindowList[i];
                    break;
                }
            }

            cout<<"test pointer ... 3 "<<(void*)p<<endl;
            cout<<"test "<<((unsigned short*)p)[0]<<endl;

            if(localwin == 0)
            {
                localwin = callback.newImageWindow(winname);
                callback.setImage(localwin, tmpimg);
                //callback->setLandmark(localwin, local_landmark);
                callback.setSWC(localwin, fragment);
                //callback.updateImageWindow(localwin);
                callback.open3DWindow(localwin);
                callback.pushObjectIn3DWindow(localwin);
                callback.pushImageIn3DWindow(localwin);
            }
            else
            {
                callback.setImage(localwin, tmpimg);
                //callback->setLandmark(localwin, local_landmark);
                callback.setSWC(localwin, fragment);
                //callback.updateImageWindow(localwin);
                callback.open3DWindow(localwin);
                callback.pushImageIn3DWindow(localwin);
                callback.pushObjectIn3DWindow(localwin);
            }

            cout<<"test pointer ... 4 "<<(void*)p<<endl;
            cout<<"test "<<((unsigned short*)p)[0]<<endl;

//            if(p)
//            {
//                delete []p;
//            }
        }

        // step 2.1. load related blocks (caching)

        // step 2.2. re-orientation (transform)

        // step 2.3. re-center and crop

        // step 2.4. push to 3d viewer

        // step 2.5. flythrough

        // step 2.5.1. view angle, ...



    }
    else if (menu_name == tr("about"))
    {
        v3d_msg(tr("a plugin to create a fly-through virtual reality. developed by Yang Yu, 11/20/2018. "));
    }
    else
    {
        v3d_msg(tr("a plugin to create a fly-through virtual reality. developed by Yang Yu, 11/20/2018. "));
    }
}

bool FlyThroughPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    //
    if (func_name == tr("flythrough"))
    {
        return flythrough_func(input, output, callback);
    }
    else if (func_name == tr("help"))
    {
        cout<<"vaa3d -x flythrough -f flythrough -i inputdir input.swc -o outputdir -p scale<0/1/2/3/4/5/...>"<<endl;
        return true;
    }

    //
    return true;
}

//
FlyThroughDialog::FlyThroughDialog(V3DPluginCallback &callback, QWidget *parentWidget)
{
    // input image
    label_img = new QLabel(QObject::tr("Choose the brain in TeraFly format (finest resolution): "));

    edit_img = new QLineEdit(QDir::currentPath());
    pb_browse_img = new QPushButton("Browse...");

    // input swc
    label_swc = new QLabel(QObject::tr("Choose a neuron reconstruction (.swc): "));

    edit_swc = new QLineEdit(QDir::currentPath());
    pb_browse_swc = new QPushButton("Browse...");

    // size
    v_size_x = new QSpinBox();
    v_size_y = new QSpinBox();
    v_size_z = new QSpinBox();

    v_size_x->setMaximum(4096); v_size_x->setMinimum(1); v_size_x->setValue(32); // 256
    v_size_y->setMaximum(4096); v_size_y->setMinimum(1); v_size_y->setValue(32);
    v_size_z->setMaximum(4096); v_size_z->setMinimum(1); v_size_z->setValue(32);

    label_size = new  QLabel(QObject::tr("Image Size: "));
    label_size_x = new QLabel(QObject::tr(" x "));
    label_size_y = new QLabel(QObject::tr(" y "));
    label_size_z = new QLabel(QObject::tr(" z "));

    //
    ok     = new QPushButton("OK");
    cancel = new QPushButton("Cancel");

    // gridlayout
    QGridLayout* gridLayout = new QGridLayout();
    // gridLayout->setSizeConstraint( QLayout::SetFixedSize );

    gridLayout->addWidget(label_img, 0,0, Qt::AlignRight); gridLayout->addWidget(edit_img, 0,1,1,5); gridLayout->addWidget(pb_browse_img, 0,6);
    gridLayout->addWidget(label_swc, 1,0, Qt::AlignRight); gridLayout->addWidget(edit_swc, 1,1,1,5); gridLayout->addWidget(pb_browse_swc, 1,6);

    gridLayout->addWidget(label_size, 2,0,1,1, Qt::AlignRight);
    gridLayout->addWidget(label_size_x, 2,2,1,1, Qt::AlignRight); gridLayout->addWidget(v_size_x, 2,1,1,1);
    gridLayout->addWidget(label_size_y, 3,2,1,1, Qt::AlignRight); gridLayout->addWidget(v_size_y, 3,1,1,1);
    gridLayout->addWidget(label_size_z, 4,2,1,1, Qt::AlignRight); gridLayout->addWidget(v_size_z, 4,1,1,1);

    gridLayout->addWidget(cancel, 5,5,Qt::AlignRight); gridLayout->addWidget(ok, 5,6,Qt::AlignRight);

    setLayout(gridLayout);
    setWindowTitle(QString("FlyThrough"));

    // signal and slot
    connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

    connect(v_size_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(v_size_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(v_size_z, SIGNAL(valueChanged(int)), this, SLOT(update()));

    connect(pb_browse_img, SIGNAL(clicked()), this, SLOT(get_img_name()));
    connect(pb_browse_swc, SIGNAL(clicked()), this, SLOT(get_swc_name()));
}

FlyThroughDialog::~FlyThroughDialog()
{

}

void FlyThroughDialog::update()
{
    bsx = v_size_x->text().toInt();
    bsy = v_size_y->text().toInt();
    bsz = v_size_z->text().toInt();
}

void FlyThroughDialog::get_img_name()
{
    fn_img = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory of the finest resolution "),
                                               QDir::currentPath(),
                                               QFileDialog::ShowDirsOnly);


    edit_img->setText(fn_img);
}

void FlyThroughDialog::get_swc_name()
{
    fn_swc = QFileDialog::getOpenFileName(this, tr("Open a neuron reconstruction (.swc)"),
                                          QDir::currentPath(),
                                          tr("SWC (*.*)"));


    edit_swc->setText(fn_swc);
}

