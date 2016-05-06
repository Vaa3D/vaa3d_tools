#include "sigen/binarizer/binarizer.h"
#include "sigen/builder/builder.h"
#include "sigen/extractor/extractor.h"
#include "sigen/loader/file_loader.h"
#include "sigen/toolbox/toolbox.h"
#include "sigen/writer/fileutils.h"
#include "sigen/writer/swc_writer.h"
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <vector>

// disable specified warning options
// https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html#Diagnostic-Pragmas
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <cmdline/cmdline.h>
#pragma GCC diagnostic pop

void initGlog(const char *program_name) {
  // https://google-glog.googlecode.com/svn/trunk/doc/glog.html
  // FLAGS_log_dir = "log";
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(program_name);
  google::InstallFailureSignalHandler(); // print stack trace if program fault
}

cmdline::parser parse_args(int argc, char *argv[]) {
  cmdline::parser a;
  a.add<std::string>("input", 'i', "input image directory");
  a.add<std::string>("output", 'o', "output filename");
  a.add<double>("scale-xy", '\0', "", false, 1.0);
  a.add<double>("scale-z", '\0', "", false, 1.0);
  a.add<double>("dt", '\0', "distance threshold", false, 0.0);
  a.add<int>("vt", '\0', "volume threshold", false, 0);
  a.add<int>("clipping", '\0', "clipping level", false, 0);
  a.add<int>("smoothing", '\0', "smoothing level", false, 0);
  a.parse_check(argc, argv);
  return a;
}

int main(int argc, char *argv[]) {
  initGlog(argv[0]);

  cmdline::parser args = parse_args(argc, argv);

  sigen::FileLoader loader;
  sigen::ImageSequence is = loader.Load(args.get<std::string>("input"));
  LOG(INFO) << "load (done)";

  sigen::Binarizer bin;
  sigen::BinaryCube cube = bin.Binarize(is);
  is.clear();
  LOG(INFO) << "binarize (done)";

  sigen::Extractor ext(cube);
  std::vector<sigen::ClusterPtr> clusters = ext.Extract();
  cube.Clear();
  LOG(INFO) << "extract (done)";

  sigen::Builder builder(clusters, args.get<double>("scale-xy"), args.get<double>("scale-z"));
  std::vector<sigen::Neuron> ns = builder.Build();
  LOG(INFO) << "build (done)";

  const double dt = args.get<double>("dt");
  const int vt = args.get<int>("vt");
  if (vt > 0) {
    ns = sigen::Interpolate(ns, dt, vt);
    LOG(INFO) << "interpolate (done)";
  }

  const int smoothing_level = args.get<int>("smoothing");
  if (smoothing_level > 0) {
    ns = sigen::Smoothing(ns, smoothing_level);
    LOG(INFO) << "smoothing (done)";
  }

  const int clipping_level = args.get<int>("clipping");
  if (clipping_level > 0) {
    ns = sigen::Clipping(ns, clipping_level);
    LOG(INFO) << "clipping (done)";
  }

  sigen::SwcWriter writer;
  for (int i = 0; i < (int)ns.size(); ++i) {
    std::string filename =
        args.get<std::string>("output") + "/" + std::to_string(i) + ".swc";
    filename = sigen::FileUtils::AddExtension(filename, ".swc");
    writer.Write(filename.c_str(), ns[i]);
  }
  LOG(INFO) << "write (done)";
}
