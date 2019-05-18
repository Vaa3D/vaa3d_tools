#include "binary_connected.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"
#include <set>
#include <stack>

#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define MHDIS(a,b) ( (fabs((a).x-(b).x)) + (fabs((a).y-(b).y)) + (fabs((a).z-(b).z)) )

uint qHash(const Coordinate key)
{
    return key.x + key.y + key.z;
}
uint qHash(const vector<Coordinate> key)
{
    return key[0].x + key[0].y + key[0].z;
}

bool binary_connected(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return false;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return false;
    }

    cout<<"**********choose threshold.**********"<<endl;
    unsigned char* data1d = p4DImage->getRawData();

    V3DLONG c=1;
    V3DLONG in_sz[4];
    in_sz[0]=p4DImage->getXDim();
    in_sz[1]=p4DImage->getYDim();
    in_sz[2]=p4DImage->getZDim();
    in_sz[3]=p4DImage->getCDim();

    double imgAve, imgStd, bkg_thresh;
    mean_and_std(p4DImage->getRawData(), p4DImage->getTotalUnitNumberPerChannel(), imgAve, imgStd);

    //double td= (imgStd<15)? 15: imgStd;
    //bkg_thresh = imgAve +0.8*td;
    //bkg_thresh = imgAve + 10;

    double td= (imgStd<10)? 10: imgStd;
    bkg_thresh = imgAve +0.7*td;

    QString avgAndstd=QString("avg:%1  std:%2").arg(imgAve).arg(imgStd);
    QString threshold=QString("threshold:%1").arg(bkg_thresh);
    qDebug() << "avgAndstd = "<<avgAndstd<<endl;
    qDebug() << "bkg_thresh = "<<bkg_thresh<<endl;
    v3d_msg("checkout");

    cout<<"**********make binary image.**********"<<endl;
    V3DLONG M = in_sz[0];
    V3DLONG N = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG C = in_sz[3];
    V3DLONG pagesz = M*N;
    V3DLONG tol_sz = pagesz * P;

    unsigned char *im_cropped = 0;
    try {im_cropped = new unsigned char [tol_sz];}
     catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

    for(V3DLONG i=0;i<tol_sz;i++)
    {
        if(double(data1d[i]) <  bkg_thresh)
        {
            im_cropped[i] = 0;
        }
        else
        {
            im_cropped[i] = 255;
        }
    }

    PARA.inimg_file = p4DImage->getFileName();
    QString binaryImage = PARA.inimg_file + "_binary.v3draw";
    if(!simple_saveimage_wrapper(callback,binaryImage.toStdString().c_str(),im_cropped,in_sz,1))
    {
        cerr<<"save image "<<binaryImage.toStdString()<<" error!"<<endl;
        return false;
    }
    v3d_msg("binary done");

    cout<<"**********creat gray(.txt) and coordinate(.marker) file**********"<<endl;
     QList<ImageMarker> marker;
     vector<float> signal_list;
     vector<ForefroundCoordinate> vec_all;
     ImageMarker * p_pt=0;
     V3DLONG signal_loc = 0;

     for(V3DLONG i = 0; i < tol_sz; i++)
     {
         if(data1d[i] >= bkg_thresh)
         {
             signal_loc = i;
             ImageMarker signal_marker(signal_loc % M, signal_loc % pagesz / M, signal_loc / pagesz);
             signal_list.push_back(data1d[i]);
             marker.push_back(signal_marker);
         }

     }

     for (int i=0;i<marker.size(); i++)
     {
         ForefroundCoordinate coordinate;
         p_pt = (ImageMarker *)(&(marker.at(i)));
         coordinate.x = p_pt->x;
         coordinate.y = p_pt->y;
         coordinate.z = p_pt->z;
         coordinate.signal = signal_list[i];
         vec_all.push_back(coordinate);
     }

     cout<<endl<<"marker = "<<marker.size()<<endl;
     cout<<"signal_list = "<<signal_list.size()<<endl;
     cout<<"vec_all = "<<vec_all.size()<<endl;

     export_TXT(vec_all,QString(p4DImage->getFileName()).append("_forefroundCoordinate.txt"));
     //writeTXT_file(QString(p4DImage->getFileName()).append("_coordinate.txt"),marker);
     //writeMarker_file(QString(p4DImage->getFileName()).append(".marker"),marker);

    cout<<"**********make connected region for binary image.**********"<<endl;

    //step1:get coordinata3D of binary image
     bool first_floor = false;
     vector<vector<vector<V3DLONG> > > coordinate3D,mark3D,bri3D;
     vector<vector<V3DLONG> > coodinate2D,mark2D,bri2D;
     vector<V3DLONG> coodinate1D,mark1D,bri1D;
     for(V3DLONG iz = 0; iz < P; iz++)
     {
         V3DLONG offsetk = iz*M*N;
         for(V3DLONG iy = 0; iy < N; iy++)
         {
             V3DLONG offsetj = iy*N;
             for(V3DLONG ix = 0; ix < M; ix++)
             {
                 V3DLONG tmp = im_cropped[offsetk + offsetj + ix];
                 V3DLONG bri = int(data1d[offsetk + offsetj + ix]);
                 if(tmp==255)
                 {
                     int one=1;
                     mark1D.push_back(one);
                 }
                 else
                 {
                     int zero=0;
                     mark1D.push_back(zero);
                 }
                 coodinate1D.push_back(tmp);
                 bri1D.push_back(bri);
             }
             coodinate2D.push_back(coodinate1D);
             mark2D.push_back(mark1D);
             bri2D.push_back(bri1D);
             coodinate1D.clear();
             mark1D.clear();
             bri1D.clear();
         }
         coordinate3D.push_back(coodinate2D);
         mark3D.push_back(mark2D);
         bri3D.push_back(bri2D);
         coodinate2D.clear();
         mark2D.clear();
         bri2D.clear();
     }
    cout<<"coordinate3D = "<<coordinate3D.size()<<endl;

    //step2:make connected region
    vector<vector<Coordinate> >connected_region;
    QHash<Coordinate,int> map_index;
    vector<Coordinate> each_region1, each_region2;
    QList<ImageMarker> marker_all;
    double connected_thres = 1.5;
    int j = 1,u = 0,g = 0;
    int black1 = 0,black2 = 0,white = 0;
    int unknow = 0;
    int all = 0;
    for(int iz = 0; iz < P; iz++)
    {
        for(int iy = 0; iy < N; iy++)
        {
            for(int ix = 0; ix < M; ix++)
            {
                all++;
                Coordinate coord_curr;
                if(coordinate3D[iz][iy][ix]==255)
                {
                    if(ix==1)
                    {
                        coord_curr.x = ix;
                        coord_curr.y = iy;
                        coord_curr.z = iz;
                        coord_curr.bri = bri3D[iz][iy][ix];
                        each_region1.push_back(coord_curr);
                        connected_region.push_back(each_region1);
                        map_index.insert(coord_curr,j);
                        each_region1.clear();
                        j++;
                        g++;
                        first_floor = true;
                        unknow++;
                        continue;
                    }
                }
                else
                {
                    black1++;
                    continue;
                }
                if(ix!=1) //first_floor
                {
                    if(coordinate3D[iz][iy][ix]==255)
                    {
                        white++;
                        int dex;

                        bool ind1 = false;
                        coord_curr.x = ix;
                        coord_curr.y = iy;
                        coord_curr.z = iz;
                        coord_curr.bri = bri3D[iz][iy][ix];

                        ImageMarker m;
                        m.x = coord_curr.x;
                        m.y = coord_curr.y;
                        m.z = coord_curr.z;
                        m.radius = 0.5;
                        m.type = 2;
                        marker_all.push_back(m);

                        for(int l=0;l<connected_region.size();l++)
                        {
                            for(int k=0;k<connected_region[l].size();k++)
                            {
                                if(if_is_connect(coord_curr,connected_region[l][k],mark3D,connected_thres))
                                {
                                    dex = l;
                                    ind1 = true;
                                    break;
                                }
                            }
                            if(ind1)
                            {
                                break;
                            }

                        }
                        if(ind1)
                        {
                            connected_region[dex].push_back(coord_curr);
                            map_index.insert(coord_curr,map_index[connected_region[dex][0]]);
                            ind1 = false;
                            //break;
                        }
                        else
                        {
                            each_region2.clear();
                            each_region2.push_back(coord_curr);
                            u++;
                            connected_region.push_back(each_region2);
                            map_index.insert(coord_curr,j);
                            j++;

                        }
                        each_region2.clear();

                    }
                    else
                    {
                        black2++;
                    }

                }
            }
        }

    }

    cout<<"marker_all = "<<marker_all.size()<<endl;
    cout<<"connected_region.size() = "<<connected_region.size()<<endl;
    //ouput connected region
    for(V3DLONG i = 0;i < connected_region.size();i++)
    {
        QList<ImageMarker> region_marker;
        for(V3DLONG j = 0;j < connected_region[i].size();j++)
        {
            ImageMarker m;
            m.x = connected_region[i][j].x;
            m.y = connected_region[i][j].y;
            m.z = connected_region[i][j].z;
            m.radius = 0;
            m.type = 274;
            region_marker.push_back(m);

        }
        //writeMarker_file(QString(p4DImage->getFileName()+QString::number(i)+"_connected_region.marker"),region_marker);  //each region .marker
       // writeTXT_file(QString(p4DImage->getFileName()+QString::number(i)+"_connected_region.txt"),region_marker);
    }
    //v3d_msg("connected_region!");

    //step3:make max connected region   --make distance matrix
    //vector<vector<int> > dist_matrix=vector<vector<int> >(n,vector<int>());
    //vector< vector<int> > b(10, vector<int>(5));        //创建一个10*5的int型二维向量
    vector<vector<double> > origin_dist_matrix;
    double dis;
    for(int i=0;i<connected_region.size();i++)
    {
        vector<double> v;
        for(int j=0;j<connected_region.size();j++)
        {
            v.push_back(0);
        }
        origin_dist_matrix.push_back(v);
    }

    for(V3DLONG i = 0;i < connected_region.size();i++)
    {
        for(V3DLONG j = i+1;j< connected_region.size();j++)
        {
            double min_dis = 1000000000;
            for(V3DLONG k = 0;k < connected_region[i].size();k++)
            {
                for(V3DLONG l = 0;l < connected_region[j].size();l++)
                {
                    dis = NTDIS(connected_region[i][k],connected_region[j][l]);
                    if(dis < min_dis)
                         min_dis = dis;
                }
                //cout<<"min_dis = "<<min_dis<<endl;
            }
            origin_dist_matrix[i][j] = min_dis;
        }
    }
//    cout<<"origin distance matrix:"<<endl;
//    for(int i = 0;i <connected_region.size();i++)
//    {
//        for(int j = 0;j<connected_region.size();j++)
//        {
//            cout<<origin_dist_matrix[i][j]<<"\t";
//        }
//        cout<<endl;
//    }
    //v3d_msg("show origin distance");

    //create origin distance matrix.txt file
//    QString filename = PARA.inimg_file + "_distance.txt";
//    QFile data(filename);

//    if(!data.open(QFile::WriteOnly | QIODevice::Text))
//    {
//        //return -1;
//        QMessageBox::information(0, "", "create .txt file failed");
//        //return;
//    }

//    QTextStream out(&data);
//    for(int i = 0;i <connected_region.size();i++)
//    {
//        for(int j = 0;j<connected_region.size();j++)
//        {
//            out<<origin_dist_matrix[i][j]<<" ";
//        }
//        out<<"\n";
//        cout<<endl;
//    }
//    data.close();

    vector<vector<int> > dist_matrix;
    for(int i=0;i<connected_region.size();i++)
    {
        vector<int> v;
        for(int j=0;j<connected_region.size();j++)
        {
            v.push_back(0);
        }
        dist_matrix.push_back(v);
    }

    for(int i = 0;i < connected_region.size();i++)
    {
        for(int j = 0;j< connected_region.size();j++)
        {
            double min_dis = 1000000000;
            for(V3DLONG k = 0;k < connected_region[i].size();k++)
            {
                for(V3DLONG l = 0;l < connected_region[j].size();l++)
                {
                    dis = NTDIS(connected_region[i][k],connected_region[j][l]);
                    if(dis < min_dis)
                         min_dis = dis;
                }

            }
            if(min_dis <= connected_thres)
            {
                dist_matrix[i][j] = 1;
            }
        }
    }

//    cout<<"distance matrix:"<<endl;
//    for(int i = 0;i <connected_region.size();i++)
//    {
//        for(int j = 0;j<connected_region.size();j++)
//        {
//            cout<<dist_matrix[i][j]<<"\t";
//        }
//        cout<<endl;
//    }
    //v3d_msg("show distance matrix");

//    QHash<int,set<int> > vec;
//    int number = 0;
//    for(int i=0;i<connected_region.size();i++)
//    {
//        for(int j=0;j<connected_region.size();j++)
//        {
//            vec[i].insert(dist_matrix[i][j]);
//        }
//        cout<<"vec "<<i<<" = "<<vec[i].size()<<"  ";
//        for(set<int>::iterator it=vec[i].begin();it != vec[i].end();++it)
//        {
//            cout<<*it<<" ";
//        }
//        cout<<endl;
//        if(vec[i].size()==1)
//            number++;
//    }
//    cout<<"number = "<<number<<endl;
    //v3d_msg("number");

    //boost graph libiary
    Graph G;
    for(int i = 0;i <connected_region.size();i++)
    {
        for(int j = 0;j<connected_region.size();j++)
        {
            if(dist_matrix[i][j] == 1)
                 add_edge(i,j,G);
        }
    }

    std::vector<int> comp(num_vertices(G));
    int num = connected_components (G, comp.data());
    std::cout << "Total number of components: " << num << std::endl;

    std::cout << std::endl;
    std::vector < int >::iterator i;
    for (i = comp.begin(); i != comp.end(); ++i)
        std::cout << "Vertex " << i - comp.begin()<< " is in component " << *i << std::endl;

    std::vector<Graph* > comps(num);
    for(size_t i=0;i<num;++i)
    {
        comps[i] = & G.create_subgraph();
    }

    for(size_t i=0;i<num_vertices(G);++i)
    {
        cout<<"add vetex "<<i<<"to sub graph "<<comp[i]<<endl;
        add_vertex(i, *comps[comp[i]]);
    }

    pair<vertex_iter, vertex_iter> vip;
    cout << "Vertices in G  = [ ";
    vip = vertices(G);

    for(vertex_iter vi = vip.first; vi != vip.second; ++vi)
    {
         cout << *vi << " ";
    }
    cout << "]" << endl;

    for(size_t i=0;i<num;i++)
    {
        cout << "Vertices (local) in comps[i]' = [ ";
        pair<vertex_iter, vertex_iter> lvip;
        lvip = vertices(*comps[i]);

        for(vertex_iter vi = lvip.first; vi != lvip.second; ++vi)
        {
            cout << (*comps[i]).local_to_global(*vi) << " ";
        }

        cout << "]" << endl;

    }

    vector<vector<Coordinate> >max_connected_region;
    QHash<int,set<int> > max_relate;
    for(size_t i=0;i<num;i++)
    {
        pair<vertex_iter, vertex_iter> lvip;
        lvip = vertices(*comps[i]);

        for(vertex_iter vi = lvip.first; vi != lvip.second; ++vi)
        {
            max_relate[i].insert((*comps[i]).local_to_global(*vi));
        }
        cout<<"max_relate "<<i<<" = "<<max_relate[i].size()<<"   ";
        for(set<int>::iterator it = max_relate[i].begin();it != max_relate[i].end();++it)
        {
            cout<<*it<<" ";
        }
        cout<<endl;

    }

    for(QHash<int,set<int> >::iterator it=max_relate.begin();it!=max_relate.end();++it)
    {
        vector<Coordinate> tmp;
        for(set<int>::iterator it2=it.value().begin();it2!=it.value().end();++it2)
        {
            for(int i=0;i<connected_region[*it2].size();i++)
            {
                tmp.push_back(connected_region[*it2][i]);
            }

        }
        max_connected_region.push_back(tmp);
    }

    cout<<"max_connected_region = "<<max_connected_region.size()<<endl;

    for(V3DLONG i=0;i<max_connected_region.size();i++)
    {
        QList<ImageMarker> connected_region_marker;
        for(V3DLONG j=0;j<max_connected_region[i].size();j++)
        {
            ImageMarker m;
            m.x = max_connected_region[i][j].x;
            m.y = max_connected_region[i][j].y;
            m.z = max_connected_region[i][j].z;
            m.radius = 0.0;
            m.type = 274;
            connected_region_marker.push_back(m);

        }
        //writeMarker_file(QString(p4DImage->getFileName()+QString::number(i)+"_max_connected_region.marker"),connected_region_marker);
    }
    //v3d_msg("max_connected_region!");

    //step4:final connected region: remove noise interference is based on point number
    vector<vector<Coordinate> >final_connected_region;
    for(V3DLONG i =0;i< max_connected_region.size();i++)
    {
        if(max_connected_region[i].size() > 50)//this threshold need changes
            final_connected_region.push_back(max_connected_region[i]);
    }
    cout<<"final_connected_region = "<<final_connected_region.size()<<endl;

    for(V3DLONG i=0;i<final_connected_region.size();i++)
    {
        QList<ImageMarker> final_region_marker;
        for(V3DLONG j=0;j<final_connected_region[i].size();j++)
        {
            ImageMarker m;
            m.x = final_connected_region[i][j].x;
            m.y = final_connected_region[i][j].y;
            m.z = final_connected_region[i][j].z;
            m.radius = 0.0;
            m.type = 274;
            final_region_marker.push_back(m);

        }
        writeMarker_file(QString(p4DImage->getFileName()+QString::number(i)+"final_connected_region.marker"),final_region_marker);

    }

    cout<<"********make connected region finised********"<<endl;

    QString final_connected_region_number=QString("final_connected_region:%1").arg(final_connected_region.size());
    v3d_msg(final_connected_region_number);

    //get each region and all foreground .marker and .swc file
    QList<ImageMarker> marker_list;
    for(V3DLONG i = 0;i < final_connected_region.size();i++)
    {
        QList<ImageMarker> each_marker;
        for(V3DLONG j = 0;j < final_connected_region[i].size();j++)
        {
            ImageMarker m;
            m.x = final_connected_region[i][j].x;
            m.y = final_connected_region[i][j].y;
            m.z = final_connected_region[i][j].z;
            m.radius = 0;
            m.type = 274;
            each_marker.push_back(m);
            marker_list.push_back(m);

        }
        //writeMarker_file(QString(p4DImage->getFileName()+QString::number(i)+".marker"),each_marker);  //each region .marker
    }
    //writeMarker_file(QString(p4DImage->getFileName()).append("_foreground.marker"),marker_list); //all foreground .marker
    cout<<"marker_list = "<<marker_list.size()<<endl;

    QList<NeuronSWC> neuron_list = marker_to_swc(marker_list);
    NeuronTree nt_list;
    nt_list.listNeuron = neuron_list;
    //writeSWC_file(QString(p4DImage->getFileName()).append("_foreground.swc"),nt_list); //all foreground .swc
    cout<<"nt_list = "<<nt_list.listNeuron.size()<<endl;

    cout<<"**********make each connected region skeleton**********"<<endl;
    // input skeleton .marker and auto .swc file
    QString fileskeleton,fileAuto;
    fileskeleton = QFileDialog::getOpenFileName(0, QObject::tr("Open skeleton marker File"),
                                                "",

                                                QObject::tr("Supported file (*.marker)"

                                            ));

    fileAuto = QFileDialog::getOpenFileName(0, QObject::tr("Open auto swc File"),
                                                "",

                                                QObject::tr("Supported file (*.swc *.SWC *.eswc)"

                                            ));

    QList <ImageMarker> file_inmarkers;
    file_inmarkers = readMarker_file(fileskeleton);
    cout<<"file_inmarkers = "<<file_inmarkers.size()<<endl;

    QList<QList<NeuronSWC> > all_skeleton_swc;
    for(V3DLONG j = 0;j < final_connected_region.size();j++)
    {
        QList<ImageMarker> skeleton_marker,each_markers;
        for(V3DLONG k = 0;k < final_connected_region[j].size();k++)
        {
            ImageMarker m1;
            m1.x = final_connected_region[j][k].x;
            m1.y = final_connected_region[j][k].y;
            m1.z = final_connected_region[j][k].z;
            m1.radius = 0;
            m1.type = 274;
            each_markers.push_back(m1);
        }

        for(V3DLONG i = 0;i < file_inmarkers.size();i++)
        {
            ImageMarker m2;
            m2.x = file_inmarkers[i].x;
            m2.y = file_inmarkers[i].y;
            m2.z = file_inmarkers[i].z;
            m2.radius = 0;
            m2.type = 274;

            for(V3DLONG l = 0;l < each_markers.size();l++)
            {
                ImageMarker m3;
                m3.x = each_markers[l].x;
                m3.y = each_markers[l].y;
                m3.z = each_markers[l].z;
                m3.radius = 0;
                m3.type = 274;

                if(m2.x == m3.x && m2.y == m3.y && m2.z == m3.z)
                    skeleton_marker.push_back(m2);
            }

        }

        QString filenameSkeletonmarker = p4DImage->getFileName()+QString::number(j)+"_skeleton.marker";
        //writeMarker_file(filenameSkeletonmarker,skeleton_marker);   //each skeleton .marker

        QList<ImageMarker> each_skeleton_marker;
        //each_skeleton_marker = readMarker_file(filenameSkeletonmarker);
        each_skeleton_marker = skeleton_marker;
        cout<<"each_skeleton_marker = "<<each_skeleton_marker.size()<<endl;

        QList<NeuronSWC> skeleton_swc = marker_to_swc(each_skeleton_marker);
        NeuronTree skeleton_nt;
        skeleton_nt.listNeuron = skeleton_swc;
        //writeSWC_file(QString(p4DImage->getFileName()+QString::number(j)).append("_skeleton.swc"),skeleton_nt);   //each skeleton .swc

        all_skeleton_swc.push_back(skeleton_swc);
    }
    cout<<"all_skeleton_swc = "<<all_skeleton_swc.size()<<endl;
    //v3d_msg("get skeleton swc");

    cout<<"**********make final swc**********"<<endl;
    NeuronTree nt,nt_skeleton,nt_skeleton1,nt_skeleton2,nt_swc1,nt_swc2,nt_swc3,nt_final_temp,nt_final;
    QList<NeuronSWC> choose_skeleton,choose_skeleton1,choose_skeleton2,choose_swc1,choose_swc2,swc_eachregion,swc_final_temp,swc_final;
    double thres,thres1,thres2;
    for(V3DLONG i = 0;i < all_skeleton_swc.size();i++)
    {
        for(V3DLONG j = i+1; j < all_skeleton_swc.size();j++)
        {
            //step1:select the nearest point on each skeleton of connected region
            thres = choose_distance(all_skeleton_swc[i],all_skeleton_swc[j]);
            cout<<"thres = "<<thres<<endl;
            thres = (thres > 20)? 20 : thres;
            thres1 = thres; thres2 = 1.2*thres;

            choose_point(all_skeleton_swc[i],all_skeleton_swc[j],choose_skeleton,choose_skeleton1,choose_skeleton2,thres1,thres2);  //thres need changes
            int count = 0;
            while(choose_skeleton1.size() < 8 || choose_skeleton2.size() < 8)
            {
                thres2 = thres2 + 0.1*thres2;
                choose_point(all_skeleton_swc[i],all_skeleton_swc[j],choose_skeleton,choose_skeleton1,choose_skeleton2,thres1,thres2);
                count++;
                if(count >= 5)
                    break;
            }

            cout<<"choose_skeleton = "<<choose_skeleton.size()<<"   choose_skeleton1 = "<<choose_skeleton1.size()<<"   choose_skeleton2 = "<<choose_skeleton2.size()<<endl;
            if(choose_skeleton.size() == 0)  continue;
            if(choose_skeleton1.size() == 0)  continue;
            if(choose_skeleton2.size() == 0)  continue;

            nt_skeleton.listNeuron = choose_skeleton;
            nt_skeleton1.listNeuron = choose_skeleton1;
            nt_skeleton2.listNeuron = choose_skeleton2;
            QString fileChooseSkeleton = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_chooseSkeletonPoint.swc";
            writeSWC_file(fileChooseSkeleton,nt_skeleton);

            QString fileChooseSkeleton1 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_chooseSkeletonPoint1.swc";
            QString fileChooseSkeleton2 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_chooseSkeletonPoint2.swc";
            writeSWC_file(fileChooseSkeleton1,nt_skeleton1);
            writeSWC_file(fileChooseSkeleton2,nt_skeleton2);
            //v3d_msg("choose skeleton finised");

            //step2:select the nearest point on auto_swc is based on each_skeleton
            nt = readSWC_file(fileAuto);
            double dist_thres = 1.75,dist_thres1;
            dist_thres1 = dist_thres;
            choose_swc1 = choose_point2(nt.listNeuron,choose_skeleton1,0.0,dist_thres1);
            int count2 = 0;
            while(choose_swc1.size() < 8)
            {
                dist_thres1 = dist_thres1 + 0.2*dist_thres1;
                choose_swc1 = choose_point2(nt.listNeuron,choose_skeleton1,0.0,dist_thres1);
                count2++;
                if(count2 >= 2)
                    break;

            }
            cout<<"choose_swc1 = "<<choose_swc1.size()<<endl;
            if(choose_swc1.size() == 0)   continue;

            choose_swc2 = choose_point2(nt.listNeuron,choose_skeleton2,0.0,dist_thres1);
            int count3 = 0;
            while(choose_swc2.size() < 8)
            {
                dist_thres1 = dist_thres1 + 0.2*dist_thres1;
                choose_swc2 = choose_point2(nt.listNeuron,choose_skeleton2,0.0,dist_thres1);
                count3++;
                if(count3 >= 2)
                    break;

            }
            cout<<"choose_swc2 = "<<choose_swc2.size()<<endl;
            if(choose_swc2.size() == 0)   continue;

            //method1:choose_swc3
//            QString fileAvgSwc1 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_swcAvgPoint1.swc";
//            QString fileAvgSwc2 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_swcAvgPoint2.swc";
//            Average avg1 = get_average(choose_swc1,fileAvgSwc1);
//            cout<<"avg1.x = "<<avg1.x<<"   avg1.y = "<<avg1.y<<"   avg1.z = "<<avg1.z<<endl;

//            Average avg2 = get_average(choose_swc2,fileAvgSwc2);
//            cout<<"avg2.x = "<<avg2.x<<"   avg2.y = "<<avg2.y<<"   avg2.z = "<<avg2.z<<endl;

//            QList<NeuronSWC> choose_swc3;
//            for(V3DLONG k =0;k < nt.listNeuron.size();k++)
//            {
//                avg1.x = min(avg1.x,avg2.x); avg2.x = max(avg1.x,avg2.x);
//                avg1.y = min(avg1.y,avg2.y); avg2.y = max(avg1.y,avg2.y);
//                avg1.z = min(avg1.z,avg2.z); avg2.z = max(avg1.z,avg2.z);

//                if((nt.listNeuron[k].x>avg1.x && nt.listNeuron[k].x<avg2.x) && (nt.listNeuron[k].y>avg1.y && nt.listNeuron[k].y<avg2.y) && (nt.listNeuron[k].z>avg1.z && nt.listNeuron[k].z<avg2.z))
//                    choose_swc3.push_back(nt.listNeuron[k]);
//            }
//            cout<<"choose_swc3_tmp1 = "<<choose_swc3.size()<<endl;
//            if(choose_swc3.size() ==0)
//            {
//                for(V3DLONG k =0;k < nt.listNeuron.size();k++)
//                {
//                    avg1.x = min(avg1.x,avg2.x); avg2.x = max(avg1.x,avg2.x);
//                    avg1.y = min(avg1.y,avg2.y); avg2.y = max(avg1.y,avg2.y);

//                    if((nt.listNeuron[k].x>avg1.x && nt.listNeuron[k].x<avg2.x) && (nt.listNeuron[k].y>avg1.y && nt.listNeuron[k].y<avg2.y))
//                        choose_swc3.push_back(nt.listNeuron[k]);
//                }

//            }
//            cout<<"choose_swc3_tmp2 = "<<choose_swc3.size()<<endl;
//            if(choose_swc3.size() ==0)
//            {
//                for(V3DLONG k =0;k < nt.listNeuron.size();k++)
//                {
//                    avg1.x = min(avg1.x,avg2.x); avg2.x = max(avg1.x,avg2.x);

//                    if((nt.listNeuron[k].x>avg1.x && nt.listNeuron[k].x<avg2.x))
//                        choose_swc3.push_back(nt.listNeuron[k]);
//                }

//            }
//            cout<<"choose_swc3 = "<<choose_swc3.size()<<endl;
//            //if(choose_swc3.size() == 0)   continue;

            //method2:choose_swc3
            QList<NeuronSWC> choose_swc3_tmp,choose_swc3,skeleton_swc_all;
            skeleton_swc_all = marker_to_swc(file_inmarkers);
            choose_swc3_tmp = choose_point3(nt.listNeuron,skeleton_swc_all,2.0);
            cout<<"choose_swc3_tmp = "<<choose_swc3_tmp.size()<<endl;
            NeuronTree nt_swc3_tmp;
            nt_swc3_tmp.listNeuron = choose_swc3_tmp;
            QString fileChooseSwc3tmp = p4DImage->getFileName()+QString::number(i)+QString::number(j) + "_chooseSwcPoint3_tmp.swc";
            writeSWC_file(fileChooseSwc3tmp,nt_swc3_tmp);

            QString fileAvgSwc1 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_swcAvgPoint1.swc";
            QString fileAvgSwc2 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_swcAvgPoint2.swc";
            Average avg1 = get_average(choose_swc1,fileAvgSwc1);
            cout<<"avg1.x = "<<avg1.x<<"   avg1.y = "<<avg1.y<<"   avg1.z = "<<avg1.z<<endl;

            Average avg2 = get_average(choose_swc2,fileAvgSwc2);
            cout<<"avg2.x = "<<avg2.x<<"   avg2.y = "<<avg2.y<<"   avg2.z = "<<avg2.z<<endl;

            avg1.x = min(avg1.x,avg2.x); avg2.x = max(avg1.x,avg2.x);
            avg1.y = min(avg1.y,avg2.y); avg2.y = max(avg1.y,avg2.y);
            avg1.z = min(avg1.z,avg2.z); avg2.z = max(avg1.z,avg2.z);
            cout<<"avg1.x = "<<avg1.x<<"   avg2.x = "<<avg2.x<<endl;
            cout<<"avg1.y = "<<avg1.y<<"   avg2.y = "<<avg2.y<<endl;
            cout<<"avg1.z = "<<avg1.z<<"   avg2.z = "<<avg2.z<<endl;

            for(V3DLONG k =0;k < choose_swc3_tmp.size();k++)
            {
                if((choose_swc3_tmp[k].x>avg1.x && choose_swc3_tmp[k].x<avg2.x)||(choose_swc3_tmp[k].y>avg1.y && choose_swc3_tmp[k].y<avg2.y)||(choose_swc3_tmp[k].z>avg1.z && choose_swc3_tmp[k].z<avg2.z))
                    choose_swc3.push_back(choose_swc3_tmp[k]);
            }
            cout<<"choose_swc3 = "<<choose_swc3.size()<<endl;
            if(choose_swc3.size() == 0)
            {
                for(V3DLONG k =0;k < choose_swc3_tmp.size();k++)
                {
                    if((choose_swc3_tmp[k].x>avg1.x || choose_swc3_tmp[k].x<avg2.x))
                        choose_swc3.push_back(choose_swc3_tmp[k]);
                }
            }
            cout<<"choose_swc3 = "<<choose_swc3.size()<<endl;
            if(choose_swc3.size() == 0) continue;

            nt_swc1.listNeuron = choose_swc1;
            nt_swc2.listNeuron = choose_swc2;
            nt_swc3.listNeuron = choose_swc3;
            QString fileChooseSwc1 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_chooseSwcPoint1.swc";
            QString fileChooseSwc2 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_chooseSwcPoint2.swc";
            QString fileChooseSwc3 = p4DImage->getFileName()+QString::number(i)+QString::number(j) + "_chooseSwcPoint3.swc";
            writeSWC_file(fileChooseSwc1,nt_swc1);
            writeSWC_file(fileChooseSwc2,nt_swc2);
            writeSWC_file(fileChooseSwc3,nt_swc3);
            v3d_msg("choose swc finised");

            //step3:select three average point
            QString fileAvgSwc3 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_swcAvgPoint3.swc";
            QString fileAvgSwc31 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_swcAvgPoint31.swc";
            QString fileAvgSwc32 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_swcAvgPoint32.swc";
            QString fileAvgSwc33 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "_swcAvgPoint33.swc";

            Average avg3 = get_average(choose_swc3,fileAvgSwc3);
            cout<<"avg3.x = "<<avg3.x<<"   avg3.y = "<<avg3.y<<"   avg3.z = "<<avg3.z<<endl;

            Average avg31 = get_average1(choose_swc3,fileAvgSwc31);
            cout<<"avg31.x = "<<avg31.x<<"   avg31.y = "<<avg31.y<<"   avg31.z = "<<avg31.z<<endl;

            Average avg32 = get_average2(choose_swc3,fileAvgSwc32);
            cout<<"avg32.x = "<<avg32.x<<"   avg32.y = "<<avg32.y<<"   avg32.z = "<<avg32.z<<endl;

            Average avg33 = get_average3(choose_swc3,fileAvgSwc33);
            cout<<"avg33.x = "<<avg33.x<<"   avg33.y = "<<avg33.y<<"   avg33.z = "<<avg33.z<<endl;

            //step4:calculation angle
            double dis_a,dis_b,dis_b1,dis_b2,dis_b3,dis_c,dis_c1,dis_c2,dis_c3;
            double angle,angle1,angle2,angle3,final_angle;
            dis_a = NTDIS(avg1,avg2);
            dis_b = NTDIS(avg2,avg3);  dis_c = NTDIS(avg1,avg3);

            dis_b1 = NTDIS(avg2,avg31);  dis_c1 = NTDIS(avg1,avg31);

            dis_b2 = NTDIS(avg2,avg32);  dis_c2 = NTDIS(avg1,avg32);

            dis_b3 = NTDIS(avg2,avg33);  dis_c3 = NTDIS(avg1,avg33);

            cout<<"dis_a = "<<dis_a<<"   dis_b = "<<dis_b<<"   dis_c = "<<dis_c<<endl;
            cout<<"dis_a = "<<dis_a<<"   dis_b1 = "<<dis_b1<<"   dis_c1 = "<<dis_c1<<endl;
            cout<<"dis_a = "<<dis_a<<"   dis_b2 = "<<dis_b1<<"   dis_c2 = "<<dis_c1<<endl;
            cout<<"dis_a = "<<dis_a<<"   dis_b3 = "<<dis_b1<<"   dis_c3 = "<<dis_c1<<endl;

            angle = ((dis_b*dis_b)+(dis_c*dis_c)-(dis_a*dis_a))/(2*dis_b*dis_c);
            angle1 = ((dis_b1*dis_b1)+(dis_c1*dis_c1)-(dis_a*dis_a))/(2*dis_b1*dis_c1);
            angle1 = ((dis_b2*dis_b2)+(dis_c2*dis_c2)-(dis_a*dis_a))/(2*dis_b2*dis_c2);
            angle1 = ((dis_b3*dis_b3)+(dis_c3*dis_c3)-(dis_a*dis_a))/(2*dis_b3*dis_c3);

            cout<<"angle = "<<angle<<endl;
            cout<<"angle1 = "<<angle1<<endl;
            cout<<"angle2 = "<<angle2<<endl;
            cout<<"angle3 = "<<angle3<<endl;

            final_angle = max(max(angle,angle1),max(angle2,angle3));
            cout<<"final_angle = "<<final_angle<<endl;

            //step5:get each region final swc
//            //method1:
//            NeuronTree nt_eachregion1,nt_eachregion2;
//            QList<NeuronSWC> swc_eachregion1,swc_eachregion2;
//            if(final_angle > 0)
//            {
//                for(V3DLONG i = 0;i < nt.listNeuron.size();i++)
//                {
//                    if(nt.listNeuron[i].x > avg3.x || nt.listNeuron[i].y > avg3.y || nt.listNeuron[i].z > avg3.z)
//                    {
//                        swc_eachregion1.push_back(nt.listNeuron[i]);
//                    }
//                    else
//                        swc_eachregion2.push_back(nt.listNeuron[i]);
//                }
//            }
//            cout<<"nt = "<<nt.listNeuron.size()<<endl;
//            cout<<"swc_eachregion1 = "<<swc_eachregion1.size()<<"    2 = "<<swc_eachregion2.size()<<endl;
//            nt_eachregion1.listNeuron = swc_eachregion1;
//            nt_eachregion2.listNeuron = swc_eachregion2;
//            QString file_eachregion1 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "final_eachregion1.swc";
//            QString file_eachregion2 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "final_eachregion2.swc";
//            writeSWC_file(file_eachregion1,nt_eachregion1);
//            writeSWC_file(file_eachregion2,nt_eachregion2);

            //method2:
//            if(final_angle > 0)
//            {
//                for(V3DLONG i = 0;i < nt.listNeuron.size();i++)
//                {
//                    for(V3DLONG j = 0;j < choose_swc3.size();j++)
//                    {
//                        if(nt.listNeuron[i].x == choose_swc3[j].x && nt.listNeuron[i].y == choose_swc3[j].y && nt.listNeuron[i].z == choose_swc3[j].z)
//                            nt.listNeuron.removeAt(i);
//                    }
//                }
//            }
//            cout<<"nt = "<<nt.listNeuron.size()<<endl;
//            QString file_removeregion = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "remove_connected.swc";
//            writeSWC_file(file_removeregion,nt);


            //method3:
            NeuronTree nt_eachregion1,nt_eachregion2;
            QList<NeuronSWC> swc_eachregion1,swc_eachregion2;
            if(final_angle > 0)
            {
                swc_eachregion1 = choose_point4(nt.listNeuron,all_skeleton_swc[i],4.5);
                swc_eachregion2 = choose_point4(nt.listNeuron,all_skeleton_swc[j],4.5);
            }
            cout<<"nt = "<<nt.listNeuron.size()<<endl;
            cout<<"swc_eachregion1 = "<<swc_eachregion1.size()<<"    swc_eachregion2 = "<<swc_eachregion2.size()<<endl;
            nt_eachregion1.listNeuron = swc_eachregion1;
            nt_eachregion2.listNeuron = swc_eachregion2;
            QString file_eachregion1 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "final_eachregion1.swc";
            QString file_eachregion2 = p4DImage->getFileName()+QString::number(i)+QString::number(j)+ "final_eachregion2.swc";
            writeSWC_file(file_eachregion1,nt_eachregion1);
            writeSWC_file(file_eachregion2,nt_eachregion2);

            for(V3DLONG i = 0;i <swc_eachregion1.size();i++)
            {
                if(nt.listNeuron[1].x == swc_eachregion1[i].x && nt.listNeuron[1].y == swc_eachregion1[i].y && nt.listNeuron[1].z == swc_eachregion1[i].z)
                    swc_final_temp = swc_eachregion1;
            }
            for(V3DLONG i = 0;i <swc_eachregion2.size();i++)
            {
                if(nt.listNeuron[1].x == swc_eachregion2[i].x && nt.listNeuron[1].y == swc_eachregion2[i].y && nt.listNeuron[1].z == swc_eachregion2[i].z)
                    swc_final_temp = swc_eachregion2;
            }
            swc_final.append(swc_final_temp);
            cout<<"swc_final_temp = "<<swc_final_temp.size()<<endl;
            cout<<"swc_final = "<<swc_final.size()<<endl;
            nt_final_temp.listNeuron = swc_final_temp;
            QString file_final_temp = PARA.inimg_file + "_final_temp.swc";
            writeSWC_file(file_final_temp,nt_final_temp);
        }
        QList<NeuronSWC> parent_list;
        for(int l = 0;l < nt.listNeuron.size();l++)
        {
            if(nt.listNeuron[l].parent == -1)
                parent_list.append(nt.listNeuron[l]);
        }
        cout<<"parent_list = "<<parent_list.size()<<endl;

        if(parent_list.size() > 1)
        {
            swc_eachregion = choose_point4(nt.listNeuron,all_skeleton_swc[i],4.5);
            for(int k = 0;k < swc_eachregion.size();k++)
            {
                if(swc_eachregion[k].parent == -1)
                    swc_final.append(swc_eachregion);
            }
        }

    }
    cout<<"swc_final = "<<swc_final.size()<<endl;
    nt_final.listNeuron = swc_final;
    QString file_final = PARA.inimg_file + "_final.swc";
    writeSWC_file(file_final,nt_final);

    return true;
}

bool binary_connected(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA)
//bool binary_connected(V3DPluginCallback2 &callback,unsigned char* data1d,V3DLONG in_sz[4],V3DPluginArgList &output)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString inimage = infiles[0];
    cout<<"inimageName :"<<inimage.toStdString()<<endl;

    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int dataType;
    if(!simple_loadimage_wrapper(callback, inimage.toStdString().c_str(), data1d, in_sz, dataType))
    {
        cerr<<"load image "<<inimage.toStdString()<<" error!"<<endl;
        return false;
    }

    return true;
}

template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev)
{
    if (!data || n<=0)
      return false;

    int j;
    double ep=0.0,s,p;

    if (n <= 1)
    {
      //printf("len must be at least 2 in mean_and_std\n");
      ave = data[0];
      sdev = (T2)0;
      return true; //do nothing
    }

    s=0.0;
    for (j=0;j<n;j++) s += data[j];
    double ave_double=(T2)(s/n); //use ave_double for the best accuracy

    double var=0.0;
    for (j=0;j<n;j++) {
        s=data[j]-(ave_double);
        var += (p=s*s);
    }
    var=(var-ep*ep/n)/(n-1);
    sdev=(T2)(sqrt(var));
    ave=(T2)ave_double; //use ave_double for the best accuracy

    return true;
}


bool if_is_connect(Coordinate &curr,Coordinate &b,vector<vector<vector<V3DLONG> > > &mark3D,double connected_thres)
{

    double dist = NTDIS(curr,b);
    //cout<<"dist = "<<dist<<endl;
    if(dist<connected_thres)
    {
        return true;//this
        if(mark3D[curr.z][curr.y][curr.x]==1)
        {
            mark3D[curr.z][curr.y][curr.x]==0;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

int choose_distance(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2)
{
    vector<V3DLONG> vec_min;
    double dis,min_dis,thres;
    for(V3DLONG i=0;i<neuron1.size();i++)
    {
        min_dis = 10000000;
        for(V3DLONG j=0;j<neuron2.size();j++)
        {
            dis = MHDIS(neuron1[i],neuron2[j]);
            if(dis<min_dis)
            {
                min_dis = dis;
            }
        }
        //cout<<"min_dis = "<<min_dis<<endl;
        vec_min.push_back(min_dis);
    }
    for(V3DLONG i=0;i<neuron2.size();i++)
    {
        min_dis = 10000000;
        for(V3DLONG j=0;j<neuron1.size();j++)
        {
            dis = MHDIS(neuron2[i],neuron1[j]);
            if(dis<min_dis)
            {
                min_dis = dis;
            }
        }
        vec_min.push_back(min_dis);
    }
    float min=100000000;
    float max=-1;
    for(V3DLONG i = 0;i < vec_min.size();i++)
    {
        if(vec_min[i] > max)  max = vec_min[i];
        if(vec_min[i] < min)  min = vec_min[i];
    }
    thres = min;
    cout<<"max = "<<max<<"   min = "<<min<<endl;
    //cout<<"thres = "<<thres<<endl;
    //v3d_msg("thres");

    return thres;
}

void choose_point(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,QList<NeuronSWC> &choose,QList<NeuronSWC> &choose1,QList<NeuronSWC> &choose2,double thre1,double thre2)
{
    //QList<NeuronSWC> choose,choose1,choose2;
    choose.clear();
    choose1.clear();
    choose2.clear();
    double dis,min_dis;

    for(V3DLONG i=0;i<neuron1.size();i++)
    {
        min_dis = 10000000;
        for(V3DLONG j=0;j<neuron2.size();j++)
        {
            dis = MHDIS(neuron1[i],neuron2[j]);
            //cout<<"dis1 = "<<dis<<endl;
            if(dis<min_dis)
            {
                min_dis = dis;
            }
        }

        if((thre1<=min_dis)&&(min_dis<thre2))
        {
            choose1.push_back(neuron1[i]);
            choose.push_back(neuron1[i]);
        }
    }

    for(V3DLONG i=0;i<neuron2.size();i++)
    {
        min_dis = 10000000;
        for(V3DLONG j=0;j<neuron1.size();j++)
        {
            dis = MHDIS(neuron2[i],neuron1[j]);
            //cout<<"dis2 = "<<dis<<endl;
            if(dis<min_dis)
            {
                min_dis = dis;
            }
        }
        if((thre1<=min_dis)&&(min_dis<thre2))
        {
            choose2.push_back(neuron2[i]);
            choose.push_back(neuron2[i]);
        }
    }

    return;
}


QList<NeuronSWC> choose_point2(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,double thre1,double thre2)
{
    QList<NeuronSWC> choose;
    choose.clear();
    double min_dis;
    double dis;

    for(V3DLONG i=0;i<neuron1.size();i++)
    {
        min_dis = 10000000;
        for(V3DLONG j=0;j<neuron2.size();j++)
        {
            dis = MHDIS(neuron1[i],neuron2[j]);
            //cout<<"dis = "<<dis<<endl;
            if(dis<min_dis)
            {
                min_dis = dis;
            }
        }

        if((thre1<=min_dis)&&(min_dis<thre2))
        {
            choose.push_back(neuron1[i]);
        }
    }
    //v3d_msg("checkout dis");

    return choose;
}

QList<NeuronSWC> choose_point3(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,double thres)
{
    QList<NeuronSWC> choose;
    choose.clear();
    double min_dis;
    double dis;

    for(V3DLONG i=0;i<neuron1.size();i++)
    {
        min_dis = 10000000;
        for(V3DLONG j=0;j<neuron2.size();j++)
        {
            dis = MHDIS(neuron1[i],neuron2[j]);
            //cout<<"dis = "<<dis<<endl;
            if(dis<min_dis)
            {
                min_dis = dis;
            }
        }

        if(min_dis>=thres)
        {
            choose.push_back(neuron1[i]);
        }
    }
    //v3d_msg("checkout dis");

    return choose;
}

QList<NeuronSWC> choose_point4(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,double thres)
{
    QList<NeuronSWC> choose;
    choose.clear();
    double min_dis;
    double dis;

    for(V3DLONG i=0;i<neuron1.size();i++)
    {
        min_dis = 10000000;
        for(V3DLONG j=0;j<neuron2.size();j++)
        {
            dis = MHDIS(neuron1[i],neuron2[j]);
            //cout<<"dis = "<<dis<<endl;
            if(dis<min_dis)
            {
                min_dis = dis;
            }
        }

        if(min_dis<=thres)
        {
            choose.push_back(neuron1[i]);
        }
    }
    //v3d_msg("checkout dis");

    return choose;
}

QList<NeuronSWC> marker_to_swc(QList<ImageMarker> & inmarkers)
{
    QList<NeuronSWC> swc_list;
    int swc_id = 1;
    for(V3DLONG m = 0;m < inmarkers.size();m++)
    {
        NeuronSWC cur;
        cur.n = swc_id;
        cur.x = inmarkers[m].x;
        cur.y = inmarkers[m].y;
        cur.z = inmarkers[m].z;
        cur.radius = 1.0;
        cur.type = 274;
        if(m==0)
            cur.pn =-1;
        else
            cur.pn =swc_id-1;

        swc_list.push_back(cur);
        swc_id++;
    }
    return swc_list;
}

Average get_average(QList<NeuronSWC> &swc_list,QString filename)
{
    Average avg;
    double sum_x = 0,sum_y = 0,sum_z = 0;
    double avg_x,avg_y,avg_z;
    for(V3DLONG i = 0;i < swc_list.size();i++)
    {
        sum_x = sum_x + swc_list[i].x;
        sum_y = sum_y + swc_list[i].y;
        sum_z = sum_z + swc_list[i].z;
    }
    avg_x = sum_x/swc_list.size();
    avg_y = sum_y/swc_list.size();
    avg_z = sum_z/swc_list.size();
    avg.x = avg_x;
    avg.y = avg_y;
    avg.z = avg_z;

    QList<NeuronSWC> choose_avg;
    NeuronSWC cur;
    cur.x = avg.x;
    cur.y = avg.y;
    cur.z = avg.z;
    cur.n = 1;
    cur.radius =1.0;
    cur.type = 274;
    cur.parent = -1;
    choose_avg.push_back(cur);
    NeuronTree nt_avg;
    nt_avg.listNeuron = choose_avg;
    writeSWC_file(filename,nt_avg);

    return avg;
}

Average get_average1(QList<NeuronSWC> &swc_list,QString filename)
{
    Average avg;
    double sum_x = 0,sum_y = 0,sum_z = 0;
    double avg_x,avg_y,avg_z;
    for(V3DLONG i = 0;i < 0.5*swc_list.size();i++)
    {
        sum_x = sum_x + swc_list[i].x;
        sum_y = sum_y + swc_list[i].y;
        sum_z = sum_z + swc_list[i].z;
    }
    avg_x = sum_x/(0.5*swc_list.size());
    avg_y = sum_y/(0.5*swc_list.size());
    avg_z = sum_z/(0.5*swc_list.size());
    avg.x = avg_x;
    avg.y = avg_y;
    avg.z = avg_z;

    QList<NeuronSWC> choose_avg;
    NeuronSWC cur;
    cur.x = avg.x;
    cur.y = avg.y;
    cur.z = avg.z;
    cur.n = 1;
    cur.radius =1.0;
    cur.type = 274;
    cur.parent = -1;
    choose_avg.push_back(cur);
    NeuronTree nt_avg;
    nt_avg.listNeuron = choose_avg;
    writeSWC_file(filename,nt_avg);

    return avg;
}

Average get_average2(QList<NeuronSWC> &swc_list,QString filename)
{
    Average avg;
    double sum_x = 0,sum_y = 0,sum_z = 0;
    double avg_x,avg_y,avg_z;
    cout<<"swc_list1 = "<<swc_list.size()<<endl;
    for(V3DLONG i = 0;i < 0.5*swc_list.size();i++)
    {
        swc_list.removeAt(i);
    }
    cout<<"swc_list2 = "<<swc_list.size()<<endl;
    for(V3DLONG i = 0;i < swc_list.size();i++)
    {
        sum_x = sum_x + swc_list[i].x;
        sum_y = sum_y + swc_list[i].y;
        sum_z = sum_z + swc_list[i].z;
    }
    avg_x = sum_x/swc_list.size();
    avg_y = sum_y/swc_list.size();
    avg_z = sum_z/swc_list.size();
    avg.x = avg_x;
    avg.y = avg_y;
    avg.z = avg_z;

    QList<NeuronSWC> choose_avg;
    NeuronSWC cur;
    cur.x = avg.x;
    cur.y = avg.y;
    cur.z = avg.z;
    cur.n = 1;
    cur.radius =1.0;
    cur.type = 274;
    cur.parent = -1;
    choose_avg.push_back(cur);
    NeuronTree nt_avg;
    nt_avg.listNeuron = choose_avg;
    writeSWC_file(filename,nt_avg);

    return avg;
}

Average get_average3(QList<NeuronSWC> &swc_list,QString filename)
{
    Average avg;
    double sum_x = 0,sum_y = 0,sum_z = 0;
    double avg_x,avg_y,avg_z;
    cout<<"swc_list1 = "<<swc_list.size()<<endl;
    for(V3DLONG i = 0;i < 0.35*swc_list.size();i++)
    {
        swc_list.removeAt(i);
    }
    cout<<"swc_list2 = "<<swc_list.size()<<endl;
    for(V3DLONG j = swc_list.size()-1;j>0.7*swc_list.size();j--)
    {
        swc_list.removeAt(j);
    }
    cout<<"swc_list3 = "<<swc_list.size()<<endl;
    for(V3DLONG i = 0;i < swc_list.size();i++)
    {
        cout<<swc_list[i].x<<"  "<<swc_list[i].y<<"  "<<swc_list[i].z<<endl;
        sum_x = sum_x + swc_list[i].x;
        sum_y = sum_y + swc_list[i].y;
        sum_z = sum_z + swc_list[i].z;
    }
    avg_x = sum_x/swc_list.size();
    avg_y = sum_y/swc_list.size();
    avg_z = sum_z/swc_list.size();
    avg.x = avg_x;
    avg.y = avg_y;
    avg.z = avg_z;

    QList<NeuronSWC> choose_avg;
    NeuronSWC cur;
    cur.x = avg.x;
    cur.y = avg.y;
    cur.z = avg.z;
    cur.n = 1;
    cur.radius =1.0;
    cur.type = 274;
    cur.parent = -1;
    choose_avg.push_back(cur);
    NeuronTree nt_avg;
    nt_avg.listNeuron = choose_avg;
    writeSWC_file(filename,nt_avg);

    return avg;
}


int GetStack(vector<vector<int> > dist_matrix,int i,int n,stack<int> &st)
{
    for(int j = 0;j < n;j++)
    {
        if(dist_matrix[i][j] == 1)
            st.push(j);
    }
    return st.size();
}

int GetChildGrape(vector<vector<int> > dist_matrix,int n)
{
    stack<int> st;
    map<int,int> mapstor;
    int num = 0;
    for(int i = 0;i < n;i++)
    {
        if(mapstor.count(i) == 1)  continue;
        else
            mapstor[i] = 1;
        int result = GetStack(dist_matrix,i,n,st);
        if(result == 0)
            num++;
        else
        {
            while(st.size() != 0)
            {
                int tmp = st.top();
                mapstor[tmp] = 1;
                st.pop();
                GetStack(dist_matrix,tmp,n,st);
            }
            num++;
        }
    }
    return num;
}

bool export_TXT(vector<ForefroundCoordinate> &vec_coord,QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    //myfile<<"x       y       z       gray"<<endl;
    myfile<<"x\ty\tz\tgray"<<endl;
    ForefroundCoordinate * p_pt=0;
    for (int i=0;i<vec_coord.size(); i++)
    {
        //then save
        p_pt = (ForefroundCoordinate *)(&(vec_coord.at(i)));
        //myfile << p_pt->x<<"       "<<p_pt->y<<"       "<<p_pt->z<<"       "<<p_pt->signal<<endl;
        myfile << p_pt->x<<"\t"<<p_pt->y<<"\t"<<p_pt->z<<"\t"<<p_pt->signal<<endl;
    }

    file.close();
    cout<<"txt file "<<fileSaveName.toStdString()<<" has been generated, size: "<<vec_coord.size()<<endl;
    return true;
}

bool writeTXT_file(const QString & filename, const QList <ImageMarker> & listMarker)
{
   QFile file(filename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    myfile<<"x      y       z"<<endl;
    ImageMarker * p_pt=0;
    for (int i=0;i<listMarker.size(); i++)
    {
        //then save
        p_pt = (ImageMarker *)(&(listMarker.at(i)));
        myfile << p_pt->x<<"     "<<p_pt->y<<"     "<<p_pt->z<<"     "<<endl;

    }

    file.close();

    return true;
}













