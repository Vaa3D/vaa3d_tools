#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdio.h>

#include "tz_utilities.h"
#include "zargumentprocessor.h"
#include "zpoint.h"
#include "tz_fmatrix.h"
#include "tz_farray.h"
#include "tz_darray.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_attribute.h"
#include "zstring.h"
#include "tz_objdetect.h"
#include "zobject3darray.h"
#include "zkmeansclustering.h"
#include "tz_iarray.h"

using namespace std;

//#define _TEST_ 1

struct TbarSaParameter {
  bool saveScreenShot; //
  double tbarWidth;
  vector<double> thds; //thresholds?
  bool clusterCC;
  int offset;
  double minDistCtrs;
  int max_cluster_sz;
};

FMatrix* hdf5read_prediction(const char *filePath)
{
    cout << "Reading prediction ..." << endl;

    hid_t file_id = H5Fopen(filePath, H5F_ACC_RDONLY, H5P_DEFAULT);

    const char *dset_name = "/volume/prediction";
    int ndim = 0;
    H5LTget_dataset_ndims (file_id, dset_name, &ndim);

    cout << "ndim: " << ndim << endl;
    if (ndim > 5) {
        cerr << "Too many dimenstions.";
    }

    hsize_t *dims = new hsize_t[ndim];
    H5T_class_t class_id;
    size_t type_size;

    H5LTget_dataset_info (file_id, dset_name, dims, &class_id, &type_size );

    cout << "class_id: " << class_id << endl;
    if (class_id == H5T_FLOAT) {
        cout << "float type" << endl;
    }
    cout << "type_size: " << type_size << endl;

    dim_type mat_dims[MAX_DIM];
    ndim_type mat_ndim = ndim;

    cout << "Size:";
    for (int i = 0; i < ndim; i++) {
        cout << " " << dims[i];
        mat_dims[i] = dims[ndim - i - 1];
    }
    cout << endl;

    FMatrix *data = Make_FMatrix(mat_dims, mat_ndim);

    H5LTread_dataset_float(file_id, dset_name, data->array);

    return data;
}


static void parse_param(const char *filePath, TbarSaParameter *param)
{
    static const int nparam = 7;

    static const char *ParameterKeyword[] = {
        "tbar_width", "cluster_cc", "max_cluster_sz",
        "boundary_offset", "thds", "min_dist_ctrs", "save_screenshot"
    };

    static const int ParameterType[] = {
        FLOAT64_TD, INT_TD, INT_TD, INT_TD, FLOAT64_TD, FLOAT64_TD, INT_TD
    };

    static const bool IsArrayType[] = {
        false, false, false, false, true, false, false
    };

    FILE *fp = fopen(filePath, "r");
    if (fp == 0) {
        cerr << "Cannot open " << filePath << endl;
    }
    ZString str;

    while (str.readLine(fp)) {
        vector<string> wordArray = str.toWordArray();
        if (wordArray.size() > 1) {
            if (wordArray[0] == "tbar_width") {
                param->tbarWidth = ZString(wordArray[1]).firstDouble();
            } else if (wordArray[0] == "cluster_cc") {
                param->clusterCC = ZString(wordArray[1]).firstInteger();
            } else if (wordArray[0] == "max_cluster_sz") {
                param->max_cluster_sz = ZString(wordArray[1]).firstInteger();
            } else if (wordArray[0] == "boundary_offset") {
                param->offset = ZString(wordArray[1]).firstInteger();
            } else if (wordArray[0] == "thds") {
                param->thds.clear();
                for (size_t i = 1; i < wordArray.size(); i++) {
                    param->thds.push_back(ZString(wordArray[i]).firstDouble());
                }
            } else if (wordArray[0] == "min_dist_ctrs") {
                param->minDistCtrs = ZString(wordArray[1]).firstDouble();
            } else if (wordArray[0] == "save_screenshot") {
                param->saveScreenShot = ZString(wordArray[1]).firstInteger();
            }
        }
    }

    fclose(fp);
}

//Split the objects
static vector<int> apply_clustering(const Object_3d *obj,
                                    FMatrix *detected1, double max_sz)
{
    ZDoubleVector weightArray;

    ZKmeansClustering clustering;

    int kctrs = 0;
    if (obj->size > max_sz) {
        kctrs = floor(static_cast<double>(obj->size) / max_sz);
    }

    size_t area = detected1->dim[0] * detected1->dim[1];
    size_t w = detected1->dim[0];

    if (kctrs > 1) {
        weightArray.resize(obj->size);
        for (size_t j = 0; j < weightArray.size(); j++) {
            weightArray[j] = detected1->array[(obj->voxels[j][2]) * area +
                (obj->voxels[j][1]) * w + obj->voxels[j][0]];
        }

        clustering.runKmeans(obj, kctrs, weightArray);
    }

    return clustering.labelArray();
}

static void 
Object_3d_List_Export_Csv(const char *filePath, Object_3d_List *objList)
{
  FILE *fp = fopen(filePath, "w");
  if (fp == NULL) {
    fprintf(stderr, "Unable to open file %s\n", filePath);
  } else {
    while (objList != NULL) {
      Object_3d *obj = objList->data;
      Object_3d_Csv_Fprint(obj, fp);
      objList = objList->next;
    }
  }
  fclose(fp);
}

int main(int argc, char *argv[])
{
  /* Takes '-ver' option and show the version */
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  //For testing
#if 0
  Object_3d *obj_test = Make_Object_3d(27, 6);
  ndim_type ndim_test = 3;
  dim_type dim_test[3] = {3, 3, 3};

  FMatrix *detected1_test = FMatrix_Ones(dim_test, ndim_test);
  detected1_test->array[0] = 2.0;
  detected1_test->array[7] = 2.0;
  size_t offset = 0;
  for (int k = 0; k < 3; k++) {
      for (int j = 0; j < 3; j++) {
          for (int i = 0; i < 3; i++) {
              obj_test->voxels[offset][0] = i;
              obj_test->voxels[offset][1] = j;
              obj_test->voxels[offset][2] = k;
              offset++;
          }
      }
  }
  vector<int> labelArray_test = apply_clustering(obj_test, detected1_test, 3);

  return 1;
#endif

#if 1
  static const char *Spec[] = {"--result_file <string>",
    "--param_file <string> --jsonoutputfile <string>", 
    "[--test <string>]", "[--golden <string>]", NULL};

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  cout << argv[0] << " started ..." << endl;

  cout << "Parsing parameters ..." << endl;
  TbarSaParameter opts;
  parse_param(ZArgumentProcessor::getStringArg("--param_file"), &opts);

  cout << "TBar width " << opts.tbarWidth << endl;
  cout << "cluster_cc " << opts.clusterCC << endl;
  cout << "max_cluster_sz " << opts.max_cluster_sz << endl;
  cout << "boundary_offset " << opts.offset << endl;
  cout << "thds ";
  darray_print(&(opts.thds[0]), opts.thds.size());
  cout << "min_dist_ctrs " << opts.minDistCtrs << endl;
  cout << "save_screenshot " << opts.saveScreenShot << endl;

  cout << "Loading result file ..." << endl;
  if (opts.saveScreenShot) {
      cout << "Screenshot function is not avaible yet" << endl;
    //void *vol_all = hdf5read(Get_String_Arg("--result_file"), "volume/data");
    //double *vol = hdf5_make_double_array(vol_all, type);
  }

  FMatrix *detected_all =
          hdf5read_prediction(ZArgumentProcessor::getStringArg("--result_file"));
  size_t length = Matrix_Size(detected_all->dim, detected_all->ndim);
  cout << "Overall length: " << length << endl;

  int vold = detected_all->dim[1];
  int imh = detected_all->dim[2];
  int imw = detected_all->dim[3];

  dim_type dims[3];
  dims[0] = detected_all->dim[1];
  dims[1] = detected_all->dim[2];
  dims[2] = detected_all->dim[3];
  FMatrix *detected1 = Make_FMatrix(dims, 3);

  length = Matrix_Size(detected1->dim, detected1->ndim);
  size_t offset_all = 0;
  for (size_t offset1 = 0; offset1 < length; offset1++) {
    detected1->array[offset1] = detected_all->array[offset_all];
    offset_all += detected_all->dim[0];
  }

  string testDir;
  string goldenDir;
  if (ZArgumentProcessor::isArgMatched("--golden")) {
    goldenDir = ZArgumentProcessor::getStringArg("--golden");
  }
  if (ZArgumentProcessor::isArgMatched("--test")) {
    testDir = ZArgumentProcessor::getStringArg("--test");
  }
  
  if (ZArgumentProcessor::isArgMatched("--golden") ||
      ZArgumentProcessor::isArgMatched("--test")) {
    Stack golden;
    golden.kind = FLOAT32;
    golden.width = detected1->dim[0];
    golden.height = detected1->dim[1];
    golden.depth = detected1->dim[2];
    golden.array = (uint8*) detected1->array;
    golden.text = const_cast<char*>("\0");
    if (ZArgumentProcessor::isArgMatched("--golden")) {
      Write_Stack(const_cast<char*>((goldenDir + "/detected1.tif").c_str()), 
          &golden);
    } 
    if (ZArgumentProcessor::isArgMatched("--test")) {
      Write_Stack(const_cast<char*>((testDir + "/detected1.tif").c_str()), 
          &golden);
    }
  }

  //////testing//////////
  /*
  Stack *stack = Scale_Float_Stack(detected1->array, detected1->dim[0],
                                   detected1->dim[1], detected1->dim[2],
                                   GREY);
  Write_Stack("/Users/zhaot/Work/neutube/neurolabi/data/test.tif", stack);
  */
  ///////////////////

  double avg_sz = opts.tbarWidth * opts.tbarWidth * opts.tbarWidth;
  double min_sz = avg_sz / 125.0;

  double tbar_cube_w = floor(opts.tbarWidth / 2.0);

  static const int morpho_w = 3;
  Struct_Element *se = Make_Cuboid_Se(morpho_w, morpho_w, morpho_w);

  Stack *bb = Make_Stack(GREY, vold, imh, imw);
  Stack *bb2 = Make_Stack(GREY, vold, imh, imw);
  Stack *bb3 = Make_Stack(GREY, vold, imh, imw);
  size_t volume = Stack_Voxel_Number(bb);

  for (int ti = 0; ti < opts.thds.size(); ti++) {
    double thd = opts.thds[ti];
    cout << "thd = " << thd << endl;
    for (size_t offset = 0; offset < volume; offset++) {
      if (detected1->array[offset] > thd) {
        bb->array[offset] = 1;
      } else {
        bb->array[offset] = 0;
      }
    }

    //Write_Stack(const_cast<char*>("../../../data/test.tif"), bb);
    if (ZArgumentProcessor::isArgMatched("--golden")) {
      Write_Stack(const_cast<char*>((goldenDir + "/bb.tif").c_str()), bb);
    } 

    if (ZArgumentProcessor::isArgMatched("--test")) {
      Write_Stack(const_cast<char*>((testDir + "/bb.tif").c_str()), bb);
    }

    tic();
    Stack_Erode_Fast(bb, bb2, se);
    ptoc();

    //Write_Stack(const_cast<char*>("../../../data/test2.tif"), bb2);
    if (ZArgumentProcessor::isArgMatched("--golden")) {
      Write_Stack(const_cast<char*>((goldenDir + "/bb2.tif").c_str()), bb2);
    } 

    if (ZArgumentProcessor::isArgMatched("--test")) {
      Write_Stack(const_cast<char*>((testDir + "/bb2.tif").c_str()), bb2);
    }

    tic();
    Stack_Dilate_Fast(bb2, bb3, se);
    ptoc();

    if (ZArgumentProcessor::isArgMatched("--golden")) {
      Write_Stack(const_cast<char*>((goldenDir + "/bb3.tif").c_str()), bb3);
    } 

    if (ZArgumentProcessor::isArgMatched("--test")) {
      Write_Stack(const_cast<char*>((testDir + "/bb3.tif").c_str()), bb3);
    }

    tic();
    cout << "Finding objects ..." << endl;
    Object_3d_List *objList = Stack_Find_Object_N(bb3, NULL, 1, 0, 6);
    ptoc();

    if (ZArgumentProcessor::isArgMatched("--golden")) {
      Object_3d_List_Export_Csv(const_cast<char*>((goldenDir + "/objlist.csv").c_str()), objList);
    } 

    if (ZArgumentProcessor::isArgMatched("--test")) {
      Object_3d_List_Export_Csv(const_cast<char*>((testDir + "/objlist.csv").c_str()), objList);
    }

    ZObject3dArray objArray;
    Object_3d_List *head = objList;
    cout << "split objects ..." << endl;
    tic();
    if (opts.clusterCC) {
      while (objList != NULL) {
        vector<int> labelArray =
          apply_clustering(objList->data, detected1,
              opts.max_cluster_sz);
        objArray.append(objList->data, labelArray);
        //cout << "Object number: " << objArray.size() << endl;
        objList = objList->next;
      }
      Kill_Object_3d_List(head);
    } else {
      objArray.append(objList);
    }
    ptoc();

    cout << objArray.size() << " objects" << endl;

    if (ZArgumentProcessor::isArgMatched("--golden")) {
      objArray.exportCsvFile((goldenDir + "/split_objlist.csv").c_str());
    } 

    if (ZArgumentProcessor::isArgMatched("--test")) {
      objArray.exportCsvFile((testDir + "/split_objlist.csv").c_str());
    }

    cout << "Computer centers ..." << endl;
    ZObject3d centerArray;

    int nselected = 0;
    for (size_t i = 0; i < objArray.size(); i++) {
      if (objArray[i]->size() >= min_sz) {
        nselected++;
        //objArray[i]->translate(-1, -1, -1);

        ZPoint center = objArray[i]->computeCentroid(detected1);
        int x = floor(center.x()) + 1;
        int y = floor(center.y()) + 1;
        int z = floor(center.z()) + 1;

        if (z >= opts.offset &&
            z <= (detected1->dim[2] - opts.offset) &&
            y >= opts.offset &&
            y <= (detected1->dim[1] - opts.offset)) {
          centerArray.append(x, y, z);
        }
      }
    }
    cout << centerArray.size() << " centers calculated." << endl;

    if (ZArgumentProcessor::isArgMatched("--golden")) {
      centerArray.exportCsvFile((goldenDir + "/center.csv").c_str());
    } 

    if (ZArgumentProcessor::isArgMatched("--test")) {
      centerArray.exportCsvFile((testDir + "/center.csv").c_str());
    }

    cout << "Merge centers ..." << endl;

    vector<size_t> indexArray = centerArray.toIndexArray(detected1->dim[0],
                                                         detected1->dim[1],
                                                         detected1->dim[2],
                                                         -1, -1, -1);

    vector<float> weightArray(indexArray.size());
    for (int i = 0; i < indexArray.size(); i++) {
      weightArray[i] = detected1->array[indexArray[i]];
    }

    vector<int> rankArray(indexArray.size());
    farray_qsort(&(weightArray[0]), &(rankArray[0]), weightArray.size());

    vector<bool> merged(centerArray.size(), false);
    int nmerged = 0;
    for (int i = centerArray.size() - 1; i >= 0; i--) {
      if (!merged[rankArray[i]]) {
        for (int j = 0; j < centerArray.size(); j++) {
          if (!merged[j] && j != rankArray[i]) {
            int dx = centerArray.x(rankArray[i]) - centerArray.x(j);
            int dy = centerArray.y(rankArray[i]) - centerArray.y(j);
            int dz = centerArray.z(rankArray[i]) - centerArray.z(j);
            int d = dx * dx + dy * dy + dz * dz;
            if (sqrt(d) < opts.minDistCtrs) {
              merged[j] = true;
              nmerged++;
            }
          }
        }
      }
    }

    //centerArray.exportSwcFile("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
    cout << nmerged << " merged" << endl;

    ///test///

    if (ZArgumentProcessor::isArgMatched("--golden")) {
      ofstream stream((goldenDir + "/merged_center.csv").c_str());

      for (size_t i = 0; i < centerArray.size(); i++) {
        if (!merged[i]) {
          stream << centerArray.x(i) - 1 << "," << centerArray.y(i) - 1 << ","
            << centerArray.z(i) - 1 << endl;
        }
      }

      stream.close();
    } 

    if (ZArgumentProcessor::isArgMatched("--test")) {
      ofstream stream((testDir + "/merged_center.csv").c_str());

      for (size_t i = 0; i < centerArray.size(); i++) {
        if (!merged[i]) {
          stream << centerArray.x(i) - 1 << "," << centerArray.y(i) - 1 << ","
            << centerArray.z(i) - 1 << endl;
        }
      }

      stream.close();
    } 

    ofstream stream("/Users/zhaot/Work/neutube/neurolabi/data/test2.swc");

    for (size_t i = 0; i < centerArray.size(); i++) {
      if (!merged[i]) {
        stream << i + 1 << " " << 2 << " "
               << centerArray.x(i) - 1 << " " << centerArray.y(i) - 1 << " "
               << centerArray.z(i) - 1
               << " " << 3.0 << " " << -1 << endl;
      }
    }

    stream.close();


    //////////

    cout << "Writing results to "
         << ZArgumentProcessor::getStringArg("--jsonoutputfile")
         << "..." << endl;

    FILE *jfid = fopen(ZArgumentProcessor::getStringArg(("--jsonoutputfile")),
                     "w");
    fprintf(jfid,"{\n  \"data\": [\n");
    bool tbarcheck = false;
    for (int i = 0; i < centerArray.size(); i++) {
      if (!merged[i] /*|| kept[i]*/) {
        int mx = centerArray.z(i);
        int my = centerArray.y(i);
        int mz = centerArray.x(i);

        if (tbarcheck) {
          fprintf(jfid, ",\n    { \n");
        } else {
          fprintf(jfid, "    { \n");
        }

        fprintf(jfid,"\t \"T-bar\": { \n");
        fprintf(jfid,"\t\t\"status\": \"working\", \n");
        fprintf(jfid,"\t\t\"confidence\": 1.0, \n");
        fprintf(jfid,"\t\t\"body ID\": -1, \n");
        fprintf(jfid,"\t\t\"location\": [ \n");
        fprintf(jfid,"\t\t   %d, \n", mx);
        fprintf(jfid,"\t\t   %d, \n", detected1->dim[1] - my);
        fprintf(jfid,"\t\t   %d  \n", mz);
        fprintf(jfid,"\t\t ] \n");
        fprintf(jfid,"\t }, \n");
        fprintf(jfid,"\t\"partners\": [  ]\n");
        fprintf(jfid,"    } ");

        tbarcheck=true;
      }
    }

    fprintf(jfid,"\n  ],\n");
    fprintf(jfid,"  \"metadata\": {\n");
    fprintf(jfid,"\t  \"username\": \"dummy\",\n");
    fprintf(jfid,"\t  \"software version\": \"dummy\",\n");
    fprintf(jfid,"\t  \"description\": \"synapse annotations\",\n");
    fprintf(jfid,"\t  \"file version\": 1, \n");
    fprintf(jfid,"\t  \"software version\": \"dummy\",\n");
    fprintf(jfid,"\t  \"computer\": \"dummy\",\n");
    fprintf(jfid,"\t  \"date\": \"dummy\",\n");
    fprintf(jfid,"\t  \"session path\": \"dummy\",\n");
    fprintf(jfid,"\t  \"software\": \"dummy\"\n");
    fprintf(jfid,"  }\n}");

    fclose(jfid);
  }
#endif


#ifdef _TEST_
  cout << "Testing hdf5 now ... " << endl;

  hid_t file_id;
  herr_t status;

  file_id = H5Fcreate("../../../data/test.h5", H5F_ACC_TRUNC, H5P_DEFAULT,
      H5P_DEFAULT);

  status = H5Fclose(file_id);

  hid_t dataset_id, dataspace_id;
  hsize_t dims[2];

  file_id = H5Fcreate("../../../data/test.h5", H5F_ACC_TRUNC, H5P_DEFAULT,
      H5P_DEFAULT);
  
  dims[0] = 4;
  dims[1] = 6;
  dataspace_id = H5Screate_simple(2, dims, NULL);

  dataset_id = H5Dcreate(file_id, "/dset", H5T_STD_I32BE, dataspace_id,
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  status = H5Dclose(dataset_id);
  status = H5Sclose(dataspace_id);
  status = H5Fclose(file_id);

  int i, j, dset_data[4][6];
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 6; j++) {
      dset_data[i][j] = i * 6 + j + 1;
    }
  }

  file_id = H5Fopen("../../../data/test.h5", H5F_ACC_RDWR, H5P_DEFAULT);

  dataset_id = H5Dopen(file_id, "/dset", H5P_DEFAULT);

  status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
      dset_data);
  status = H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
      dset_data);

  status = H5Dclose(dataset_id);
  status = H5Fclose(file_id);

  dims[0] = 2;
  dims[1] = 3;
  int data[6] = {1,2,3,4,5,6};

  file_id = H5Fcreate("../../../data/test.h5", H5F_ACC_TRUNC, H5P_DEFAULT,
      H5P_DEFAULT);

  status = H5LTmake_dataset(file_id, "/dset", 2, dims, H5T_NATIVE_INT, data);

  status = H5Fclose(file_id);

  size_t nrow, n_values;

  file_id = H5Fopen("../../../data/test.h5", H5F_ACC_RDONLY, H5P_DEFAULT);
  status = H5LTread_dataset_int(file_id, "/dset", data);

  status = H5LTget_dataset_info(file_id, "/dset", dims, NULL, NULL);
  n_values = (size_t) (dims[0] * dims[1]);
  nrow = (size_t) dims[1];

  cout << n_values << " " << nrow << endl;
  for (i = 0; i < n_values / nrow; i++) {
    for (j = 0; j < nrow; j++) {
      printf(" %d", data[i * nrow + j]);
    }
    printf("\n");
  }
  status = H5Fclose(file_id);

  hid_t group_id;
  file_id = H5Fcreate("../../../data/test.h5", H5F_ACC_TRUNC, H5P_DEFAULT,
      H5P_DEFAULT);
  group_id = H5Gcreate(file_id, "/MyGroup", H5P_DEFAULT, H5P_DEFAULT,
      H5P_DEFAULT);
  status = H5Gclose(group_id);
  status = H5Fclose(file_id);

  hid_t group1_id, group2_id, group3_id;

  file_id = H5Fcreate("../../../data/test.h5", H5F_ACC_TRUNC, H5P_DEFAULT,
      H5P_DEFAULT);

  group1_id = H5Gcreate(file_id, "/MyGroup", H5P_DEFAULT, H5P_DEFAULT,
      H5P_DEFAULT);
  group2_id = H5Gcreate(file_id, "/MyGroup/Group_A", H5P_DEFAULT, H5P_DEFAULT,
      H5P_DEFAULT);

  group3_id = H5Gcreate(group1_id, "Group_B", H5P_DEFAULT, H5P_DEFAULT, 
      H5P_DEFAULT);

  status = H5Gclose(group1_id);
  status = H5Gclose(group2_id);
  status = H5Gclose(group3_id);

  status = H5Fclose(file_id);

  int dset1_data[3][3], dset2_data[2][10];
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      dset1_data[i][j] = j + 1;
    }
  }

  for (i = 0; i < 2; i++) {
    for (j = 0; j < 10; j++) {
      dset2_data[i][j] = j + 1;
    }
  }

  file_id = H5Fopen("../../../data/test.h5", H5F_ACC_RDWR, H5P_DEFAULT);

  dims[0] = 3;
  dims[1] = 3;
  dataspace_id = H5Screate_simple(2, dims, NULL);

  dataset_id = H5Dcreate(file_id, "/MyGroup/dset1", H5T_STD_I32BE, dataspace_id,
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
      dset1_data);

  status = H5Sclose(dataspace_id);
  status = H5Dclose(dataset_id);

  group_id = H5Gopen(file_id, "/MyGroup/Group_A", H5P_DEFAULT);

  dims[0] = 2;
  dims[1] = 10;
  dataspace_id = H5Screate_simple(2, dims, NULL);

  dataset_id = H5Dcreate(group_id, "dset2", H5T_STD_I32BE, dataspace_id,
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
      dset2_data);

  status = H5Sclose(dataspace_id);
  status = H5Gclose(group_id);
  status = H5Fclose(file_id);

  cout << "Done!" << endl;
#endif

  return 0;
}
