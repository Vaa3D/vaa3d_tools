#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <basic_surf_objs.h>
#include <v3d_message.h>

#include "SIGEN_plugin.h"

#include "sigen/common/binary_cube.h"
#include "sigen/interface.h"
Q_EXPORT_PLUGIN2(SIGEN, SigenPlugin);

struct input_PARA {
  QString inimg_file;
  V3DLONG channel;
};

void reconstruction_func(
    V3DPluginCallback2 &callback,
    QWidget *parent,
    input_PARA &PARA,
    bool via_gui);

QStringList SigenPlugin::menulist() const {
  return QStringList()
         << tr("Trace")
         << tr("About");
}

QStringList SigenPlugin::funclist() const {
  return QStringList()
         << tr("trace")
         << tr("help");
}

void SigenPlugin::domenu(
    const QString &menu_name,
    V3DPluginCallback2 &callback,
    QWidget *parent) {
  if (menu_name == tr("Trace")) {
    input_PARA PARA;
    reconstruction_func(callback, parent, PARA, /* via_gui = */ true);
  } else if (menu_name == tr("About")) {
    v3d_msg(tr("This plugin is porting of SIGEN (https://sites.google.com/site/sigenproject/) for Vaa3D."));
  } else {
    assert(false);
  }
}

// static std::string GetArgString(const V3DPluginArgList &input, int index, int index2) {
//   if (index >= (int)input.size())
//     return "";
//   const std::vector<char *> &vchar = *(std::vector<char *> *)input[index].p;
//   if (index2 >= (int)vchar.size())
//     return "";
//   return vchar[index];
// }

bool SigenPlugin::dofunc(
    const QString &func_name,
    const V3DPluginArgList &input,
    V3DPluginArgList & /* output */,
    V3DPluginCallback2 &callback,
    QWidget *parent) {
  using std::vector;
  if (func_name == tr("trace")) {
    input_PARA PARA;
    vector<char *> *pinfiles = (input.size() >= 1) ? (vector<char *> *)input[0].p : 0;
    vector<char *> *pparas = (input.size() >= 2) ? (vector<char *> *)input[1].p : 0;
    vector<char *> infiles = (pinfiles != 0) ? *pinfiles : vector<char *>();
    vector<char *> paras = (pparas != 0) ? *pparas : vector<char *>();
    if (infiles.empty()) {
      fprintf(stderr, "Need input image. \n");
      return false;
    } else {
      PARA.inimg_file = infiles[0];
    }
    int k = 0;
    PARA.channel = ((int)paras.size() >= k + 1) ? atoi(paras[k]) : 1;
    k++;
    reconstruction_func(callback, parent, PARA, /* via_gui = */ false);
  } else if (func_name == tr("help")) {
    ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN
    printf("**** Usage of SIGEN tracing **** \n");
    printf("vaa3d -x SIGEN -f trace -i <inimg_file> -p <channel> <other parameters>\n");
    printf("inimg_file       The input image\n");
    printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
    printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");
  } else {
    return false;
  }
  return true;
}

static sigen::BinaryCube convertToBinaryCube(
    const unsigned char *p,
    const int unit_byte,
    const int xdim,
    const int ydim,
    const int zdim,
    const int /* channel_dim */,
    const int channel) {
  const int stride_x = unit_byte;
  const int stride_y = unit_byte * xdim;
  const int stride_z = unit_byte * xdim * ydim;
  const int stride_c = unit_byte * xdim * ydim * zdim;
  sigen::BinaryCube cube(xdim, ydim, zdim);
  for (int x = 0; x < xdim; ++x) {
    for (int y = 0; y < ydim; ++y) {
      for (int z = 0; z < zdim; ++z) {
        if (p[stride_x * x + stride_y * y + stride_z * z + stride_c * channel] >= 128) {
          cube[x][y][z] = true;
        } else {
          cube[x][y][z] = false;
        }
      }
    }
  }
  return cube;
}

static QLineEdit *addIntEdit(const QString &default_value, QWidget *parent) {
  QIntValidator *v = new QIntValidator(parent);
  v->setBottom(0);
  QLineEdit *e = new QLineEdit(default_value, parent);
  e->setValidator(v);
  return e;
}

static QLineEdit *addDoubleEdit(const QString &default_value, QWidget *parent) {
  QDoubleValidator *v = new QDoubleValidator(parent);
  v->setBottom(0.0);
  QLineEdit *e = new QLineEdit(default_value, parent);
  e->setValidator(v);
  return e;
}

static bool getConfig(QWidget *parent, sigen::interface::Options *options) {
  // http://vivi.dyndns.org/vivi/docs/Qt/layout.html
  QFormLayout *fLayout = new QFormLayout(parent);
  fLayout->setLabelAlignment(Qt::AlignRight);

  // http://doc.qt.io/qt-4.8/qlineedit.html
  QLineEdit *sxy_lineEdit = addDoubleEdit("1.0", parent);
  fLayout->addRow(QObject::tr("Scale XY"), sxy_lineEdit);

  QLineEdit *sz_lineEdit = addDoubleEdit("1.0", parent);
  fLayout->addRow(QObject::tr("Scale Z"), sz_lineEdit);

  QCheckBox *interp_checkbox = new QCheckBox("Interpolation", parent);
  interp_checkbox->setCheckState(Qt::Checked);
  fLayout->addRow("", interp_checkbox);

  QLineEdit *vt_lineEdit = addIntEdit("0", parent);
  fLayout->addRow(QObject::tr("Interpolation VT"), vt_lineEdit);

  QLineEdit *dt_lineEdit = addDoubleEdit("0.0", parent);
  fLayout->addRow(QObject::tr("Interpolation DT"), dt_lineEdit);

  // http://www.qtforum.org/article/2430/qcheckbox.html
  QObject::connect(interp_checkbox, SIGNAL(toggled(bool)), vt_lineEdit, SLOT(setEnabled(bool)));
  QObject::connect(interp_checkbox, SIGNAL(toggled(bool)), dt_lineEdit, SLOT(setEnabled(bool)));

  QCheckBox *smoothing_checkbox = new QCheckBox("Smoothing", parent);
  smoothing_checkbox->setCheckState(Qt::Checked);
  fLayout->addRow("", smoothing_checkbox);

  QLineEdit *sm_lineEdit = addIntEdit("0", parent);
  fLayout->addRow(QObject::tr("Smoothing Level"), sm_lineEdit);

  QObject::connect(smoothing_checkbox, SIGNAL(toggled(bool)), sm_lineEdit, SLOT(setEnabled(bool)));

  QCheckBox *clipping_checkbox = new QCheckBox("Clipping", parent);
  clipping_checkbox->setCheckState(Qt::Checked);
  fLayout->addRow("", clipping_checkbox);

  QLineEdit *cl_lineEdit = addIntEdit("0", parent);
  fLayout->addRow(QObject::tr("Clipping Level"), cl_lineEdit);

  QObject::connect(clipping_checkbox, SIGNAL(toggled(bool)), cl_lineEdit, SLOT(setEnabled(bool)));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal,
      parent);

  QVBoxLayout *vLayout = new QVBoxLayout(parent);
  vLayout->addLayout(fLayout);
  vLayout->addWidget(buttonBox);

  QDialog *dialog = new QDialog(parent);
  dialog->setWindowTitle("SIGEN");
  dialog->setLayout(vLayout);
  dialog->setFixedSize(dialog->sizeHint());

  QObject::connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

  bool retval;
  switch (dialog->exec()) {
  case QDialog::Accepted:
    retval = true;
    break;
  case QDialog::Rejected:
    retval = false;
    break;
  default:
    assert(false);
    break;
  }

  // v3d_msg(vt_lineEdit->text() + "/" + dt_lineEdit->text() + "/" + sm_lineEdit->text() + "/" + cl_lineEdit->text(), true);

  if (retval) {
    options->scale_xy = sxy_lineEdit->text().toDouble();
    options->scale_z = sz_lineEdit->text().toDouble();

    options->enable_interpolation = interp_checkbox->checkState() == Qt::Checked;
    options->volume_threshold = vt_lineEdit->text().toInt();
    options->distance_threshold = dt_lineEdit->text().toDouble();

    options->enable_smoothing = smoothing_checkbox->checkState() == Qt::Checked;
    options->smoothing_level = sm_lineEdit->text().toInt();

    options->enable_clipping = clipping_checkbox->checkState() == Qt::Checked;
    options->clipping_level = cl_lineEdit->text().toInt();
  }

  return retval;
}

void reconstruction_func(
    V3DPluginCallback2 &callback,
    QWidget *parent,
    input_PARA &PARA,
    bool via_gui) {
  unsigned char *data1d = NULL;
  V3DLONG N, M, P, sc, c;
  V3DLONG in_sz[4];
  if (via_gui) {
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin) {
      QMessageBox::information(0, "", "You don't have any image open in the main window.");
      return;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);
    if (!p4DImage) {
      QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
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
      c = QInputDialog::getInteger(parent, "Channel", "Enter channel NO:", 1, 1, sc, 1, &ok1);
    }
    if (!ok1) {
      return;
    }
    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;
    in_sz[3] = sc;
    PARA.inimg_file = p4DImage->getFileName();
  } else {
    int datatype = 0;
    if (!simple_loadimage_wrapper(callback, PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype)) {
      fprintf(stderr, "Error happens in reading the subject file [%s]. Exit. \n", PARA.inimg_file.toStdString().c_str());
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
  //main neuron reconstruction code
  //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE

  // show configure GUI window
  sigen::interface::Options options;
  bool retval = getConfig(parent, &options);
  if (!retval) {
    return;
  }
  // check config
  // v3d_msg((retval ? QString("OK") : QString("Cancel")), via_gui);
  // v3d_msg(QString("VT = %1\nDT = %2\nSM = %3\nCL = %4").arg(options.volume_threshold).arg(options.distance_threshold).arg(options.smoothing_level).arg(options.clipping_level), via_gui);
  // return;

  sigen::BinaryCube cube = convertToBinaryCube(data1d, /* unit_byte = */ 1, N, M, P, sc, c - 1);
  std::vector<int> out_n, out_type, out_pn;
  std::vector<double> out_x, out_y, out_z, out_r;
  sigen::interface::Extract(
      cube, out_n, out_type,
      out_x, out_y, out_z,
      out_r, out_pn, options);

  // construct NeuronTree
  NeuronTree nt;
  nt.name = "SIGEN";
  nt.comment = "SIGEN";
  for (int i = 0; i < (int)out_n.size(); ++i) {
    NeuronSWC pt;
    pt.n = out_n[i];
    pt.type = out_type[i];
    pt.x = out_x[i];
    pt.y = out_y[i];
    pt.z = out_z[i];
    pt.r = out_r[i];
    pt.pn = out_pn[i];
    nt.listNeuron.push_back(pt);
  }

  QString swc_name = PARA.inimg_file + "_SIGEN.swc";
  writeSWC_file(swc_name.toStdString().c_str(), nt);
  if (!via_gui) {
    if (data1d) {
      delete[] data1d;
      data1d = NULL;
    }
  }
  v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()), via_gui);
}
