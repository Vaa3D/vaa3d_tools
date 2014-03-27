/* neuTube_zhi_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-02-17 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuTube_zhi_plugin.h"
//#include "zstack.hxx"
#include <fstream>
#include <iostream>

#include "c_stack.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_attribute.h"
#include "tz_stack_lib.h"
#include "tz_int_histogram.h"
#include "tz_stack_threshold.h"
#include "tz_stack_bwmorph.h"
#include "tz_objdetect.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_sampling.h"
#include "tz_u16array.h"
#include "tz_u8array.h"
#include "tz_farray.h"
#include "tz_object_3d.h"
#include "tz_object_3d_linked_list.h"
#include "tz_geo3d_utils.h"
#include "tz_voxel.h"
#include "tz_voxel_linked_list.h"
#include "tz_pixel_array.h"
#include "tz_stack_utils.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_workspace.h"
#include "tz_trace_defs.h"
#include "tz_local_neuroseg.h"
#include "tz_darray.h"
#include "tz_math.h"
#include "tz_locseg_chain.h"
#include "tz_trace_utils.h"
#include "tz_locseg_chain_com.h"
#include "tz_stack_graph.h"
#include "tz_neuroseg.h"
#include "tz_bitmask.h"
#include "tz_neuropos.h"
#include "tz_3dgeom.h"
#include "tz_coordinate_3d.h"
#include "tz_geo3d_point_array.h"
#include "tz_geometry.h"
#include "tz_cont_fun.h"
#include "tz_perceptor.h"
#include "tz_optimize_utils.h"
#include "tz_geoangle_utils.h"
#include "tz_neurofield.h"
#include "tz_locseg_node.h"
#include "tz_locseg_node_doubly_linked_list.h"
#include "tz_geo3d_vector.h"
#include "tz_locseg_chain_knot.h"
#include "tz_unipointer_arraylist.h"
#include "tz_unipointer_linked_list.h"
#include "tz_stack_math.h"
#include "tz_stack_relation.h"
#include "tz_geo3d_circle.h"
#include "tz_xz_orientation.h"
#include "tz_swc_cell.h"
#include "tz_error.h"

#include "image_lib.h"



using namespace std;
Q_EXPORT_PLUGIN2(neuTube_zhi, neuTube_zhi);

void autotrace(V3DPluginCallback2 &callback, QWidget *parent);
bool autotrace(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);

int autoThreshold(Stack *stack);
void loadTraceMask(bool traceMasked,Trace_Workspace *m_traceWorkspace);
double confidence(Locseg_Chain *chain,Stack *stack);
Swc_Tree* swcReconstruction(Locseg_Chain **chain, int nchian,Stack *stack);

QStringList neuTube_zhi::menulist() const
{
	return QStringList() 
		<<tr("trace")
		<<tr("about");
}

QStringList neuTube_zhi::funclist() const
{
	return QStringList()
		<<tr("help");
}

void neuTube_zhi::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("trace"))
	{
        autotrace(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2014-02-17"));
	}
}

bool neuTube_zhi::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("neutube_trace"))
    {
        return autotrace(input, output,callback);
    }
    else if (func_name == tr("help"))
	{
        cout<<"Usage : v3d -x dllname -f neutube_trace -i <inimg_file> -p <ch> <merge>"<<endl;
        cout<<endl;
        cout<<"ch           the input channel value, start from 1, default 1"<<endl;
        cout<<"merge        Merge close nodes after enhancement, 1: merge, 0: not merge, default 0"<<endl;
        cout<<"The output swc file will be named automatically based on the input image file nmae"<<endl;
        cout<<endl;
        cout<<endl;
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

    //Mc_Stack *stack = NVInterface::makeStack(p4DImage);

    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    bool ok1;
    int c,p = 0;

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

    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Merge Close Nodes?"), QMessageBox::Yes, QMessageBox::No))    p = 1;


    V3DLONG pagesz = N*M*P;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    Stack *stack;
    switch (pixeltype)
    {
    case V3D_UINT8: stack = Make_Stack(GREY,(int)N,(int)M,(int)P);break;
    default: v3d_msg("Invalid data type. Do nothing."); return;
    }

    V3DLONG offsetc = (c-1)*pagesz;
    for(V3DLONG z = 0; z < P; z++)
    {
         V3DLONG offsetk = z*M*N;
        for(V3DLONG y = 0; y < M; y++)
        {
            V3DLONG offsetj = y*N;
            for(V3DLONG x = 0; x < N; x++)
            {
                   double dataval = data1d[offsetc + offsetk + offsetj + x];
                   Set_Stack_Pixel(stack,x,y,z,0,dataval);
            }
        }
    }

    int thre = autoThreshold(stack);

    Stack *mask = Copy_Stack(stack);
    Stack_Threshold_Binarize(mask, thre);
    Translate_Stack(mask, GREY, 1);

    /* alloc <mask2> */
    Stack *mask2 = Stack_Majority_Filter_R(mask, NULL, 26, 4);

    /* free <mask>, <mask2> => <mask> */
    Kill_Stack(mask);
    mask = mask2;

    double z_scale = 1.0;

    mask2 = mask;
    /* resample the stack for dist calc if z is different */
    if (z_scale != 1.0) {
      mask2 = Resample_Stack_Depth(mask, NULL, z_scale);
      Stack_Binarize(mask2);
    }

    /* alloc <dist> */
    Stack *dist = Stack_Bwdist_L_U16(mask2, NULL, 0);

    if (mask != mask2) {
      Kill_Stack(mask2);
    }

    /* alloc <seeds> */
    Stack *seeds = Stack_Locmax_Region(dist, 26);

    /* alloc <objs> */
    Object_3d_List *objs = Stack_Find_Object_N(seeds, NULL, 1, 0, 26);

    Zero_Stack(seeds);

    int objnum = 0;
    while (objs != NULL) {
      Object_3d *obj = objs->data;
      Voxel_t center;
      Object_3d_Central_Voxel(obj, center);
      Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);
      objs = objs->next;
      objnum++;
    }

    /* free <objs> */
    Kill_Object_3d_List(objs);

    /* alloc <list> */
    Voxel_List *list = Stack_To_Voxel_List(seeds);

    /* free <seeds> */
    Kill_Stack(seeds);

    /* alloc <pa> */
    Pixel_Array *pa = Voxel_List_Sampling(dist, list);

    /* free <dist> */
    Kill_Stack(dist);

    /* alloc <voxel_array> */
    Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);

    uint16 *pa_array = (uint16 *) pa->array;

    /* alloc <seed_field> */
    Geo3d_Scalar_Field *seed_field = Make_Geo3d_Scalar_Field(pa->size);
    seed_field->size = 0;
    int i;
    for (i = 0; i < pa->size; i++) {
      seed_field->points[seed_field->size][0] = voxel_array[i]->x;
      seed_field->points[seed_field->size][1] = voxel_array[i]->y;
      seed_field->points[seed_field->size][2] = z_scale * voxel_array[i]->z;
      seed_field->values[seed_field->size] = sqrt((double)pa_array[i]);
      seed_field->size++;
    }

    /* free <list> */
    Kill_Voxel_List(list);

    /* free <voxel_array> */
    free(voxel_array);

    /* free <pa> */
    Kill_Pixel_Array(pa);

    /* now the seeds are in <field> */
    /* <mask> => <seed_mask> */

    Trace_Workspace *m_traceWorkspace  = New_Trace_Workspace();
    Locseg_Chain_Default_Trace_Workspace(m_traceWorkspace, stack);

    m_traceWorkspace->fit_workspace = New_Locseg_Fit_Workspace();

    m_traceWorkspace->tune_end = TRUE;
    m_traceWorkspace->add_hit = TRUE;


    Stack *seed_mask = mask;
    Zero_Stack(seed_mask);
    Locseg_Fit_Workspace *fws =
        (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
    Stack_Fit_Score old_fs = fws->sws->fs;

    fws->sws->fs.n = 2;
    fws->sws->fs.options[0] = STACK_FIT_DOT;
    fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;

    /* alloc <locseg> */
    Local_Neuroseg *locseg = (Local_Neuroseg *)
                             malloc(seed_field->size * sizeof(Local_Neuroseg));
    /* alloc <values> */
    double *values = darray_malloc(seed_field->size);

    /* fit segment on each seed */
    for (i = 0; i < seed_field->size; i++) {
      qDebug("-----------------------------> seed: %d / %d\n",
             i, seed_field->size);

      int index = i;
      int x = iround(seed_field->points[index][0]);
      int y = iround(seed_field->points[index][1]);
      int z = iround(seed_field->points[index][2]);

      double width = seed_field->values[index];

      ssize_t seed_offset =
          Stack_Util_Offset(x, y, z, stack->width, stack->height,
               stack->depth);

      if (seed_offset < 0) {
        continue;
      }

      if (width < 3.0) {
        width += 0.5;
      }

      Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_DEFAULT_H,
                   TZ_PI_2, 0.0, 0.0, 0.0, 1.0 / z_scale);
  /*
      Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_HEIGHT,
                   0.0, 0.0, 0.0, 0.0, 1.0 / z_scale);
  */
      double cpos[3];
      cpos[0] = x;
      cpos[1] = y;
      cpos[2] = z;

      Set_Neuroseg_Position(&(locseg[i]), cpos, NEUROSEG_CENTER);

      if (seed_mask->array[seed_offset] > 0) {
        qDebug("labeled\n");
        values[i] = 0.0;
        continue;
      }

      Local_Neuroseg_Optimize_W(locseg + i, stack, 1.0, 0, fws);

      values[i] = fws->sws->fs.scores[1];

      qDebug("%g\n", values[i]);

      /* label seed_mask */
      if (values[i] > LOCAL_NEUROSEG_MIN_CORRCOEF) {
        Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 2, 1.0);
      } else {
        Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 1, 1.0);
      }

      //addLocsegChain(new ZLocsegChain(Copy_Local_Neuroseg(locseg + i)));
    }


    fws->sws->fs = old_fs;
    Kill_Stack(seed_mask);

    /* make trace mask */
    if (m_traceWorkspace->trace_mask == NULL) {
      m_traceWorkspace->trace_mask =
        Make_Stack(GREY16, stack->width, stack->height,
       stack->depth);
    }

    Zero_Stack(m_traceWorkspace->trace_mask);

    /* trace all seeds */

   // m_traceWorkspace->min_chain_length = 20;
   // m_traceWorkspace->min_score = 0.35;
     int nchain;
     Locseg_Chain **chain =
     Trace_Locseg_S(stack, 1.0, locseg, values, seed_field->size,
                     m_traceWorkspace, &nchain);


    /* tune ends */
    Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
    ws->signal = stack;
    ws->sratio = 1.0;
    ws->sdiff = 0.0;
    ws->option = 6;

    /*Zero_Stack(m_traceWorkspace->trace_mask);

    for (i = 0; i < nchain; i++) {
      if(chain[i] != NULL) {
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1,
                             ws);
      }
    }*/

    Stack_Binarize(m_traceWorkspace->trace_mask);

    double old_step = m_traceWorkspace->trace_step;
    BOOL old_refit = m_traceWorkspace->refit;
    BOOL traceMasked = Trace_Workspace_Is_Masked(m_traceWorkspace);

    loadTraceMask(true,m_traceWorkspace);
    m_traceWorkspace->trace_step = 0.1;
    m_traceWorkspace->refit = FALSE;

    for (i = 0; i < nchain; i++) {
      if (chain[i] != NULL) {
        /* erase the mask */
        ws->option = 7;
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1,
                             ws);

        m_traceWorkspace->trace_status[0] = TRACE_NORMAL;
        m_traceWorkspace->trace_status[1] = TRACE_NORMAL;
        Trace_Locseg(stack, 1.0, chain[i], m_traceWorkspace);
        Locseg_Chain_Down_Sample(chain[i]);

        Locseg_Chain_Tune_End(chain[i], stack, 1.0,
                              m_traceWorkspace->trace_mask, DL_HEAD);
        Locseg_Chain_Tune_End(chain[i], stack, 1.0,
                              m_traceWorkspace->trace_mask, DL_TAIL);

        if (Locseg_Chain_Length(chain[i]) > 0) {
          ws->option = 6;
          Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                               0, Locseg_Chain_Length(chain[i]) - 1, ws);
        }
      }
    }


    ws->signal = NULL;
    Kill_Locseg_Label_Workspace(ws);

    m_traceWorkspace->trace_step = old_step;
    m_traceWorkspace->refit = old_refit;
    loadTraceMask(traceMasked,m_traceWorkspace);

    Zero_Stack(m_traceWorkspace->trace_mask);
    m_traceWorkspace->chain_id = 0;

    QString outswc_file = QString(p4DImage->getFileName()) + "_neutube.swc";

    FILE *tube_fp = fopen(outswc_file.toStdString().c_str(), "w");
    int start_id = 1;
    for (i = 0; i < nchain; i++)
    {
        if (chain[i] != NULL && Locseg_Chain_Length(chain[i]) > 0)
        {
            if(confidence(chain[i],stack) > 0.5)
            {
                  int n = Locseg_Chain_Swc_Fprint_T(tube_fp,chain[i],2, start_id,-1, DL_FORWARD, 1.0, NULL);
                  start_id += n;
            }
        }
    }
    fclose(tube_fp);

    if(p)
    {
            Swc_Tree *tree =  Read_Swc_Tree(outswc_file.toStdString().c_str());
            if(tree != NULL)
            {
                Swc_Tree_Node *tn = NULL;
                Swc_Tree_Node *tmp_tn = NULL;

                Swc_Tree_Iterator_Start(tree, 2, FALSE);
                tn = Swc_Tree_Next(tree);
                while ((tmp_tn = Swc_Tree_Next(tree)) != NULL) {
                  if (Swc_Tree_Node_Data(tmp_tn)->d > Swc_Tree_Node_Data(tn)->d) {
                    tn = tmp_tn;
                  }
                }

                if (tn != NULL) {
                  Swc_Tree_Node_Set_Root(tn);
                }
                Swc_Tree_Remove_Zigzag(tree);
                Swc_Tree_Tune_Branch(tree);
                Swc_Tree_Remove_Spur(tree);
                Swc_Tree_Merge_Close_Node(tree, 0.01);

                Swc_Tree_Remove_Overshoot(tree);
                Swc_Tree_Resort_Id(tree);

                Swc_Tree_Reconnect(tree, 1.0, 20.0);
                Write_Swc_Tree(outswc_file.toStdString().c_str(),tree);
                Kill_Swc_Tree(tree);

            }
    }


    free(chain);

    /* free <seed_field> */
    Kill_Geo3d_Scalar_Field(seed_field);

    /* free <values> */
    free(values);

    /* free <locseg> */
    free(locseg);

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(outswc_file));
    return;
}

bool autotrace(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to neutube tracing"<<endl;
    unsigned int c=1, p = 0;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) c = atoi(paras.at(0));
        if(paras.size() >= 2) p = atoi(paras.at(1));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);

    cout<<"ch = "<<c<<endl;
    cout<<"Merge Close Nodes = "<<p<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;

    Image4DSimple *subject = callback.loadImage(inimg_file);
    if(!subject || !subject->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (subject) {delete subject; subject=0;}
         return false;
    }

    V3DLONG N = subject->getXDim();
    V3DLONG M = subject->getYDim();
    V3DLONG P = subject->getZDim();

    V3DLONG pagesz = N*M*P;
    int datatype = subject->getDatatype();

    unsigned char *data1d = subject->getRawData();

    Stack *stack;
    switch (datatype)
    {
        case V3D_UINT8: stack = Make_Stack(GREY,(int)N,(int)M,(int)P);break;
        default: v3d_msg("Invalid data type. Do nothing."); return false;
    }

    V3DLONG offsetc = (c-1)*pagesz;
    for(V3DLONG z = 0; z < P; z++)
    {
         V3DLONG offsetk = z*M*N;
        for(V3DLONG y = 0; y < M; y++)
        {
            V3DLONG offsetj = y*N;
            for(V3DLONG x = 0; x < N; x++)
            {
                   double dataval = data1d[offsetc + offsetk + offsetj + x];
                   Set_Stack_Pixel(stack,x,y,z,0,dataval);
            }
        }
    }

    int thre = autoThreshold(stack);

    Stack *mask = Copy_Stack(stack);
    Stack_Threshold_Binarize(mask, thre);
    Translate_Stack(mask, GREY, 1);

    /* alloc <mask2> */
    Stack *mask2 = Stack_Majority_Filter_R(mask, NULL, 26, 4);

    /* free <mask>, <mask2> => <mask> */
    Kill_Stack(mask);
    mask = mask2;

    double z_scale = 1.0;

    mask2 = mask;
    /* resample the stack for dist calc if z is different */
    if (z_scale != 1.0) {
      mask2 = Resample_Stack_Depth(mask, NULL, z_scale);
      Stack_Binarize(mask2);
    }

    /* alloc <dist> */
    Stack *dist = Stack_Bwdist_L_U16(mask2, NULL, 0);

    if (mask != mask2) {
      Kill_Stack(mask2);
    }

    /* alloc <seeds> */
    Stack *seeds = Stack_Locmax_Region(dist, 26);

    /* alloc <objs> */
    Object_3d_List *objs = Stack_Find_Object_N(seeds, NULL, 1, 0, 26);

    Zero_Stack(seeds);

    int objnum = 0;
    while (objs != NULL) {
      Object_3d *obj = objs->data;
      Voxel_t center;
      Object_3d_Central_Voxel(obj, center);
      Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);
      objs = objs->next;
      objnum++;
    }

    /* free <objs> */
    Kill_Object_3d_List(objs);

    /* alloc <list> */
    Voxel_List *list = Stack_To_Voxel_List(seeds);

    /* free <seeds> */
    Kill_Stack(seeds);

    /* alloc <pa> */
    Pixel_Array *pa = Voxel_List_Sampling(dist, list);

    /* free <dist> */
    Kill_Stack(dist);

    /* alloc <voxel_array> */
    Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);

    uint16 *pa_array = (uint16 *) pa->array;

    /* alloc <seed_field> */
    Geo3d_Scalar_Field *seed_field = Make_Geo3d_Scalar_Field(pa->size);
    seed_field->size = 0;
    int i;
    for (i = 0; i < pa->size; i++) {
      seed_field->points[seed_field->size][0] = voxel_array[i]->x;
      seed_field->points[seed_field->size][1] = voxel_array[i]->y;
      seed_field->points[seed_field->size][2] = z_scale * voxel_array[i]->z;
      seed_field->values[seed_field->size] = sqrt((double)pa_array[i]);
      seed_field->size++;
    }

    /* free <list> */
    Kill_Voxel_List(list);

    /* free <voxel_array> */
    free(voxel_array);

    /* free <pa> */
    Kill_Pixel_Array(pa);

    /* now the seeds are in <field> */
    /* <mask> => <seed_mask> */

    Trace_Workspace *m_traceWorkspace  = New_Trace_Workspace();
    Locseg_Chain_Default_Trace_Workspace(m_traceWorkspace, stack);

    m_traceWorkspace->fit_workspace = New_Locseg_Fit_Workspace();

    m_traceWorkspace->tune_end = TRUE;
    m_traceWorkspace->add_hit = TRUE;


    Stack *seed_mask = mask;
    Zero_Stack(seed_mask);
    Locseg_Fit_Workspace *fws =
        (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
    Stack_Fit_Score old_fs = fws->sws->fs;

    fws->sws->fs.n = 2;
    fws->sws->fs.options[0] = STACK_FIT_DOT;
    fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;

    /* alloc <locseg> */
    Local_Neuroseg *locseg = (Local_Neuroseg *)
                             malloc(seed_field->size * sizeof(Local_Neuroseg));
    /* alloc <values> */
    double *values = darray_malloc(seed_field->size);

    /* fit segment on each seed */
    for (i = 0; i < seed_field->size; i++) {
      qDebug("-----------------------------> seed: %d / %d\n",
             i, seed_field->size);

      int index = i;
      int x = iround(seed_field->points[index][0]);
      int y = iround(seed_field->points[index][1]);
      int z = iround(seed_field->points[index][2]);

      double width = seed_field->values[index];

      ssize_t seed_offset =
          Stack_Util_Offset(x, y, z, stack->width, stack->height,
               stack->depth);

      if (seed_offset < 0) {
        continue;
      }

      if (width < 3.0) {
        width += 0.5;
      }

      Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_DEFAULT_H,
                   TZ_PI_2, 0.0, 0.0, 0.0, 1.0 / z_scale);
  /*
      Set_Neuroseg(&(locseg[i].seg), width * z_scale, 0.0, NEUROSEG_HEIGHT,
                   0.0, 0.0, 0.0, 0.0, 1.0 / z_scale);
  */
      double cpos[3];
      cpos[0] = x;
      cpos[1] = y;
      cpos[2] = z;

      Set_Neuroseg_Position(&(locseg[i]), cpos, NEUROSEG_CENTER);

      if (seed_mask->array[seed_offset] > 0) {
        qDebug("labeled\n");
        values[i] = 0.0;
        continue;
      }

      Local_Neuroseg_Optimize_W(locseg + i, stack, 1.0, 0, fws);

      values[i] = fws->sws->fs.scores[1];

      qDebug("%g\n", values[i]);

      /* label seed_mask */
      if (values[i] > LOCAL_NEUROSEG_MIN_CORRCOEF) {
        Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 2, 1.0);
      } else {
        Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 1, 1.0);
      }

      //addLocsegChain(new ZLocsegChain(Copy_Local_Neuroseg(locseg + i)));
    }


    fws->sws->fs = old_fs;
    Kill_Stack(seed_mask);

    /* make trace mask */
    if (m_traceWorkspace->trace_mask == NULL) {
      m_traceWorkspace->trace_mask =
        Make_Stack(GREY16, stack->width, stack->height,
       stack->depth);
    }

    Zero_Stack(m_traceWorkspace->trace_mask);

    /* trace all seeds */

   // m_traceWorkspace->min_chain_length = 20;
   // m_traceWorkspace->min_score = 0.35;
     int nchain;
     Locseg_Chain **chain =
     Trace_Locseg_S(stack, 1.0, locseg, values, seed_field->size,
                     m_traceWorkspace, &nchain);


    /* tune ends */
    Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
    ws->signal = stack;
    ws->sratio = 1.0;
    ws->sdiff = 0.0;
    ws->option = 6;

    /*Zero_Stack(m_traceWorkspace->trace_mask);

    for (i = 0; i < nchain; i++) {
      if(chain[i] != NULL) {
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1,
                             ws);
      }
    }*/

    Stack_Binarize(m_traceWorkspace->trace_mask);

    double old_step = m_traceWorkspace->trace_step;
    BOOL old_refit = m_traceWorkspace->refit;
    BOOL traceMasked = Trace_Workspace_Is_Masked(m_traceWorkspace);

    loadTraceMask(true,m_traceWorkspace);
    m_traceWorkspace->trace_step = 0.1;
    m_traceWorkspace->refit = FALSE;

    for (i = 0; i < nchain; i++) {
      if (chain[i] != NULL) {
        /* erase the mask */
        ws->option = 7;
        Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                             0, Locseg_Chain_Length(chain[i]) - 1,
                             ws);

        m_traceWorkspace->trace_status[0] = TRACE_NORMAL;
        m_traceWorkspace->trace_status[1] = TRACE_NORMAL;
        Trace_Locseg(stack, 1.0, chain[i], m_traceWorkspace);
        Locseg_Chain_Down_Sample(chain[i]);

        Locseg_Chain_Tune_End(chain[i], stack, 1.0,
                              m_traceWorkspace->trace_mask, DL_HEAD);
        Locseg_Chain_Tune_End(chain[i], stack, 1.0,
                              m_traceWorkspace->trace_mask, DL_TAIL);

        if (Locseg_Chain_Length(chain[i]) > 0) {
          ws->option = 6;
          Locseg_Chain_Label_W(chain[i], m_traceWorkspace->trace_mask, 1.0,
                               0, Locseg_Chain_Length(chain[i]) - 1, ws);
        }
      }
    }


    ws->signal = NULL;
    Kill_Locseg_Label_Workspace(ws);

    m_traceWorkspace->trace_step = old_step;
    m_traceWorkspace->refit = old_refit;
    loadTraceMask(traceMasked,m_traceWorkspace);

    Zero_Stack(m_traceWorkspace->trace_mask);
    m_traceWorkspace->chain_id = 0;

    QString outswc_file = QString(inimg_file) + "_neutube.swc";

    if(p)
    {
            Swc_Tree *rawTree = swcReconstruction(chain,nchain,stack);
            if (rawTree != NULL)
            {
              Write_Swc_Tree(outswc_file.toStdString().c_str(), rawTree);
              Kill_Swc_Tree(rawTree);
            }
        }
    else
    {
        FILE *tube_fp = fopen(outswc_file.toStdString().c_str(), "w");
        int start_id = 1;
        for (i = 0; i < nchain; i++)
        {
            if (chain[i] != NULL && Locseg_Chain_Length(chain[i]) > 0)
            {
                if(confidence(chain[i],stack) > 0.5)
                {
                      int n = Locseg_Chain_Swc_Fprint_T(tube_fp,chain[i],2, start_id,-1, DL_FORWARD, 1.0, NULL);
                      start_id += n;
                }
            }
        }
        fclose(tube_fp);
    }


    free(chain);

    /* free <seed_field> */
    Kill_Geo3d_Scalar_Field(seed_field);

    /* free <values> */
    free(values);

    /* free <locseg> */
    free(locseg);

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(outswc_file),0);
    return true;

}

int autoThreshold(Stack *stack)
{
  int thre = 0;
  if (stack->array != NULL) {

      /*double scale = 1.0*stack->width * stack->height * stack->depth * stack->kind /
          (2.0*1024*1024*1024);
      if (scale >= 1.0) {
        scale = ceil(sqrt(scale + 0.1));
        stack = Resize_Stack(stack, stack->width/scale, stack->height/scale, stack->depth);
      }
*/
    int conn = 18;
    Stack *locmax = Stack_Locmax_Region(stack, conn);
    Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
    int nvoxel = Stack_Voxel_Number(locmax);
    int i;

    for (i = 0; i < nvoxel; i++) {
      if (locmax->array[i] < 3) {
        locmax->array[i] = 0;
      } else {
        locmax->array[i] = 1;
      }
    }

    int *hist = Stack_Hist_M(stack, locmax);
    Kill_Stack(locmax);

    int low, high;
    Int_Histogram_Range(hist, &low, &high);

    thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);

    free(hist);
  }
  return thre;
}

void loadTraceMask(bool traceMasked,Trace_Workspace *m_traceWorkspace)
{
  if (traceMasked) {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, m_traceWorkspace->trace_mask);
  } else {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, NULL);
  }
}

double confidence(Locseg_Chain *chain,Stack *stack)
{

    /* parameters learned from logistic regression */
    double c[3] = {1.1072, 5.3103, -2.0465};

    double x1 = Locseg_Chain_Geolen(chain);
    double x2 = Locseg_Chain_Average_Score(chain, stack,1.0,STACK_FIT_CORRCOEF);

    return 1.0 / (1.0 + exp((x1 * c[0] + x2 * c[1] + x1 * x2 * c[2])));

}

Swc_Tree* swcReconstruction(Locseg_Chain **chain, int chain_number, Stack *stack)
{

    Swc_Tree *tree = NULL;
    if(chain_number > 0)
    {
        Neuron_Component *chain_array = Make_Neuron_Component_Array(chain_number);
        int chain_number2 = 0;
        for (int i = 0; i < chain_number; i++)
        {
          if(chain[i] != NULL && Locseg_Chain_Length(chain[i]) > 0)
          {
              if(confidence(chain[i],stack) > 0.5)
              {
                    Set_Neuron_Component(chain_array + chain_number2,NEUROCOMP_TYPE_LOCSEG_CHAIN,Copy_Locseg_Chain(chain[i]));
                    chain_number2++;
              }
          }
        }

        Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();

        Neuron_Structure *ns = NULL;
        ns = Locseg_Chain_Comp_Neurostruct(chain_array, chain_number2,NULL, 1.0, ctw);
        Process_Neuron_Structure(ns);
        Neuron_Structure* ns2= Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
        tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);

        Swc_Tree_Node *tn = NULL;
        Swc_Tree_Node *tmp_tn = NULL;

        Swc_Tree_Iterator_Start(tree, 2, FALSE);
        tn = Swc_Tree_Next(tree);
        while ((tmp_tn = Swc_Tree_Next(tree)) != NULL) {
          if (Swc_Tree_Node_Data(tmp_tn)->d > Swc_Tree_Node_Data(tn)->d) {
            tn = tmp_tn;
          }
        }

        if (tn != NULL) {
          Swc_Tree_Node_Set_Root(tn);
        }

        Swc_Tree_Remove_Zigzag(tree);
        Swc_Tree_Tune_Branch(tree);
        Swc_Tree_Remove_Spur(tree);
        Swc_Tree_Merge_Close_Node(tree, 0.01);

        //Swc_Tree_Remove_Overshoot(tree);
        Swc_Tree_Resort_Id(tree);

        /* free <ns2> */
        Kill_Neuron_Structure(ns2);
        /* free <ns> */
        ns->comp = NULL;
        Kill_Neuron_Structure(ns);

        /* free <chain_array> */
        Clean_Neuron_Component_Array(chain_array, chain_number);
        free(chain_array);

    }
    return tree;

}
