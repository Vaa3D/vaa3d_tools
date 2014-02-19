/* neuTube_zhi_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-02-17 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuTube_zhi_plugin.h"
//#include "zstack.hxx"


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

#include "image_lib.h"

using namespace std;
Q_EXPORT_PLUGIN2(neuTube_zhi, neuTube_zhi);

void autotrace(V3DPluginCallback2 &callback, QWidget *parent);
int autoThreshold(Stack *stack);

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

	if (func_name == tr("help"))
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

    //Mc_Stack *stack = NVInterface::makeStack(p4DImage);

    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG pagesz = N*M*P;
    int c = 1;
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
                   Set_Stack_Pixel(stack,x,y,z,c-1,dataval);
            }
        }
    }

    int thre = autoThreshold(stack);

    Stack *mask = Copy_Stack(stack);
    Stack_Threshold_Binarize(mask, thre);
    Translate_Stack(mask, GREY, 1);

   // Trace_Workspace *m_traceWorkspace = NULL;

    double z_scale = 1.0;

    Stack *mask2 = mask;
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


    printf("threshold is %d\n\n",thre);
    return;
}

int autoThreshold(Stack *stack)
{
  int thre = 0;
  if (stack->array != NULL) {
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
