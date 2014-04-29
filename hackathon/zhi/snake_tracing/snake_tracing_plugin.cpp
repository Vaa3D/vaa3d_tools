/* snake_tracing_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-04-01 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "snake_tracing_plugin.h"

#include "OpenSnakeTracer.h"
#include "TracingCore/ImageOperation.h"
#include "TracingCore/SnakeOperation.h"
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"

ImageOperation *IM;
OpenSnakeTracer *Tracer;

void autotrace(V3DPluginCallback2 &callback, QWidget *parent);
void findBranch_Raw(int snake_label, int root_label, Point3D root_point, vnl_vector<int>*snake_visit_label, int *point_id, QTextStream *out_txt, PointList3D *wrote_pt, PointList3D *all_pt, std::vector<int> *branch_label);


using namespace std;
Q_EXPORT_PLUGIN2(snake_tracing, snake_tracing);
 
QStringList snake_tracing::menulist() const
{
	return QStringList() 
        <<tr("trace")
		<<tr("about");
}

QStringList snake_tracing::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void snake_tracing::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace"))
	{
        autotrace(callback,parent);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by Zhi Zhou, 2014-04-01"));
	}
}

bool snake_tracing::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
    {
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void autotrace(V3DPluginCallback2 &callback, QWidget *parent)
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


    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    int in_sz[3];
    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;


    bool ok1;
    int c;

    if(sc==1)
    {
        c=1;
        ok1=true;
    }
    else
    {
        c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok1);
    }

    if(!ok1)
        return;

    IM = new ImageOperation;
    Tracer = new OpenSnakeTracer;

    IM->Imcreate(data1d,in_sz);
    //preprocessing
    std::cout<<"Compute Gradient Vector Flow..."<<std::endl;
    IM->computeGVF(1000,5,1);
    std::cout<<"Compute Vesselness (CPU)..."<<std::endl;
    IM->ComputeGVFVesselness();
    std::cout<<"Detect Seed Points..."<<std::endl;
    IM->SeedDetection(IM->v_threshold,0,0);
    std::cout<<"Adjust Seed Points..."<<std::endl;
    IM->SeedAdjustment(10);
    std::cout<<"Preprocessing Finished..."<<std::endl;

    IM->ImComputeInitBackgroundModel(IM->v_threshold);
    IM->ImComputeInitForegroundModel();

    //tracing
    std::cout<<"--------------Tracing--------------"<<std::endl;
    IM->ImRefresh_LabelImage();
    Tracer->SetImage(IM);
    Tracer->Init();


    float alpha = 0;
    int iter_num = 50;
    int ITER = 5;

    int pt_distance = 2;

    float beta = 0.05;
    float kappa = 1;
    float gamma = 1;
    float stretchingRatio = 3;

    int collision_dist = 1;
    int minimum_length = 5;
    bool automatic_merging = true;
    int max_angle = 99;
    bool freeze_body = true;
    int s_force = 1;
    int tracing_model = 0;
    int coding_method = 0;
    float sigma_ratio = 1;
    int border = 0;
    Tracer->setParas(pt_distance,gamma,stretchingRatio,minimum_length,collision_dist,5,5,automatic_merging,max_angle,
                     freeze_body,s_force,tracing_model,false,coding_method,sigma_ratio,border);
    IM->SetCodingMethod(0);
            //tracing finished
    while( IM->SeedPt.GetSize() != IM->visit_label.sum() )
    {
       Tracer->Open_Curve_Snake_Tracing();
       Tracer->RemoveSeeds();
    }

    std::cout<<std::endl;
    std::cout<<"--------------Processing Finished--------------"<<std::endl;

    QString fileName = callback.getImageName(curwin) + "_snake.swc";
    QFile swc_file(fileName);

    PointList3D wrote_pt, All_Pt;
    if (swc_file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream *out_txt;
        out_txt = new QTextStream(&swc_file);

        vnl_vector<int> *snake_visit_label;
        snake_visit_label = new vnl_vector<int>(Tracer->SnakeList.NSnakes);
        snake_visit_label->fill(0);
        int *point_id;
        point_id = new int[1];
        point_id[0] = 1;


        All_Pt.RemoveAllPts();

        std::vector<int> *branch_label;
        branch_label = new std::vector<int>[1];

        for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
        {
            if( snake_visit_label[0](i) == 1 )
                continue;
            if( Tracer->SnakeList.valid_list[i] == 0 )
                continue;

            wrote_pt.RemoveAllPts();
            int snake_id = i;
            findBranch_Raw( snake_id, -1, Tracer->SnakeList.Snakes[i].Cu.GetFirstPt(), snake_visit_label, point_id, out_txt, &wrote_pt, &All_Pt, branch_label );

        }
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(fileName));
    return;

}


void findBranch_Raw(int snake_label, int root_label, Point3D root_point, vnl_vector<int>*snake_visit_label, int *point_id, QTextStream *out_txt, PointList3D *wrote_pt, PointList3D *all_pt, std::vector<int> *branch_label)
{
    int collision_dist = 1;
    int ratio = 1;
    int min_length = 5;

    snake_visit_label[0](snake_label) = 1;

    if( root_point.GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt()) <
        root_point.GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.GetFirstPt()))
    {
        //Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
        Tracer->SnakeList.Snakes[snake_label].Ru = Tracer->SnakeList.Snakes[snake_label].Cu.Flip_4D(Tracer->SnakeList.Snakes[snake_label].Ru);
    }

    bool check_merging = true;
    while( check_merging )
    {
        check_merging = false;
        //find out if the tip (not the root point) of current main branch is actually a branch point
        for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
        {
            if( snake_visit_label[0](j) == 1 )
                continue;
            if( Tracer->SnakeList.valid_list[j] == 0 )
                continue;

            vnl_vector<float> temp_dist(Tracer->SnakeList.Snakes[j].Cu.GetSize());
            for( int i = 0; i < Tracer->SnakeList.Snakes[j].Cu.GetSize(); i++ )
            {
                temp_dist(i) = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt().GetDistTo(Tracer->SnakeList.Snakes[j].Cu.Pt[i]);
            }

            if( temp_dist.min_value() <= collision_dist )
            {
                int pt_id = temp_dist.arg_min();

                if( pt_id != 0 && pt_id != Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
                {

                    float L1 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,0);
                    float L2 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,1);

                    //rearrangement
                    if( L1 >= min_length && L2 >= min_length )
                    {
                        Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
                        Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
                        //add points to current snake
                        for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
                        {
                            Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
                            Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
                        }
                        //remove points for the snake
                        Tracer->SnakeList.Snakes[j].Cu.Resize(pt_id+1);
                        Tracer->SnakeList.Snakes[j].Ru.resize(pt_id+1);

                    }
                    //merging
                    else if( L1 < min_length && L2 >= min_length )
                    {
                        Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
                        Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
                        //add points to current snake
                        for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
                        {
                            Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
                            Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
                        }
                        //delete the snake
                        Tracer->SnakeList.valid_list[j] = 0;

                        check_merging = true;
                    }
                    else if( L2 < min_length && L1 >= min_length )
                    {
                        Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
                        Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
                        //add points to current snake
                        for( int im = pt_id; im > 0; im-- )
                        {
                            Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
                            Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
                        }
                        //delete the snake
                        Tracer->SnakeList.valid_list[j] = 0;

                        check_merging = true;
                    }
                    else
                    {
                        //delete the snake
                        Tracer->SnakeList.valid_list[j] = 0;
                    }
                }
                else if( pt_id == 0 )
                {
                    Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
                    Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
                    for( int im = 0; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
                    {
                        Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
                        Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
                    }
                    Tracer->SnakeList.valid_list[j] = 0;

                    check_merging = true;
                }
                else if( pt_id == Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
                {
                    Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
                    Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
                    for( int im = Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1; im >= 0; im-- )
                    {
                        Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
                        Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
                    }
                    Tracer->SnakeList.valid_list[j] = 0;

                    check_merging = true;
                }

                //change the label image for snake selection
                IM->ImCoding( Tracer->SnakeList.Snakes[snake_label].Cu, Tracer->SnakeList.Snakes[snake_label].Ru, snake_label + 1, false );
                break;
            }
        }
    }

    //record the point id of current main branch
    vnl_vector<int> snake_point_id(Tracer->SnakeList.Snakes[snake_label].Cu.GetSize());
    snake_point_id.fill(0);
    //write the points of current main branch to swc file
    for( int i = 0; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
    {
        if( root_label == -1 )
        {

            *out_txt<<point_id[0]<<" ";
            *out_txt<<"2"<<" ";
            if( i == 0)
            {
                *out_txt<<root_point.x * ratio<<" ";
                *out_txt<<root_point.y * ratio<<" ";
                *out_txt<<root_point.z<<" ";
                *out_txt<<"0"<<" ";
                *out_txt<<root_label<<"\n";

                wrote_pt->AddPt(root_point.x, root_point.y, root_point.z);
                all_pt->AddPt(root_point.x, root_point.y, root_point.z);
                branch_label->push_back(1);
            }
            else
            {
                *out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" ";
                *out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" ";
                *out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z<<" ";

                //*out_txt<<"0"<<" ";
                *out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * ratio<<" ";

                *out_txt<<point_id[0]-1<<"\n";
                wrote_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
                all_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
                branch_label->push_back(0);
            }

            snake_point_id(i) = point_id[0];
            point_id[0]++;
        }
        else
        {
            if( i == 0 )
                continue;


            if( i == Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() - 1 )
            {
                bool loop = false;
                for( int k = 0; k < wrote_pt->NP; k++ )
                {
                    if( Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].GetDistTo( wrote_pt->Pt[k] ) <= collision_dist )
                    {
                        loop = true;
                        break;
                    }
                }
                if( loop )
                    break;
            }

            *out_txt<<point_id[0]<<" ";
            *out_txt<<"2"<<" ";
            *out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" ";
            *out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" ";
            *out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z<<" ";
            //*out_txt<<"0"<<" ";
            //*out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * sqrt((double)2)<<" ";
            *out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * ratio<<" ";

            wrote_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
            all_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
            branch_label->push_back(0);

            if( i == 1 )
                *out_txt<<root_label<<"\n";
            else
                *out_txt<<point_id[0]-1<<"\n";

            snake_point_id(i) = point_id[0];
            point_id[0]++;
        }
    }

    //find side branches and do recursion
    for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
    {
        if( snake_visit_label[0](j) == 1 )
            continue;
        if( Tracer->SnakeList.valid_list[j] == 0 )
            continue;

        vnl_vector<float> tail_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
        tail_dist.fill(100000);
        vnl_vector<float> head_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
        head_dist.fill(100000);
        for( int i = 1; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
        {
            if( snake_point_id(i) == 0 )
                continue;
            tail_dist(i) = Tracer->SnakeList.Snakes[j].Cu.GetFirstPt().GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]);
            head_dist(i) = Tracer->SnakeList.Snakes[j].Cu.GetLastPt().GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]);
        }

        if( tail_dist.min_value() <= collision_dist )
        {
            int id_temp = tail_dist.arg_min();
            int new_snake_id = j;
            Point3D new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[id_temp];
            int new_root_label = snake_point_id(id_temp);
            branch_label[0][new_root_label-1] += 1;
            findBranch_Raw(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, out_txt, wrote_pt, all_pt, branch_label);
        }
        else if( head_dist.min_value() <= collision_dist )
        {
            int id_temp = head_dist.arg_min();
            int new_snake_id = j;
            Point3D new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[id_temp];
            int new_root_label = snake_point_id(id_temp);
            branch_label[0][new_root_label-1] += 1;
            findBranch_Raw(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, out_txt, wrote_pt, all_pt, branch_label);
        }

    }
    return;
}

