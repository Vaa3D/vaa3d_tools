/* SomaDetect_plugin.cpp
 * SomaDetect algorithm for .
 * 2017-6-2 : 
 */

#include "SomaDetect_plugin.h"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"
#include "rivulet.h"
#include "stackutil.h"
#include "v3d_message.h"
#include "vn_rivulet.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include <fstream>

Q_EXPORT_PLUGIN2(Rivulet2, RivuletPlugin);

using namespace rivulet;

struct input_PARA {
  QString inimg_file;
  QString outswc_file;
  QString outsomaimg_file;
  V3DLONG channel;
  unsigned char threshold;
  bool quality;
  bool prune;
};

void save_swc(SWC* swc, QString fname);
void show_logo();
void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent,
                         input_PARA &PARA, bool bmenu);

QStringList RivuletPlugin::menulist() const {
  return QStringList() << tr("tracing") << tr("about");
}

QStringList RivuletPlugin::funclist() const {
  return QStringList() << tr("tracing_func") << tr("help");
}

void RivuletPlugin::domenu(const QString &menu_name,
                           V3DPluginCallback2 &callback, QWidget *parent) {
  cout<<"in domenu"<<endl;
  if (menu_name == tr("tracing")) {
    bool bmenu = true;
    input_PARA PARA;

    if (callback.getImageWindowList().empty()) {
      v3d_msg("Oops... No image opened in V3D...");
      return;
    }
    PARA_RIVULET p;

    // fetch parameters from dialog
    if (!p.rivulet_dialog())
      return;
    PARA.channel = p.channel;
    PARA.threshold = p.threshold;
    PARA.quality = p.quality;

    reconstruction_func(callback, parent, PARA, bmenu);
    v3d_msg(tr("Soma detection prototype is completed. The soma swc file containing one single point is generated."));

  } else if (menu_name == tr("about")){
    v3d_msg(tr("Soma Detection algorithm for Hackathon Nanjing by USYD team. You can also find the matlab package at https://github.com/RivuletStudio/Rivulet-Neuron-Tracing-Toolbox and the python3 package at https://github.com/lsqshr/rivuletpy"));
  }
}

bool RivuletPlugin::dofunc(const QString &func_name,
                           const V3DPluginArgList &input,
                           V3DPluginArgList &output,
                           V3DPluginCallback2 &callback, QWidget *parent) {
  if (func_name == tr("tracing_func")) {
    bool bmenu = false;
    input_PARA PARA;

    vector<char *> *pinfiles =
        (input.size() >= 1) ? (vector<char *> *)input[0].p : 0;
    vector<char *> *poutfiles =
        (output.size() >= 1) ? (vector<char *> *)output[0].p : 0;
    vector<char *> *pparas =
        (input.size() >= 2) ? (vector<char *> *)input[1].p : 0;
    vector<char *> infiles = (pinfiles != 0) ? *pinfiles : vector<char *>();
    vector<char *> outfiles = (poutfiles != 0) ? *poutfiles : vector<char *>();
    vector<char *> paras = (pparas != 0) ? *pparas : vector<char *>();

    if (infiles.empty()) {
      fprintf(stderr, "Need input image. \n");
      return false;
    } else
      PARA.inimg_file = infiles[0];
    int k = 0;
    PARA.channel = (paras.size() >= k + 1) ? atoi(paras[k]) : 1;
    k++;
    PARA.threshold = (paras.size() >= k + 1) ? atoi(paras[k]) : 0;
    k++;
    PARA.quality = (paras.size() >= k + 1) ? atoi(paras[k]) : 0;
    k++;
    PARA.prune = (paras.size() >= k + 1) ? atoi(paras[k]) : 0;
    k++;
    if (!outfiles.empty())
      PARA.outswc_file = outfiles[0];
    else{
      PARA.outswc_file = PARA.inimg_file + ".r2.swc";
    }
    reconstruction_func(callback, parent, PARA, bmenu);
  } else if (func_name == tr("help")) {
    printf("====== Usage of Rivulet tracing ====== \n");
    printf(
        "vaa3d -x Rivulet -f tracing_func -i your_image.v3draw [-o out.swc] -p "
        "channel threshold quality prune\n");
    printf("channel\tData channel for tracing. Start from 1 (default 1).\n");
    printf("threshold\tThe background threshold for segmentation (default 0).\n");
    // printf("quality\tEnhance the quality of tracing, which is more time consuming. If it is set 1, the second derivatives and cross neighbours will be used in multi-stencils fast-marching. (default 0).\n");
    // printf("prune\tPrune the result swc by elliminating the unreachable nodes from the soma. It also prune short branches less than 5 voxels. (default 0).\n");
    printf("==================================\n");
  } else {
    return false;
  }

  return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent,
                         input_PARA &PARA, bool bmenu) {
  unsigned char *data1d = 0;
  V3DLONG N, M, P, sc, c;
  V3DLONG in_sz[4];
  if (bmenu) {
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin) {
      QMessageBox::information(
          0, "", "You don't have any image open in the main window.");
      return;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);

    if (!p4DImage) {
      QMessageBox::information(0, "",
                               "The image pointer is invalid. Ensure your data "
                               "is valid and try again!");
      return;
    }

    data1d = p4DImage->getRawData();
    N = p4DImage->getXDim();
    M = p4DImage->getYDim();
    P = p4DImage->getZDim();
    sc = p4DImage->getCDim();

    bool ok1;

    if (sc == 1) {
      c = 1;
      ok1 = true;
    } else {
      c = QInputDialog::getInteger(parent, "Channel", "Enter channel NO:", 1, 1,
                                   sc, 1, &ok1);
    }

    if (!ok1)
      return;

    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;
    in_sz[3] = sc;

    PARA.inimg_file = p4DImage->getFileName();
  } else {
    int datatype = 0;
    if (!simple_loadimage_wrapper(callback,
                                  PARA.inimg_file.toStdString().c_str(), data1d,
                                  in_sz, datatype)) {
      fprintf(stderr,
              "Error happens in reading the subject file [%s]. Exit. \n",
              PARA.inimg_file.toStdString().c_str());
      return;
    }
    if (PARA.channel < 1 || PARA.channel > in_sz[3]) {
      fprintf(stderr, "Invalid channel number. \n");
      return;
    }
    N = in_sz[0];
    M = in_sz[1];
    P = in_sz[2];
    sc = in_sz[3];
    c = PARA.channel;
  }

  // Main neuron reconstruction code
  cout << "====== WELCOME TO SOMA DETECTION ======" << endl;
  show_logo();
  printf("Image Size: %d, %d, %d\n", in_sz[0], in_sz[1], in_sz[2]);
  long *in_sz3 = new V3DLONG[3];
  in_sz3[0] = in_sz[0];
  in_sz3[1] = in_sz[1];
  in_sz3[2] = in_sz[2];
  Image3<unsigned char> *img =
      new Image3<unsigned char>(data1d, in_sz3); // Make the Image
  img->set_destroy(false);

  // The meaty part
  R2Tracer *tracer = new R2Tracer();
  tracer->set_quality(PARA.quality);
  tracer->set_prune(PARA.prune);
  cout<<"test : The threshold value for soma detection is "<<(int) PARA.threshold<<endl;
  SWC *swc = tracer->trace(img, PARA.threshold);
  cout<<"test: ...."<<endl;


  cout << "====== END ======" << endl;

  if (tracer) {
    delete tracer;
    tracer = NULL; 
  }

  // 
  if(PARA.outswc_file.isEmpty()){
    PARA.outswc_file = PARA.inimg_file + ".somapoint.swc";
  }
  save_swc(swc, PARA.outswc_file);
  PARA.outsomaimg_file = PARA.inimg_file + ".somaimg.tif";
//  QByteArray ba = PARA.outsomaimg_file.toLatin1();
//  char* c_str2 = ba.data();
//  cout<<"The output path is "<<c_str2<<endl;
  tracer->soma_img->save(PARA.outsomaimg_file.toStdString().c_str());
//  cout<<"The output path is "<<PARA.outsomaimg_file<<endl;
//  tracer->soma_img->save("/home/donghao/Desktop/vaa3d/vaa3d_tools/hackathon/donghao/Bigneuron_donghao_soma/test_data/somaxxx.tif");
  // Clean up
  if (in_sz3) {
    delete[] in_sz3;
    in_sz3 = NULL;
  }

  if (img) {
    delete img;
    img = NULL;
  }

  if (swc) {
    delete swc;
    swc = NULL;
  }
  
  return;
}

void save_swc(SWC *swc, QString fname) {
  NeuronTree nt;
  QList<NeuronSWC> listNeuron;
  QHash<int, int> hashNeuron;
  listNeuron.clear();
  hashNeuron.clear();
  NeuronSWC S;
  for (int i = 0; i < swc->size(); i++) {
    SWCNode n = swc->get_node(i);
    S.n = n.id;
    S.type = n.type;
    S.x = n.p.x;
    S.y = n.p.y;
    S.z = n.p.z;
    S.r = floor(n.radius);
    S.pn = n.pid;
    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size() - 1);
  }
  nt.n = -1;
  nt.on = true;
  nt.listNeuron = listNeuron;
  nt.hashNeuron = hashNeuron;
  nt.name = "r2";
  writeSWC_file(fname.toStdString().c_str(), nt);
}

void show_logo(){
  // cout<<"\n\n\n";
//   cout<<"8888888b.  d8b                   888          888           .d8888b.  \n"
// "888   Y88b Y8P                   888          888          d88P  Y88b \n"
// "888    888                       888          888                 888 \n"
// "888   d88P 888 888  888 888  888 888  .d88b.  888888            .d88P \n"
// "8888888P\"  888 888  888 888  888 888 d8P  Y8b 888           .od888P\"  \n"
// "888 T88b   888 Y88  88P 888  888 888 88888888 888          d88P\"      \n"
// "888  T88b  888  Y8bd8P  Y88b 888 888 Y8b.     Y88b.        888\"       \n"
// "888   T88b 888   Y88P    \"Y88888 888  \"Y8888   \"Y888       888888888  \n\n\n\n";
  cout<<"Soma Detection"<<endl;
}
