#include <iostream>
#include <sstream>
#include <fstream>

#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tz_utilities.h"
#include "zsegmentmaparray.h"
#include "zsuperpixelmaparray.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_stack_attribute.h"
#include "tz_stack_document.h"
#include "tz_xml_utils.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_objlabel.h"
#include "tz_swc_tree.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "tz_sp_grow.h"
#include "zspgrowparser.h"
#include "tz_stack_stat.h"
#include "tz_intpair_map.h"
#include "tz_stack_utils.h"
#include "zfilelist.h"
#include "zstring.h"
#include "tz_imatrix.h"
#include "zobject3dscan.h"
#include "zgraph.h"

using namespace std;

static void map_plane_body(Stack *stack, Intpair_Map *superpixelBodyMap,
    Stack *out, Stack *out2)
{
  size_t nvoxel = Stack_Voxel_Number(stack);

  switch (stack->kind) {
    case GREY16:
      {
        uint16_t *array16 = (uint16_t*) stack->array;
        uint16_t *out_array16 = (uint16_t*) out->array;
        for (size_t k = 0; k < nvoxel; k++) {
          out_array16[k] = (uint16_t) Intpair_Map_Value(superpixelBodyMap,
              0, array16[k]);
        }
      }
      break;
    case COLOR:
      if (out->kind == COLOR) {
        color_t *arrayc = (color_t*) stack->array;
        color_t *out_arrayc = (color_t*) out->array;
        for (size_t k = 0; k < nvoxel; k++) {
          int value = Intpair_Map_Value(superpixelBodyMap, 0, 
              Color_To_Value(arrayc[k]));
          if (value < 0) {
            out_arrayc[k][0] = 0;
            out_arrayc[k][1] = 0;
            out_arrayc[k][2] = 0;
            out2->array[k] = 0;
          } else {
            out2->array[k] = 
              Value_To_Color(value, out_arrayc[k]);
          }
        }
      } else if (out->kind == GREY16) {
        color_t *arrayc = (color_t*) stack->array;
        uint16_t *out_array16 = (uint16_t*) out->array;
        for (size_t k = 0; k < nvoxel; k++) {
          int value = Intpair_Map_Value(superpixelBodyMap, 0, 
              Color_To_Value(arrayc[k]));
          if (value >= 0) {
            out_array16[k] = value;
          } else {
            out_array16[k] = 0;
          }
        }
      }
      break;
    default:
      break;
  }
}

static void map_plane_body_i(Stack *stack, Intpair_Map *superpixelBodyMap,
    IMatrix *out)
{
  size_t nvoxel = Stack_Voxel_Number(stack);

  Image_Array ima;
  ima.array = stack->array;
  switch (stack->kind) {
    case GREY16:
      for (size_t k = 0; k < nvoxel; ++k) {
        out->array[k] = Intpair_Map_Value(superpixelBodyMap, 0, ima.array16[k]);

        if (out->array[k] < 0) {
          std::cerr << "WARNING: Negative body id" << std::endl;
        }
      }
    break;
    case COLOR:
    {
      //color_t *arrayc = (color_t*) stack->array;
      for (size_t k = 0; k < nvoxel; k++) {
        out->array[k] = Intpair_Map_Value(superpixelBodyMap, 0, 
            Color_To_Value(ima.arrayc[k]));
        if (out->array[k] < 0) {
          std::cerr << "WARNING: Negative body id" << std::endl;
          std::cout << "Superpixel may not be mapped: "
                    << Color_To_Value(ima.arrayc[k]) << std::endl;
        }
      }
    }
      break;
    default:
      break;
  }
}

static ZString get_sobj_path(const std::string &bodyDir, int bodyId)
{
  ZString stackedObjPath = bodyDir + "/";
  stackedObjPath.appendNumber(bodyId);
  stackedObjPath += ".sobj";

  return stackedObjPath;
}

static set<int> load_body_set(const std::string &bodyFile)
{
  set<int> bodySet;

  ZString line;
  FILE *fp = fopen(bodyFile.c_str(), "r");
  while (line.readLine(fp)) {
    vector<int> idArray = line.toIntegerArray();
    if (!idArray.empty()) {
      /*if (idArray[0] < 0) {
        std::cout << "Invalid id: " << line << std::endl;
      } else*/ {
        bodySet.insert(idArray.begin(), idArray.end());
      }
    }
  }

  fclose(fp);

  return bodySet;
}

static void compare_body(const IMatrix *imat1, const IMatrix *imat2,
                         ZGraph *graph)
{
  int width = imat1->dim[0];
  int height = imat1->dim[1];
  int depth = imat1->dim[2];

  size_t volume = (size_t) width * height * depth;

  for (size_t i = 0; i < volume; ++i) {
    int v1 = imat1->array[i];
    int v2 = imat2->array[i];
    if (v1 >= 0 && v2 >= 0) {
      if (v1 != v2) {
        graph->addEdge(v1, v2);
      }
    }
  }
}

#define CC const_cast<char*>

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.4") == 1) {
    return 0;
  }

  static char const *Spec[] = {
    "<input:string> -o <string>", "[--body_map <string> | --sobj]",
    "[--compare_body <string>]",
    "[--body_list <string>]",
    "[--intv <int> <int> <int>]",
    "[--cluster <int> --script <string> --cmd_dir <string>]",
    "[--range <int> <int>]", "[--output_format <string>]",
    "[--stacked_dir <string>]", "[--bodysize_file <string>]",
    "[--minsize <int(10000000)>] [--z_offset <int(0)>]",
    "[--maxsize <int>] [--overwrite_level <int(0)>] [--append]", NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  char *dataDir = Get_String_Arg(const_cast<char*>("input"));

  int overwriteLevel = Get_Int_Arg(const_cast<char*>("--overwrite_level"));

  int zOffset = Get_Int_Arg(const_cast<char*>("--z_offset"));

  int zStart = 0;
  int zEnd = 0;

  if (Is_Arg_Matched(CC("--range"))) {
    zStart = Get_Int_Arg(CC("--range"), 1);
    zEnd = Get_Int_Arg(CC("--range"), 2);
  } else {
    ZFileList fileList;
    if (Is_Arg_Matched("--body_map")) {
      fileList.load(std::string(dataDir) + "/superpixel_maps", "png",
                    ZFileList::SORT_BY_LAST_NUMBER);
    } else {
      fileList.load(dataDir, "imat", ZFileList::SORT_BY_LAST_NUMBER);
    }
    zStart = String_Last_Integer(fileList.getFilePath(0));
    zEnd = String_Last_Integer(fileList.getFilePath(fileList.size() - 1));
  }

  if (Is_Arg_Matched(const_cast<char*>("--compare_body"))) {
    cout << "Comparing bodies ..." << endl;

    ZGraph *graph = new ZGraph(ZGraph::DIRECTED_WITH_WEIGHT);
    for (int z = zStart; z <= zEnd; ++z) {
      cout << "Z " << z << endl;

      ZString filePath = ZString(dataDir) + "/body_map";
      filePath.appendNumber(z, 5);
      filePath = filePath + ".imat";

      if (fexist(filePath.c_str())) {
#if 0
        ZString objDir = bodyDir + "/";
        objDir.appendNumber(z, 5);

        if (!dexist(objDir.c_str())) {
          mkdir(objDir.c_str(), 0755);
        }
#endif

        cout << "Loading " << filePath << endl;

        IMatrix *imat1 = IMatrix_Read(filePath.c_str());


        ZString compareFilePath =
            ZString(Get_String_Arg(const_cast<char*>("--compare_body"))) +
            "/body_map";
        compareFilePath.appendNumber(z, 5);
        compareFilePath = compareFilePath + ".imat";
        if (fexist(compareFilePath.c_str())) {
          IMatrix *imat2 = IMatrix_Read(compareFilePath.c_str());
          compare_body(imat1, imat2, graph);
          Kill_IMatrix(imat2);
        } else {
          cout << "File not found: " << compareFilePath << endl;
        }
        Kill_IMatrix(imat1);
      } else {
        cout << "File not found: " << filePath << endl;
      }

      graph->exportTxtFile(Get_String_Arg(const_cast<char*>("-o")));
    }
    return 0;
  }

  //Generate sparse objects
  if (Is_Arg_Matched(const_cast<char*>("--sobj"))) {
    ZString bodyDir = Get_String_Arg(CC("-o"));

    set<int> bodyIdSet;
    vector<pair<int, int> > zRange;

    vector<map<int, ZObject3dScan*> > bodySetArray;

    bool bodySetSpecified = Is_Arg_Matched(CC("--body_list"));

    for (int z = zStart; z <= zEnd; ++z) {
      map<int, ZObject3dScan*> bodySet;
      ZString filePath = ZString(dataDir) + "/body_map";
      filePath.appendNumber(z, 5);
      filePath = filePath + ".imat";

      if (fexist(filePath.c_str())) {
#if 0
        ZString objDir = bodyDir + "/";
        objDir.appendNumber(z, 5);

        if (!dexist(objDir.c_str())) {
          mkdir(objDir.c_str(), 0755);
        }
#endif

        cout << "Loading " << filePath << endl;

        IMatrix *mat = IMatrix_Read(filePath.c_str());
        int width = mat->dim[0];
        int height = mat->dim[1];
        int depth = mat->dim[2];

        cout << "extracting bodies" << endl;
        tic();
        ZObject3dScan::extractAllObject(mat->array, width, height, depth,
                                        z + zOffset - zStart, &bodySet);
        Kill_IMatrix(mat);
        ptoc();

#if _DEBUG_
        for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          if (iter->second == NULL) {
            cout << "null object" << endl;
          }
        }
#endif

#if 0
        cout << "Saving bodies ..." << endl;
        tic();
        for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          ZString filePath = objDir + "/";
          filePath.appendNumber(iter->first, 0);
          filePath += ".sobj";

          if (overwriteLevel == 0 || !fexist(filePath.c_str())) {
            iter->second->save(filePath);
          }
#ifdef _DEBUG_2
          if (!iter->second->isCanonizedActually()) {
            cout << "Invalid object" << endl;
            return 1;
          }
#endif
        }
        ptoc();
#endif
        if (!bodySetSpecified) {
          for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
               iter != bodySet.end(); ++iter) {
            //delete iter->second;
            //iter->second = NULL;
            if (iter->first > 0) {//ignore id 0
              bodyIdSet.insert(iter->first);
            }
          }
        }
        //bodySet.clear();

        cout << bodySet.size() << " bodies found" << endl;
#ifdef _DEBUG_2
        cout << bodySet[0] << endl;
#endif
        bodySetArray.push_back(bodySet);
        bodySet.clear();

#ifdef _DEBUG_2
        cout << bodySetArray[0].size() << endl;
        cout << bodySetArray[0][0] << endl;
#endif

        /*
          ZFileList fileList;
          fileList.load(objDir, "sobj");
          for (int i = 0; i < fileList.size(); ++i) {
            int id = String_Last_Integer(fileList.getFilePath(i));
            if (id > 0) {
              bodyIdSet.insert(bodyIdSet.end(), id);
            }
          }
          */
      }
    }

    if (bodySetSpecified) {
      bodyIdSet = load_body_set(Get_String_Arg(CC("--body_list")));
    }

    std::string stackedDir = "stacked";

    if (Is_Arg_Matched(CC("--stacked_dir"))) {
      stackedDir = Get_String_Arg(CC("--stacked_dir"));
    }

    ZString fullBodyDir = bodyDir + "/" + stackedDir;
    if (!dexist(fullBodyDir.c_str())) {
      mkdir(fullBodyDir.c_str(), 0755);
    }

    vector<std::pair<size_t, int> > objSizeArray;
    cout << "Creating stacked objects ..." << endl;
    size_t index = 0;
    for (set<int>::const_iterator iter = bodyIdSet.begin(); iter != bodyIdSet.end();
         ++iter, ++index) {
      int bodyId = *iter;
      cout << index + 1 << "/" << bodyIdSet.size() << ": " << bodyId << endl;
      ZObject3dScan obj;
      for (int z = zStart; z < zEnd; ++z) {
#if 0
        ZString objPath = bodyDir + "/";
        objPath.appendNumber(z, 5);
        objPath += "/";
        objPath.appendNumber(bodyId);
        objPath += ".sobj";
        ZObject3dScan objSlice;
        if (fexist(objPath.c_str())) {
          if (objSlice.load(objPath)) {
            obj.concat(objSlice);
          }
        }
#endif
        if (bodySetArray[z - zStart].count(bodyId) > 0) {
#ifdef _DEBUG_2
          cout << bodySetArray[z - zStart].size() << endl;
          cout << bodySetArray[z - zStart][bodyId] << endl;
          cout << bodySetArray[0][0] << endl;
#endif
          obj.concat(*(bodySetArray[z - zStart][bodyId]));
        }
      }

      std::pair<size_t, int> objSize;
      objSize.first = obj.getVoxelNumber();
      objSize.second = bodyId;


      objSizeArray.push_back(objSize);

      if (objSize.first > (size_t) Get_Int_Arg(CC("--minsize"))) {
        bool isTooBig = false;
        if (Is_Arg_Matched(CC("--maxsize"))) {
          if (objSize.first > (size_t) Get_Int_Arg(CC("--maxsize"))) {
            isTooBig = true;
          }
        }

        if (!isTooBig) {
          ZString stackedObjPath = get_sobj_path(fullBodyDir, bodyId);
          /*
        ZString stackedObjPath = fullBodyDir + "/";
        stackedObjPath.appendNumber(bodyId);
        stackedObjPath += ".sobj";
        */
          obj.save(stackedObjPath);
        }
      }
    }

    const char *bodyFile = NULL;

    if (Is_Arg_Matched(const_cast<char*>("--bodysize_file"))) {
      bodyFile = Get_String_Arg(const_cast<char*>("--bodysize_file"));
    }

    if (bodyFile != NULL) {
      std::cout << "Creating bodysize file ..." << std::endl;
      sort(objSizeArray.begin(), objSizeArray.end());
      ofstream stream(bodyFile);
      for (vector<std::pair<size_t, int> >::const_reverse_iterator
           iter = objSizeArray.rbegin(); iter != objSizeArray.rend(); ++iter) {
        int bodyId = iter->second;
        stream << bodyId << ", " << iter->first << endl;
      }
      stream.close();
    }

#if 0
    stream.open((bodyDir +"/bodylist.txt").c_str());

    for (vector<std::pair<size_t, int> >::const_reverse_iterator
         iter = objSizeArray.rbegin(); iter != objSizeArray.rend(); ++iter) {
      int bodyId = iter->second;
      ZObject3dScan obj;
      obj.load(get_sobj_path(fullBodyDir, bodyId));
      //ZCuboid cuboid = obj.getBoundBox();
      //if (cuboid.volume() <= 2000000000) {
      vector<size_t> sizeArray = obj.getConnectedObjectSize();
      if (!sizeArray.empty()) {
        stream << bodyId << ", " << iter->first << ", " << sizeArray.size()
               << ", " << sizeArray[0] << ", ";
        if (sizeArray.size() > 1) {
          stream << sizeArray[1];
        } else {
          stream << 0;
        }
        stream << endl;
      }
      //}

      //stream << iter->second << ", " << iter->first << endl;
    }

    stream.close();
#endif

    return 0;
  }

  //Get the plane range of the stacks
  ZFileList fileList;
  fileList.load(string(dataDir) + "/superpixel_maps", "png", 
      ZFileList::SORT_BY_LAST_NUMBER);
  int startPlane = fileList.startNumber();
  int endPlane = fileList.endNumber();
  int planeNumber = endPlane - startPlane + 1;

  char filePath[500];
  //Load compress map
  sprintf(filePath, "%s/body_compress_map.txt", 
      Get_String_Arg(const_cast<char*>("--body_map")));
  bool compressed = fexist(filePath);
  map<int, int> bodyIdDict;
  if (compressed) {
    FILE *fp = fopen(filePath, "r");
    ZString str;
    while (str.readLine(fp)) {
      vector<int> value = str.toIntegerArray();
      if (value.size() == 2) {
        bodyIdDict[value[0]] = value[1];
      }
    }
    fclose(fp);
  }

  if (Is_Arg_Matched(const_cast<char*>("--cluster"))) {
    FILE *fp = fopen(Get_String_Arg(const_cast<char*>("--script")), "w");
    int planeNumberPerJob = Get_Int_Arg(const_cast<char*>("--cluster"));
    int currentIndex = 0;
    while (currentIndex < planeNumber) {
      if (Is_Arg_Matched(const_cast<char*>("--intv"))) {
        fprintf(fp, "qsub -N map_body -j y -o /dev/null -b y -cwd -V '%s/map_body %s -o %s --body_map %s --intv %d %d %d --range %d %d > %s/map_body_%d_%d.txt'\n",
            Get_String_Arg(const_cast<char*>("--cmd_dir")),
            Get_String_Arg(const_cast<char*>("input")),
            Get_String_Arg(const_cast<char*>("-o")),
            Get_String_Arg(const_cast<char*>("--body_map")),
            Get_Int_Arg(const_cast<char*>("--intv"), 1),
            Get_Int_Arg(const_cast<char*>("--intv"), 2),
            Get_Int_Arg(const_cast<char*>("--intv"), 3),
            currentIndex, currentIndex + planeNumberPerJob - 1,
            Get_String_Arg(const_cast<char*>("-o")),
            currentIndex, currentIndex + planeNumberPerJob - 1);
        currentIndex += planeNumberPerJob;
      } else {
        fprintf(fp, "qsub -N map_body -j y -o /dev/null -b y -cwd -V '%s/map_body %s -o %s --body_map %s --range %d %d > %s/map_body_%d_%d.txt'\n",
            Get_String_Arg(const_cast<char*>("--cmd_dir")),
            Get_String_Arg(const_cast<char*>("input")),
            Get_String_Arg(const_cast<char*>("-o")),
            Get_String_Arg(const_cast<char*>("--body_map")),
            currentIndex, currentIndex + planeNumberPerJob - 1,
            Get_String_Arg(const_cast<char*>("-o")),
            currentIndex, currentIndex + planeNumberPerJob - 1);
        currentIndex += planeNumberPerJob;
      }
    }

    fclose(fp);

    return 0;
  }

  int startPlaneIndex = 0;
  int endPlaneIndex = planeNumber - 1;

  if (Is_Arg_Matched(const_cast<char*>("--range"))) {
    startPlaneIndex = imax2(0, Get_Int_Arg(const_cast<char*>("--range"), 1));
    endPlaneIndex = imin2(endPlaneIndex, 
        Get_Int_Arg(const_cast<char*>("--range"), 2));
  }

  for (int i = startPlaneIndex; i <= endPlaneIndex; i++) {
    int planeId = startPlane + i;
    cout << planeId << endl;

    ZSuperpixelMapArray superpixelMapArray;
    sprintf(filePath, "%s/superpixel_to_body_map%05d.txt", 
        Get_String_Arg(const_cast<char*>("--body_map")), planeId);
    superpixelMapArray.load(filePath, planeId);

    Intpair_Map *superpixelBodyMap = Make_Intpair_Map(100000);
    for (size_t j = 0; j < superpixelMapArray.size(); j++) {
      if (compressed) {
        Intpair_Map_Add(superpixelBodyMap, 0, 
            superpixelMapArray[j].superpixelId(), 
            bodyIdDict[superpixelMapArray[j].bodyId()]);
      } else {
        Intpair_Map_Add(superpixelBodyMap, 0, 
            superpixelMapArray[j].superpixelId(), 
            superpixelMapArray[j].bodyId());
      }
    }
    
    cout << "Read image plane..." << endl;
    Stack *stack = Read_Stack_U(fileList.getFilePath(i));

    if (stack == NULL) {
      cout << "Failed to read " << fileList.getFilePath(i) << endl;
    }

    Stack *ds_stack = stack;
    if (Is_Arg_Matched(const_cast<char*>("--intv"))) {
      cout << "Downsampling ..." << endl;
      ds_stack = Downsample_Stack(stack, 
          Get_Int_Arg(const_cast<char*>("--intv"), 1),
          Get_Int_Arg(const_cast<char*>("--intv"), 2), 
          Get_Int_Arg(const_cast<char*>("--intv"), 3));
    }

    cout << "Preparing output ..." << endl;

    string outputFormat = "tif";
    if (Is_Arg_Matched(const_cast<char*>("--output_format"))) {
      outputFormat = Get_String_Arg(const_cast<char*>("--output_format"));
    }

    if (outputFormat == "tif") {
      Stack *out = NULL;
      Stack *out2 = NULL;
      if (compressed) {
        cout << "Compressed. " << ds_stack->width << " x " << ds_stack->height
          << endl;
        out = Make_Stack(GREY16, ds_stack->width, ds_stack->height, 1);
      } else {
        cout << ds_stack->width << " x " << ds_stack->height
          << endl;
        out = Make_Stack(COLOR, ds_stack->width, ds_stack->height, 1);
        out2 = Make_Stack(GREY, ds_stack->width, ds_stack->height, 1);
      }

      cout << "Mapping ..." << endl;
      map_plane_body(ds_stack, superpixelBodyMap, out, out2);
      sprintf(filePath, "%s/body_map%05d.tif", 
          Get_String_Arg(const_cast<char*>("-o")), planeId);
      Write_Stack(filePath, out);
      if (out2 != NULL) {
        sprintf(filePath, "%s/a/body_map%05d.tif", 
            Get_String_Arg(const_cast<char*>("-o")), planeId);
        Write_Stack(filePath, out2);
        Free_Stack(out2);
      }

      Free_Stack(out);
    } else if (outputFormat == "imat") {
      IMatrix *out = Make_3d_IMatrix(ds_stack->width, ds_stack->height, 1);
      cout << "Mapping ..." << endl;
      map_plane_body_i(ds_stack, superpixelBodyMap, out);
      sprintf(filePath, "%s/body_map%05d.imat", 
          Get_String_Arg(const_cast<char*>("-o")), planeId);
      IMatrix_Write(filePath, out);
      
      Free_IMatrix(out);
    }

    Free_Stack(stack);
    if (ds_stack != stack) {
      Free_Stack(ds_stack);
    }
    Kill_Intpair_Map(superpixelBodyMap);
  }

  printf("map_body succeeded\n");

  return 0;
}
