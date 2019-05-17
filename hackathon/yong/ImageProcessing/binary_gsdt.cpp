//#include "binary_gsdt.h"

//bool binary_gsdt(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
//{
//    v3dhandle curwin = callback.currentImageWindow();
//    if (!curwin)
//    {
//        QMessageBox::information(0, "", "You don't have any image open in the main window.");
//        return false;
//    }

//    Image4DSimple* p4DImage = callback.getImage(curwin);

//    if (!p4DImage)
//    {
//        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
//        return false;
//    }

//    cout<<"**********make bkg_threshold.**********"<<endl;
//    unsigned char* data1d = p4DImage->getRawData();

//    V3DLONG c=1;
//    V3DLONG in_sz[4];
//    in_sz[0]=p4DImage->getXDim();
//    in_sz[1]=p4DImage->getYDim();
//    in_sz[2]=p4DImage->getZDim();
//    in_sz[3]=p4DImage->getCDim();

//    double imgAve, imgStd, bkg_thresh;
//    mean_and_std(p4DImage->getRawData(), p4DImage->getTotalUnitNumberPerChannel(), imgAve, imgStd);

////    double td= (imgStd<10)? 10: imgStd;
////    bkg_thresh = imgAve +0.7*td ;
//    double td= (imgStd<15)? 15: imgStd;
//    bkg_thresh = imgAve +0.8*td;

//    QString avgAndstd=QString("avg:%1  std:%2").arg(imgAve).arg(imgStd);
//    QString threshold=QString("threshold:%1").arg(bkg_thresh);
//    qDebug() << "avgAndstd = "<<avgAndstd<<endl;
//    qDebug() << "bkg_thresh = "<<bkg_thresh<<endl;

//    //v3d_msg(avgAndstd);
//    //v3d_msg(threshold);

//    V3DLONG M = in_sz[0];
//    V3DLONG N = in_sz[1];
//    V3DLONG P = in_sz[2];
//    V3DLONG C = in_sz[3];
//    V3DLONG pagesz = M*N;
//    V3DLONG tol_sz = pagesz * P;

//    cout<<"**********make binary image.**********"<<endl;
//    unsigned char *im_cropped = 0;
//    try {im_cropped = new unsigned char [tol_sz];}
//     catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

//    for(V3DLONG i=0;i<tol_sz;i++)
//    {
//        if(double(data1d[i]) <  bkg_thresh)
//        {
//            im_cropped[i] = 0;
//        }
//        else
//        {
//            im_cropped[i] = 255;
//        }
//    }

//    PARA.inimg_file = p4DImage->getFileName();
//    QString binaryImage = PARA.inimg_file + "_binary.v3draw";
//    //QString binary_gsdtImage = PARA.inimg_file + "_binary_gsdt.v3draw";

//    if(!simple_saveimage_wrapper(callback,binaryImage.toStdString().c_str(),im_cropped,in_sz,1))
//    {
//        cerr<<"save image "<<binaryImage.toStdString()<<" error!"<<endl;
//        return false;
//    }

//    cout<<"**********creat gray(.txt) and coordinate(.marker) file**********"<<endl;
//     QList<ImageMarker> marker;
//     vector<float> signal_list;
//     vector<Coordinate> vec_coord;
//     ImageMarker * p_pt=0;
//     V3DLONG signal_loc = 0;

//     for(V3DLONG i = 0; i < tol_sz; i++)
//     {
//         if(data1d[i] > bkg_thresh)
//         {
//             signal_loc = i;
//             ImageMarker signal_marker(signal_loc % M, signal_loc % pagesz / M, signal_loc / pagesz);
//             signal_list.push_back(data1d[i]);
//             marker.push_back(signal_marker);
//         }

//     }

//     for (int i=0;i<marker.size(); i++)
//     {
//         Coordinate coordinate;
//         p_pt = (ImageMarker *)(&(marker.at(i)));
//         coordinate.x = p_pt->x;
//         coordinate.y = p_pt->y;
//         coordinate.z = p_pt->z;
//         coordinate.signal = signal_list[i];
//         vec_coord.push_back(coordinate);
//     }

//     cout<<endl<<"marker = "<<marker.size()<<endl;
//     cout<<"signal_list = "<<signal_list.size()<<endl;
//     cout<<"vec_coord = "<<vec_coord.size()<<endl;

//     export_TXT(vec_coord,QString(p4DImage->getFileName()).append(".txt"));
//     writeTXT_file(QString(p4DImage->getFileName()).append("_coordinate.txt"),marker);
//     writeMarker_file(QString(p4DImage->getFileName()).append(".marker"),marker);
//     //v3d_msg("checkout1!");

//     cout<<"**********call gsdt plugin**********"<<endl;
//// #if  defined(Q_OS_LINUX)
//// QString cmd_gsdt = QString("%1/vaa3d -x gsdt -f gsdt -i %2 -o %3 -p %4")
////         .arg(getAppPath().toStdString().c_str()).arg(binaryImage.toStdString().c_str()).arg(binary_gsdtImage.toStdString().c_str()).arg("0,1,0,1.0");
//// system(qPrintable(cmd_gsdt));
//// #endif

//    unsigned char * total1dData = 0;
//    unsigned char * out1dData = 0;
//    //V3DLONG in_sz[4];
//    int dataType;
//    if(!simple_loadimage_wrapper(callback, binaryImage.toStdString().c_str(), total1dData, in_sz, dataType))
//    {
//        cerr<<"load binary image "<<binaryImage.toStdString()<<" error!"<<endl;
//        return false;
//    }
//    Image4DSimple* total4DImage = new Image4DSimple;
//    total4DImage->setData((unsigned char*)total1dData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);

//    gsdt(total1dData,in_sz,callback,out1dData,PARA);

//    return true;
//}

//bool binary_gsdt(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA)
//{
//    return true;
//}

//template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev)
//{
//    if (!data || n<=0)
//      return false;

//    int j;
//    double ep=0.0,s,p;

//    if (n <= 1)
//    {
//      //printf("len must be at least 2 in mean_and_std\n");
//      ave = data[0];
//      sdev = (T2)0;
//      return true; //do nothing
//    }

//    s=0.0;
//    for (j=0;j<n;j++) s += data[j];
//    double ave_double=(T2)(s/n); //use ave_double for the best accuracy

//    double var=0.0;
//    for (j=0;j<n;j++) {
//        s=data[j]-(ave_double);
//        var += (p=s*s);
//    }
//    var=(var-ep*ep/n)/(n-1);
//    sdev=(T2)(sqrt(var));
//    ave=(T2)ave_double; //use ave_double for the best accuracy

//    return true;
//}


//bool gsdt(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback,unsigned char * &out1dData,input_PARA &PARA)
//{
//    cout<<"Welcome to gsdt!"<<endl;
//    int bkg_thresh = 0, cnn_type = 3, channel = 0, z_thickness = 1.0;
//    float * phi = 0;
//    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
//    unsigned char * inimg1d_ch1 = 0;

//    try {inimg1d_ch1 = new unsigned char [pagesz];}
//    catch(...)  {v3d_msg("cannot allocate memory for inimg1d_ch1.",0); return false;}

//    for(V3DLONG i = 0; i<pagesz; i++)
//        inimg1d_ch1[i] = data1d[channel*pagesz+i];
//    delete [] data1d; data1d = 0;

//    fastmarching_dt(inimg1d_ch1, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh, z_thickness);

//    float min_val = phi[0], max_val = phi[0];
//    V3DLONG tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
//    out1dData = new unsigned char[tol_sz];

//    for(V3DLONG i = 0; i < tol_sz; i++)
//    {
//        if(phi[i] == INF) continue;
//        min_val = MIN(min_val, phi[i]);
//        max_val = MAX(max_val, phi[i]);
//    }
//    cout<<"min_val = "<<min_val<<" max_val = "<<max_val<<endl;
//    //v3d_msg("checkout2!");

//    max_val -= min_val;
//    if(max_val == 0.0)
//        max_val = 0.00001;

//    for(V3DLONG i = 0; i < tol_sz; i++)
//    {
//        if(phi[i] == INF)
//            out1dData[i] = 0;
//        else if(phi[i] ==0)
//            out1dData[i] = 0;
//        else
//        {
//            out1dData[i] = (phi[i] - min_val)/max_val * 255 + 0.5;
//            out1dData[i] = MAX(out1dData[i], 1);
//        }
//    }
//    in_sz[3]=1;

//    QString binary_gsdtImage = PARA.inimg_file + "_binary_gsdt.v3draw";
//    simple_saveimage_wrapper(callback,binary_gsdtImage.toStdString().c_str(), out1dData, in_sz, 1);

//    delete [] phi; phi = 0;
//    delete [] inimg1d_ch1; inimg1d_ch1 = 0;
//    delete [] out1dData; out1dData = 0;
//    return true;
//}


////bool export_TXT(vector<Coordinate> &vec_coord,QString fileSaveName)
////{
////    QFile file(fileSaveName);
////    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
////        return false;
////    QTextStream myfile(&file);

////    myfile<<"x       y       z       gray"<<endl;
////    Coordinate * p_pt=0;
////    for (int i=0;i<vec_coord.size(); i++)
////    {
////        //then save
////        p_pt = (Coordinate *)(&(vec_coord.at(i)));
////        myfile << p_pt->x<<"       "<<p_pt->y<<"       "<<p_pt->z<<"       "<<p_pt->signal<<endl;
////    }

////    file.close();
////    cout<<"txt file "<<fileSaveName.toStdString()<<" has been generated, size: "<<vec_coord.size()<<endl;
////    return true;
////}

////bool writeTXT_file(const QString & filename, const QList <ImageMarker> & listMarker)
////{
////   QFile file(filename);
////    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
////        return false;
////    QTextStream myfile(&file);

////    myfile<<"x       y       z"<<endl;
////    ImageMarker * p_pt=0;
////    for (int i=0;i<listMarker.size(); i++)
////    {
////        //then save
////        p_pt = (ImageMarker *)(&(listMarker.at(i)));
////        myfile << p_pt->x<<"       "<<p_pt->y<<"       "<<p_pt->z<<"       "<<endl;

////    }

////    file.close();

////    return true;
////}
