#if !defined(__MINGW32__)

#include "ztest.h"

#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __GLIBCXX__
#include <tr1/memory>
#else
#include <memory>
#endif
#include <string>
#include "zopencv_header.h"

#include "neutube.h"
#include "zstackprocessor.h"
#include "zfilelist.h"
#include "tz_sp_grow.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_stat.h"
#include "tz_stack_attribute.h"
#include "zspgrowparser.h"
#include "zvoxelarray.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_threshold.h"
#include "zsuperpixelmaparray.h"
#include "zsegmentmaparray.h"
#include "tz_xml_utils.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "znormcolormap.h"
#include "tz_graph_defs.h"
#include "tz_graph_utils.h"
#include "tz_workspace.h"
#include "tz_graph.h"
#include "flyemskeletonizationdialog.h"
#include "zstackaccessor.h"
#include "zmatrix.h"
#include "zswcbranch.h"
#include "zswctreematcher.h"
#include "parameterdialog.h"
#include "zstring.h"
#include "zrandomgenerator.h"
#include "zjsonobject.h"
#include "zpoint.h"
#include "flyem/zfileparser.h"
#include "zswcgenerator.h"
#include "flyem/zsynapseannotationarray.h"
#include "flyem/zfileparser.h"
#include "flyem/zsynapseannotationanalyzer.h"
#include "flyem/zneuronnetwork.h"
#include "tz_geo3d_utils.h"
#include "zsvggenerator.h"
#include "zdendrogram.h"
#include "zcuboid.h"
#include "zswcsizefeatureanalyzer.h"
#include "zobject3darray.h"
#include "zswcshollfeatureanalyzer.h"
#include "zswcspatialfeatureanalyzer.h"
#include "swctreenode.h"
#include "zswcnetwork.h"
#include "zdoublevector.h"
#include "zswcdisttrunkanalyzer.h"
#include "zswcbranchingtrunkanalyzer.h"
#include "tz_error.h"
#include "flyem/zsynapselocationmatcher.h"
#include "flyem/zsynapselocationmetric.h"
#include "zstackfile.h"
#include "c_stack.h"
#include "zstack.hxx"
#include "flyem/zsegmentationanalyzer.h"
#include "flyem/zsegmentationbundle.h"
#include "zstackblender.h"
#include "zgraph.h"
#include "zarray.h"
#include "tz_iarray.h"
#include "zintpairmap.h"
#include "tz_u8array.h"
#include "zfiletype.h"
#include "tz_geometry.h"
#include "z3dgraph.h"
#include "zpunctum.h"
#include "zswcsizetrunkanalyzer.h"
#include "zswcweighttrunkanalyzer.h"
#include "zdebug.h"
#include "tz_color.h"
#include "zhdf5reader.h"
#include "tz_farray.h"
#include "zxmldoc.h"
#include "neutubeconfig.h"
#include "tz_darray.h"
#include "zhdf5writer.h"
#include "flyem/zbcfset.h"
#include "flyem/zflyemstackframe.h"
#include "zmoviemaker.h"
#include "z3dmesh.h"
#include "zstackdoc.h"
#include "z3dwindow.h"
#include "z3dswcfilter.h"
#include "z3dinteractionhandler.h"
#include "z3dcompositor.h"
#include "z3dvolumeraycaster.h"
#include "z3dvolumeraycasterrenderer.h"
#include "z3dvolumesource.h"
#include "z3dpunctafilter.h"
#include "tz_stack.h"
#include "zswclayerfeatureanalyzer.h"
#include "flyem/zflyemdatabundle.h"
#include "mainwindow.h"
#include "zmoviescriptgenerator.h"
#include "zobject3dscan.h"
#include "zswclayertrunkanalyzer.h"
#include "zswclayershollfeatureanalyzer.h"
#include "zstackgraph.h"
#include "zgraphcompressor.h"
#include "zswcpositionadjuster.h"
#include "zgraph.h"
#include "tz_cuboid_i.h"
#include "zswcglobalfeatureanalyzer.h"
#include "zlogmessagereporter.h"
#include "swc/zswcresampler.h"
#include "misc/miscutility.h"
#include "test/zjsontest.h"
#include "test/zswctreetest.h"
#include "test/zobject3dscantest.h"
#include "test/zswcpathtest.h"
#include "test/zgraphtest.h"
#include "test/zstackgraphtest.h"
#include "test/zstringtest.h"
#include "test/zobject3dtest.h"
#include "test/zswcanalyzertest.h"
#include "test/zellipsoidtest.h"
#include "test/zstitchgridtest.h"
#include "test/zcuboidtest.h"
#include "test/zflyemqualitycontroltest.h"
#include "test/zflyemsynaseannotationtest.h"
#include "test/zstackdoctest.h"
#include "test/ztreetest.h"
#include "test/zprogresstest.h"
#include "test/zswctreematchertest.h"
#include "test/zswctreenodetest.h"
#include "test/zhistogramtest.h"
#include "test/zflyemneuronrangetest.h"
#include "swc/zswcterminalsurfacemetric.h"
#include "test/zflyemneuronfiltertest.h"
#include "test/zswcmetrictest.h"

using namespace std;

namespace {

template <typename T>
bool checkEqual(T expValue, T value, const QString &valueName, T eps,
                const char* file="", int line=-1, const char *function="")
{
  if (std::abs(expValue - value) < eps)
    return true;
  else {
    LERRORF(file,line,function) << "Value" << valueName << "is wrong.";
    if (eps != 0)
      LERRORF(file,line,function) << "expected:" << expValue  << "!=" << "actual:" << value << "(eps:" << eps << ")";
    else
      LERRORF(file,line,function) << "expected:" << expValue  << "!=" << "actual:" << value;
    return false;
  }
}

#define CHECK_EQUAL(exp, actual, name, eps) checkEqual((exp),(actual),(name),(eps),__FILE__,__LINE__,__PRETTY_FUNCTION__)

}  // empty namespace

ostream& ZTest::m_failureStream = cerr;


ZTest::ZTest()
{

}

bool ZTest::testTreeIterator(ZSwcTree &tree,
                             const ZTestSwcTreeIteratorConfig &config,
                             int *truthArray,
                             int truthCount, bool testReverse)
{
  int count = -1;

  if (config.start == NULL && config.blocker == NULL) {
    count = tree.updateIterator(config.option, TRUE);
  } else if (config.blocker == NULL) {
    count = tree.updateIterator(config.option, config.start, TRUE);
  } else if (config.start == NULL) {
    count = tree.updateIterator(config.option, *(config.blocker), TRUE);
  } else {
    count = tree.updateIterator(config.option, config.start, *(config.blocker),
                                TRUE);
  }

#ifdef _USE_GTEST_
  EXPECT_EQ(count, truthCount) << "Unmatched node number";
#else
  if (count != truthCount) {
    cerr << "Unmatched node number" << endl;
    return false;
  }
#endif

  if (truthArray == NULL) {
    if (tree.begin() != NULL) {
      return false;
    }
  }

  for (Swc_Tree_Node *tn = tree.begin(); tn != tree.end(); tn = tree.next()) {
    if (testReverse) {
#ifdef _USE_GTEST_
      EXPECT_EQ(SwcTreeNode::id(tn), truthArray[count - 1 - SwcTreeNode::index(tn)])
          << "Unmatched node number";
#else
      if (SwcTreeNode::id(tn) != truthArray[count - 1 - SwcTreeNode::index(tn)]) {
        cout << "Unmatched id" << endl;
        tree.print(SWC_TREE_ITERATOR_NO_UPDATE);
        return false;
      }
#endif
    } else {
      if (SwcTreeNode::id(tn) != truthArray[SwcTreeNode::index(tn)]) {
        cout << "Unmatched id" << endl;
        tree.print(SWC_TREE_ITERATOR_NO_UPDATE);
        return false;
      }
    }
  }

  return true;
}

bool ZTest::testTreeIterator()
{
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/breadth_first.swc");

  ZTestSwcTreeIteratorConfig config;

  {
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    int array[7] = { -1, 1, 2, 3, 4, 5, 6 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    config.option = SWC_TREE_ITERATOR_REVERSE;
    int array[7] = { 6, 5, 4, 3, 2, 1, -1 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    config.option = SWC_TREE_ITERATOR_LEAF;
    int array[3] = { 5, 6, 4 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    config.option = SWC_TREE_ITERATOR_REVERSE;
    int array[3] = { 5, 6, 4 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true),
        "tree iteration failed");
  }

  //With blockers
  set<Swc_Tree_Node*> blocker;
  blocker.insert(tree.data()->root->first_child);

  {
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    config.blocker = &blocker;
    int array[1] = { -1 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    int array[1] = { -1 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    config.option = SWC_TREE_ITERATOR_REVERSE;
    int array[1] = { -1 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    config.option = SWC_TREE_ITERATOR_LEAF;
    TZ_ASSERT(testTreeIterator(tree, config, NULL, 0),
        "tree iteration failed");
  }

  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child);
  {
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    int array[2] = { -1, 1 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    int array[2] = { -1, 1 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    config.option = SWC_TREE_ITERATOR_REVERSE;
    int array[2] = { 1, -1 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child->first_child);
  {
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    int array[4] = { -1, 1, 2, 4 };
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    int array[4] = { -1, 1, 2, 4 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true),
            "tree iteration failed");
  }

  {
    int array[4] = { -1, 1, 2, 4 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {

    int array[4] = { -1, 1, 2, 4 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true),
            "tree iteration failed");
  }

  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child->first_child->next_sibling);
  {
    int array[6] = { -1, 1, 2, 3, 5, 6 };
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    int array[6] = { -1, 1, 2, 3, 5, 6 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true),
            "tree iteration failed");
  }

  {
    int array[6] = { -1, 1, 2, 3, 5, 6 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    int array[6] = { -1, 1, 2, 3, 5, 6 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true),
            "tree iteration failed");
  }

  {
    int array[2] = { 5, 6 };
    config.option = SWC_TREE_ITERATOR_LEAF;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
            "tree iteration failed");

    config.option = SWC_TREE_ITERATOR_REVERSE;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true),
        "tree iteration failed");
  }

  blocker.insert(tree.data()->root->first_child->first_child->first_child);
  {
    int array[3] = { -1, 1, 2 };
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    int array[3] = { -1, 1, 2 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true),
            "tree iteration failed");
  }

  {
    int array[3] = { -1, 1, 2 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    int array[3] = { -1, 1, 2 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true),
            "tree iteration failed");
  }

  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child->first_child->first_child);
  {
    int array[6] = { -1, 1, 2, 3, 4, 6 };
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    int array[6] = { -1, 1, 2, 3, 6, 4 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  Swc_Tree_Node *start = tree.data()->root->first_child->first_child;
  {
    int array[4] = { 2, 3, 4, 6 };
    config.start = start;
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    int array[4] = { 2, 3, 6, 4 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  ////////////
  config.start = tree.data()->root->first_child->first_child->first_child;
  {
    int array[2] = { 3, 6 };
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }

  {
    int array[2] = { 3, 6 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    TZ_ASSERT(testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0])),
        "tree iteration failed");
  }


#if 0
  tree.print(SWC_TREE_ITERATOR_DEPTH_FIRST);
  cout << endl;

  tree.print(SWC_TREE_ITERATOR_BREADTH_FIRST);
  cout << endl;

  tree.deactivateIterator();
  tree.print(SWC_TREE_ITERATOR_DEPTH_FIRST);
  cout << endl;

  tree.activateIterator();
  tree.print(SWC_TREE_ITERATOR_DEPTH_FIRST);
  cout << endl;

  tree.print(SWC_TREE_ITERATOR_LEAF);
  cout << endl;

  set<Swc_Tree_Node*> blocker;
  blocker.insert(tree.data()->root->first_child);

  tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, blocker);
  tree.print(SWC_TREE_ITERATOR_NO_UPDATE);
  cout << endl;

  tree.updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST, blocker);
  tree.print(SWC_TREE_ITERATOR_NO_UPDATE);
  cout << endl;

  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child->first_child);
  tree.updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST, blocker);
  tree.print(SWC_TREE_ITERATOR_NO_UPDATE);
  cout << endl;

  blocker.clear();
  tree.updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST,
                      tree.data()->root->first_child->first_child->first_child,
                      blocker);
  tree.print(SWC_TREE_ITERATOR_NO_UPDATE);
  cout << endl;

  blocker.insert(tree.data()->root->first_child->first_child->first_child->first_child);
  tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST,
                      tree.data()->root->first_child->first_child->first_child,
                      blocker);
  tree.print(SWC_TREE_ITERATOR_NO_UPDATE);
  cout << endl;
#endif

  return true;
}

#ifdef _JANELIA_WORKSTATION_
const static string dataPath("/groups/flyem/home/zhaot/Work/neutube_ws/neurolabi/data");
#else
const static string dataPath("/Users/zhaot/Work/neutube/neurolabi/data");
#endif

int ZTest::runUnitTest(int argc, char *argv[])
{
#ifdef _USE_GTEST_
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
#else
  return 0;
#endif
}

void ZTest::test(MainWindow *host)
{
  UNUSED_PARAMETER(host);
#if 0
  ZStackFrame *frame = (ZStackFrame *) mdiArea->currentSubWindow();
  if (frame != NULL) {
    //QMessageBox::question(frame, tr("test"), tr("change size"),
    //			  QMessageBox::Yes | QMessageBox::No);
    frame->resize(frame->view()->imageWidget()->screenSize().width(),
      frame->height());
  }
#endif

#if 0
  ::testtrace();
  Stack *stack = Read_Stack("../data/diadem_e1.tif");
  LocationSimple *ptarray = new LocationSimple[3];
  ptarray[0].x = 165;
  ptarray[0].y = 248;
  ptarray[0].z = 40;
  ptarray[0].radius = 3;

  ptarray[1].x = 195;
  ptarray[1].y = 262;
  ptarray[1].z = 43;
  ptarray[1].radius = 3;

  ptarray[0].x = 231;
  ptarray[0].y = 249;
  ptarray[0].z = 44;
  ptarray[0].radius = 3;

  main_zhaot_neuron_tracing(stack->array, stack->width, stack->height, stack->depth,
      ptarray, 3);

  delete []ptarray;
  Kill_Stack(stack);
#endif

#if 0
  //The minimum and maximum is the number of steps in the operation for which this progress dialog shows progress.
  //for example here 0 and 100.
  QProgressDialog* progress = new QProgressDialog("Fetching data...", "Cancel", 0, 100);

  //Set dialog as modal dialog, if you want.
  progress->setWindowModality(Qt::WindowModal);
  progress->show();
  for (int i = 0; i <= 100; i++) {
    progress->setValue(i);

  }
//  delete progress;
#endif

#if 0
  m_progress->setRange(0, 0);
  m_progress->show();
  QApplication::processEvents();
  currentStackFrame()->document()->test();
  currentStackFrame()->updateView();
  m_progress->reset();
#endif

#if 0
  QGraphicsScene *scene = new QGraphicsScene(0);
  scene->clear();

  QGraphicsSvgItem *m_svgItem = new QGraphicsSvgItem("../data/test.svg");
      //new QGraphicsSvgItem("/Developer/Examples/Qt/painting/svgviewer/files/spheres.svg");
  m_svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
  m_svgItem->setCacheMode(QGraphicsItem::NoCache);
  m_svgItem->setZValue(0);

  scene->addItem(m_svgItem);

  QGraphicsView *gv = new QGraphicsView(scene, 0);
  gv->show();

#endif

#if 0
  QProgressDialog *pd = new QProgressDialog("Testing", "Cancel", 0, 100, this);

  pd->setRange(0, 100);
  pd->show();
  QApplication::processEvents();

  loadFile("../data/benchmark/mouse_neuron_single/stack.tif");

  pd->setValue(m_progress->value() + 10);
  pd->show();
  QApplication::processEvents();

  if (currentStackFrame() != NULL) {
    currentStackFrame()->presenter()->autoTrace();
    currentStackFrame()->updateView();
  }

  pd->setValue(m_progress->value() + 10);
  pd->show();
  QApplication::processEvents();

  currentStackFrame()->document()->
      exportSwcTree("../data/benchmark/mouse_neuron_single/auto.swc");

  pd->setValue(m_progress->maximum());
  pd->reset();

  delete pd;

  QMessageBox::information(currentStackFrame(), "Testing Completed",
                           "No problem found.", QMessageBox::Ok);
#endif

#if 0
  BcAdjustDialog dlg;
  dlg.setRange(0, 255);
  dlg.setValue(10, 100);
  dlg.exec();
#endif

#if 0
  ZStackFrame *frame = new ZStackFrame(this);
  const char *filePath = "E:\\data\\diadem\\diadem1\\nc_01.tif";
  Mc_Stack *stack = Read_Mc_Stack(filePath, -1);
  frame->document()->loadStack(Mc_Stack_To_Stack(stack, -1, NULL));
  frame->document()->setStackSource(filePath);
  frame->setWindowTitle(filePath);
  frame->presenter()->optimizeStackBc();
  frame->view()->reset();
  setCurrentFile(filePath);
  addStackFrame(frame);
#endif

#if 0
  ZSwcTree tree1;
  tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/result/swc2/C2_214.swc");

  std::vector<double> angleArray = tree1.computeAllContinuationAngle();
  double mu = darray_mean(angleArray.data(),
                          angleArray.size());
  double sigma = sqrt(darray_var(angleArray.data(),
                                 angleArray.size()));

  std::vector<double> branchAngleArray = tree1.computeAllBranchingAngle();
  double branchMu = darray_mean(branchAngleArray.data(),
                                branchAngleArray.size());
  double branchSigma = sqrt(darray_var(branchAngleArray.data(),
                                       branchAngleArray.size()));

  double probBranch = static_cast<double>(branchAngleArray.size()) /
      (angleArray.size() + branchAngleArray.size());

  int n = tree1.size();

  ZSwcTree *tree = ZSwcTree::generateRandomSwcTree(n, probBranch, mu, sigma,
                                                   branchMu, branchSigma);

  angleArray = tree->computeAllContinuationAngle();
  mu = darray_mean(angleArray.data(),
                            angleArray.size());
  sigma = sqrt(darray_var(angleArray.data(),
                                   angleArray.size()));

  tree->resortId();
  tree->save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#if 0
  ZSwcTree tree1;
  tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/demo/circuit/C2_214.swc");
  ZSwcTree tree2;
  tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/demo/circuit/L1_209.swc");
  ZSwcTree tree3;
  tree3.load("/Users/zhaot/Work/neutube/neurolabi/data/demo/circuit/Mi1_215.swc");

  ZSwcTree connect_tree;
  connect_tree.load(
        "/Users/zhaot/Work/neutube/neurolabi/data/209_214.swc");

  Swc_Tree *raw_tree1 = tree1.data();
  Swc_Tree *raw_tree2 = tree2.data();
  Swc_Tree *raw_tree3 = tree3.data();

  double bound1[6], bound2[6], bound3[6];
  Swc_Tree_Bound_Box(raw_tree1, bound1);
  Swc_Tree_Bound_Box(raw_tree2, bound2);
  Swc_Tree_Bound_Box(raw_tree3, bound3);

  Cuboid_I cuboid1;
  Cuboid_I cuboid2;
  Cuboid_I cuboid3;
  Cuboid_I intersect_cuboid;
  Cuboid_I union_cuboid;

  Cuboid_I_Set_S(&cuboid1, iround(bound1[0]), iround(bound1[1]),
                 iround(bound1[2]), iround(bound1[3] - bound1[0]),
                 iround(bound1[4] - bound1[1]),
                 iround(bound1[5] - bound1[2]));
  Cuboid_I_Set_S(&cuboid2, iround(bound2[0]), iround(bound2[1]),
                 iround(bound2[2]),
                 iround(bound2[3] - bound2[0]),
                 iround(bound2[4] - bound2[1]),
                 iround(bound2[5] - bound2[2]));
  Cuboid_I_Set_S(&cuboid3, iround(bound3[0]), iround(bound3[1]),
                 iround(bound3[2]),
                 iround(bound3[3] - bound3[0]),
                 iround(bound3[4] - bound3[1]),
                 iround(bound3[5] - bound3[2]));

  Cuboid_I_Union(&cuboid1, &cuboid2, &union_cuboid);
  Cuboid_I_Intersect(&cuboid1, &cuboid2, &intersect_cuboid);
  double scale;

  if (Cuboid_I_Is_Valid(&intersect_cuboid)) {
    for (int i = 0; i < 3; i++) {
      int margin = intersect_cuboid.cb[i] - union_cuboid.cb[i];
      int distance = intersect_cuboid.ce[i] - intersect_cuboid.cb[i] + 1;
      scale = static_cast<double>(distance) / static_cast<double>(margin);
      break;
    }
  }

  Cuboid_I_Intersect(&cuboid1, &cuboid3, &intersect_cuboid);

  if (Cuboid_I_Is_Valid(&intersect_cuboid)) {

    for (int i = 0; i < 3; i++) {
      int margin = intersect_cuboid.cb[i] - union_cuboid.cb[i];
      int distance = intersect_cuboid.ce[i] - intersect_cuboid.cb[i] + 1;
      scale = std::max(scale,
                       static_cast<double>(distance) /
                       static_cast<double>(margin));
      break;
    }
  }


  double offset1[3] = {0.0, 0.0, 0.0};
  double offset2[3] = {0.0, 0.0, 0.0};
  double offset3[3] = {0.0, 0.0, 0.0};

  for (int i = 0; i < 2; i++) {
    offset1[i] = scale * (cuboid1.cb[i] - union_cuboid.cb[i]);
    offset2[i] = scale * (cuboid2.cb[i] - union_cuboid.cb[i]);
    offset3[i] = scale * (cuboid3.cb[i] - union_cuboid.cb[i]);
  }

  Swc_Tree_Translate(raw_tree1, offset1[0], offset1[1], offset1[2]);
  Swc_Tree_Translate(raw_tree2, offset2[0], offset2[1], offset2[2]);
  Swc_Tree_Translate(raw_tree3, offset3[0], offset3[1], offset3[2]);

  connect_tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  for (Swc_Tree_Node *tn = connect_tree.begin(); tn != NULL;
       tn = connect_tree.next()) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Is_Root(tn)) {
        tn->node.type = 6;
      } else {
        tn->node.type = 1;
      }
      if (Swc_Tree_Node_Label(tn) == 209) {
        Swc_Tree_Node_Translate(tn, offset1[0], offset1[1], offset1[2]);
        tree1.moveToSurface(&(tn->node.x), &(tn->node.y), &(tn->node.z));
      } else if (Swc_Tree_Node_Label(tn) == 214) {
        Swc_Tree_Node_Translate(tn, offset2[0], offset2[1], offset2[2]);
        tree2.moveToSurface(&(tn->node.x), &(tn->node.y), &(tn->node.z));
      } else if (Swc_Tree_Node_Label(tn) == 215) {
        Swc_Tree_Node_Translate(tn, offset3[0], offset3[1], offset3[2]);
        tree3.moveToSurface(&(tn->node.x), &(tn->node.y), &(tn->node.z));
      } else {
        Swc_Tree_Node_Merge_To_Parent(tn);
      }
    }
  }

  tree1.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
  tree2.save("/Users/zhaot/Work/neutube/neurolabi/data/test2.swc");
  tree3.save("/Users/zhaot/Work/neutube/neurolabi/data/test3.swc");
  connect_tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test4.swc");
#endif

#if 0
  ZCuboid cuboid1(0, 0, 0, 2, 1, 3);
  ZCuboid cuboid2(0, 0, 0, 2, 1, 3);

  cuboid1.moveOutFrom(cuboid2, 1);

  std::ofstream stream("/Users/zhaot/Work/neutube/neurolabi/data/test.txt");

  stream << cuboid1[0] << ' ' << cuboid1[1] << ' ' << cuboid1[2] << ' '
         << cuboid1[3] << ' ' << cuboid1[4] << ' ' << cuboid1[5] << std::endl;

  stream << cuboid2[0] << ' ' << cuboid2[1] << ' ' << cuboid2[2] << ' '
         << cuboid2[3] << ' ' << cuboid2[4] << ' ' << cuboid2[5] << std::endl;

  stream.close();
#endif

#if 0
  ZCuboid b0(0, 0, 0, 2, 3, 10);

  std::vector<ZCuboid> cuboidArray;
  cuboidArray.push_back(ZCuboid(1, 1, 0, 4, 4, 10));
  cuboidArray.push_back(ZCuboid(5, 1, 0, 8, 4, 10));
  cuboidArray.push_back(ZCuboid(7, 0, 0, 9, 5, 10));
  cuboidArray.push_back(ZCuboid(3, 2, 0, 6, 6, 10));
  cuboidArray.push_back(ZCuboid(4, 5, 0, 5, 8, 10));
  cuboidArray.push_back(ZCuboid(2, 7, 0, 8, 9, 10));

  std::ofstream stream("/Users/zhaot/Work/neutube/neurolabi/data/test.txt");

  stream << b0[0] << ' ' << b0[1] << ' ' << b0[2] << ' '
         << b0[3] << ' ' << b0[4] << ' ' << b0[5] << std::endl;

  b0.layout(&cuboidArray, 1.0);

  for (size_t i = 0; i < cuboidArray.size(); i++) {
    stream << cuboidArray[i][0] << ' ' << cuboidArray[i][1] << ' '
           << cuboidArray[i][2] << ' '
           << cuboidArray[i][3] << ' ' << cuboidArray[i][4] << ' '
           << cuboidArray[i][5] << std::endl;
  }

  stream.close();
#endif

#if 0
  FlyEm::ZSwcNetwork network;
  network.import("/Users/zhaot/Work/neutube/neurolabi/data/test.txt");
  //network.import(
  //      "/Users/zhaot/Work/neutube/neurolabi/data/benchmark/flyem/network.txt");

  network.exportSwcFile("/Users/zhaot/Work/neutube/neurolabi/data/test.swc",
                        209);
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/result/swc3/adjusted/Y6_3_6097.swc");
  ZSwcBranch *branch = tree.extractFurthestBranch();
  branch->label(3);

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
  delete branch;
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/result/swc3/adjusted/C2_214.swc");

  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");

  tree.label(0);

  ZSwcBranch *branch = tree.extractFurthestBranch();
  branch->label(1);

  tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  //For every node in the tree
  Swc_Tree_Node *tn;
  for (tn = tree.begin(); tn != tree.end(); tn = tree.next()) {
    //If the node is not labeled and its parent is labeled
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Label(tn) == 0 && Swc_Tree_Node_Label(tn->parent) > 0) {
        //Get the downstream leaves
        vector<Swc_Tree_Node*> leafArray = tree.extractLeaf(tn);
        //Get the furthest leaf
        Swc_Tree_Node *leaf = leafArray[0];
        double maxDist = Swc_Tree_Node_Dist(tn, leaf);

        for (size_t i = 0; i < leafArray.size(); i++) {
          double dist = Swc_Tree_Node_Dist(tn, leafArray[i]);
          if (dist > maxDist) {
            dist = maxDist;
            leaf = leafArray[i];
          }
        }

        //Label the branch from the leaf to the node parent with parent.label+1
        Swc_Tree_Node_Set_Label(tn, Swc_Tree_Node_Label(tn->parent) + 1);
        while (leaf != tn) {
          Swc_Tree_Node_Set_Label(leaf, Swc_Tree_Node_Label(tn));
          leaf = leaf->parent;
        }
      }
    }
  }

  Swc_Tree_Set_Type_As_Label(tree.data());

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
  delete branch;
#endif

#if 0
  FlyEm::ZSwcNetwork network;
  //network.import(
  //      "/Users/zhaot/Work/neutube/neurolabi/data/benchmark/flyem/network.txt");

  network.import("/Users/zhaot/Work/neutube/neurolabi/data/test.txt");

  Graph *graph = network.toGraph();
  Graph_To_Dot_File(graph, "/Users/zhaot/Work/neutube/neurolabi/data/test.dot");
  Kill_Graph(graph);

  network.layoutSwc();

  network.exportSwcFile("/Users/zhaot/Work/neutube/neurolabi/data/test.swc",
                        209, FlyEm::ZSwcNetwork::EXPORT_ALL);

#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/L1_209.swc");
  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");
  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/single_branch.swc");
  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/fork2.swc");
  tree.resample(1000.0);
  tree.resortId();
  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;

  tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/Mi1_215.swc");
  tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/Mi1a_446263.swc");
  //tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare2.swc");
  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare3.swc");

  ZSwcTree *originalTree1 = tree1.clone();
  ZSwcTree *originalTree2 = tree2.clone();

  tree1.resample(20.0);
  tree2.resample(20.0);

  //tree2.translate(10, 0, 50.0);

  ZSwcTreeMatcher matcher;
  ZSwcLayerFeatureAnalyzer analyzer;
  //ZSwcSizeFeatureAnalyzer analyzer;
  //ZSwcShollFeatureAnalyzer analyzer;

  //analyzer.setExcludedLabel(0);
  matcher.setFeatureAnalyzer(&analyzer);

  matcher.matchAll(tree1, tree2);

  vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > matchingResult =
      matcher.matchingResult();

  ZSwcTree tree;
  tree.merge(originalTree1->data(), false);
  tree.merge(originalTree2->data(), false);

  for (size_t i = 0; i < matchingResult.size(); i++) {
    if (!Swc_Tree_Node_Is_Continuation(matchingResult[i].first) ||
        !Swc_Tree_Node_Is_Continuation(matchingResult[i].second)) {
      ZPoint startPos(matchingResult[i].first->node.x,
                      matchingResult[i].first->node.y,
                      matchingResult[i].first->node.z);
      ZPoint endPos(matchingResult[i].second->node.x,
                    matchingResult[i].second->node.y,
                    matchingResult[i].second->node.z);

      Swc_Tree_Node *tn =
          ZSwcTree::makeArrow(startPos, 1, 6, endPos, 0.2, 0);
      Swc_Tree_Node_Set_Parent(tn, tree.data()->root);
    }
  }

  tree.resortId();

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#if 0
  ZObject3dArray objArray;
  ZObject3d obj;
  obj.append(0, 0, 0);
  obj.append(0, 0, 1);
  objArray.append(obj);

  obj.clear();
  obj.append(0, 1, 0);
  obj.append(1, 1, 0);
  objArray.append(obj);

  objArray.writeIndex("/Users/zhaot/Work/neutube/neurolabi/data/test.txt",
                      100, 100, 100);
#endif

#if 0
  ZObject3dArray objArray;
  objArray.readIndex("/Users/zhaot/Work/neutube/neurolabi/data/test.txt",
                     100, 100, 100);
  cout << objArray.size() << endl;

  objArray.writeIndex("/Users/zhaot/Work/neutube/neurolabi/data/test2.txt",
                      100, 100, 100);
#endif

#if 0
  ZString str("L1_209.swc");
  std::vector<std::string> parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/L1_209.swc";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/L1_209.swc/";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "test/L1_209.swc";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/test/L1_209.swc";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/test.test/L1_209.swc";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/test.test/L1_209";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/test.test/";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }
#endif

#if 0
  ZString str("L1_209.swc");
  cout << str.changeExt("tif") << endl;

  str = "/L1_209.swc";
  cout << str.changeExt("tif") << endl;

  str = "/L1_209.swc/";
  cout << str.changeExt("tif") << endl;

  str = "test/L1_209.swc";
  cout << str.changeExt("tif") << endl;

  str = "/test/L1_209.swc";
  cout << str.changeExt("tif") << endl;

  str = "/test.test/L1_209.swc";
  cout << str.changeExt("tif") << endl;

  str = "/test.test/L1_209";
  cout << str.changeExt("tif") << endl;

  str = "/test.test/";
  cout << str.changeExt("tif") << endl;
#endif

#if 0
  ZString str("L1_209.swc");
  cout << str.changeDirectory("test") << endl;

  str = "/L1_209.swc";
  cout << str.changeDirectory("test") << endl;

  str = "/L1_209.swc/";
  cout << str.changeDirectory("test") << endl;

  str = "test/L1_209.swc";
  cout << str.changeDirectory("test") << endl;

  str = "/test/L1_209.swc";
  cout << str.changeDirectory("test") << endl;

  str = "/test.test/L1_209.swc";
  cout << str.changeDirectory("test") << endl;

  str = "/test.test/L1_209";
  cout << str.changeDirectory("test") << endl;

  str = "/test.test/";
  cout << str.changeDirectory("/test") << endl;
#endif

#if 0
  //Read a tree
  ZSwcTree tree;
  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/L1_209.swc");

  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");

  //Generate 100 random trees and save the trees above a center size threshold
  ZString prefix = "/Users/zhaot/Work/neutube/neurolabi/data/tmp/L1_209/p";
  tree.save((prefix + ".swc").c_str());

  for (int i = 0; i < 5; i++) {
    ZSwcTree *partialTree = tree.clone();
    partialTree->removeRandomBranch();
    ZString filePath = prefix;

    filePath.appendNumber(i + 1, 3);

    filePath += ".swc";

    cout << filePath << endl;

    partialTree->save(filePath.c_str());

    cout << partialTree->size() << endl;

    delete partialTree;
  }

  //Compare each random tree with the original tree
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/breadth_first.swc");
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  tree.labelTrunkLevel(&trunkAnalyzer);

  cout << SwcTreeNode::downstreamSize(tree.data()->root,
                                      SwcTreeNode::labelDifference) << endl;

  Swc_Tree_Node *tn = tree.queryNode(2);
  cout << SwcTreeNode::downstreamSize(tn,
                                      SwcTreeNode::labelDifference) << endl;

#endif

#if 0
  ZSwcTree *tree1;
  ZSwcTree *tree2;


  vector<string> fileList;
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2_214.swc");
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2c_3668.swc");
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2d_207375.swc");
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2e_228998.swc");
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2f_445362.swc");
  //const char *swcFile = "/Users/zhaot/Work/neutube/neurolabi/data/tmp/L1_209/p.swc";

  vector<ZSwcTree*> treeArray(fileList.size());

  ZSwcTree *maxTree = NULL;
  int maxSize = 0;

  for (size_t i = 0; i < fileList.size(); i++) {
    treeArray[i] = new ZSwcTree;
    treeArray[i]->load(fileList[i].c_str());
    if (maxSize < treeArray[i]->size()) {
      maxSize = treeArray[i]->size();
      maxTree = treeArray[i];
    }
  }

  cout << maxTree->source() << endl;

  tree1 = maxTree;
  tree2 = treeArray[1];
  /*
  for (size_t i = 0; i < fileList.size(); i++) {
    if (tree1 != treeArray[i]) {
      tree2 = treeArray[i];
      break;
    }
  }
  */

  /*
  tree1.load(swcFile);
  tree2.load(swcFile);

  tree2.removeRandomBranch();

  while (tree2.regularDepth() < 2) {
    tree2.load(swcFile);
    tree2.removeRandomBranch();
  }
*/


  //tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/test.nnt000.swc");
  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/test.nnt001.swc");

  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/L1_209/p005.swc");

  //tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/L1_209.swc");
  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/L1d_181639.swc");
  //tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");
  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare3.swc");

  tree1->resample(50.0);
  tree2->resample(50.0);

  //tree2.translate(0, 0, 50.0);

  ZSwcTreeMatcher matcher;

  //ZSwcSpatialFeatureAnalyzer analyzer;



  ZSwcShollFeatureAnalyzer analyzer;
  vector<double> parameterArray(3);
  parameterArray[0] = 5.0;
  parameterArray[1] = 100.0;
  parameterArray[2] = 30.0;
  analyzer.setParamter(parameterArray);


  /*
  ZSwcSizeFeatureAnalyzer analyzer;
  analyzer.setExcludedLabel(0);
*/

  ZSwcDistTrunkAnalyzer trunkAnalyzer;

  matcher.setFeatureAnalyzer(&analyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  matcher.matchAllG(*tree1, *tree2);

  vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > matchingResult =
      matcher.matchingResult();

  Swc_Tree_Node_Set_Root(matchingResult[0].first);
  tree1->setDataFromNode(matchingResult[0].first, ZSwcTree::LEAVE_ALONE);
  Swc_Tree_Node_Set_Root(matchingResult[0].second);
  tree2->setDataFromNode(matchingResult[0].second, ZSwcTree::LEAVE_ALONE);

  tree1->setType(0);
  tree2->setType(1);


  tree1->resortId();
  tree2->translate(50, 0, 0);
  tree2->resortId();

  ZSwcNetwork network;
  network.addSwcTree(tree1);
  network.addSwcTree(tree2);

  tree1->labelTrunkLevel(&trunkAnalyzer);
  tree2->labelTrunkLevel(&trunkAnalyzer);

  tree1->setTypeByLabel();
  tree2->setTypeByLabel();

  for (size_t i = 0; i < matchingResult.size(); i++) {
    if (!Swc_Tree_Node_Is_Continuation(matchingResult[i].first) ||
        !Swc_Tree_Node_Is_Continuation(matchingResult[i].second)) {

      int size1 = SwcTreeNode::downstreamSize(matchingResult[i].first,
                                              SwcTreeNode::labelDifference);
      int size2 = SwcTreeNode::downstreamSize(matchingResult[i].second,
                                              SwcTreeNode::labelDifference);
      double weight = 1.0 -
          static_cast<double>(min(size1, size2)) / max(size1, size2);
      /*
      int size1 = SwcTreeNode::downstreamSize(matchingResult[i].first);
      int size2 = SwcTreeNode::downstreamSize(matchingResult[i].second);

      int size3 = SwcTreeNode::singleTreeSize(matchingResult[i].first)
                  - size1 + 1;
      int size4 = SwcTreeNode::singleTreeSize(matchingResult[i].second)
                  - size2 + 1;

      cout << "Size: " << size1 << " " << size2 << " " << endl;
      cout << "Size 2: " << size3 << " " << size4 << " " << endl;

      double weight = 1.0 -
          min(static_cast<double>(min(size1, size2)) / max(size1, size2),
              static_cast<double>(min(size3, size4)) / max(size3, size4));
*/
      if (weight > 0.5) {
        network.addConnection(matchingResult[i].first, matchingResult[i].second,
                              weight);
      }
    }
  }

  network.exportTxtFile("/Users/zhaot/Work/neutube/neurolabi/data/test.nnt");

  for (size_t i = 0; i < treeArray.size(); i++) {
    delete treeArray[i];
  }

#if 0
  ZSwcTree tree;
  tree.merge(tree1.data(), false);
  tree2.translate(0, 0, 50.0);
  tree.merge(tree2.data(), false);

  for (size_t i = 0; i < matchingResult.size(); i++) {
    if (!Swc_Tree_Node_Is_Continuation(matchingResult[i].first) ||
        !Swc_Tree_Node_Is_Continuation(matchingResult[i].second)) {
      ZPoint startPos(matchingResult[i].first->node.x,
                      matchingResult[i].first->node.y,
                      matchingResult[i].first->node.z);
      ZPoint endPos(matchingResult[i].second->node.x,
                    matchingResult[i].second->node.y,
                    matchingResult[i].second->node.z);

      int size1 = SwcTreeNode::downstreamSize(matchingResult[i].first);
      int size2 = SwcTreeNode::downstreamSize(matchingResult[i].second);

      size1 = min(size1,
                  SwcTreeNode::singleTreeSize(matchingResult[i].first)
                  - size1 + 1);
      size2 = min(size2,
                  SwcTreeNode::singleTreeSize(matchingResult[i].second)
                  - size2 + 1);

      cout << "Size: " << size1 << " " << size2 << " " << endl;

      int colorCode = 6;

      if (min(size1, size2) * 3 <= max(size1, size2) * 2) {
        colorCode = 2;
      }

      cout << SwcTreeNode::singleTreeSize(matchingResult[i].first) << endl;

      Swc_Tree_Node *tn =
          ZSwcTree::makeArrow(startPos, 1, colorCode, endPos, 0.2, 0);
      Swc_Tree_Node_Set_Parent(tn, tree.data()->root);
    }
  }

  tree.resortId();

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");

  ZSwcDistTrunkAnalyzer analyzer;
  analyzer.labelTraffic(&tree);
  //tree.labelBusyLevel();
  tree.setTypeByLabel();

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#if 0
  //Load all swc files
  ZFileList fileList;
  fileList.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/labeled",
                "swc");

  ZDoubleVector ratio(fileList.size(), 0.0);

  //For each swc file
  for (int i = 0; i < fileList.size(); i++) {
    //Extract the main trunk
    ZSwcTree  tree;
    tree.load(fileList.getFilePath(i));

    ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
    trunkAnalyzer.setDistanceWeight(1.0, 0.0);
    ZSwcPath branch = tree.mainTrunk(&trunkAnalyzer);

    //Count the nodes that have type 0
    int count = 0;

    for (ZSwcPath::iterator iter = branch.begin(); iter != branch.end();
         ++iter) {
      if (SwcTreeNode::type(*iter) == 0) {
        count++;
      }
    }

    //Calculate the ratio
    ratio[i] = static_cast<double>(count) / branch.size();
  }

  ratio.exportDataFile("/Users/zhaot/Work/neutube/neurolabi/data/test.bn");

#endif

#if 0
  //json project test

#endif

#if 0
  if (testTreeIterator()) {
    cout << "Testing passed." << endl;
  }
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/breadth_first.swc");
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  tree.labelTrunkLevel(&trunkAnalyzer);
#endif

#if 0
  ZCuboid cuboid1;
  ZCuboid cuboid2;

  /*
  cuboid1.set(1, 3, 1, 6, 8, 2);
  cuboid2.set(3, 1, 1, 7, 5, 2);

  ZPoint movingVec(0.5, 0.5, 0);

  cout << cuboid2.estimateSeparateScale(cuboid1, movingVec) << endl;
  */

  cuboid1.set(365.136, 205.338, -4.12311, 804.793, 739.338, 1462.38);
  cuboid2.set(373.934, 208.455, -7, 857.17, 740.455, 1433.12);
  //cout << cuboid2.estimateSeparateScale(cuboid1, ZPoint(1, 1, 1)) << endl;

  double scale = cuboid2.estimateSeparateScale(cuboid1, ZPoint(1, 1, 1));

  cuboid1.scale(scale);
  cuboid2.scale(scale);

  cuboid1.print();
  cuboid2.print();
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/test/flyem/adjusted/test_65535.swc");
  ZPoint pt;
  Swc_Tree_Centroid(tree.data(), pt.xRef(), pt.yRef(), pt.zRef());

  cout << pt.x() << " " << pt.y() << " " << pt.z() << endl;

  double corner[6];
  tree.boundBox(corner);

#endif

#if 0
  tr1::shared_ptr<int> a(new int[2]);
  cout << a.use_count() << endl;

  tr1::shared_ptr<int> b(a);
  cout << a.use_count() << endl;
  cout << b.use_count() << endl;

  tr1::shared_ptr<int> c;
  c = a;
  cout << a.use_count() << endl;
  cout << b.use_count() << endl;
  cout << c.use_count() << endl;

  a.reset();
  cout << a.use_count() << endl;
  cout << b.use_count() << endl;
  cout << c.use_count() << endl;

#endif

#if 0
  ZString str("/home/zhaot/test/test.tif");
  cout << str.toDirPath() << endl;

  ZString str2("Test2.tif");
  cout << str2.toAbsolutePath(str.toDirPath()) << endl;

  cout << str2.isAbsolutePath() << endl;
  cout << str2.toAbsolutePath(str.toDirPath()).isAbsolutePath() << endl;

  str2 = "Test3.tif";
  cout << str2 << endl;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa1;
  FlyEm::ZSynapseAnnotationArray sa2;

  sa1.loadJson(dataPath + "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/"
               "psd-530-exports/annotations-synapse_part.json");
  sa2.loadJson(dataPath + "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/"
               "psd-529-exports/annotations-synapse_part.json");
  /*
  sa1.loadJson(dataPath + "/benchmark/flyem/psd/synapse_annotation1.json");
  sa2.loadJson(dataPath + "/benchmark/flyem/psd/synapse_annotation2.json");
*/
  sa1.printSummary();
  sa2.printSummary();

  FlyEm::ZSynapseLocationMatcher matcherForEvaluate;
  /*
  matcherForEvaluate.load(sa1, sa2,
                          dataPath + "/benchmark/flyem/psd/golden_match.txt");
                          */
  matcherForEvaluate.load(sa1, sa2,
                          dataPath +
                          "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/"
                          "changl_sigmundc_matches.csv");
  FlyEm::ZSynapseLocationMatcher overallPsdMatcher;

  FlyEm::ZSynapseLocationAngleMetric metric;
  //FlyEm::ZSynapseLocationEuclideanMetric metric;
  overallPsdMatcher.setMetric(&metric);


  overallPsdMatcher.matchPsd(sa1, sa2, 0.4);

  cout << "Matching result: " << endl;
  for (size_t i = 0; i < overallPsdMatcher.size(); i++) {
    cout << overallPsdMatcher.getIndex(
              i, FlyEm::ZSynapseLocationMatcher::LEFT_SIDE) << " "
         << overallPsdMatcher.getIndex(
              i, FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE) << endl;
  }

  cout << "Unmatched: "
       << overallPsdMatcher.unmatchedSize(
            FlyEm::ZSynapseLocationMatcher::LEFT_SIDE)
       << " "
       << overallPsdMatcher.unmatchedSize(
            FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE)
       << endl;

  cout << "Golden result: " << endl;
  for (size_t i = 0; i < matcherForEvaluate.size(); i++) {
    cout << matcherForEvaluate.getIndex(
              i, FlyEm::ZSynapseLocationMatcher::LEFT_SIDE) << " "
         << matcherForEvaluate.getIndex(
              i, FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE) << endl;
  }

  overallPsdMatcher.evaluate(matcherForEvaluate);
  vector<pair<int, int> > wrongMatch = overallPsdMatcher.falseMatch();
  vector<pair<int, int> > missingMatch = overallPsdMatcher.missingMatch();

  vector<FlyEm::SynapseLocation*> locArray1 = sa1.toSynapseLocationArray();
  vector<FlyEm::SynapseLocation*> locArray2 = sa2.toSynapseLocationArray();

  cout << "Wrong matches:" << endl;
  for (size_t i = 0; i < wrongMatch.size(); i++) {
    FlyEm::SynapseLocation *loc1 = locArray1[wrongMatch[i].first];
    FlyEm::SynapseLocation *loc2 = locArray2[wrongMatch[i].second];
    cout << wrongMatch[i].first << " " << wrongMatch[i].second << " "
         << loc1->pos().toString() << " " << loc2->pos().toString() << endl;
  }

  cout << "Missing matches:" << endl;
  for (size_t i = 0; i < missingMatch.size(); i++) {
    FlyEm::SynapseLocation *loc1 = locArray1[missingMatch[i].first];
    FlyEm::SynapseLocation *loc2 = locArray2[missingMatch[i].second];
    cout << missingMatch[i].first << " " << missingMatch[i].second << " "
         << loc1->pos().toString() << " " << loc2->pos().toString() << endl;
  }

  overallPsdMatcher.exportPerformance(dataPath + "/test.html");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson(dataPath +
              "/benchmark/flyem/psd/synapse_annotation_multiple.json");
  ZIntTree mtbar = sa.buildTbarSequence(10.0);
  mtbar.print();
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson(dataPath +
              "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/psd-528-exports/annotations-synapse.json");
  vector<vector<FlyEm::SynapseLocation*> > mtbar = sa.buildTbarSequence(50.0);

  for (size_t i = 0; i < mtbar.size(); i++) {
    cout << mtbar[i][0]->pos().toString() ;
    for (size_t j = 1; j < mtbar[i].size(); j++) {
      cout << "-->" << mtbar[i][j]->pos().toString() << " " << endl;
    }
  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa1;
  FlyEm::ZSynapseAnnotationArray sa2;

  sa1.loadJson(dataPath +
               "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/psd-529-exports/annotations-synapse.json");
  sa2.loadJson(dataPath +
               "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/psd-531-exports/annotations-synapse.json");

  //For the first and second file
  vector<FlyEm::SynapseLocation*> tbarSet1 = sa1.toTBarRefArray();
  cout << "Tbar number 1: " << tbarSet1.size() << endl;

  vector<FlyEm::SynapseLocation*> tbarSet2 = sa2.toTBarRefArray();
  cout << "Tbar number 2: " << tbarSet2.size() << endl;

  //Match the tbars
  FlyEm::ZSynapseLocationMatcher matcher;
  const double tbarDistThre = 100.0 / 10.0;

  cout << "Matching TBars ..." << endl;
  matcher.match(tbarSet1, tbarSet2, tbarDistThre);
  cout << "Done." << endl;

  for (size_t i = 0; i < matcher.size(); i++) {
    int index1 = matcher.getIndex(i, FlyEm::ZSynapseLocationMatcher::LEFT_SIDE);
    int index2 = matcher.getIndex(i, FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE);

    cout << i << ": "
         << index1
         << " "
         << index2 << " "
         << tbarSet1[index1]->pos().distanceTo(tbarSet2[index2]->pos())
         << " | " << tbarSet1[index1]->pos().toString() << " "
         << tbarSet2[index2]->pos().toString()
         << endl;
  }

  cout << "Unmatched in 1: " << endl;
  for (size_t i = 0;
       i < matcher.unmatchedSize(FlyEm::ZSynapseLocationMatcher::LEFT_SIDE);
       i++) {
    int index = matcher.getIndex(i, FlyEm::ZSynapseLocationMatcher::LEFT_SIDE,
                                 FlyEm::ZSynapseLocationMatcher::UNMATCHED);
    cout << index << " " << tbarSet1[index]->pos().toString() << endl;

  }

  cout << "Unmatched in 2: " << endl;
  for (size_t i = 0;
       i < matcher.unmatchedSize(FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE);
       i++) {
    int index = matcher.getIndex(i, FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE,
                                 FlyEm::ZSynapseLocationMatcher::UNMATCHED);
    cout << index << " " << tbarSet2[index]->pos().toString() << endl;

  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson("/Users/zhaot/Work/neutube/neurolabi/cpp/psd/test/"
              "synapse_annotation1.json");
  sa.exportJsonFile(dataPath + "/test.json");

  vector<vector<int> > selected(2);
  selected[0].resize(2);
  selected[0][0] = 0;
  selected[0][1] = 1;

  selected[1].resize(4);
  selected[1][0] = 2;
  selected[1][1] = 0;
  selected[1][2] = 2;
  selected[1][3] = 3;

  sa.exportJsonFile(dataPath + "/test2.json", &selected);
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson("/Users/zhaot/Work/neutube/neurolabi/cpp/psd/test/"
              "synapse_annotation1.json");
  sa.printSummary();

  int count = 0;
  size_t n = sa.size() + sa.getPsdNumber();

  for (size_t index = 0; index <= n; index++) {
    pair<int, int> sub = sa.relativePsdIndex(index);
    if (sub.first >= 0 && sub.second >= 0) {
      if (sa.getPsdIndex(sub.first, sub.second) != (int) index) {
        cout << "Wrong index: " << index << " " << sub.first << " "
             << sub.second << " " << sa.getPsdIndex(sub.first, sub.second)
             << endl;
      }
    } else {
      cout << "Invalid index: " << index << " " << sub.first << " "
           << sub.second << endl;
      count++;
    }
  }
  cout << count << endl;

#endif

#if 0
  ZStackFile sf;
  sf.import(dataPath + "/*.tif");
  sf.print();
  /*
  int kind, width, height, depth;
  sf.retrieveAttribute(&kind, &width, &height, &depth);
  cout << width << " " << height << " " << depth << " " << kind << endl;
  ZStack *stack = sf.readStack();
  stack->save((dataPath + "/test.tif").c_str());
  */
#endif

#if 0
  ZSuperpixelMapArray mapArray;
  mapArray.load(dataPath +
                "/flyem/segmentation/ground_truth/superpixel_to_segment_map.txt",
                0);
  mapArray.print();

  ZIntMap segMapArray;
  segMapArray.load(dataPath +
                   "/flyem/segmentation/ground_truth/segment_to_body_map.txt");
  //segMapArray.print();

  mapArray.setBodyId(segMapArray);
  mapArray.print();
#endif

#if 0
  //string segPath = dataPath + "/flyem/segmentation/assignments/assignment_2";
  string segPath = dataPath + "/flyem/segmentation/ground_truth";
  ZSuperpixelMapArray mapArray;
  /*
  mapArray.load(dataPath +
                "/benchmark/flyem/superpixel_to_segment_map.txt");
                */
  mapArray.load(segPath + "/superpixel_to_segment_map.txt");
  //mapArray.print();

  ZIntMap segMapArray;
  //segMapArray.load(dataPath + "/benchmark/flyem/segment_to_body_map.txt");
  segMapArray.load(segPath +"/segment_to_body_map.txt");
  mapArray.setBodyId(segMapArray);
  //mapArray.print();


  ZStackFile file;

  /*
  file.importImageSeries(dataPath +
                         "/benchmark/flyem/superpixel_maps/sp_map.00030.png");
                         */
  file.importImageSeries(segPath + "/superpixel_maps/*.png");

  ZStack *stack = file.readStack();

  ZStack *newStack = mapArray.mapStack(*stack);
  newStack->save((segPath + "/body.tif").c_str());

  delete stack;
  delete newStack;
#endif

#if 0
  FlyEm::ZSegmentationAnalyzer analyzer;
  analyzer.compare(trueSeg, testSeg);

  ZStackFile file;
  file.importSeries(superpixelPath);
  ZStack *superpixel = file.readStack();

  ZStack *stack = analyzer.createErrorStack(superpixel);
  stack->save("../data/test.tif");
#endif

#if 0
  ZStackFile file;
  string groundTruthPath = dataPath + "/flyem/segmentation/ground_truth";
  file.import(groundTruthPath + "/superpixel_maps/*.png");
  ZStack *stack = file.readStack();

  ZStackProcessor processor;
  processor.mexihatFilter(stack, 3.0);
  stack->save(dataPath + "/test.tif");

#endif

#if 0
  string groundTruthPath = dataPath + "/flyem/segmentation/ground_truth";
  string testPath = dataPath + "/flyem/segmentation/assignments/assignment_2";

  ZStackFile greyFile;
  greyFile.import(groundTruthPath + "/superpixel_maps/*.png");

  ZStackFile superpixelFile;
  superpixelFile.import(groundTruthPath + "/superpixel_maps/*.png");

  string superpixelMapFile = groundTruthPath + "/superpixel_to_segment_map.txt";
  string segmentationMapFile = groundTruthPath + "/segment_to_body_map.txt";

  FlyEm::ZSegmentationBundle bundle;

  bundle.setGreyScaleSource(greyFile);
  bundle.setSuperpixelSource(superpixelFile);
  bundle.setSuperpixelMapSource(superpixelMapFile);
  bundle.setSegmentMapSource(segmentationMapFile);
  bundle.update();

  ZStack *stack = bundle.createBodyStack();
  stack->save(dataPath + "/test.tif");

  /*
  delete stack;

  bundle.setGreyScaleSource(greyFile);
  bundle.setSuperpixelMapSource(superpixelMapFile);
  bundle.setSegmentMapSource(segmentationMapFile);

  bundle.update(FlyEm::ZSegmentationBundle::SUPERPIXEL);
  bundle.update(FlyEm::ZSegmentationBundle::SUPERPIXEL_MAP);

  stack = bundle.createBodyStack();
  stack->save(dataPath + "/test2.tif");
  */

  superpixelFile.import(testPath + "/superpixel_maps/*.png");
  superpixelMapFile = testPath + "/superpixel_to_segment_map.txt";
  segmentationMapFile = testPath + "/segment_to_body_map.txt";
  bundle.setSuperpixelSource(superpixelFile);
  bundle.setSuperpixelMapSource(superpixelMapFile);
  bundle.setSegmentMapSource(segmentationMapFile);
  bundle.update(FlyEm::ZSegmentationBundle::SUPERPIXEL);
  bundle.update(FlyEm::ZSegmentationBundle::SUPERPIXEL_MAP);

  ZStack *testStack = bundle.createBodyBoundaryStack();
  testStack->save(dataPath + "/test2.tif");

  ZStackBlender blender;
  ZStack *blending = blender.blend(*stack, *testStack, 0.9);
  blending->save(dataPath + "/test3.tif");

  delete stack; stack = NULL;
  delete testStack; testStack = NULL;
  delete blending; blending = NULL;
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(dataPath + "/benchmark/flyem/segmentation.json");
  bundle.print();

  bundle.compressBodyId();

  bundle.getSuperpixelMap()->print();

  ZStack *stack = bundle.getBodyStack();

  uint8_t color_map[] = {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF};
  int ncolor = sizeof(color_map) / 3;

  Stack *out = Stack_Blend_Label_Field(bundle.getGreyScaleStack()->c_stack(),
                                       stack->c_stack(), 0.5, color_map, ncolor, NULL);


  C_Stack::write(dataPath + "/test.tif", out);
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(dataPath + "/flyem/TEM/slice_figure/segmentation/segmentation.json");
  bundle.trimSuperpixelMap();
  bundle.compressBodyId(0);
  //bundle.getSuperpixelMap()->print();

  ZStack *stack = bundle.getBodyStack();

  stack->save(dataPath + "/flyem/TEM/slice_figure/segmentation/blend_color.tif");
  Stack *out = Stack_Blend_Label_Field(
        bundle.getGreyScaleStack()->c_stack(),
        stack->c_stack(), 0.5, Discrete_Colormap, Discrete_Color_Number, NULL);

  C_Stack::write(dataPath + "/flyem/TEM/slice_figure/segmentation/blend_body.tif", out);
#endif

#if 0
  ZGraph *graph  = new ZGraph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);
  graph->addEdge(0, 2);
  graph->addEdge(1, 2);
  graph->addEdge(3, 2);
  graph->addEdge(3, 4);
  graph->addEdge(4, 5);
  graph->addEdge(2, 4);
  graph->addEdge(0, 4);
  graph->addEdge(0, 6);
  graph->addEdge(1, 7);
  graph->addEdge(0, 8);
  graph->addEdge(4, 9);
  graph->print();

  graph->exportDotFile(dataPath + "/test.dot");

  set<int> neighborSet = graph->getNeighborSet(2);

  cout << "Neighbors of " << 2 << ":" << endl;
  for (set<int>::const_iterator iter = neighborSet.begin();
       iter != neighborSet.end(); ++iter) {
    cout << *iter << " ";
  }
  cout << endl;

  vector<int> vertexArray(3);
  vertexArray[0] = 0;
  vertexArray[1] = 1;
  vertexArray[2] = 2;

  neighborSet = graph->getNeighborSet(vertexArray);

  cout << "Neighbors" << ":" << endl;
  for (set<int>::const_iterator iter = neighborSet.begin();
       iter != neighborSet.end(); ++iter) {
    cout << *iter << " ";
  }
  cout << endl;

  delete graph; graph = NULL;
#endif

#if 0
  ZArray::Dimn_Type dims[2] = { 2, 3 };
  ZArray array(mylib::UINT8_TYPE, 2, dims);
  array.printInfo();

  ZArray array2 = array;
  array2.printInfo();

#endif

#if 0
  ZStackFile file;
  file.import(dataPath + "/flyem/segmentation/assignments/assignment_2/mask.tif");

  tic();
  ZStack *stack = file.readStack();
  ptoc();

  vector<vector<double> > borderColor;

  int m, n;
  int *array = iarray_load_matrix(
        (dataPath + "/flyem/segmentation/assignments/assignment_2/seed.txt").c_str(),
        NULL, &m, &n);

  borderColor.resize(n);
  for (int i = 0; i < n; ++i) {
    borderColor[i].resize(m);
    for (int j = 0; j < m; ++j) {
      borderColor[i][j] = array[m * i + j];
    }
  }

  for (size_t i = 0; i < borderColor.size(); ++i) {
    for (size_t j = 0; j < borderColor[i].size(); ++j) {
      cout << borderColor[i][j] << " ";
    }
    cout<< endl;
  }

  free(array);

/*
  c1[0] = 151;
  c1[1] = 67;
  c1[2] = 0;

  c2[0] = 244;
  c2[1] = 64;
  c2[2] = 0;

  ZStack *stack = new ZStack(GREY, 3, 3, 1, 3);
  stack->setValue(0, 0, c1[0]);
  stack->setValue(0, 1, c1[1]);
  stack->setValue(0, 2, c1[2]);

  stack->setValue(1, 0, c2[0]);
  stack->setValue(1, 1, c2[1]);
  stack->setValue(1, 2, c2[2]);
*/

  for (size_t i = 0; i < borderColor.size() / 2; ++i){
    vector<double> feature = FlyEm::ZSegmentationAnalyzer::touchFeature(
          *stack, borderColor[i * 2], borderColor[i * 2 + 1]);
    cout << feature[0] << " " << feature[1] << endl;
  }

  delete stack;
#endif

#if 0
  ZIntPairMap pairMap;
  pairMap.incPairCount(1, 2);
  pairMap.incPairCount(2, 3);
  pairMap.incPairCount(1, 2);
  pairMap.incPairCount(3, 4);
  pairMap.printSummary();
  pairMap.print();

  ZIntMap intMap;
  intMap.incValue(1);
  intMap.incValue(2);
  intMap.incValue(2);
  intMap.incValue(2);
  intMap.incValue(3);
  intMap.incValue(4);
  intMap.print();

  ZIntMap bodyCorrespondence =
      FlyEm::ZSegmentationAnalyzer::inferBodyCorrespondence(
        pairMap, intMap);
  bodyCorrespondence.print();
  bodyCorrespondence.print(ZIntMap::KEY_GROUP);
#endif

#if 0
  string testSegPath = dataPath + "/flyem/segmentation/assignments/assignment_2/segmentation.json";
  string trueSegPath = dataPath + "/flyem/segmentation/ground_truth/segmentation.json";

  //string testSegPath = dataPath + "/benchmark/flyem2/test/segmentation.json";
  //string trueSegPath = dataPath + "/benchmark/flyem2/truth/segmentation.json";

  FlyEm::ZSegmentationBundle testSegBundle;
  FlyEm::ZSegmentationBundle trueSegBundle;

  cout << "Load " << dataPath << endl;
  testSegBundle.importJsonFile(testSegPath);
  //testSegBundle.update();

  cout << "Load " << trueSegPath << endl;
  trueSegBundle.importJsonFile(trueSegPath);
  //trueSegBundle.update();

  ZStack *testBodyStack = testSegBundle.getBodyStack();
  ZStack *trueBodyStack = trueSegBundle.getBodyStack();

  ZStack *b1 = testSegBundle.getBodyBoundaryStack();
  ZStack *b2 = trueSegBundle.getBodyBoundaryStack();

  uint8_t *array1 = b1->array8(0);
  uint8_t *array2 = b2->array8(0);

  size_t volume = b1->getVoxelNumber();

  u8array_max2(array1, array2, volume);

#ifdef _DEBUG_2
  printf("%p\n", testBodyStack->singleChannelStack()->array8());
  cout << *(testBodyStack->singleChannelStack()->array8() + 100) << endl;
  ZDoubleVector::print(testBodyStack->color(100));
  printf("%p\n", testBodyStack->singleChannelStack()->array8());
  cout << (int) *(testBodyStack->singleChannelStack(0)->array8() + 100) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(1)->array8() + 100) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(2)->array8() + 100) << endl;
  ZDoubleVector::print(testBodyStack->color(100));
  cout << testBodyStack->singleChannelStack(0)->value(0) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(0)->array8() + 100) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(1)->array8() + 100) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(2)->array8() + 100) << endl;
  cout << FlyEm::ZSegmentationAnalyzer::channelCodeToId(testBodyStack->color(100)) << endl;
  ZDoubleVector::print(testBodyStack->color(100));
  return;
#endif

  ZIntPairMap overlap = FlyEm::ZSegmentationAnalyzer::computeOverlap(
        testBodyStack, trueBodyStack, b1);

  cout << "Overlap: " << endl;
  overlap.print();

  ZIntMap testBodySize =
      FlyEm::ZSegmentationAnalyzer::computeBodySize(testBodyStack,
                                                    testSegBundle.getBodyBoundaryStack());

  cout << "Body size: " << endl;
  ofstream testBodyStream((dataPath + "/test_body_size.txt").c_str());

  testBodySize.print(testBodyStream);

#ifdef _DEBUG_2
  return;
#endif

  ZIntMap bodyCorrespondence =
      FlyEm::ZSegmentationAnalyzer::inferBodyCorrespondence(
        overlap, testBodySize);

  cout << "Body correspondence: " << endl;
  bodyCorrespondence.print();

  bodyCorrespondence.print(cout, ZIntMap::KEY_GROUP);

  ZGraph *bodyGraph = testSegBundle.getBodyGraph();
  cout << "Body graph: " << endl;
  bodyGraph->exportDotFile(dataPath + "/test.dot");

#ifdef _DEBUG_2
  bodyGraph->clean();
  //bodyGraph->addEdge(68206, 61888);
  bodyGraph->addEdge(17303, 9128);
  bodyGraph->print();
#endif
/*
  vector<vector<double> > feature =
      FlyEm::ZSegmentationAnalyzer::touchFeature(*testSegBundle.getBodyStack(),
                                                 *bodyGraph,
                                                 *testSegBundle.getBodyBoundaryStack());
*/
  vector<vector<double> > feature =
      FlyEm::ZSegmentationAnalyzer::touchFeature(testSegBundle);
  //ZGraph oversplit(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  ofstream stream((dataPath + "/feature.txt").c_str());

  for (size_t i = 0; i < feature.size(); i++) {
    int id1 = bodyGraph->edgeStart(i);
    int id2 = bodyGraph->edgeEnd(i);

    stream << id1 << " " << id2 << " ";
    for (vector<double>::const_iterator featureIter = feature[i].begin();
         featureIter != feature[i].end(); ++featureIter) {
      stream << *featureIter << " ";
    }

    if (bodyCorrespondence[id1] == bodyCorrespondence[id2]) {
      stream << 1 << endl;
    } else {
      stream << 0 << endl;
    }
  }

  stream.close();

/*
  cout << "oversplit: " << endl;
  oversplit.print();
  oversplit.exportDotFile(dataPath + "/test.dot");

  ZDoubleVector::exportTxtFile(feature, dataPath + "/feature.txt");
  */
#endif

#if 0
  vector<double> vec(10);
  for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = i;
  }

  vector<vector<double> > array2d = ZDoubleVector::reshape(vec, 4);

  ZDoubleVector::print(vec);
  ZDoubleVector::print(array2d);

#endif

#if 0
  Stack *stack = Read_Stack_U(
        (dataPath + "/benchmark/binary/2d/disk_n1.tif").c_str());
  int seedArray[] = { 2935, 1111, 2927 };

  vector<double> direction =
      FlyEm::ZSegmentationAnalyzer::computeRayburstDirection(
        stack, seedArray, sizeof(seedArray) / sizeof(int));

  ZDoubleVector::print(direction);

  double x, y, z;
  Geo3d_Orientation_Normal(direction[0], direction[1], &x, &y, &z);
  cout << x << " " << y << " " << z << endl;
#endif

#if 0
  ZJsonObject jsonObject;
  jsonObject.load(dataPath + "/test.json");
  map<const char*, json_t *> entryMap = jsonObject.toEntryMap();
  for (map<const char*, json_t *>::const_iterator iter = entryMap.begin();
       iter != entryMap.end(); ++iter) {
    cout << "\"" << iter->first << "\": " << ZJsonParser::type(iter->second)
         << endl;
  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationAnalyzer analyzer;
  analyzer.loadConfig(dataPath + "/config.txt");
  analyzer.print();

  cout << endl;
  analyzer.loadConfig(dataPath + "/config.json");
  analyzer.print();
#endif

#if 0
  //Test if a body is mitochondria
  FlyEm::ZSegmentationBundle bundle;

  string testPath = dataPath + "/flyem/segmentation/assignments/assignment_2";
  string segFile = testPath + "/segmentation.json";

  cout << "Load segmetnation ..." << endl;
  bundle.importJsonFile(segFile);
  //bundle.update();

  cout << "Retrieve body stack ..." << endl;
  ZStack *bodyStack = bundle.getBodyStack();

  vector<double> bodyCode(3);
  bodyCode[0] = 147;
  bodyCode[1] = 231;
  bodyCode[2] = 0;

  cout << "Create body mask ..." << endl;
  vector<vector<double> > selected;
  selected.push_back(bodyCode);
  ZStack *objStack = bodyStack->createSubstack(selected);

  //objStack->save(dataPath + "/test.tif");

  string mitoConfidenceFile = testPath + "/mito_pred.tif";

  cout << "Load prediction ..." << endl;
  ZStackFile stackFile;
  stackFile.import(mitoConfidenceFile);
  ZStack *mito = stackFile.readStack();

  cout << "Average intensity:" << endl;
  double conf = mito->averageIntensity(objStack);

  cout << conf << endl;

  map<int, double> allConf =
      FlyEm::ZSegmentationAnalyzer::computeAverageIntensity(bodyStack, mito);

  ofstream stream((testPath + "/mito_conf.txt").c_str());

  vector<int> mitoId;

  for (map<int, double>::const_iterator iter = allConf.begin();
       iter != allConf.end(); ++iter) {
    stream << iter->first << " " << iter->second << endl;
    if (iter->second > 80.0) {
      mitoId.push_back(iter->first);
    }
  }

  cout << "Extract orphan bodies ..." << endl;
  set<int> orphanSet = bundle.getAllOrphanBody();
  cout << "Retrieve body graph ..." << endl;
  ZGraph* bodyGraph = bundle.getBodyGraph();

  bodyGraph->exportTxtFile(dataPath + "/graph.txt");

  //Output ophan bodies connected to mitochondria
  //For each mito id
  for (vector<int>::const_iterator iter = mitoId.begin(); iter != mitoId.end();
       ++iter) {
    cout << "Mito " << *iter << ": ";
    //Find the neighbors
    set<int> neighbors = bodyGraph->getNeighborSet(*iter);
    //For each neighbor
    for (set<int>::const_iterator neighborIter = neighbors.begin();
         neighborIter != neighbors.end(); ++neighborIter) {
      //If it is an ophan
      if (orphanSet.count(*neighborIter) > 0) {
        //Output the id
        cout << *neighborIter << " ";
      } else {
        cout << "[" << *neighborIter << "]" << " ";
      }
    }
    cout << endl;
  }
#endif

#if 0
  ZString str("test.tif");
  cout << str.endsWith("tif") << endl;
  cout << str.endsWith("TIF") << endl;
  cout << str.endsWith(".tif") << endl;
  cout << str.endsWith(".TIF") << endl;
  cout << str.endsWith(".tiff") << endl;
  cout << endl;

  cout << str.endsWith("tif", ZString::CASE_INSENSITIVE) << endl;
  cout << str.endsWith("TIF", ZString::CASE_INSENSITIVE) << endl;
  cout << str.endsWith(".tif", ZString::CASE_INSENSITIVE) << endl;
  cout << str.endsWith(".TIF", ZString::CASE_INSENSITIVE) << endl;
  cout << str.endsWith(".tiff", ZString::CASE_INSENSITIVE) << endl;
  cout << endl;

  cout << str.startsWith("test") << endl;
  cout << str.startsWith("Test") << endl;
  cout << str.startsWith("test", ZString::CASE_INSENSITIVE) << endl;
  cout << str.startsWith("Test", ZString::CASE_INSENSITIVE) << endl;

  cout << ZFileType::typeName(ZFileType::fileType(str.c_str())) << endl;

  cout << ZFileType::typeName(ZFileType::fileType("threata.LSM")) << endl;
#endif

#if 0
  string testSegPath = dataPath + "/benchmark/flyem2/test/segmentation.json";
  //string trueSegPath = dataPath + "/benchmark/flyem2/truth/segmentation.json";

  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(testSegPath);
  bundle.update();

  vector<int> *bodyList = bundle.getBodyList();
  iarray_print(&(*bodyList)[0], bodyList->size());

  map<int, size_t> *bodyIndexMap = bundle.getBodyIndexMap();
  for (map<int, size_t>::const_iterator iter = bodyIndexMap->begin();
       iter != bodyIndexMap->end(); ++iter) {
    cout << iter->first << " : " << iter->second << endl;
  }

#endif

#if 0
  string testSegPath = dataPath + "/benchmark/flyem3/test/segmentation.json";
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(testSegPath);
  //bundle.update();

  bundle.getSuperpixelMap()->print();

  /*
  set<int> orphans = bundle.getAllOrphanBody();
  for (set<int>::const_iterator iter = orphans.begin(); iter != orphans.end();
       ++iter) {
    cout << *iter << " ";
  }
  cout << endl;

  bundle.getBodyStack()->save(dataPath + "/test.tif");
  */
  ZDoubleVector::print(bundle.getBodyStack()->color(0));
#endif

#if 0
  ZStack stack;
  ZStackFile stackFile;
  stackFile.import(dataPath + "/benchmark/series/image*.tif");
  stackFile.readStack(&stack);

  stack.save(dataPath + "/test.tif");
#endif

#if 0
  ZStackFile stackFile;
  stackFile.import(dataPath + "/benchmark/fork_2d.tif");
  ZStack *stack = stackFile.readStack();

  //stack->binarize(0);

  //stack->clone()->save(dataPath + "/test.tif");

  ZDoubleVector::print(stack->color(10));
  ZDoubleVector::print(stack->color(10));
#endif

#if 0
  ZStack *stack = new ZStack(COLOR, 10, 10, 5, 1);
  stack->setValue(0, 0, 255);
  //cout << stack->hasSameValue(0, 1) << endl;
  //cout << stack->hasSameValue(1, 2) << endl;

  stack->save(dataPath + "/test.tif");
#endif

#if 0
  //Calculate boundary features
  string boundaryFilePath =
      dataPath + "/flyem/segmentation/assignments/assignment_2/boundary_pred.tif";
  string segFilePath =
      dataPath + "/flyem/segmentation/assignments/assignment_2/segmentation.json";

/*
  string boundaryFilePath =
      dataPath + "/benchmark/flyem2/test/boundary_pred.tif";
  string segFilePath =
      dataPath + "/benchmark/flyem2/test/segmentation.json";
*/
  ZStackFile stackFile;
  stackFile.import(boundaryFilePath);

  ZStack *boundaryProb = stackFile.readStack();

  FlyEm::ZSegmentationBundle bundle;

  bundle.importJsonFile(segFilePath);

  ZGraph *bodyGraph = bundle.getBodyGraph();

  ZGraph newBodyGraph(ZGraph::UNDIRECTED_WITH_WEIGHT);

  //For each edge in the body graph, calculate the weight
  for (size_t edgeIndex = 0; edgeIndex < bodyGraph->size(); ++edgeIndex) {
    cout << edgeIndex + 1 << "/" << bodyGraph->size() << endl;

    int id1 = bodyGraph->edgeStart(edgeIndex);
    int id2 = bodyGraph->edgeEnd(edgeIndex);

    ZObject3d *border = bundle.getBodyBoundaryObject(id1, id2);

    //border->print();

    if (border->size() == 0) {
      border = bundle.getBodyBoundaryObject(id1, id2);
      cout << id1 << " " << id2 << " " <<bodyGraph->getEdgeWeight(id1, id2) << endl;
      cout << "debug here" << endl;
    }

    double v = border->averageIntensity(boundaryProb->c_stack());
    newBodyGraph.addEdge(id1, id2, v);

    delete border;
  }

  string graphFilePath = dataPath + "/graph.txt";

  newBodyGraph.exportTxtFile(graphFilePath);
#endif

#if 0
  ofstream script((dataPath + "/test.sh").c_str());

  ZFileList fileList;
  fileList.load("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/T4viz",
                "swc");
  for (int i = 0; i < fileList.size(); ++i) {
    ZString str = fileList.getFilePath(i);
    ZString output = str;
    output.replace(".swc", ".marker");
    int id = str.lastInteger();
    script << "/Users/zhaot/Work/neutube/neurolabi/cpp/debug/bin/project_synapse --json /Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/annotations-synapse.json --swc "
         << str.c_str() << " --body_id " << id << " -o " << output.c_str()
         << endl;
  }

  script.close();
#endif

#if 0
  ZJsonValue jValue;
  jValue.decodeString("{ \"test\": 1, \"value\": [1, 2, 3, 4] }");
  jValue.print();
#endif

#if 0
  FlyEm::ZFileParser::readVaa3dMarkerFile(dataPath + "/flyem/TEM/T4viz/T4_1_277709_2.marker");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(dataPath + "/benchmark/flyem/annotations-synapse.json");
  synapseArray.print();

  FlyEm::SynapseDisplayConfig displayConfig;
  displayConfig.mode = FlyEm::SynapseDisplayConfig::PSD_ONLY;
  displayConfig.bodyId = 16711935;

  vector<ZVaa3dMarker> markerArray =
      synapseArray.toMarkerArray(FlyEm::SynapseAnnotationConfig(),
                                 FlyEm::SynapseLocation::ORIGINAL_SWC_SPACE,
                                 displayConfig);

  for (size_t i = 0; i < markerArray.size(); ++i) {
    cout << markerArray[i].toString() << endl;
  }
#endif

#if 0
  ZPoint pt1(10, 0, 0);
  ZPoint pt2(0, 0, 0);
  ZPoint pt = pt2 - pt1;

  cout << pt.toString() << endl;
#endif

#if 0
  //string neuron = "T4_1_277709";
  //string neuron = "T4_2_386464";
  //string neuron = "T4_10_476680";
  //string neuron = "T4_16_475117";
  string neuronArray[4] = { "T4_2_386464", "T4_17_547009", "T4_10_476680",
                            "T4_16_475117" };

  for (size_t neuronIndex = 0; neuronIndex < 4; ++neuronIndex) {
    string neuron = neuronArray[neuronIndex];

    string markerFile = dataPath +
        "/skeletonization/session2/len15/adjusted/" + neuron + ".marker";
    vector<ZVaa3dMarker> markerArray =
        FlyEm::ZFileParser::readVaa3dMarkerFile(markerFile);

    string mi1ListFile = dataPath + "/flyem/TEM/mi1_list.txt";
    string tm3ListFile = dataPath + "/flyem/TEM/tm3_list.txt";

    set<int> mi1IdArray;
    set<int> tm3IdArray;

    ZString str;
    FILE *fp = fopen(mi1ListFile.c_str(), "r");
    while (str.readLine(fp)) {
      int bodyId = str.firstInteger();
      if (bodyId > 0) {
        mi1IdArray.insert(bodyId);
      }
    }
    fclose(fp);

    fp = fopen(tm3ListFile.c_str(), "r");
    while (str.readLine(fp)) {
      int bodyId = str.firstInteger();
      if (bodyId > 0) {
        tm3IdArray.insert(bodyId);
      }
    }
    fclose(fp);

    vector<ZVaa3dMarker> newMarkerArray;

    for (size_t i = 0; i < markerArray.size(); ++i) {
      int id = String_Last_Integer(markerArray[i].name().c_str());
      markerArray[i].setRadius(markerArray[i].radius() * 2.5);
      if (mi1IdArray.count(id) > 0) {
        markerArray[i].setName("Mi1");
        markerArray[i].setColor(0, 0, 255);
        newMarkerArray.push_back(markerArray[i]);
      }
      if (tm3IdArray.count(id) > 0) {
        markerArray[i].setName("Tm3");
        markerArray[i].setColor(255, 0, 0);
        newMarkerArray.push_back(markerArray[i]);
      }
    }

    string outFile = dataPath +
        "/skeletonization/session2/len15/adjusted/" + neuron + "_sorted.marker";
    FlyEm::ZFileParser::writeVaa3dMakerFile(outFile, newMarkerArray);
  }
#endif

#if 0
  ZStack stack;
  stack.load("/Users/feng/Downloads/For_Programming.lsm");
  stack.logLSMInfo();
#endif

#if 0
  string mi1ListFile = dataPath + "/flyem/TEM/mi1_list.txt";
  ZString str;
  FILE *fp = fopen(mi1ListFile.c_str(), "r");

  while (str.readLine(fp)) {
    vector<string> wordArray = str.toWordArray();
    if (wordArray.size() == 2) {
      cout << "sh -x ./flyem_skeletonize " << wordArray[0]
           << " /groups/flyem/home/zhaot/Work/neutube_ws/neurolabi/data/skeletonization/session2 "
           << wordArray[1] << endl << endl;
    } else if (wordArray.size() == 1 || wordArray.size() > 3) {
      cout << "bug?" << endl;
    }
  }
  fclose(fp);
#endif

#if 0
  string neuronArray[4] = { "T4_2_386464", "T4_17_547009", "T4_10_476680",
                            "T4_16_475117" };

  for (size_t neuronIndex = 0; neuronIndex < 4; ++neuronIndex) {
    ZSwcTree tree;
    string filePath = dataPath + "/flyem/skeletonization/session2/len15/adjusted/" +
        neuronArray[neuronIndex] + "_rooted.swc";
    tree.load(filePath.c_str());
    ofstream output((filePath + ".angle.txt").c_str());

    tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    //double vec[3];
    double angle;
    for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
      if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
        ZPoint vec = SwcTreeNode::localDirection(tn, 5);
        //cout << vec[0] << " " << vec[1] << " " << vec[2] << endl;
        angle = Vector_Angle(vec.x(), vec.y());
        output << SwcTreeNode::localRadius(tn, 5) << " " << angle << endl;
      }
    }
    output.close();
  }
#endif

#if 0
  Z3DGraph graph;
  graph.importJsonFile(dataPath + "/test.json");
  graph.print();
#endif

#if 0
  //string swcFile = dataPath + "/flyem/skeletonization/session2/len15/adjusted/T4_2_386464_trunk.swc";
  //string swcFile = dataPath + "/flyem/skeletonization/session2/len15/adjusted/T4_10_476680_trunk.swc";
  //string swcFile = dataPath + "/flyem/skeletonization/session2/len15/adjusted/T4_16_475117_trunk.swc";
  string neuronArray[] = {
    "T4_1_277709", "T4_2_386464", "T4_3_280303", "T4_4_2341",
    "T4_5_278848", "T4_6_545716", "T4_7_455131", "T4_8_545944",
    "T4_9_5189", "T4_10_476680", "T4_11_588435", "T4_12_460193",
    "T4_13_515936", "T4_14_586983", "T4_15_546035", "T4_16_475117",
    "T4_17_547009", "T4_18_547221", "T4_19_455135"
  };

  for (size_t neuronIndex = 0; neuronIndex < sizeof(neuronArray) / sizeof(string); ++neuronIndex) {
    string swcFile = dataPath + "/flyem/skeletonization/session2/len15/adjusted/" + neuronArray[neuronIndex] + "_labeled.swc";
    //string swcFile = dataPath + "/benchmark/swc/fork.swc";
    string outFile = swcFile + ".angle.txt";

    ZSwcTree tree;
    tree.load(swcFile.c_str());

    tree.labelBranchLevelFromLeaf();
/*
    ZSwcWeightTrunkAnalyzer trunkAnalyzer;
    trunkAnalyzer.labelTraffic(&tree);
    */
    //tree.labelBranchLevel();
    //ZSwcDistTrunkAnalyzer trunkAnalyzer;
    /*
    ZSwcWeightTrunkAnalyzer trunkAnalyzer;
    tree.setBranchSizeWeight();
    //trunkAnalyzer.labelTraffic(&tree);
    tree.labelTrunkLevel(&trunkAnalyzer);
    //tree.print();
*/
    ofstream out(outFile.c_str());

    tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
      if (SwcTreeNode::type(tn) != 1) {
        ZWeightedPointArray segment = SwcTreeNode::localSegment(tn, 3);
        if (segment.size() == 7) {
          //cout << "Segment: " << segment[0].toString() << "-->"
          //     << segment.back().toString() << endl;
          //ZPoint vec = segment.principalDirection();
          ZPoint vec = segment.front() -segment.back();
          vec.setZ(0);
          vec.normalize();

          //cout << "Direction: ";

          out << SwcTreeNode::label(tn) << " " << Vector_Angle(vec.x(), vec.y())
              << endl;
          //out << vec.x() << " " << vec.y() << " "
          //    << SwcTreeNode::label(tn) << endl;

          /*
        if (neuronIndex == 3) {
          cout << outFile.c_str();
          cout << vec.x() << " " << vec.y() << " "
              << SwcTreeNode::label(tn) << endl;
        }
        */
        }
      }
    }

    out.close();
  }
#endif

#if 0
  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/swc/forest1.swc").c_str());
  tree.labelBranchLevel();

  tree.print();
#endif

#if 0
  vector<int> array(10);
  for (int i = 0; i < 10; i++) {
    array[i] = 10;
  }

  ZDebugPrintArray(array, 1, 5);

  array.resize(5);
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(dataPath + "/flyem/TEM/slice_figure/segmentation/segmentation.json");
  bundle.print();

//  ZStack *stack = bundle.getBodyStack();

//  stack->save(dataPath + "test.tif");

  ofstream stream((dataPath + "/flyem/TEM/slice_figure/segmentation/segment_to_body_map_partial.txt").c_str());

  ZSuperpixelMapArray *mapArray = bundle.getSuperpixelMap();
  //mapArray->print();
  int count = 0;
  for (ZSuperpixelMapArray::iterator iter = mapArray->begin();
       iter != mapArray->end(); ++iter) {

    stream << iter->segmentId() << " " << iter->bodyId() << endl;

    if (iter->planeId() >= 638 && iter->planeId() <= 640) {
      count++;
    }
  }

  //stream.close();
  cout << count << " superpixels" << endl;

#endif

#if 0
  vector<Swc_Tree_Node*> nodeArray(5);
  for (size_t i = 0; i < 5; ++i) {
    nodeArray[i] = SwcTreeNode::makeVirtualNode();
    SwcTreeNode::setId(nodeArray[i], i);
    SwcTreeNode::setPos(nodeArray[i], i * i, 0, 0);
  }

  SwcTreeNode::connect(nodeArray);

  ZSwcTree tree;
  tree.setDataFromNodeRoot(nodeArray[0]);

  tree.print();
#endif

#if 0
  ZSuperpixelMapArray mapArray;
  mapArray.append(0, 1, 2, 3);
  mapArray.append(0, 2, 3, 13);
  mapArray.append(1, 3, 5, 35);

  mapArray.print();

  mapArray.compressBodyId();
  mapArray.print();
#endif

#if 0
  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/swc/compare/compare1.swc").c_str());
  tree.labelBranchLevelFromLeaf();
  tree.print();
#endif

#if 0
  ZSwcTree tree;
  tree.load((dataPath + "/flyem/TEM/T4_Axon/T4_2_w_axon_final.swc").c_str());

  ZSwcTree *boxTree = tree.createBoundBoxSwc();
  boxTree->save((dataPath + "/test.swc").c_str());

  delete boxTree;
#endif

#if 0
  ZCuboid box(200, -400, 1000, 800, 600, 3000);
  ZSwcTree *boxTree = ZSwcTree::createCuboidSwc(box);
  boxTree->save((dataPath + "/test.swc").c_str());

  delete boxTree;

  ZSwcTree tickSwc;
  tickSwc.forceVirtualRoot();


  for (int z = 1000; z <= 3000; z += 100) {
    double x = box.corner(0).x();
    double y = box.corner(0).y();
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(ZPoint(x, y, z), 10);
    SwcTreeNode::setParent(tn, tickSwc.root());
    Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(ZPoint(x + 100, y, z), 10);
    SwcTreeNode::setParent(tn2, tn);
  }

  for (int z = 1000; z <= 3000; z += 100) {
    double x = box.corner(3).x();
    double y = box.corner(3).y();
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(ZPoint(x, y, z), 10);
    SwcTreeNode::setParent(tn, tickSwc.root());
    Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(ZPoint(x - 100, y, z), 10);
    SwcTreeNode::setParent(tn2, tn);
  }

  tickSwc.resortId();

  tickSwc.save((dataPath + "/test2.swc").c_str());

#endif

#if 0
  hid_t       file_id, dataset_id, dataspace_id;  /* identifiers */
  hsize_t     dims[2];
  herr_t      status;

  /* Create a new file using default properties. */
  file_id = H5Fcreate((dataPath + "/test.h5").c_str(),
                      H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  /* Create the data space for the dataset. */
  dims[0] = 4;
  dims[1] = 6;
  dataspace_id = H5Screate_simple(2, dims, NULL);

  /* Create the dataset. */
  dataset_id = H5Dcreate(file_id, "/dset", H5T_STD_I32BE, dataspace_id,
                         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* End access to the dataset and release resources used by it. */
  status = H5Dclose(dataset_id);

  /* Terminate access to the data space. */
  status = H5Sclose(dataspace_id);

  /* Close the file. */
  status = H5Fclose(file_id);

#endif

#if 0
  ZHdf5Reader reader;
  reader.open(dataPath + "/test.h5");

  mylib::Array *array = reader.readArray("/dset");
  mylib::printArrayInfo(array);
  mylib::Print_Array(array, stdout, 0, "%d");
#endif

#if 0
  ZHdf5Reader reader(dataPath + "/flyem/segmentation/assignments/assignment_2/stack.h5");

  reader.printInfo();

  mylib::Array *array = reader.readArray("/segment_superpixels");
  mylib::printArrayInfo(array);
#endif

#if 0

#if defined(_USE_OPENCV_)
  int rowNumber, columnNumber;
  float *trainingArray = farray_load_matrix((dataPath + "/train.txt").c_str(), NULL,
                                          &columnNumber, &rowNumber);
  float *labelArray = farray_malloc(rowNumber);
  for (int i = 0; i < rowNumber; ++i) {
    labelArray[i] = trainingArray[i * columnNumber + columnNumber - 1] - 1;
  }

  for (int i = 0; i < rowNumber; ++i) {
    //cout << "Move from " << i * columnNumber << " to " << i * (columnNumber - 1) << endl;
    memmove(trainingArray + i * (columnNumber - 1), trainingArray + i * columnNumber,
            sizeof(float) * (columnNumber - 1));
  }

  cv::Mat trainingData(rowNumber, columnNumber - 1, CV_32FC1, trainingArray);
  cv::Mat trainingLabel(rowNumber, 1, CV_32FC1, labelArray);

  cv::Mat var_type = cv::Mat(columnNumber, 1, CV_8U);
  var_type.setTo(cv::Scalar(CV_VAR_NUMERICAL) );

  float priors[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  CvRTParams params = CvRTParams(5, // max depth
                                 2, // min sample count
                                 0, // regression accuracy: N/A here
                                 false, // compute surrogate split, no missing data
                                 2, // max number of categories (use sub-optimal algorithm for larger numbers)
                                 priors, // the array of priors
                                 true,  // calculate variable importance
                                 0,      // number of variables randomly selected at node and used to find the best split(s).
                                 5,	 // max number of trees in the forest
                                 0.01f,				// forrest accuracy
                                 CV_TERMCRIT_EPS // termination cirteria
                                 );
  CvRTrees* rtree = new CvRTrees;

  rtree->train(trainingData, CV_ROW_SAMPLE, trainingLabel,
               cv::Mat(), cv::Mat(), var_type, cv::Mat(), params);
  rtree->save(dataPath + "/segmentation_classifier.cvt");


  float *testingArray = farray_load_matrix((dataPath + "/testing.txt").c_str(), NULL,
                                           &columnNumber, &rowNumber);
  float *testingLabelArray = farray_malloc(rowNumber);
  for (int i = 0; i < rowNumber; ++i) {
    testingLabelArray[i] = testingArray[i * columnNumber + columnNumber - 1] - 1;
  }

  for (int i = 0; i < rowNumber; ++i) {
    //cout << "Move from " << i * columnNumber << " to " << i * (columnNumber - 1) << endl;
    memmove(testingArray + i * (columnNumber - 1), testingArray + i * columnNumber,
            sizeof(float) * (columnNumber - 1));
  }

  cv::Mat testingData(rowNumber, columnNumber - 1, CV_32FC1, testingArray);

  for (int i = 0; i < testingData.rows; ++i) {
    float result = rtree->predict(testingData.row(i), cv::Mat());
    cout << " " << testingLabelArray[i] << " " << (result > 0.5) << endl;
  }
#endif

#endif

#if 0

#if defined(_USE_OPENCV_)
  int rowNumber, columnNumber;
  float *trainingArray = farray_load_matrix((dataPath + "/flyem/train.txt").c_str(), NULL,
                                          &columnNumber, &rowNumber);
  float *labelArray = farray_malloc(rowNumber);
  for (int i = 0; i < rowNumber; ++i) {
    labelArray[i] = trainingArray[i * columnNumber + columnNumber - 1];
  }

  for (int i = 0; i < rowNumber; ++i) {
    //cout << "Move from " << i * columnNumber << " to " << i * (columnNumber - 1) << endl;
    memmove(trainingArray + i * (columnNumber - 1), trainingArray + i * columnNumber,
            sizeof(float) * (columnNumber - 1));
  }

  cv::Mat trainingData(rowNumber, columnNumber - 1, CV_32FC1, trainingArray);
  cv::Mat trainingLabel(rowNumber, 1, CV_32FC1, labelArray);

  cv::Mat var_type = cv::Mat(columnNumber, 1, CV_8U);
  var_type.setTo(cv::Scalar(CV_VAR_NUMERICAL) );

  float priors[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  CvRTParams params = CvRTParams(5, // max depth
                                 3, // min sample count
                                 0, // regression accuracy: N/A here
                                 false, // compute surrogate split, no missing data
                                 2, // max number of categories (use sub-optimal algorithm for larger numbers)
                                 priors, // the array of priors
                                 true,  // calculate variable importance
                                 3,      // number of variables randomly selected at node and used to find the best split(s).
                                 100,	 // max number of trees in the forest
                                 0.01f,				// forrest accuracy
                                 CV_TERMCRIT_EPS // termination cirteria
                                 );
  CvRTrees* rtree = new CvRTrees;

  rtree->train(trainingData, CV_ROW_SAMPLE, trainingLabel,
               cv::Mat(), cv::Mat(), var_type, cv::Mat(), params);
  NeutubeConfig &config = NeutubeConfig::getInstance();
  rtree->save(config.getPath(NeutubeConfig::FLYEM_BODY_CONN_CLASSIFIER).c_str());
#endif

#endif

#if 0

#if defined(_USE_OPENCV_)
  int rowNumber, columnNumber;
  float *trainingArray = farray_load_matrix((dataPath + "/flyem/train.txt").c_str(), NULL,
                                          &columnNumber, &rowNumber);
  float *labelArray = farray_malloc(rowNumber);
  for (int i = 0; i < rowNumber; ++i) {
    labelArray[i] = trainingArray[i * columnNumber + columnNumber - 1];
  }

  for (int i = 0; i < rowNumber; ++i) {
    //cout << "Move from " << i * columnNumber << " to " << i * (columnNumber - 1) << endl;
    memmove(trainingArray + i * (columnNumber - 1), trainingArray + i * columnNumber,
            sizeof(float) * (columnNumber - 1));
  }

  cv::Mat trainingData(rowNumber, columnNumber - 1, CV_32FC1, trainingArray);
  cv::Mat trainingLabel(rowNumber, 1, CV_32FC1, labelArray);

  CvRTrees rtree;
  rtree.load((dataPath + "/segmentation_classifier.cvt").c_str());

  for (int i = 0; i < trainingData.rows; ++i) {
    float result = rtree.predict(trainingData.row(i), cv::Mat());
    cout << " " << trainingLabel.at<float>(i) << " " << (result > 0.5) << endl;
  }
#endif

#endif

#if 0
  ZXmlDoc doc;
  doc.parseFile(dataPath + "/test.xml");
  doc.printInfo();
#endif

#if 0
  ZObject3d obj;
  obj.append(1, 1, 1);
  obj.append(2, 2, 2);
  obj.append(3, 3, 3);
  int offset[3];
  Stack *stack = obj.toStack(offset);
  cout << offset[0] << " " << offset[1] << " " << offset[2] << endl;
  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0
  ZObject3d obj;
  obj.append(1, 1, 1);
  obj.append(2, 2, 2);
  obj.append(3, 3, 3);

  ZObject3d seed;
  seed.append(1, 1, 1);
  ZObject3dArray *objArray = obj.growLabel(seed);

  Stack *stack = objArray->toStack();
  C_Stack::write(dataPath + "/test.tif", stack);
  /*
  Stack *stack = C_Stack::make(GREY, 5, 5, 5);
  C_Stack::setZero(stack);
  obj.labelStack(stack, 255);
  C_Stack::write(dataPath + "/test.tif", stack);
  */
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  std::string filePath = dataPath + "/flyem/segmentation/assignments/assignment_2/segmentation.json";
  bundle.importJsonFile(filePath);
  ZObject3d *obj = bundle.getBodyBoundaryObject(66746);
  ZObject3d *seed = bundle.getBodyBorderObject(66746, 58351);

  ZObject3dArray *objArray = obj->growLabel(*seed);

  Stack *stack = objArray->toStack();
  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0 //test BCF

  FlyEm::ZSegmentationBundle bundle;
  std::string filePath = dataPath + "/benchmark/flyem2/test/segmentation.json";
  bundle.importJsonFile(filePath);
  vector<double> feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 600, 800, FlyEm::ZSegmentationAnalyzer::BCF_RAYBURST);
  darray_print(&(feature[0]), feature.size());
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  std::string filePath = dataPath + "/flyem/segmentation/assignments/assignment_2/segmentation.json";
  bundle.importJsonFile(filePath);

  vector<double> feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_RAYBURST);
  darray_print(&(feature[0]), feature.size());

  feature =
        FlyEm::ZSegmentationAnalyzer::computeBcf(
          bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_BODY_GROW);
  darray_print(&(feature[0]), feature.size());

  feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_BODY_SIZE);
  darray_print(&(feature[0]), feature.size());

  feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_BORDER_INTENSITY);
  darray_print(&(feature[0]), feature.size());

  feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_BOUNDARY_GROW);
  darray_print(&(feature[0]), feature.size());
#endif

#if 0
  ZObject3d *obj1 = new ZObject3d;
  obj1->append(1, 1, 1);
  obj1->append(2, 2, 2);
  obj1->append(3, 3, 3);

  ZObject3d *obj2 = new ZObject3d;
  obj2->append(4, 1, 1);
  obj2->append(5, 2, 2);
  obj2->append(6, 3, 3);

  ZObject3dArray objArray;
  objArray.push_back(obj1);
  objArray.push_back(obj2);

  /*
  Stack *stack = C_Stack::make(GREY, 10, 5, 5);
  C_Stack::setZero(stack);
  objArray.labelStack(stack);
  C_Stack::write(dataPath + "/test.tif", stack);
  */

  Stack *stack = objArray.toStack();
  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0
  ZHdf5Writer writer;
  writer.open(dataPath + "/test.h5");
  writer.createGroup("/bcf");

  int ndims = 3;
  mylib::Dimn_Type dims[3] = {10, 10, 5};
  mylib::Array *array = mylib::Make_Array(
        mylib::PLAIN_KIND, mylib::UINT8_TYPE, ndims, dims);
  writer.writeArray("/bcf/feature1", array);
  mylib::Kill_Array(array);

  writer.close();

  ZHdf5Reader reader;
  reader.open(dataPath + "/test.h5");
  reader.printInfo();
  reader.close();
#endif

#if 0
  ZMatrix matrix(3, 4);
  for (int i = 0; i < matrix.getRowNumber(); ++i) {
    for (int j = 0; j < matrix.getColumnNumber(); ++j) {
      matrix.at(i, j) = i * j;
    }
  }

  matrix.debugOutput();

  matrix.resize(20, 20);
  matrix.resize(3, 4);
  matrix.debugOutput();
#endif

#if 0
  ZHdf5Reader reader;
  reader.open(dataPath + "/benchmark/flyem2/test/Bcf/Boundary_Grow.h5");
  reader.printInfo();

  mylib::Array *array = reader.readArray("Boundary_Grow");
  mylib::printArrayInfo(array);
#endif

#if 0
  FlyEm::ZBcfSet bcfSet(dataPath + "/benchmark/flyem2/test/Bcf");
  ZMatrix *matrix = bcfSet.load("Boundary_Grow");

  matrix->debugOutput();

  delete matrix;
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(dataPath + "/flyem/segmentation/assignments/assignment_1/segmentation.json");
  bundle.print();

  int id1, id2;
  id1 = 32661;
  id2 = 40999;

  cout << bundle.getGalaProbability(id1, id2) << endl;

#endif

#if 0
  ZString str("1geag");
  cout << str.containsDigit() << endl;

  str = "efe";
  cout << str.containsDigit() << endl;

  str = "feat2fe";
  cout << str.containsDigit() << endl;
#endif

#if 0
  NeutubeConfig &config = NeutubeConfig::getInstance();
  config.load(config.getConfigPath());

  ZFlyEmStackFrame::trainBodyConnection();
#endif

#if 0
  ZStackFile stackFile;
  string filePath = dataPath + "/test.json";
  stackFile.importJsonFile(filePath);
  ZStack *stack = stackFile.readStack();

  Mc_Stack *stackdata = C_Stack::translate(stack->data(), GREY);

  C_Stack::write(dataPath + "/test.tif", stackdata);
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  std::string filePath = dataPath + "/flyem/segmentation/assignments/assignment_1/segmentation.json";
  bundle.importJsonFile(filePath);

  ZStack *stack = bundle.getPixelClassfication();
  Mc_Stack *stackdata = C_Stack::translate(stack->data(), GREY);

  C_Stack::write(dataPath + "/test.tif", stackdata);
#endif

#if 0
  ZMovieScript script;

  script.addActor(1, dataPath + "/tmp/swc3/adjusted/C2_214.swc");
  script.addActor(2, dataPath + "/tmp/swc3/adjusted/C2c_3668.swc");

  ZMovieScene scene;
  scene.setDuration(2.0);
  MovieAction action;
  action.actorId = 1;
  action.isVisible = true;
  //action.movingOffset.set(10, 0, 0);
  scene.addAction(action);

  action.actorId = 2;
  action.isVisible = false;
  scene.addAction(action);

  scene.setCameraRotation(ZPoint(0.0, 1.0, 0.0), 0.1);
  script.addScene(scene);

  scene.setDuration(2.0);
  action.actorId = 2;
  action.isVisible = true;
  scene.addAction(action);

  scene.setCameraRotation(ZPoint(0.0, 1.0, 0.0), 0.1);
  script.addScene(scene);

  ZMovieMaker director;
  director.setScript(script);
  director.make(dataPath + "/test/movie");
#endif

#if 0
  ZFileList fileList;
  fileList.load(dataPath + "/tmp/swc3/adjusted", "swc");

  std::vector<std::string> input;
  for (int i = 0; i < fileList.size(); ++i) {
    input.push_back(fileList.getFilePath(i));
  }

  //input.resize(1);

  for (std::vector<std::string>::const_iterator inputIter = input.begin();
       inputIter != input.end(); ++inputIter) {
    tr1::shared_ptr<ZStackDoc> academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);
    ZSwcTree *tree = new ZSwcTree;
    tree->load((*inputIter).c_str());
    tree->setColor(255, 0, 0);
    academy->addSwcTree(tree);

    Z3DWindow *stage = new Z3DWindow(academy, false, NULL);

    //stage->getSwcFilter()->getRendererBase()->setZScale(0.5);

    Z3DCameraParameter* camera = stage->getCamera();
    camera->setProjectionType(Z3DCamera::Orthographic);
    //camera->setUpVector(glm::vec3(0.0, 0.0, -1.0));
    stage->getInteractionHandler()->getTrackball()->rotate(
          glm::vec3(1.0, 0.0, 0.0), TZ_PI_2);
    stage->resetCameraClippingRange();
    stage->getBackground()->setFirstColor(1, 1, 1, 1);
    stage->getBackground()->setSecondColor(1, 1, 1, 1);

    stage->show();
    stage->getSwcFilter()->setColorMode("intrinsic");

    stage->takeScreenShot((*inputIter + ".tif").c_str(), 1024, 1024, MonoView);
    stage->close();
  }

#endif

#if 0
  Stack *stack = C_Stack::make(GREY, 250, 10, 1300);
  Zero_Stack(stack);
  stack->depth = 25;
  One_Stack(stack);
  stack->depth = 1300;

  Write_Stack_U((dataPath + "/flyem/skeletonization/session3/scale_bar.tif").c_str(),
                stack, NULL);
#endif

#if 0
  //Generate neuron figures
  std::string sessionDir = "flyem/skeletonization/session3";
  std::string dataDir = sessionDir + "/smoothed/snapshots/contrast/selected";
  ZFileList fileList;
  fileList.load(dataPath + "/" + dataDir, "tif", ZFileList::SORT_ALPHABETICALLY);

  FILE *fp = fopen((dataPath + "/" + sessionDir + "/" + "neuron_type.txt").c_str(), "r");
  if (fp == NULL) {
    std::cout << "Cannot open " << sessionDir + "/" + "neuron_type.txt" << std::endl;
  }
  ZString neuronTypeLine;
  std::vector<std::string> neuronTypeArray;
  neuronTypeArray.push_back("scale_bar");
  while (neuronTypeLine.readLine(fp)) {
    neuronTypeLine.trim();
    if ((neuronTypeLine[0] >= 'A' && neuronTypeLine[0] <= 'Z') ||
        (neuronTypeLine[0] >= 'a' && neuronTypeLine[0] <= 'z')) {
      neuronTypeArray.push_back(neuronTypeLine);
    }
  }

  fclose(fp);
  std::cout << neuronTypeArray.size() << " neuron types" << std::endl;

  std::vector<Stack*> textPatchArray;
  Stack *textImage = Read_Stack_U((dataPath + "/benchmark/mlayer_label.tif").c_str());
  for (int i =0; i < 10; ++i) {
    Stack textPatch2 = *textImage;
    textPatch2.depth = 1;
    textPatch2.array = textImage->array +
        i * C_Stack::width(textImage) * C_Stack::height(textImage);
    Stack *textPatch = C_Stack::boundCrop(&textPatch2);
    textPatchArray.push_back(textPatch);
  }
  C_Stack::kill(textImage);

  cout << neuronTypeArray.size() << " cell types" << endl;
  //ParameterDialog dlg;

  int totalCellNumber = 0;

  //if (dlg.exec()) {
  for (size_t neuronTypeIndex = 0; neuronTypeIndex < neuronTypeArray.size();
       ++neuronTypeIndex) {
    //std::string neuronType = dlg.parameter().toStdString();
    std::string neuronType = neuronTypeArray[neuronTypeIndex];

    std::vector<std::string> input;
    for (int i = 0; i < fileList.size(); ++i) {
      ZString path(fileList.getFilePath(i));
      std::vector<std::string> parts = path.fileParts();
      ZString fileName = parts[1];
      if (fileName.startsWith(neuronType)) {
        bool isTarget = true;
        if (isdigit(neuronType[neuronType.length() - 1])) {
          if (isdigit(fileName[neuronType.length()])) {
            isTarget = false;
          }
        }
        if (isTarget) {
          input.push_back(path.c_str());
        }
      } else {
        if (neuronType == "T4") {
          if (fileName.startsWith("nT4")) {
            input.push_back(path.c_str());
          }
        }
      }
    }

    /*
    if (neuronType == "Tm3") {
      input.resize(11);
    }
*/
    std::cout << neuronType << ": " << input.size() << " cells" << std::endl;

    if (neuronType != "scale_bar") {
      totalCellNumber += input.size();
    }

    /*
    int textSpace = 40;
    int textWidth = 96;
    Stack *croppedTextImage = C_Stack::boundCrop(textImage);
    C_Stack::kill(textImage);
    int left = 0;
    for (int i =0; i < 10; ++i) {
      int width;
      if (i == 0) {
        width = textWidth + textSpace;
      } else if (i == 9) {
        width = textWidth * 2 + textSpace * 2;
      } else {
        width = textWidth + textSpace * 2;
      }
      Stack *textPatch2 = C_Stack::crop(croppedTextImage, left, 0, 0,
                                       width, C_Stack::height(croppedTextImage), 1, NULL);
      Stack *textPatch = C_Stack::boundCrop(textPatch2);
      C_Stack::kill(textPatch2);
      textPatchArray.push_back(textPatch);
      left += width;
    }

    C_Stack::kill(croppedTextImage);
    */

    int mLayerStart = 317;
    //int mLayerStart = 692;
    //int mLayerEnd = 3538;
    int mLayerEnd = 3534;

    double layerPercent[] = {0, 9.4886, 18.5061, 29.6097, 33.3782, 40.3096,
                             46.9717, 57.8735, 72.1400, 89.0982, 100.0000};
    int layerArray[11];
    for (int i = 0; i < 11; ++i) {
      layerArray[i] = mLayerStart +
          iround(layerPercent[i] * (mLayerEnd - mLayerStart) / 100.0);
    }
    //input.resize(5);

    int rowSize = 5;
    int nrow = input.size() / rowSize + (input.size() % rowSize > 0);

    std::vector<std::string>::const_iterator inputIter = input.begin();
    for (int row = 0; row < nrow; ++row) {
      std::vector<Stack*> stackArray;

      int finalWidth = 0;
      int count = 1;
      for (; inputIter != input.end(); ++inputIter) {
        if (count > rowSize) {
          break;
        }
        Stack *stack = Read_Stack_U(inputIter->c_str());
        Stack *stack2 = Crop_Stack(stack, 200, 0, 0, C_Stack::width(stack) - 201,
                                   C_Stack::height(stack), 1, NULL);
        C_Stack::kill(stack);
        Cuboid_I box;
        Stack_Bound_Box(stack2, &box);
        int left = box.cb[0] -50;
        int width = box.ce[0] - box.cb[0] + 100;
        Stack *crop = Crop_Stack(stack2, left, 0, 0, width, stack2->height, 1, NULL);

        if (neuronType != "scale_bar") {
          //Draw body id
          int bodyId = String_Last_Integer(inputIter->c_str());
          int interval = 15;
          int intWidth = C_Stack::integerWidth(bodyId, interval);
          C_Stack::drawInteger(crop, bodyId,
                               (C_Stack::width(crop) - intWidth) / 2,
                               C_Stack::height(crop) - 200, 0, interval);
        }

        finalWidth += width;
        stackArray.push_back(crop);
        C_Stack::kill(stack2);
        ++count;
      }

      int leftMargin = 100;
      int rightMargin = 800;

      Stack *out = C_Stack::make(GREY, finalWidth + leftMargin + rightMargin, C_Stack::height(stackArray[0]), 1);

      Zero_Stack(out);
      uint8_t *outArray = out->array;

      for (int h = 0; h < C_Stack::height(stackArray[0]); ++h) {
        outArray += leftMargin;
        for (size_t i = 0; i < stackArray.size(); ++i) {
          memcpy(outArray,
                 stackArray[i]->array + h * C_Stack::width(stackArray[i]),
                 C_Stack::width(stackArray[i]));
          outArray += C_Stack::width(stackArray[i]);
        }
        outArray += rightMargin;
      }

      for (size_t i = 0; i < stackArray.size(); ++i) {
        C_Stack::kill(stackArray[i]);
      }

      if (neuronType != "scale_bar") {
        //Draw lines
        for (int i = 0; i < C_Stack::width(out); ++i) {
          int v = 250;
          for (int layer = 0; layer < 11; ++layer) {
            if (out->array[i + C_Stack::width(out) * layerArray[layer]] == 0) {
              out->array[i + C_Stack::width(out) * layerArray[layer]] = v;
            }
            for (int w = 1; w <= 2; ++w) {
              if (out->array[i + C_Stack::width(out) * (layerArray[layer] - w)] == 0) {
                out->array[i + C_Stack::width(out) * (layerArray[layer] - w)] = v / (w + 1);
              }
              if (out->array[i + C_Stack::width(out) * (layerArray[layer] + w)] == 0) {
                out->array[i + C_Stack::width(out) * (layerArray[layer] + w)] = v / (w + 1);
              }
            }
          }
        }

        //Draw texts
        for (int layer = 0; layer < 10; ++layer) {
          int dx = C_Stack::width(out) - 250;
          int dy = (layerArray[layer] + layerArray[layer + 1]) / 2 -
              C_Stack::height(textPatchArray[layer]) / 2;
          C_Stack::drawPatch(out, textPatchArray[layer], dx, dy, 0, 0);
        }

        Stack *scaleBar = Read_Stack_U((dataPath + "/" + dataDir + "/row/scale_bar_row1.tif").c_str());
        Stack *croppedScaleBar = C_Stack::boundCrop(scaleBar);
        C_Stack::kill(scaleBar);
        C_Stack::drawPatch(out, croppedScaleBar, C_Stack::width(out) - 700, C_Stack::height(out) - 200, 0, 0);
        C_Stack::kill(croppedScaleBar);
      }

      std::ostringstream stream;
      stream << dataPath + "/" + dataDir + "/row/" + neuronType
             << "_row" << row + 1 << ".tif";

      Write_Stack_U(stream.str().c_str(), out, NULL);
      C_Stack::kill(out);
    }
  }

  for (int layer = 0; layer < 10; ++layer) {
    C_Stack::kill(textPatchArray[layer]);
  }

  cout << "Total: " << totalCellNumber << " neurons" << endl;
#endif

#if 0
  //Volume rendering snapshots

  std::string dataDir = "flyem/skeletonization/session3/smoothed";
  //std::string dataDir = "benchmark/binary/3d/block";
  ZFileList fileList;
  fileList.load(dataPath + "/" + dataDir, "tif");

  std::vector<std::string> input;
  for (int i = 0; i < fileList.size(); ++i) {
    input.push_back(fileList.getFilePath(i));
  }

  //input.resize(1);
  //Filter_3d *filter = Gaussian_Filter_3d(0.5, 0.5, 0.5);
  //input.resize(1);
  //input[0] = dataPath + "/" + dataDir + "/" + "Pm2-8_171795.tif";

  for (std::vector<std::string>::const_iterator inputIter = input.begin();
       inputIter != input.end(); ++inputIter) {
    std::string output;
    ZString inputPath(*inputIter);
    std::vector<std::string> parts = inputPath.fileParts();
    output = dataPath + "/" + dataDir + "/snapshots/" + parts[1] + ".tif";

    if (!fexist(output.c_str())) {

      std::string offsetFile = *inputIter + ".offset.txt";
      FILE *fp = fopen(offsetFile.c_str(), "r");
      ZString offsetStr;
      offsetStr.readLine(fp);
      std::vector<int> offset =offsetStr.toIntegerArray();
      fclose(fp);

      tr1::shared_ptr<ZStackDoc> academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);

      academy->loadFile((*inputIter).c_str());

      Z3DWindow *stage = new Z3DWindow(academy, false, NULL);
      stage->getVolumeSource()->setZScale(1.125);

      //const std::vector<double> &boundBox = stage->getBoundBox();

      Z3DCameraParameter* camera = stage->getCamera();
      camera->setProjectionType(Z3DCamera::Orthographic);
      //stage->resetCamera();

      //camera->setUpVector(glm::vec3(0.0, 0.0, -1.0));
      /*
  stage->getInteractionHandler()->getTrackball()->rotate(
        glm::vec3(1.0, 0.0, 0.0), TZ_PI_2);
        */

      glm::vec3 referenceCenter = camera->getCenter();

      double eyeDistance = 3000;//boundBox[3] - referenceCenter[1] + 2500;
      //double eyeDistance = 2000 - referenceCenter[1];
      glm::vec3 viewVector(0, -1, 0);

      viewVector *= eyeDistance;
      glm::vec3 eyePosition = referenceCenter - viewVector;

      referenceCenter[2] = (650 - offset[2]) * 1.125;
      camera->setCenter(referenceCenter);
      eyePosition[2] = (650 - offset[2]) * 1.125;
      camera->setEye(eyePosition);
      camera->setUpVector(glm::vec3(0, 0, -1));
      stage->resetCameraClippingRange();

      stage->getBackground()->setFirstColor(0, 0, 0, 1);
      stage->getBackground()->setSecondColor(0, 0, 0, 1);

      //std::cout << "scales: " << stage->getVolumeRaycaster()->getRenderer()->getCoordScales() << std::endl;

      camera->setNearDist(2000.0);

      stage->show();

      std::cout << output << std::endl;
      stage->takeScreenShot(output.c_str(), 4000, 4000, MonoView);
      stage->close();
      delete stage;
    }
  }
#endif

#if 0
  //Draw synapses
  ZFileList fileList;
  fileList.load(dataPath + "/flyem/skeletonization/session3/len15/adjusted", "swc");

  std::vector<std::string> input;
  for (int i = 0; i < fileList.size(); ++i) {
    input.push_back(fileList.getFilePath(i));
  }

  //input.resize(1);

  std::map<std::string, QColor> punctaColorMap;
  punctaColorMap[""] = QColor(0, 255, 0);
  punctaColorMap["Unknown"] = QColor(0, 255, 0);
  punctaColorMap["_1"] = QColor(255, 0, 0);
  punctaColorMap["_2"] = QColor(0, 0, 255);
  punctaColorMap["Tm3"] = QColor(255, 0, 0);
  punctaColorMap["Mi1"] = QColor(0, 0, 255);

  for (std::vector<std::string>::const_iterator inputIter = input.begin();
       inputIter != input.end(); ++inputIter) {
    ZString markerFile = ZString(*inputIter).changeExt("marker");
    markerFile = markerFile.changeDirectory(dataPath + "/flyem/TEM/T4viz/synapse2");
    if (fexist(markerFile.c_str())) {
      tr1::shared_ptr<ZStackDoc> academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);
      ZSwcTree *tree = new ZSwcTree;
      tree->load((*inputIter).c_str());
      tree->setColor(255, 255, 0);
      academy->addSwcTree(tree);


      academy->loadFile(markerFile.c_str());

      QList<ZPunctum*> *puncta = academy->punctaList();
      for (QList<ZPunctum*>::const_iterator punctaIter = puncta->begin();
           punctaIter != puncta->end(); ++punctaIter) {
        (*punctaIter)->setColor(punctaColorMap[(*punctaIter)->name().toStdString()]);
      }

      Z3DWindow *stage = new Z3DWindow(academy, false, NULL);

      //stage->getSwcFilter()->getRendererBase()->setZScale(0.5);

      const std::vector<double> &boundBox = stage->getBoundBox();
      Z3DCameraParameter* camera = stage->getCamera();
      camera->setProjectionType(Z3DCamera::Orthographic);
      glm::vec3 referenceCenter = camera->getCenter();

      double eyeDistance = boundBox[5] - referenceCenter[2] + 40000;
      glm::vec3 viewVector(0, 0, 1);

      viewVector *= eyeDistance;
      glm::vec3 eyePosition = referenceCenter - viewVector;

      camera->setEye(eyePosition);
      camera->setUpVector(glm::vec3(0, -1, 0));

      stage->resetCameraClippingRange();
      stage->getBackground()->setFirstColor(1, 1, 1, 1);
      stage->getBackground()->setSecondColor(1, 1, 1, 1);

      stage->getPunctaFilter()->setSizeScale(2.0);
      stage->getPunctaFilter()->setColorMode("Original Point Color");

      stage->getSwcFilter()->setColorMode("Intrinsic");
      stage->getSwcFilter()->getRendererBase()->setOpacity(0.3);

      stage->show();

      stage->takeScreenShot((markerFile + ".tif").c_str(), 2048, 2048, MonoView);
      stage->close();
    }
  }
#endif

#if 0
  ZMovieCamera camera;

  ZMovieScene scene;

  scene.print();
#endif

#if 0
  ZMovieScript script;
  int frameNumber = 0;
  string outDir = dataPath + "/test/movie";

  if (script.loadScript(dataPath + "/flyem/TEM/movie/script4.json")) {
    ZMovieMaker director;
    director.setFrameSize(512, 512);
    director.setScript(script);
    director.setFrameInterval(40);
    frameNumber = director.make(outDir);
  }

  /*
  if (host != NULL) {
    ZStackFile stackFile;
    for (int index = 0; index < frameNumber; ++index) {
      stackFile.appendUrl(ZMovieMaker::getFramePath(outDir, index));
    }
    stackFile.setType("file list");
    ZStack *stack = stackFile.readStack();
    if (stack != NULL) {
      ZStackFrame *frame = new ZStackFrame;
      frame->loadStack(stack);
      host->addStackFrame(frame);
    }
  }
  */
#endif

#if 0
  ZMovieScriptGenerator writer;

  writer.addActor("slice",
                  "/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/gray_ds10_avg/xy-grayscale-01267.tif");

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle2.json");

  ZPoint colorL1(1.0, 1.0, 0.0);
  ZPoint colorMi1(0.0, 0.0, 1.0);
  ZPoint colorTm3(1.0, 0.0, 0.0);
  ZPoint colorT4(0.0, 1.0, 0.0);
  ZPoint colorT4Mi1(0.0, 0.0, 1.0);
  ZPoint colorT4Tm3(1.0, 0.0, 0.0);

  vector<string> L1Neurons;
  const vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "L1") {
      L1Neurons.push_back(iter->getName());
    }
  }

  set<string> excludedL1Neurons;
  excludedL1Neurons.insert("L1-i8");
  excludedL1Neurons.insert("L1-k10");

  vector<string> selectedL1Neurons;
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter) {
    if (excludedL1Neurons.count(*iter) == 0) {
      selectedL1Neurons.push_back(*iter);
    }
  }

  vector<string> T4Neurons;
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "T4") {
      T4Neurons.push_back(iter->getName());
    }
  }

  vector<string> Mi1Neurons;
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "Mi1") {
      Mi1Neurons.push_back(iter->getName());
    }
  }

  string Mi1Neuron_T4_12 = "Mi1-a";
  vector<string> Tm3Neuron_T4_12;
  Tm3Neuron_T4_12.push_back("Tm3-A");
  Tm3Neuron_T4_12.push_back("Tm3-b-A");
  Tm3Neuron_T4_12.push_back("Tm3-r5-P");
  Tm3Neuron_T4_12.push_back("Tm3-h7-A");
  Tm3Neuron_T4_12.push_back("Tm3-f-P");

  vector<string> T4_12_connectedNeurons;
  T4_12_connectedNeurons.push_back(Mi1Neuron_T4_12);
  T4_12_connectedNeurons.insert(T4_12_connectedNeurons.end(),
                                Tm3Neuron_T4_12.begin(),
                                Tm3Neuron_T4_12.end());

  set<string> Tm3Neuron_T4_12_set;
  for (vector<string>::const_iterator iter = Tm3Neuron_T4_12.begin();
       iter != Tm3Neuron_T4_12.end(); ++iter) {
    Tm3Neuron_T4_12_set.insert(*iter);
  }


  vector<string> Tm3Neurons;
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "Tm3") {
      Tm3Neurons.push_back(iter->getName());
    }
  }

  //Write cast
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "T4" || iter->getClass() == "Tm3" ||
        iter->getClass() == "Mi1" || iter->getClass() == "L1")
    writer.addActor(iter->getName(), iter->getModelPath());
  }

  writer.addActor("T4-Mi1", dataPath + "/flyem/TEM/movie/actor/puncta/T4_12_Mi1_a.marker");
  writer.addActor("T4-Tm3", dataPath + "/flyem/TEM/movie/actor/puncta/T4_12_Tm3-f-P.marker");
  writer.addActor("tbar_box", dataPath + "/flyem/TEM/movie/actor/tbar-bound.swc");
  writer.addActor("arrow1", dataPath + "/flyem/TEM/movie/actor/arrow1.swc");
  writer.addActor("arrow2", dataPath + "/flyem/TEM/movie/actor/arrow2.swc");
  writer.addActor("T4-tbar", dataPath + "/flyem/TEM/movie/actor/puncta/T4_12_tbar.marker");

  std::ofstream stream((dataPath + "/flyem/TEM/movie/script11.json").c_str());
  stream << "{" << endl;

  writer.writeCast(stream, 1);
  stream << "," << endl;

  writer.writePlotStart(stream, 1);

  vector<string> actions;

  //Show Slice
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 80, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.push_back("move");
  actions.push_back("[0, 0, 13.825]");
  actions.push_back("visible");
  actions.push_back("true");
  writer.writeAction(stream, "slice", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << ",";
  stream << endl;
  writer.writeCameraStart(stream, 3);

  vector<string> reset;
  reset.push_back("eye");
  reset.push_back("[599, 599, 0]");
  reset.push_back("center");
  reset.push_back("[599, 599, 1064]");
  reset.push_back("up_vector");
  reset.push_back("[0, -1, 0]");
  writer.writeCameraReset(stream, reset, 4);
  stream << endl;

  writer.writeCameraEnd(stream, 3);
  stream << endl;

  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  vector<string> camera;

  //Zoom in
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.056, 0.056, 1.0]");
  camera.push_back("center");
  camera.push_back("[0.056, 0.056, 0]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hightlight t-bar
  int interval = 200;
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  //Show single syanpse
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 200, 3);
  stream << "," << endl;
  stream << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back(colorMi1.toJsonString());
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeAction(stream, "T4-tbar", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show T4
  writer.writeSceneStart(stream,2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;

  writer.writeActionStart(stream, 3);

  actions.clear();
  actions.push_back("color");
  actions.push_back(colorT4.toJsonString());
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");

  writer.writeAction(stream, "T4-12", actions, 4);
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;

  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //hide slice
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeAction(stream, "slice", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show T4-Mi1 and T4-Tm3 synapses
  writer.writeSceneStart(stream, 2);

  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;

  writer.writeActionStart(stream, 3);

  actions.clear();
  actions.push_back("color");
  actions.push_back(colorT4Mi1.toJsonString());
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");

  writer.writeAction(stream, "T4-Mi1", actions, 4);
  stream << "," << endl;

  actions[1] = colorT4Tm3.toJsonString();
  writer.writeAction(stream, "T4-Tm3", actions, 4);
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;

  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //zoom out
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  actions.clear();
  actions.push_back("eye");
  actions.push_back("[0.168035, 0.147384, -0.285]");
  actions.push_back("up_vector");
  actions.push_back("[-0.000189209, 0.000132543, -0.000460128]");
  writer.writeCameraMove(stream, actions, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  //Show Mi1 neuron
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("color");
  actions.push_back("[0.0, 0.0, 1.0]");
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Mi1-a", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;

  //Zoom in
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.0188, -0.1868, 0.1833]");
  camera.push_back("center");
  camera.push_back("[0.1487, -0.0397, -0.1641]");
  camera.push_back("up_vector");
  camera.push_back("[-0.000619616, 0.000370943, 0.000135057]");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 900, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.00188, -0.01868, 0.01833]");
  camera.push_back("center");
  camera.push_back("[0.01487, -0.00397, -0.01641]");
  camera.push_back("up_vector");
  camera.push_back("[-0.0000619616, 0.0000370943, 0.0000135057]");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 100, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show T4-Tm3 synapses
  /*
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("color");
  actions.push_back("[1.0, 1.0, 0.0]");
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeAction(stream, "T4-Tm3", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;
  */

  //Show Tm3
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Tm3-f-P", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;

  //zoom back
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.0188, 0.1868, -0.1833]");
  camera.push_back("center");
  camera.push_back("[-0.1487, 0.0397, 0.1641]");
  camera.push_back("up_vector");
  camera.push_back("[0.000619616, -0.000370943, -0.000135057]");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //zoom out more and show all Mi1, Tm3 neurons
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("color");
  actions.push_back(colorMi1.toJsonString());
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  actions.push_back("visible");
  actions.push_back("true");

  for (vector<string>::const_iterator iter = Mi1Neurons.begin();
       iter != Mi1Neurons.end(); ++iter) {
    if (*iter != "Mi1-a") {
      writer.writeAction(stream, *iter, actions, 4);
      stream << "," << endl;
    }
  }

  actions[1] = colorTm3.toJsonString();
  for (vector<string>::const_iterator iter = Tm3Neurons.begin();
       iter != Tm3Neurons.end(); ++iter) {
    if (*iter != "Tm3-f-P") {
      if (iter != Tm3Neurons.begin()) {
        stream << "," << endl;
      }
      writer.writeAction(stream, *iter, actions, 4);
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.186843, 0.49986, -1.17363]");
  camera.push_back("up_vector");
  camera.push_back("[-0.000372487, 0.000161579, 0.0000285799]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //zoom out more
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  actions.clear();
  actions.push_back("eye");
  actions.push_back("[0.758089, 0.584229, 0.120433]");
  actions.push_back("center");
  actions.push_back("[-0.392549, -0.54059, -0.288692]");
  actions.push_back("up_vector");
  actions.push_back("[0.000231308, 0.00041455, -0.00047294]");
  writer.writeCameraMove(stream, actions, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream <<",";
  stream << endl;

  //change view angle
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  actions.clear();
  actions.push_back("eye");
  actions.push_back("[-1.42134, -1.02811, -1.02052]");
  actions.push_back("center");
  actions.push_back("[0.578985, 0.570409, 0.390763]");
  actions.push_back("up_vector");
  actions.push_back("[-0.0000664507, -0.000593431, 0.000936398]");
  writer.writeCameraMove(stream, actions, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide Tm3 neuron
#if 0
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Tm3-f-P", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Tm3-f-P", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;
#endif

  //Add L1 neurons one by one
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (iter != L1Neurons.begin()) {
        stream << "," << endl;
      }
      writer.writeSceneStart(stream, 2);
      writer.writeDuration(stream, 200, 3);
      stream << "," << endl;
      actions.clear();
      actions.push_back("color");
      actions.push_back("[1.0, 1.0, 0.0]");
      actions.push_back("visible");
      actions.push_back("true");
      actions.push_back("alpha");
      actions.push_back("0.1");
      actions.push_back("fade");
      actions.push_back("0.005");
      writer.writeActionStart(stream, 3);
      writer.writeAction(stream, *iter, actions, 4);
      stream << endl;
      writer.writeActionEnd(stream, 3);
      stream << endl;
      writer.writeSceneEnd(stream, 2);
    }
  }
  stream << ",";
  stream << endl;

  //Adjust view
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  actions.clear();
  actions.push_back("eye");
  actions.push_back("[-0.500935, -0.538451, -0.878699]");
  actions.push_back("center");
  actions.push_back("[0.0716949, 0.0258559, 0.0328573]");
  writer.writeCameraMove(stream, actions, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

#if 0
  //Iterate through L1 neurons
  string sortedL1Neurons[] = {
    "L1-m12", "L1-n1", "L1-o2", "L1-l11", "L1-d", "L1-e", "L1-p3", "L1-c",
    "L1", "L1-f", "L1-q4", "L1-j9", "L1-b", "L1-a", "L1-r5", "L1-h7", "L1-g6"
  };
  for (size_t i = 0; i < L1Neurons.size(); ++i) {
    if (i != 0) {
      stream << "," << endl;
    }
    writer.writeSceneStart(stream, 2);
    writer.writeDuration(stream, 80, 3);
    stream << "," << endl;
    actions.clear();
    actions.push_back("color");
    actions.push_back("[1.0, 1.0, 0.6]");
    actions.push_back("visible");
    actions.push_back("true");
    writer.writeActionStart(stream, 3);
    writer.writeAction(stream, sortedL1Neurons[i], actions, 4);
    stream << endl;
    writer.writeActionEnd(stream, 3);
    stream << endl;
    writer.writeSceneEnd(stream, 2);

    stream << "," << endl;
    writer.writeSceneStart(stream, 2);
    writer.writeDuration(stream, 200, 3);
    stream << "," << endl;
    actions[1] = "[1.0, 1.0, 0.0]";
    writer.writeActionStart(stream, 3);
    writer.writeAction(stream, sortedL1Neurons[i], actions, 4);
    stream << endl;
    writer.writeActionEnd(stream, 3);
    stream << endl;
    writer.writeSceneEnd(stream, 2);
  }
  stream << ",";
  stream << endl;
#endif

  //Rotate to the other side
#if 1
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 2000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.09");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;
#endif

  //Hide Tm3 and Mi1 neurons other than connected ones
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 2);
  stream << "," << endl;
  actions.clear();
  actions.push_back("fade");
  actions.push_back("-0.001");
  actions.push_back("visible");
  actions.push_back("true");
  writer.writeActionStart(stream, 3);

  vector<string> selectedNeurons;

  selectedNeurons.clear();
  for (vector<string>::const_iterator iter = Tm3Neurons.begin();
       iter != Tm3Neurons.end(); ++iter) {
    if (Tm3Neuron_T4_12_set.count(*iter) == 0) {
      selectedNeurons.push_back(*iter);
    }
  }

  for (vector<string>::const_iterator iter = Mi1Neurons.begin();
       iter != Mi1Neurons.end(); ++iter) {
    if (*iter != "Mi1-a") {
      selectedNeurons.push_back(*iter);
    }
  }

  writer.writeAction(stream, selectedNeurons, actions, 4);
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeAction(stream, selectedNeurons, actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  //Add more Tm3 neurons
  vector<string> connectedTm3Neurons;
  connectedTm3Neurons.push_back("Tm3-A");
  connectedTm3Neurons.push_back("Tm3-b-A");
  connectedTm3Neurons.push_back("Tm3-r5-P");
  connectedTm3Neurons.push_back("Tm3-h7-A");
  connectedTm3Neurons.push_back("Tm3-f-P");

#if 0
  for (size_t i = 0; i < connectedTm3Neurons.size(); ++i) {
    if (i != 0) {
      stream << "," << endl;
    }
    writer.writeSceneStart(stream, 2);
    writer.writeDuration(stream, 1000, 3);
    stream << "," << endl;
    actions.clear();
    actions.push_back("color");
    actions.push_back("[1.0, 0.0, 0.0]");
    actions.push_back("visible");
    actions.push_back("true");
    actions.push_back("alpha");
    actions.push_back("0.1");
    actions.push_back("fade");
    actions.push_back("0.001");
    writer.writeActionStart(stream, 3);
    writer.writeAction(stream, connectedTm3Neurons[i], actions, 4);
    stream << endl;
    writer.writeActionEnd(stream, 3);
    stream << endl;
    writer.writeSceneEnd(stream, 2);
  }
  stream << ",";
  stream << endl;
#endif

#if 0
  //Rotate back
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 2000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.09");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;
#endif

  //Hide L1 neurons
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (iter != L1Neurons.begin()) {
        stream << "," << endl;
      }
      writer.writeAction(stream, *iter, actions, 4);
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (iter != L1Neurons.begin()) {
        stream << "," << endl;
      }
      writer.writeAction(stream, *iter, actions, 4);
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide Mi1 neuron
#if 0
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Mi1-a", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Mi1-a", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;
#endif

  //Rotate 180 degrees
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.18");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Change transparency of Tm3 neurons
  double connectionStrengthTm3[] = {
    4, 3, 6, 4, 3
  };

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;

  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.18");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << "," << endl;

  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("0.0");

  int i = 0;
  for (vector<string>::const_iterator iter = connectedTm3Neurons.begin();
       iter != connectedTm3Neurons.end(); ++iter, ++i) {
    if (iter != connectedTm3Neurons.begin()) {
      stream << "," << endl;
    }
    ostringstream strstream;
    strstream << (connectionStrengthTm3[i] - 6.0) / 5000.0;
    actions[3] = strstream.str();
    writer.writeAction(stream, *iter, actions, 4);
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide Tm3
#if 0
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("fade");
  actions.push_back("-0.001");

  writer.writeActionStart(stream, 3);
  for (size_t i = 0; i < connectedTm3Neurons.size(); ++i) {
    if (i != 0) {
      stream << "," << endl;
    }
    writer.writeAction(stream, connectedTm3Neurons[i], actions, 4);
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");

  writer.writeActionStart(stream, 3);
  for (size_t i = 0; i < connectedTm3Neurons.size(); ++i) {
    if (i != 0) {
      stream << "," << endl;
    }
    writer.writeAction(stream, connectedTm3Neurons[i], actions, 4);
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;
#endif

  //Add back L1
  vector<string> L1ColorArray(19);
  L1ColorArray[0] = "[1, 0.79224, 0.79224]";
  L1ColorArray[1] = "[0.5462, 0, 1]";
  L1ColorArray[2] = "[1, 0.9836, 0.9836]";
  L1ColorArray[3] = "[1, 1, 1]";
  L1ColorArray[4] = "[1, 0.99453, 0.99453]";
  L1ColorArray[5] = "[1, 0.95079, 0.95079]";
  L1ColorArray[6] = "[1, 0.61591, 0.61591]";
  L1ColorArray[7] = "[1, 0.83051, 0.83051]";
  L1ColorArray[8] = "[1, 0.90159, 0.90159]";
  L1ColorArray[9] = "[1, 1, 1]";
  L1ColorArray[10] = "[1, 1, 1]";
  L1ColorArray[11] = "[1, 1, 1]";
  L1ColorArray[12] = "[1, 1, 1]";
  L1ColorArray[13] = "[1, 1, 1]";
  L1ColorArray[14] = "[1, 1, 1]";
  L1ColorArray[15] = "[1, 1, 1]";
  L1ColorArray[16] = "[1, 0.9836, 0.9836]";
  L1ColorArray[17] = "[1, 0.92619, 0.92619]";
  L1ColorArray[18] = "[1, 0.73346, 0.73346]";

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  i = 0;
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter, ++i) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (iter != L1Neurons.begin()) {
        stream << "," << endl;
      }
      actions[3] = L1ColorArray[i];
      writer.writeAction(stream, *iter, actions, 4);
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Adjust view
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.3476, -1.0482, 0.0401]");
  camera.push_back("center");
  camera.push_back("[0.0778, 0.2182, 0.0304]");
  camera.push_back("up_vector");
  camera.push_back("[-0.0000768, -0.0000820, -0.0003514]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Color L1 neuron for Mi1
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[0.8051, 0.0, 1.0]");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeAction(stream, L1Neurons[1], actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide Tm3 and Mi1 neurons
  actions.clear();
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, T4_12_connectedNeurons, actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, T4_12_connectedNeurons, actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Zoom in
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.3020, 0.4281, -1.6749]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Add arrow
  interval = 200;
  ZPoint arrowColor(1.0, 1.0, 0.0);
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back(arrowColor.toJsonString());
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Zoom back
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.3020, -0.4281, 1.6749]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide T4-12 and turn L1 to white
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "T4-12", actions, 4);
  stream << ",";
  stream << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("transit");
  actions.push_back("0.001");
  writer.writeAction(stream, selectedL1Neurons, actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "T4-12", actions, 4);
  stream << ",";
  stream << endl;
  writer.writeAction(stream, "T4-Mi1", actions, 4);
  stream << ",";
  stream << endl;
  writer.writeAction(stream, "T4-Tm3", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Add T4-10
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back(colorT4.toJsonString());
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "T4-10", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show Tm3 and Mi1-neurons
  vector<string> Tm3Neurons_T4_10;
  string Mi1Neuron_T4_10 = "Mi1-e";
  Tm3Neurons_T4_10.push_back("Tm3-A");
  Tm3Neurons_T4_10.push_back("Tm3-e-P");
  Tm3Neurons_T4_10.push_back("Tm3-p3-P");
  Tm3Neurons_T4_10.push_back("Tm3-f-A");
  Tm3Neurons_T4_10.push_back("Tm3-o2-P");
  Tm3Neurons_T4_10.push_back("Tm3-e-A");

  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.01");
  actions.push_back("fade");
  actions.push_back("0.001");
  actions.push_back("color");
  actions.push_back(colorMi1.toJsonString());
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, Mi1Neuron_T4_10, actions, 4);
  stream << "," << endl;

  actions[7] = colorTm3.toJsonString();

  actions[5] = "0.0001";
  writer.writeAction(stream, Tm3Neurons_T4_10[0], actions, 4);
  stream << "," << endl;
  actions[5] = "0.001";
  writer.writeAction(stream, Tm3Neurons_T4_10[1], actions, 4);
  stream << "," << endl;
  actions[5] = "0.0003";
  writer.writeAction(stream, Tm3Neurons_T4_10[2], actions, 4);
  stream << "," << endl;
  actions[5] = "0.00001";
  writer.writeAction(stream, Tm3Neurons_T4_10[3], actions, 4);
  stream << "," << endl;
  actions[5] = "0.0009";
  writer.writeAction(stream, Tm3Neurons_T4_10[4], actions, 4);
  stream << "," << endl;
  actions[5] = "0.0002";
  writer.writeAction(stream, Tm3Neurons_T4_10[5], actions, 4);
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Add Mi1 neuron and Tm3 neuorns connected to T4-10
  L1ColorArray[0] = "[1, 0.49153, 0.49153]";
  L1ColorArray[1] = "[1, 0.99263, 0.99263]";
  L1ColorArray[2] = "[1, 1, 1]";
  L1ColorArray[3] = "[1, 1, 1]";
  L1ColorArray[4] = "[1, 0.68497, 0.68497]";
  L1ColorArray[5] = "[0.18755, 0, 1]";
  L1ColorArray[6] = "[1, 0.85262, 0.85262]";
  L1ColorArray[7] = "[1, 1, 1]";
  L1ColorArray[8] = "[1, 1, 1]";
  L1ColorArray[9] = "[1, 1, 1]";
  L1ColorArray[10] = "[1, 1, 1]";
  L1ColorArray[11] = "[1, 1, 1]";
  L1ColorArray[12] = "[1, 1, 1]";
  L1ColorArray[13] = "[1, 0.98342, 0.98342]";
  L1ColorArray[14] = "[1, 0.54864, 0.54864]";
  L1ColorArray[15] = "[1, 0.37546, 0.37546]";
  L1ColorArray[16] = "[1, 0.70339, 0.70339]";
  L1ColorArray[17] = "[1, 0.9392, 0.9392]";
  L1ColorArray[18] = "[1, 1, 1]";

  //Rotate 360 degrees
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 2000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.18");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Change color of L1 and fade out Mi1, Tm3
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);

  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeAction(stream, Mi1Neuron_T4_10, actions, 4);
  stream << "," << endl;

  writer.writeAction(stream, Tm3Neurons_T4_10[0], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[1], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[2], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[3], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[4], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[5], actions, 4);
  stream << "," << endl;

  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  i = 0;
  bool isFirst = true;
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter, ++i) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (L1ColorArray[i] != "[1, 1, 1]") {
        if (!isFirst) {
          stream << "," << endl;
        }
        actions[3] = L1ColorArray[i];
        writer.writeAction(stream, *iter, actions, 4);
        isFirst = false;
      }
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Zoom in
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.159282, 0.108322, -1.615]");
  camera.push_back("center");
  camera.push_back("[0.152259, -0.378408, -0.130522]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show arrow
  interval = 200;
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back(arrowColor.toJsonString());
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Zoom back
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.159282, -0.108322, 1.615]");
  camera.push_back("center");
  camera.push_back("[-0.152259, 0.378408, 0.130522]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  connectedTm3Neurons.clear();

  connectedTm3Neurons.push_back("Tm3-A");
  connectedTm3Neurons.push_back("Tm3-b-A");
  connectedTm3Neurons.push_back("Tm3-f-P");
  connectedTm3Neurons.push_back("Tm3-p3-P");
  connectedTm3Neurons.push_back("Tm3-f-A");
  connectedTm3Neurons.push_back("Tm3-r5-P");
  connectedTm3Neurons.push_back("Tm3-h7-A");
  connectedTm3Neurons.push_back("Tm3-o2-P");
  connectedTm3Neurons.push_back("Tm3-e-A");

  vector<string> fading;
  fading.push_back("0.0005");
  fading.push_back("0.001");
  fading.push_back("0.0003");
  fading.push_back("0.0006");
  fading.push_back("0.0003");
  fading.push_back("0.0001");
  fading.push_back("0.0004");
  fading.push_back("0.0003");
  fading.push_back("0.0009");

  //Put T4-12 and its connected neurons back
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.001");
  actions.push_back("fade");
  actions.push_back("0.001");
  actions.push_back("color");
  actions.push_back(colorT4.toJsonString());

  writer.writeAction(stream, "T4-12", actions, 4);
  stream << "," << endl;

  actions[7] = colorMi1.toJsonString();
  writer.writeAction(stream, "Mi1-a", actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, "Mi1-e", actions, 4);

  actions[7] = colorTm3.toJsonString();
  for (size_t index = 0; index < connectedTm3Neurons.size(); ++index) {
    stream << "," << endl;
    actions[5] = fading[index];
    writer.writeAction(stream, connectedTm3Neurons[index], actions, 4);
  }
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Change L1 color
  L1ColorArray[0] = "[1, 0.2115, 0.2115]";
  L1ColorArray[1] = "[0.38099, 0, 1]";
  L1ColorArray[2] = "[1, 0.97789, 0.97789]";
  L1ColorArray[3] = "[1, 1, 1]";
  L1ColorArray[4] = "[1, 0.6776, 0.6776]";
  L1ColorArray[5] = "[0.12122, 0, 1]";
  L1ColorArray[6] = "[1, 0.33493, 0.33493]";
  L1ColorArray[7] = "[1, 0.77155, 0.77155]";
  L1ColorArray[8] = "[1, 0.86735, 0.86735]";
  L1ColorArray[9] = "[1, 1, 1]";
  L1ColorArray[10] = "[1, 1, 1]";
  L1ColorArray[11] = "[1, 1, 1]";
  L1ColorArray[12] = "[1, 1, 1]";
  L1ColorArray[13] = "[1, 0.98342, 0.98342]";
  L1ColorArray[14] = "[1, 0.54864, 0.54864]";
  L1ColorArray[15] = "[1, 0.37546, 0.37546]";
  L1ColorArray[16] = "[1, 0.68128, 0.68128]";
  L1ColorArray[17] = "[1, 0.83972, 0.83972]";
  L1ColorArray[18] = "[1, 0.64075, 0.64075]";

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);

  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");

  i = 0;
  isFirst = true;
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter, ++i) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (!isFirst) {
        stream << "," << endl;
      }
      actions[3] = L1ColorArray[i];
      writer.writeAction(stream, *iter, actions, 4);
      isFirst = false;
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Rotate
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 4000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.09");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << endl;

  /*
  writer.writeSceneStart(stream, 2);

  writer.writeDuration(stream, )

  writer.writeSceneEnd(stream, 2);
  */

  writer.writePlotEnd(stream, 1);
  stream << endl;

  stream << "}";

#endif

#if 0 // for reconstruction movie
  ZMovieScriptGenerator writer;
  /*
  string id = "gray_stack";
  string source = dataPath + "/flyem/TEM/gray_ds_avg.tif";
  writer.addActor(id, source);
  id = "R7_205";
  source = dataPath + "/flyem/skeletonization/session3/len15/adjusted2/R7_205.swc";
  writer.addActor(id, source);
  writer.writeCast(cout, 1);
  */

  vector<ZPoint> colorArray;
  colorArray.push_back(ZPoint(1.0, 0.0, 0.0));
  colorArray.push_back(ZPoint(0.5, 0.0, 0.0));
  colorArray.push_back(ZPoint(0.0, 1.0, 0.0));
  colorArray.push_back(ZPoint(0.133, 0.545, 0.133));
  colorArray.push_back(ZPoint(0.0, 0.0, 1.0));
  colorArray.push_back(ZPoint(0.117, 0.546, 1.0));
  colorArray.push_back(ZPoint(0.25, 0.875, 0.8125));
  colorArray.push_back(ZPoint(0.439, 0.858, 0.576));
  colorArray.push_back(ZPoint(1.0, 1.0, 0.0));
  colorArray.push_back(ZPoint(1.0, 1.0, 0.5));
  colorArray.push_back(ZPoint(1.0, 0.705, 0.549));
  colorArray.push_back(ZPoint(1.0, 0.84, 0.0));
  colorArray.push_back(ZPoint(1.0, 0.64, 0.0));
  colorArray.push_back(ZPoint(0.625, 0.125, 0.9375));
  colorArray.push_back(ZPoint(1.0, 0.0, 1.0));
  colorArray.push_back(ZPoint(0.93, 0.508, 0.93));

  vector<string> laminaInputs;
  laminaInputs.push_back("R7");
  laminaInputs.push_back("R8");
  laminaInputs.push_back("L1");
  laminaInputs.push_back("L2");
  laminaInputs.push_back("L3");
  laminaInputs.push_back("L4");
  laminaInputs.push_back("L5");
  laminaInputs.push_back("C2");
  laminaInputs.push_back("C3");
  laminaInputs.push_back("T1");
  laminaInputs.push_back("lawf1 224"); //LaWF1 neuron
  //laminaInputs.push_back("lawf1 446840"); //LaWF1 neuron

  vector<string> firstOrderNeurons;
  firstOrderNeurons.push_back("Tm1");
  firstOrderNeurons.push_back("Tm2");
  firstOrderNeurons.push_back("Tm3-P");
  firstOrderNeurons.push_back("Tm4");
  firstOrderNeurons.push_back("Tm5a");
  firstOrderNeurons.push_back("Tm9");
  firstOrderNeurons.push_back("Tm20");
  firstOrderNeurons.push_back("Mi1");
  firstOrderNeurons.push_back("Mi4");
  firstOrderNeurons.push_back("Mi9");
  firstOrderNeurons.push_back("Dm2 109");
  firstOrderNeurons.push_back("Dm4 147809");
  firstOrderNeurons.push_back("Dm8");
  firstOrderNeurons.push_back("Dm9 178127");

  vector<string> secondOrderNeurons;
  secondOrderNeurons.push_back("Tm5b");
  secondOrderNeurons.push_back("Tm5Y 1170");
  secondOrderNeurons.push_back("Tm6/14 4781");
  secondOrderNeurons.push_back("unknown Tm-1 3474");
  secondOrderNeurons.push_back("Tm12/25 3450");
  secondOrderNeurons.push_back("unknown Tm-1 305305");
  secondOrderNeurons.push_back("Tm23/Tm24 132");
  secondOrderNeurons.push_back("Tm23/Tm24 89");
  secondOrderNeurons.push_back("Tm28/TmY9 195");
  secondOrderNeurons.push_back("TmY14 221791");
  secondOrderNeurons.push_back("TmY13 4248");
  secondOrderNeurons.push_back("TmY3 305999");
  secondOrderNeurons.push_back("TmY4 5816");
  secondOrderNeurons.push_back("TmY5-like 61791");
  secondOrderNeurons.push_back("TmY5a 352887");
  secondOrderNeurons.push_back("TmY10 5743");
  secondOrderNeurons.push_back("TmY11/Y13-1");
  secondOrderNeurons.push_back("Mi3-like 230");
  secondOrderNeurons.push_back("Mi10");
  secondOrderNeurons.push_back("Mi11-like 1648");
  secondOrderNeurons.push_back("Mi13 2511");
  secondOrderNeurons.push_back("Mi14 6042");
  secondOrderNeurons.push_back("Mi15");
  secondOrderNeurons.push_back("Dm1-like 306420");
  secondOrderNeurons.push_back("Dm3-like 92550");
  secondOrderNeurons.push_back("Dm5-like 22293");
  secondOrderNeurons.push_back("Dm7-like 3647");
  secondOrderNeurons.push_back("Dm10 306715");
  secondOrderNeurons.push_back("Pm1 1980");
  secondOrderNeurons.push_back("Pm2-like 171795");
  secondOrderNeurons.push_back("unknown Pm-1 1773");
  secondOrderNeurons.push_back("T2a");
  secondOrderNeurons.push_back("T3 1554");
  secondOrderNeurons.push_back("T4-11");
  secondOrderNeurons.push_back("Y4 1593971");
  secondOrderNeurons.push_back("Y3/Y6 2443");

  vector<string> interColumnarNeurons;
  interColumnarNeurons.push_back("Tm3-A");
  interColumnarNeurons.push_back("Tm3-e-P");
  interColumnarNeurons.push_back("Tm3-b-A");
  interColumnarNeurons.push_back("Tm3-j9-A");
  interColumnarNeurons.push_back("Tm3-d-P");
  interColumnarNeurons.push_back("Tm3-f-P");
  interColumnarNeurons.push_back("Tm3-d-A");
  interColumnarNeurons.push_back("Tm4-c");
  interColumnarNeurons.push_back("Tm4-e");
  interColumnarNeurons.push_back("Tm4-d");
  interColumnarNeurons.push_back("Tm4-a/f");
  interColumnarNeurons.push_back("Tm4-b/c");
  interColumnarNeurons.push_back("Dm3-like 82307");
  interColumnarNeurons.push_back("Dm3-like 133922");
  interColumnarNeurons.push_back("Dm8-b");
  interColumnarNeurons.push_back("Dm8-c");
  interColumnarNeurons.push_back("Pm1 106054");
  interColumnarNeurons.push_back("Pm1 1172");
  interColumnarNeurons.push_back("Pm1 2236");
  interColumnarNeurons.push_back("Pm1 4390");
  interColumnarNeurons.push_back("Pm2-like 1196");
  interColumnarNeurons.push_back("Pm2-like 1513");
  interColumnarNeurons.push_back("Pm2-like 1974");
  interColumnarNeurons.push_back("Pm2-like 4366");
  interColumnarNeurons.push_back("Pm2-like 19223");
  interColumnarNeurons.push_back("Pm2-like 35779");
  interColumnarNeurons.push_back("Pm2-like 105822");
  interColumnarNeurons.push_back("Pm2-like 172362");
  interColumnarNeurons.push_back("Pm2-like 172393");
  interColumnarNeurons.push_back("Pm2-like 254342");
  interColumnarNeurons.push_back("Pm2-like 310051");

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle2.json");
  const vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (ZString(iter->getName()).startsWith("T4-") && iter->getName() != "T4-11") {
      interColumnarNeurons.push_back(iter->getName());
    }
  }

  std::ofstream stream((dataPath + "/flyem/TEM/movie/script5.json").c_str());

  std::ofstream neuronOrderStream(
        (dataPath + "/flyem/TEM/movie/script5_neuron_order.txt").c_str());

  vector<string> tanNeurons;

  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZString className = iter->getClass();
    if (className.startsWith("Mt")) {
      tanNeurons.push_back(iter->getName());
    }
  }

  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZString className = iter->getClass();
    if (className.startsWith("Tangential")) {
      tanNeurons.push_back(iter->getName());
    }
  }

  set<string> priorNeuronSet;
  for (vector<string>::const_iterator iter = laminaInputs.begin();
       iter != laminaInputs.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }
  for (vector<string>::const_iterator iter = firstOrderNeurons.begin();
       iter != firstOrderNeurons.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }
  for (vector<string>::const_iterator iter = secondOrderNeurons.begin();
       iter != secondOrderNeurons.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }
  for (vector<string>::const_iterator iter = interColumnarNeurons.begin();
       iter != interColumnarNeurons.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }
  for (vector<string>::const_iterator iter = tanNeurons.begin();
       iter != tanNeurons.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }

  //Check name consistency
  for (set<string>::const_iterator iter = priorNeuronSet.begin();
       iter != priorNeuronSet.end(); ++iter) {
    if (!bundle.hasNeuronName(*iter)) {
      cout << *iter << ": unknown name" << endl;
      cout << bundle.getIdFromName(*iter) << endl;
    }
  }


  vector<string> otherNeurons;
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (priorNeuronSet.count(iter->getName()) == 0) {
      otherNeurons.push_back(iter->getName());
    }
    if (!fexist(iter->getModelPath().c_str())) {
      cout << "No model: " << iter->getName() << endl;
    }
  }

  vector<vector<string> > allNeuronList;
  allNeuronList.push_back(laminaInputs);
  allNeuronList.push_back(firstOrderNeurons);
  allNeuronList.push_back(secondOrderNeurons);
  allNeuronList.push_back(interColumnarNeurons);
  allNeuronList.push_back(tanNeurons);
  allNeuronList.push_back(otherNeurons);

  writer.clear();

  double rotateSpeed = 0.02;
  double clipSpeed = 0.17;

  //Write cast
  stream << "{" << endl;

  writer.addActor("gray_stack",
                  dataPath + "/flyem/TEM/gray_ds_avg.tif");
  //const vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    writer.addActor(iter->getName(), iter->getModelPath());
  }

  writer.writeCast(stream, 1);
  stream << "," << endl;

  //Write stack show up
  writer.clear();
  writer.addActor("gray_stack", "");
  stream << "  \"plot\": [" << endl;
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  writer.writeShowAction(stream, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"reset\": {" << endl;
  stream << "          \"center\": [621, 590, 617]," << endl;
  stream << "          \"eye\": [2603, -1871, -963]," << endl;
  stream << "          \"up_vector\": [0.267, -0.42, -0.87]" << endl;
  stream << "        }," << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;


  //Lamina inputs

  stream << "    {" << endl;
  stream << "      \"duration\": 6500," << endl;
  stream << "      \"action\": [" << endl;

  writer.clear();


  size_t colorIndex = 0;
  neuronOrderStream << "Lamina inputs" << " (" << laminaInputs.size() << ") " << endl;
  for (vector<string>::const_iterator iter = laminaInputs.begin();
       iter != laminaInputs.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != laminaInputs.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }
  stream << endl;
  stream << "      ]," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }," << endl;

  stream << "      \"clipper\": [" << endl;
  stream << "        {" << endl;
  stream << "          \"target\": \"volume\"," << endl;
  stream << "          \"axis\": \"z\"," << endl;
  stream << "          \"upper\": 0," << endl;
  stream << "          \"lower\": " << clipSpeed << endl;
  stream << "        }" << endl;
  stream << "      ]" << endl;
  stream << "    }," << endl;

  //First order neurons
  neuronOrderStream << "First order neurons" << " (" << firstOrderNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = firstOrderNeurons.begin();
       iter != firstOrderNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }," << endl;
    stream << "      \"clipper\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"target\": \"volume\"," << endl;
    stream << "          \"axis\": \"z\"," << endl;
    stream << "          \"upper\": 0," << endl;
    stream << "          \"lower\": " << clipSpeed << endl;
    stream << "        }" << endl;
    stream << "      ]" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }

  //Hide gray stack
  stream << "    {" << endl;
  stream << "      \"duration\": 40," << endl;
  stream << "      \"action\": [" << endl;
  stream << "        {" << endl;
  stream << "          \"id\":" << "\"gray_stack\"," <<endl;
  stream << "          \"visible\": false" << endl;
  stream << "        }" << endl;
  stream << "      ]" <<endl;
  stream << "    }," << endl;


  //Turn L inputs and first order neurons into white
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;

  vector<string> actions;
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 1.0, 1.0]");
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;

  //Add second order neurons
  writer.clear();
  neuronOrderStream << "Second order neurons" << " (" << secondOrderNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = secondOrderNeurons.begin();
       iter != secondOrderNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }

  //Turn second order neurons into white
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;


  //Add inter-columnar neurons
  writer.clear();
  neuronOrderStream << "Inter-columnar neurons" << " (" << interColumnarNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = interColumnarNeurons.begin();
       iter != interColumnarNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " <<*iter << ": " << bundle.getIdFromName(*iter) << endl;
  }

  //Turn inter-columnar neurons into white
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;

  //Add tangential neurons
  writer.clear();
  neuronOrderStream << "Tangential neurons" << " (" << tanNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = tanNeurons.begin();
       iter != tanNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }

  //Turn tangential neurons into white
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;

  //Add other neurons
  writer.clear();
  neuronOrderStream << "Other neurons" << " (" << otherNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = otherNeurons.begin();
       iter != otherNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    if (iter != otherNeurons.begin()) {
      stream << "," << endl;
    }

    stream << "    {" << endl;
    stream << "      \"duration\": 40," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }";

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }
  stream << "," << endl;

  //Recolor all neurons
  stream << "    {" << endl;
  stream << "      \"duration\": 18000," << endl;
  stream << "      \"action\": [" << endl;
  colorIndex = 0;
  for (vector<string>::const_iterator iter = laminaInputs.begin();
       iter != laminaInputs.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != laminaInputs.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  stream << "," << endl;

  for (vector<string>::const_iterator iter = firstOrderNeurons.begin();
       iter != firstOrderNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != firstOrderNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  stream << "," << endl;

  for (vector<string>::const_iterator iter = secondOrderNeurons.begin();
       iter != secondOrderNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != secondOrderNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  stream << "," << endl;

  for (vector<string>::const_iterator iter = interColumnarNeurons.begin();
       iter != interColumnarNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != interColumnarNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  stream << "," << endl;

  for (vector<string>::const_iterator iter = tanNeurons.begin();
       iter != tanNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != tanNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  /*
  stream << "," << endl;

  for (vector<string>::const_iterator iter = otherNeurons.begin();
       iter != otherNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != otherNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  */

  stream << endl;

  stream << "      ]," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }" << endl;

  stream << "  ]" << endl;
  stream << "}" << endl;


  /*
  actions.push_back("alpha");
  actions.push_back("0.001");
  actions.push_back("fade");
  actions.push_back("0.001");
  */
  //writer.writeShowAction(stream, 3);


#if 0
  for (vector<string>::const_iterator iter = otherNeurons.begin();
       iter != otherNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": 0.1" << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));
  }

  size_t colorIndex = 0;
  for (vector<string>::const_iterator iter = otherNeurons.begin();
       iter != otherNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": 0.1" << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));
  }

  /*
  const vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    //if (iter->getClass() == "Mi9") {
      writer.addActor(iter->getName(), iter->getModelPath());
    //}
  }

  writer.writeCast(stream, 1);
  stream << endl;
*/
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  vector<string> actions;
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 1.0, 1.0]");
  /*
  actions.push_back("alpha");
  actions.push_back("0.001");
  actions.push_back("fade");
  actions.push_back("0.001");
  */
  //writer.writeShowAction(stream, 3);
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;

  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": 0.1" << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }" << endl;
#endif


  stream.close();
  neuronOrderStream.close();

  /*
  Z3DCamera camera;
  camera.setCenter(glm::vec3(577, 574, 1200));
  camera.setEye(glm::vec3(1000, 1000, 2400));

  writer.writeCast(cout, 1);

  writer.writePlotBegin(cout, 1);

  writer.writeSceneBegin(cout, 1);
  writer.writeDuration(1000, cout, 1);
  writer.writeActionBegin(cout, 1);
  writer.writeAction("grey_stack", "show");
  writer.writeCamera("reset", )
  writer.writeActionEnd(cout, 1);
  writer.writeSceneEnd(cout, 1);

  writer.writePlotEnd(cout, 1);
  */
#endif

#if 0
  ZMovieScript script;

#if 0
  script.addActor(1, dataPath + "/tmp/swc3/adjusted/C2_214.swc");
  ZMovieScene scene;
  scene.setDuration(1.0);
  MovieAction action;
  action.actorId = 1;
  action.isVisible = true;
  scene.addAction(action);
  script.addScene(scene);
#endif

  if (script.loadScript(dataPath + "/test/L1_pathway.json")) {
    script.printSummary();
    script.print();

    ZMovieMaker director;
    director.setScript(script);
    director.setFrameInterval(40);
    director.make(dataPath + "/test/movie");
  } else {
    cout << "Failed to load " << dataPath + "/test/L1_pathway.json" << endl;
  }
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle.json");
  bundle.print();
#endif

#if 0
  ZFlyEmNeuron neuron;
  neuron.printJson(&cout, 2);
#endif

#if 0
  cout << setfill('x');
  cout << setw(10) << "" << endl;
#endif

#if 0
  ZSwcTree tree;
  tree.load((dataPath + "/test.swc").c_str());
  ZSwcTree *tree2 = tree.createBoundBoxSwc(100.0);
  tree2->save((dataPath + "/test2.swc").c_str());

#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted3/T4-1_277709.swc").c_str());
  tree2.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted3/T4-10_476680.swc").c_str());

  tree1.resample(200);
  tree2.resample(200);

  tree1.setType(0);
  tree2.setType(0);

  ZSwcTreeMatcher matcher;
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  ZSwcLayerFeatureAnalyzer featureAnalyzer;

  matcher.setFeatureAnalyzer(&featureAnalyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  matcher.matchAllG(tree1, tree2, 1);


  cout << "Score:  " << matcher.matchingScore() << endl;
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load((dataPath + "/benchmark/swc/compare/compare1.swc").c_str());
  tree2.load((dataPath + "/benchmark/swc/compare/compare2.swc").c_str());
  tree2.translate(0, 0, 50.0);

  ZSwcTreeMatcher matcher;
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  ZSwcSizeFeatureAnalyzer featureAnalyzer;

  matcher.setFeatureAnalyzer(&featureAnalyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  matcher.matchAllG(tree1, tree2, 1);

  ZSwcTree *tree = matcher.exportResultAsSwc();
  tree->merge(tree1.data());
  tree->merge(tree2.data());
  tree->resortId();
  tree->save((dataPath + "/test.swc").c_str());
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load((dataPath + "/benchmark/swc/compare/compare1.swc").c_str());
  tree2.load((dataPath + "/benchmark/swc/compare/compare2.swc").c_str());
  tree2.translate(0, 0, 50.0);

  tree1.resample(20.0);
  tree2.resample(20.0);

  tree1.setType(0);
  tree2.setType(0);

  ZSwcTreeMatcher matcher;
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  ZSwcSizeFeatureAnalyzer featureAnalyzer;

  matcher.setFeatureAnalyzer(&featureAnalyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  matcher.matchAllG(tree1, tree2, 3);

  tree1.setTypeByLabel();
  tree1.resortId();
  tree1.save((dataPath + "/test.swc").c_str());

  tree2.setTypeByLabel();
  tree2.resortId();
  tree2.save((dataPath + "/test2.swc").c_str());

  ZSwcTree *result = matcher.exportResultAsSwc();
  result->save((dataPath + "/test3.swc").c_str());

  delete result;

  cout << "Score:  " << matcher.matchingScore() << endl;
#endif

#if 0
  string modelPath = dataPath +
      "/flyem/skeletonization/session3/len15/adjusted3";

  ZFileList modelList;
  modelList.load(modelPath, "swc");

  FILE *fp = fopen((dataPath + "/flyem/TEM/neuron.csv").c_str(), "r");
  ZString str;

  ofstream stream((dataPath + "/flyem/TEM/neuron.json").c_str());

  while (str.readLine(fp)) {
    ZFlyEmNeuron neuron;
    std::vector<string> tokenArray = str.tokenize(',');
    if (tokenArray.size() == 6) {
      if (tokenArray[0] != "Old Name") {
        neuron.setId(tokenArray[1]);
        if (neuron.getId() > 0) {
          neuron.setName(tokenArray[3]);
          neuron.setClass(tokenArray[2]);
          for (int i = 0; i < modelList.size(); ++i) {
            ZString filePath(modelList.getFilePath(i));
            ostringstream stream;
            stream << "_" << neuron.getId() << ".swc";

            if (filePath.endsWith(stream.str())) {
              neuron.setModelPath(filePath.c_str());
              break;
            }
          }
          neuron.printJson(&stream, 4);
        }
      }
    }
  }
  fclose(fp);

  stream.close();
#endif

#if 0
  ZSwcTree tree;
  cout << tree.className() << endl;

  ZPoint pt;
  cout << pt.className() << endl;
#endif

#if 0
  Stack *stack = C_Stack::make(GREY, 1024, 1024, 1);
  Zero_Stack(stack);
  C_Stack::drawInteger(stack, 123456780, 100, 100, 0);

  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0
  tr1::shared_ptr<ZStackDoc> academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);
  academy->loadFile((dataPath + "/flyem/TEM/gray_ds10_avg/xy-grayscale-01267.tif").c_str());
  Z3DWindow *stage = new Z3DWindow(academy, false, NULL);

  stage->show();

  ZStack *stack = new ZStack;
  stack->load("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/movie/actor/colored_slice.tif");
  academy->loadStack(stack);
#endif

#if 0
  map<int, ZObject3dScan*> bodySet;
  Stack *stack = Read_Stack_U((dataPath + "/benchmark/rice_label.tif").c_str());
  uint8_t *array = stack->array;
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
  int y = 0;
  int z = 0;

  for (z = 0; z < depth; ++z) {
    for (y = 0; y < height; ++y) {
      int x = 0;
      while (x < width) {
        int v = array[x];
        if (bodySet.count(v) == 0) {
          bodySet[v] = new ZObject3dScan;
        }
        ZObject3dScan *obj = bodySet[v];

        int length = obj->scanArray(array, x, y, z, width);

        x += length;
        //cout << length << " " << x << endl;
      }
      array += width;
    }
  }

  C_Stack::setZero(stack);

  for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    //iter->second->print();
    iter->second->drawStack(stack, iter->first);
  }

  C_Stack::write(dataPath + "/test.tif", stack);

#endif

#if 0
  ZObject3dScan obj;
  /*
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  One_Stack(stack);
  stack->array[0] = 0;
  stack->array[4] = 0;
  stack->array[6] = 0;
  stack->array[7] = 0;
  stack->array[8] = 0;
  */

  //Stack *stack = Read_Stack_U((dataPath + "/benchmark/binary/2d/disk_n10.tif").c_str());
  Stack *stack = Read_Stack_U(
        (dataPath + "/flyem/skeletonization/session3/C2c_3668.tif").c_str());
  obj.loadStack(stack);

  C_Stack::kill(stack);

  obj.save(dataPath + "/test.obj");

  obj.load(dataPath + "/test.obj");

  obj.print();

  ZObject3d *obj2 = obj.toObject3d();
  Stack *recovered = obj2->toStack(NULL);
  C_Stack::write(dataPath + "/test.tif", recovered);
  delete obj2;
  //Stack *stack = Read_Stack(dataPath + "");
#endif

#if 0
  ZObject3dStripe stripe;
  stripe.setY(0);
  stripe.setZ(1);
  stripe.addSegment(3, 4);
  stripe.addSegment(3, 9);
  stripe.addSegment(5, 3);
//  stripe.sort();
//  stripe.print();
//  stripe.canonize();
//  stripe.print();

  ZObject3dScan obj;
  obj.addStripe(stripe);
  obj.addStripe(stripe);
  obj.print();

  obj.downsample(1, 0, 0);
  cout << "After downsample: " << endl;
  obj.print();

  obj.canonize();
  obj.print();
#endif

#if 0
  ZObject3dStripe stripe;
  stripe.setY(0);
  stripe.setZ(0);
  stripe.addSegment(3, 9);

  ZObject3dScan obj;
  obj.addStripe(stripe);

  stripe.clearSegment();
  stripe.setY(2);
  stripe.setZ(1);
  stripe.addSegment(4, 5);
  stripe.addSegment(8, 9);

  obj.addStripe(stripe);

  obj.downsampleMax(1, 1, 1);
  obj.print();

  Stack *stack = obj.toStack();

  C_Stack::print(stack);
#endif

#if 0
  ZObject3dScan obj;
  obj.load(dataPath + "/flyem/FIB/21784.sobj");
  obj.downsampleMax(4, 4, 4);
  int offset[3];
  Stack *stack = obj.toStack(offset);
  offset[2] -= 298;
  cout << offset[0] << " " << offset[1] << " " << offset[2] << endl;
  C_Stack::write(dataPath + "/test.tif", stack);

  ofstream stream((dataPath + "/flyem/FIB/21784.json").c_str());

  stream << "{" << endl;
  stream << "  \"transform\": {" << endl;
  stream << "    \"scale\": [1.6129, 1.6129, 1.2500]," << endl;
  stream << "    \"translate\": [" << offset[0] << ", " << offset[1] << ", "
         << offset[2] << "]," << endl;
  stream << "    \"scale_first\": false" << endl;
  stream << "  }" << endl;
  stream << "}";
  stream.close();
#endif

#if 0
  IMatrix *mat = IMatrix_Read((dataPath + "/test/session2/body_map/body_map00161.imat").c_str());
  int width = mat->dim[0];
  int height = mat->dim[1];
  int depth = mat->dim[2];
  map<int, ZObject3dScan*> bodySet;
  ZObject3dScan::extractAllObject(mat->array, width, height, depth, 0, &bodySet);
  Kill_IMatrix(mat);

  mat = IMatrix_Read((dataPath + "/test/session2/body_map/body_map00162.imat").c_str());
  ZObject3dScan::extractAllObject(mat->array, width, height, depth, 1, &bodySet);
  Kill_IMatrix(mat);

  Stack *stack = C_Stack::make(GREY16, width, height, 2);
  C_Stack::setZero(stack);

  int index = 1;
  for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter, ++index) {
    iter->second->drawStack(stack, index);
    ZString filePath = dataPath + "/test/session2/bodies/";
    filePath.appendNumber(iter->first, 0);
    iter->second->save(filePath + ".sobj");
  }

  C_Stack::write(dataPath + "/test.tif", stack);
  C_Stack::kill(stack);
#endif

#if 0
  map<int, ZObject3dScan*> bodySet;
  for (int z = 161; z <= 1461; ++z) {
    //ZString filePath = dataPath + "/test/session2/body_map/body_map";
    ZString filePath = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/body_map/body_map";
    filePath.appendNumber(z, 5);
    filePath = filePath + ".imat";

    if (fexist(filePath.c_str())) {
      cout << "Loading " << filePath << endl;
      IMatrix *mat = IMatrix_Read(filePath.c_str());
      int width = mat->dim[0];
      int height = mat->dim[1];
      int depth = mat->dim[2];

      tic();
      ZObject3dScan::extractAllObject(mat->array, width, height, depth, z, &bodySet);
      ptoc();
      Kill_IMatrix(mat);
    }
  }

  for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    ZString filePath = dataPath + "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/bodies/";
    filePath.appendNumber(iter->first, 0);
    iter->second->save(filePath + ".sobj");
  }

  for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    delete iter->second;
    iter->second = NULL;
  }
  bodySet.clear();

#endif

#if 0
  ZString bodyDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/bodies";
  for (int z = 161; z <= 1461; ++z) {
    map<int, ZObject3dScan*> bodySet;
    //ZString filePath = dataPath + "/test/session2/body_map/body_map";
    ZString filePath = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/body_map/body_map";
    filePath.appendNumber(z, 5);
    filePath = filePath + ".imat";

    if (fexist(filePath.c_str())) {
      cout << "Loading " << filePath << endl;
      IMatrix *mat = IMatrix_Read(filePath.c_str());
      int width = mat->dim[0];
      int height = mat->dim[1];
      int depth = mat->dim[2];

      tic();
      ZObject3dScan::extractAllObject(mat->array, width, height, depth, z, &bodySet);
      ptoc();
      Kill_IMatrix(mat);

      ZString objDir;
      objDir.appendNumber(z, 5);
      QDir dir(bodyDir.c_str());
      dir.mkdir(objDir.c_str());

      for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
           iter != bodySet.end(); ++iter) {
        ZString filePath = bodyDir + "/" + objDir + "/";
        filePath.appendNumber(iter->first, 0);
        iter->second->save(filePath + ".sobj");
      }

      for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
           iter != bodySet.end(); ++iter) {
        delete iter->second;
        iter->second = NULL;
      }
      bodySet.clear();
    }
  }

  /*
  for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    ZString filePath = dataPath + "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/bodies/";
    filePath.appendNumber(iter->first, 0);
    iter->second->save(filePath + ".sobj");
  }

  for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    delete iter->second;
    iter->second = NULL;
  }
  bodySet.clear();
*/
#endif

#if 0
  int bodyId = 209;
  ZString bodyDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/bodies";
  ZObject3dScan obj;
  for (int z = 161; z < 200; ++z) {
    ZString objDir;
    objDir.appendNumber(z, 5);
    ZString objPath = bodyDir + "/" + objDir + "/";
    objPath.appendNumber(bodyId, 0);
    objPath += ".sobj";
    ZObject3dScan objSlice;
    objSlice.load(objPath);
    obj.concat(objSlice);
  }

  obj.save(dataPath + "/test.sobj");

#endif

#if 0
  const int zStart = 1490;
  const int zEnd = 7509;
  ZString sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session3";
  ZString bodyDir = sessionDir + "/bodies";

  if (!dexist(bodyDir.c_str())) {
    mkdir(bodyDir.c_str(), 0755);
  }

  set<int> bodyIdSet;

  for (int z = zStart; z <= zEnd; ++z) {
    map<int, ZObject3dScan*> bodySet;
    ZString filePath = sessionDir + "/body_maps/body_map";
    filePath.appendNumber(z, 5);
    filePath = filePath + ".imat";

    if (fexist(filePath.c_str())) {
      ZString objDir = bodyDir + "/";
      objDir.appendNumber(z, 5);

      if (!dexist(objDir.c_str())) {
        mkdir(objDir.c_str(), 0755);

        cout << "Loading " << filePath << endl;
        IMatrix *mat = IMatrix_Read(filePath.c_str());
        int width = mat->dim[0];
        int height = mat->dim[1];
        int depth = mat->dim[2];

        ZObject3dScan::extractAllObject(mat->array, width, height, depth,
                                        z - zStart, &bodySet);
        Kill_IMatrix(mat);

        for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          ZString filePath = objDir + "/";
          filePath.appendNumber(iter->first, 0);
          iter->second->save(filePath + ".sobj");
        }

        for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          delete iter->second;
          iter->second = NULL;
        }
        bodySet.clear();
      }

      ZFileList fileList;
      fileList.load(objDir, "sobj");
      for (int i = 0; i < fileList.size(); ++i) {
        //cout << fileList.getFilePath(i) << endl;
        int id = String_Last_Integer(fileList.getFilePath(i));
        bodyIdSet.insert(bodyIdSet.end(), id);
      }
    }
  }

  ZString fullBodyDir = bodyDir + "/stacked";
  if (!dexist(fullBodyDir.c_str())) {
    mkdir(fullBodyDir.c_str(), 0755);
  }

  vector<std::pair<size_t, int> > objSizeArray;
  for (set<int>::const_iterator iter = bodyIdSet.begin(); iter != bodyIdSet.end();
       ++iter) {
    //Load the object
    int bodyId = *iter;
    ZObject3dScan obj;
    for (int z = zStart; z < zEnd; ++z) {
      ZString objPath = bodyDir + "/";
      objPath.appendNumber(z, 5);
      objPath += "/";
      objPath.appendNumber(bodyId);
      objPath += ".sobj";
      ZObject3dScan objSlice;
      if (objSlice.load(objPath)) {
        obj.concat(objSlice);
      }
    }

    std::pair<size_t, int> objSize;
    objSize.first = obj.getVoxelNumber();
    objSize.second = bodyId;

    if (objSize.first > 10000000) {
      objSizeArray.push_back(objSize);

      ZString stackedObjPath = fullBodyDir + "/";
      stackedObjPath.appendNumber(bodyId);
      stackedObjPath += ".sobj";
      obj.save(stackedObjPath);
    }
  }

  sort(objSizeArray.begin(), objSizeArray.end());
  ofstream stream((bodyDir +"/bodylist.txt").c_str());

  for (vector<std::pair<size_t, int> >::const_reverse_iterator
       iter = objSizeArray.rbegin(); iter != objSizeArray.rend(); ++iter) {
    stream << iter->second << ", " << iter->first << endl;
  }

  stream.close();
#endif

#if 0
  //Load all FIB objects
  ZString sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session1";
  ZString bodyDir = sessionDir + "/bodies";

  set<int> bodyIdSet;

  for (int z = 1490; z <= 4509; ++z) {
    ZFileList fileList;
    ZString objDir = bodyDir + "/";
    objDir.appendNumber(z, 5);

    fileList.load(objDir, "sobj");

    for (int i = 0; i < fileList.size(); ++i) {
      cout << fileList.getFilePath(i) << endl;
      int id = String_Last_Integer(fileList.getFilePath(i));
      bodyIdSet.insert(bodyIdSet.end(), id);
    }
  }

  for (set<int>::const_iterator iter = bodyIdSet.begin(); iter != bodyIdSet.end();
       ++iter) {
    //Load the object
    int bodyId = *iter;
    ZObject3dScan obj;
    for (int z = 1490; z < 4509; ++z) {
      ZString objDir;
      objDir.appendNumber(z, 5);
      ZString objPath = bodyDir + "/" + objDir + "/";
      objPath.appendNumber(bodyId, 0);
      objPath += ".sobj";
      ZObject3dScan objSlice;
      if (objSlice.load(objPath)) {
        obj.concat(objSlice);
      }
    }
    cout << *iter << ": " << obj.getVoxelNumber() << endl;

    ZString stackedObjPath = bodyDir + "/stacked/";
    stackedObjPath.appendNumber(bodyId, 0);
    stackedObjPath += ".sobj";
    obj.save(stackedObjPath);
  }
#endif

#if 0
  //Index all object
  ZString sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session1";
  ZString bodyDir = sessionDir + "/bodies";
  ZFileList fileList;
  ZString objDir = bodyDir + "/stacked";
  fileList.load(objDir, "sobj");

  int fileNumber = fileList.size();
  vector<std::pair<size_t, int> > objSize(fileNumber);
  for (int i = 0; i < fileNumber; ++i) {
    ZObject3dScan obj;
    obj.load(fileList.getFilePath(i));
    int id = String_Last_Integer(fileList.getFilePath(i));
    objSize[i].first = obj.getVoxelNumber();
    objSize[i].second = id;
    if (objSize[i].first > 100000) {
      cout << i << "/" << fileList.size() << " " << id << ": "
           << objSize[i].first << endl;
    }
  }

  sort(objSize.begin(), objSize.end());
  ofstream stream((bodyDir +"/objlist.txt").c_str());

  for (vector<std::pair<size_t, int> >::const_reverse_iterator iter = objSize.rbegin();
       iter != objSize.rend(); ++iter) {
    stream << iter->second << ", " << iter->first << endl;
  }

  stream.close();
#endif

#if 0
  ZSwcLayerTrunkAnalyzer trunkAnalyzer;
  trunkAnalyzer.setStep(1);

  ZSwcTree tree;
  //tree.load((dataPath + "/benchmark/swc/layer_test.swc").c_str());
  tree.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted2/L1_209.swc").c_str());
  ZSwcPath path = trunkAnalyzer.extractMainTrunk(&tree);

  path.setType(0);

  tree.save((dataPath + "/test.swc").c_str());
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted2/T4-10_476680.swc").c_str());
  tree2.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted2/T4-11_588435.swc").c_str());

  tree1.setType(0);
  tree2.setType(0);

  ZSwcTreeMatcher matcher;
  ZSwcLayerTrunkAnalyzer trunkAnalyzer;
  ZSwcLayerShollFeatureAnalyzer featureAnalyzer;
  featureAnalyzer.setLayerScale(4000);

  matcher.setFeatureAnalyzer(&featureAnalyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  tic();
  matcher.matchAllG(tree1, tree2, 3);
  ptoc();

  tree1.setTypeByLabel();
  tree1.resortId();
  tree1.save((dataPath + "/test.swc").c_str());

  tree2.setTypeByLabel();
  tree2.resortId();
  tree2.save((dataPath + "/test2.swc").c_str());

  ZSwcTree *result = matcher.exportResultAsSwc(ZSwcTreeMatcher::EXPORT_ALL_MATCHING);
  result->save((dataPath + "/test3.swc").c_str());

  delete result;

  cout << "Score:  " << matcher.matchingScore() << endl;
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(1, 2);
  graph.addEdge(2, 3);
  graph.addEdge(0, 3);
  graph.addEdge(3, 4);
  graph.addEdge(4, 5);
  graph.addEdge(2, 6);

  graph.print();
  graph.exportDotFile(dataPath + "/test.dot");

  int v = 1;
  int neighborNumber = graph.getNeighborNumber(v);
  cout << "Neighbors of " << v << ": ";
  for (int i = 0; i < neighborNumber; ++i) {
    cout << graph.getNeighbor(v, i) << " ";
  }
  cout << endl;

  v = 3;
  neighborNumber = graph.getNeighborNumber(v);
  cout << "Neighbors of " << v << ": ";
  for (int i = 0; i < neighborNumber; ++i) {
    cout << graph.getNeighbor(v, i) << " ";
  }
  cout << endl;

  std::vector<int> path = graph.getPath(1, 7);

  cout << "path: " << endl;
  if (!path.empty()) {
    ZDebugPrintArrayG(path, 0, path.size() - 1);
  }
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle2.json");

  std::vector<int> neuronArray;
  ZString str;
  FILE *fp = fopen((dataPath + "/flyem/TEM/neuron_id.txt").c_str(), "r");
  while (str.readLine(fp)) {
    std::vector<int> intArray = str.toIntegerArray();
    if (!intArray.empty()) {
      neuronArray.insert(neuronArray.end(), intArray.begin(), intArray.end());
    }
  }
  fclose(fp);

  std::set<int> neuronSet;

  cout << neuronArray.size() << endl;

  for (size_t i = 0; i < neuronArray.size(); ++i) {
    int bodyId = neuronArray[i];
    if (neuronSet.count(bodyId) > 0) {
      cout << "duplicated id: " << bodyId << endl;
    }
    neuronSet.insert(bodyId);
    if (bundle.getNeuron(bodyId) == NULL) {
      cout << "No neuron: " << bodyId << endl;
    }
  }

  cout << neuronSet.size() << endl;

  const vector<ZFlyEmNeuron>& allNeuronInDatabase = bundle.getNeuronArray();

  for (size_t i = 0; i < allNeuronInDatabase.size(); ++i) {
    if (neuronSet.count(allNeuronInDatabase[i].getId()) == 0) {
      cout << "Untraced neuron: " << allNeuronInDatabase[i].getId() << endl;
    }
  }
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(1, 2);
  graph.addEdge(2, 3);
  graph.addEdge(0, 3);
  graph.addEdge(3, 4);
  graph.addEdge(4, 5);
  graph.addEdge(2, 6);

  vector<int> vertexArray;
  vertexArray.push_back(1);
  vertexArray.push_back(4);
  vertexArray.push_back(6);
  graph.mergeVertex(vertexArray);

  graph.print();
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(1, 2);
  graph.addEdge(2, 3);
  graph.addEdge(0, 3);
  graph.addEdge(3, 4);
  graph.addEdge(4, 5);
  graph.addEdge(2, 6);
  graph.addEdge(4, 7);
  graph.addEdge(4, 8);
  graph.addEdge(5, 7);

  /*
  std::vector<int> path = graph.getPath(0, 0);
  graph.mergeVertex(path);
  graph.print();
  */

  vector<bool> labeled(graph.getVertexNumber(), false);
  //For each loop in the graph, label it
  for (int i = 0; i < graph.getVertexNumber(); ++i) {
    if (!labeled[i]) {
      vector<int> path = graph.getPath(i, i);
      if (path.size() > 2) {
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          labeled[*iter] = true;
        }
      }
    }
  }

  graph.exportDotFile(dataPath + "/test.dot", labeled);
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(100, 200);
  graph.addEdge(100, 300);
  graph.addEdge(300, 400);
  graph.addEdge(300, 500);
  graph.addEdge(400, 500);

  graph.print();

  ZGraphCompressor compressor;
  compressor.setGraph(&graph);
  compressor.compress();
  graph.print();
  compressor.uncompress();

  graph.print();

#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(0, 7);
  graph.addEdge(1, 7);
  graph.addEdge(6, 7);
  graph.addEdge(5, 6);
  graph.addEdge(4, 5);
  graph.addEdge(3, 4);
  graph.addEdge(3, 5);
  graph.addEdge(1, 3);
  graph.addEdge(1, 2);
  graph.addEdge(2, 3);

  vector<bool> labeled(graph.getVertexNumber(), false);
  //For each loop in the graph, label it
  for (int i = 0; i < graph.getVertexNumber(); ++i) {
    if (!labeled[i]) {
      vector<int> path = graph.getPath(i, i);
      if (path.size() > 4) {
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          labeled[*iter] = true;
        }
      }
    }
  }

  graph.exportDotFile(dataPath + "/test.dot", labeled);
#endif

#if 0
  ZStackGraph stackGraph;
  Stack *stack = Read_Stack_U(
        (dataPath + "/MAX_loop_test.tif").c_str());
  //stack->depth = 299;
  ZGraph *graph = stackGraph.buildGraph(stack);

  ZGraphCompressor compressor;
  compressor.setGraph(graph);
  compressor.compress();
  graph->exportDotFile(dataPath + "/test.dot");
#endif

#if 0
  ZStackGraph stackGraph;
  //stackGraph.setStack(stack);

/*
  Stack *stack = Read_Stack_U(
        (dataPath + "/benchmark/binary/2d/btrig2_skel.tif").c_str());
        */
  /*
  Stack *stack = Read_Stack_U(
        (dataPath + "/digit8_skel.tif").c_str());
*/

  Stack *stack = Read_Stack_U(
        (dataPath + "/loop_test.tif").c_str());
  Stack_Threshold(stack, 100);

  ZGraph *graph = stackGraph.buildGraph(stack);

  ZGraphCompressor compressor;
  compressor.setGraph(graph);
  compressor.compress();
  //graph->exportDotFile(dataPath + "/test.dot");

  vector<bool> labeled(graph->getVertexNumber(), false);
  //For each loop in the graph, label it
  for (int i = 0; i < graph->getVertexNumber(); ++i) {
    if (!labeled[i]) {
      vector<int> path = graph->getPath(i, i);
      if (path.size() > 5) {
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          labeled[*iter] = true;
        }
      }
    }
  }

  for (size_t i = 0; i < labeled.size(); ++i) {
    if (labeled[i]) {
      stack->array[compressor.uncompress(i)] = 128;
    }
  }

  C_Stack::write(dataPath + "/test.tif", stack);
  //graph->exportDotFile(dataPath + "/test.dot", labeled);
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(0, 7);
  graph.addEdge(1, 7);
  graph.addEdge(6, 7);
  //graph.addEdge(5, 6);
  graph.addEdge(4, 5);
  graph.addEdge(3, 4);
  graph.addEdge(3, 5);
  //graph.addEdge(1, 3);
  graph.addEdge(1, 2);
  //graph.addEdge(2, 3);

  graph.exportDotFile(dataPath + "/test.dot");

  std::vector<std::vector<int> > cycleArray = graph.getCycle();
  cout << cycleArray.size() << " cycles" << endl;

  for (size_t i = 0; i < cycleArray.size(); ++i) {
    for (size_t j = 0;  j < cycleArray[i].size(); ++j) {
      cout << cycleArray[i][j] << " ";
    }
    cout << endl;
  }
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(0, 3);
  //graph.addEdge(0, 7);
  //graph.addEdge(1, 7);
  graph.addEdge(6, 7);
  graph.addEdge(5, 6);
  //graph.addEdge(4, 5);
  graph.addEdge(3, 4);
  //graph.addEdge(3, 5);
  graph.addEdge(1, 3);

  graph.exportDotFile(dataPath + "/test.dot");

  const vector<ZGraph*>& subgraphArray = graph.getConnectedSubgraph();
  for (size_t i = 0; i < subgraphArray.size(); ++i) {
    cout << "Subgraph " << i << ":" << endl;
    subgraphArray[i]->print();
    cout << endl;
  }

  std::vector<std::vector<int> > cycleArray = graph.getCycle();
  cout << cycleArray.size() << " cycles" << endl;

  for (size_t i = 0; i < cycleArray.size(); ++i) {
    for (size_t j = 0;  j < cycleArray[i].size(); ++j) {
      cout << cycleArray[i][j] << " ";
    }
    cout << endl;
  }
#endif

#if 0
  cout << ZString::removeFileExt("test/test.tif") << endl;
  cout << ZString::removeFileExt("testtest.tif") << endl;
  cout << ZString::removeFileExt("test/test./tif") << endl;
  cout << ZString::removeFileExt("test/.test/tif") << endl;
  cout << ZString::removeFileExt("testtest.tif/") << endl;
  cout << ZString::removeFileExt("test/test") << endl;
  cout << ZString::removeFileExt("testtest") << endl;
#endif

#if 0
  QDomDocument doc("mydocument");
  QFile file((NeutubeConfig::getInstance().getApplicatinDir() + "/config.xml").c_str());
  if (!file.open(QIODevice::ReadOnly))
    return;
  if (!doc.setContent(&file)) {
    file.close();
    return;
  }
  file.close();

  // print out the element names of all elements that are direct children
  // of the outermost element.
  QDomElement docElem = doc.documentElement();

  QDomNode n = docElem.firstChild();
  while(!n.isNull()) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if(!e.isNull()) {
      std::cout << qPrintable(e.tagName()) << std::endl; // the node really is an element.
    } else {
      std::cout << "Non-element node" << std::endl;
    }
    n = n.nextSibling();
  }
#endif

#if 0
  if (host != NULL) {
    ZStackFrame *frame = host->currentStackFrame();
    if (frame != NULL) {
      frame->view()->setSizeHintOption(NeuTube::SIZE_HINT_TAKING_SPACE);
      frame->resize(frame->sizeHint());
    }
  }
#endif

#if 0
  Stack *stack = C_Stack::make(GREY, 100, 100, 5);
  Zero_Stack(stack);

  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/swc/fork.swc").c_str());

  ZSwcPath path(tree.firstRegularRoot(),
                SwcTreeNode::firstChild(SwcTreeNode::firstChild(tree.firstRegularRoot())));

  path.labelStackAcrossZ(stack, 255);

  C_Stack::write(dataPath + "/test.tif", stack);
#endif


#if 0
  Stack *stack = C_Stack::readSc(dataPath + "/benchmark/cross_45_10.tif");
  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/cross_45_10_2.swc").c_str());
  tree.print();

  ZSwcPath path(tree.firstRegularRoot(), tree.firstLeaf());

  path.resetPositionFromStack(stack);

  tree.print();

#endif

#if 0
  Stack *stack = C_Stack::readSc(dataPath + "/benchmark/cross_45_10.tif");
  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/cross_45_10_2.swc").c_str());
  tree.print();

  ZSwcPositionAdjuster adjuster;
  ZSwcPath path(tree.firstRegularRoot(), tree.firstLeaf());
  adjuster.setSignal(stack, NeuTube::IMAGE_BACKGROUND_DARK);

  //path.resetPositionFromStack(stack);

  adjuster.adjustPosition(path);

  //path.resetPositionFromStack(stack);

  tree.print();
#endif

#if 0
  Stack *stack = C_Stack::readSc(dataPath + "/benchmark/mouse_neuron_single/stack.tif");
  ZSwcTree tree;
  tree.load(dataPath + "/stack.Edit.swc");
  ZSwcPositionAdjuster adjuster;
  adjuster.setSignal(stack, NeuTube::IMAGE_BACKGROUND_DARK);
  //ZSwcPath path(tree.firstRegularRoot(), tree.firstLeaf());
  adjuster.adjustPosition(tree);
  tree.save((dataPath + "/test.swc").c_str());
  tree.print();
#endif

#if 0
  ZStack stack;
  stack.load(dataPath + "/biocytin/MC0509C3-2_small_small.tif");
  ZSwcTree tree;
  tree.load(dataPath + "/MC0509C3-2_small_small.Proj.Edit.swc");
  ZSwcPositionAdjuster adjuster;
  adjuster.setSignal(stack.c_stack(0), NeuTube::IMAGE_BACKGROUND_BRIGHT);
  //ZSwcPath path(tree.firstRegularRoot(), tree.firstLeaf());
  adjuster.adjustPosition(tree);
  tree.save((dataPath + "/test.swc").c_str());
  tree.print();
#endif

#if 0
  ZObject3dScan obj;
  obj.load(dataPath + "/benchmark/432.sobj");
  std::cout << obj.getVoxelNumber() << std::endl;
  std::cout << obj.getSegmentNumber() << std::endl;
  std::cout << "Really canonized? " << obj.isCanonizedActually() << std::endl;

  obj.canonize();
  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
    std::cout << obj.getStripe(i).getZ() << " " << obj.getStripe(i).getY()
              << std::endl;
  }

  std::vector<ZObject3dScan> objArray = obj.getConnectedComponent();
  std::cout << objArray.size() << std::endl;

  /*
  int offset[3];
  Stack *stack = obj.toStack(offset);

  //iarray_neg(offset, 3);
  //obj.labelStack(stack, 2, offset);

  ZObject3dScan obj2;
  obj2.loadStack(stack);
  std::cout << obj2.getVoxelNumber() << std::endl;
  std::cout << obj2.getSegmentNumber() << std::endl;
  std::cout << "Really canonized? " << obj2.isCanonizedActually() << std::endl;

  obj2.translate(offset[0], offset[1], offset[2]);

  obj2.labelStack(stack, 2, offset);
  C_Stack::write(dataPath + "/test2.tif", stack);
  */
#endif

#if 0
  Stack *stack = C_Stack::readSc(dataPath + "/test.tif");
  //stack->depth = 5;
  Stack_Binarize(stack);
  C_Stack::translate(stack, GREY, 1);
  ZObject3dScan obj;
  obj.loadStack(stack);
  cout << obj.getVoxelNumber() << endl;

  obj.labelStack(stack, 2);
  C_Stack::write(dataPath + "/test2.tif", stack);
#endif

#if 0 //FIB connection
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(dataPath + "/flyem/FIB/v1/annotations-synapse.json");
  synapseArray.printSummary();

  ZFlyEmDataBundle dataBundle;
  dataBundle.loadJsonFile(
        dataPath + "/flyem/FIB/skeletonization/session9/data_bundle.json");
  const std::vector<ZFlyEmNeuron> &neuronArray = dataBundle.getNeuronArray();
  std::set<int> bodySet;
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    std::cout << iter->getId() << std::endl;
    bodySet.insert(iter->getId());
  }

  ZGraph *graph = synapseArray.toGraph(bodySet);

  graph->exportDotFile(dataPath + "/test.dot");

  delete graph;
#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(dataPath + "/flyem/FIB/skeletonization/session9/bodies/stacked/29.sobj");
  //obj1.print();
  obj1.downsampleMax(4, 4, 4);

  //obj1.print();

  ZObject3dScan obj2;
  obj2.load(dataPath + "/flyem/FIB/skeletonization/session9/bodies/stacked/9942.sobj");
  obj2.downsampleMax(4, 4, 4);

  ZCuboid box1 = obj1.getBoundBox();
  ZCuboid box2 = obj2.getBoundBox();
  box1.print();
  box1.bind(box2);
  box1.print();

  Stack *stack = C_Stack::make(
        COLOR, box1.width() + 1, box1.height() + 1, box1.depth() + 1);
  obj1.translate(-box1.firstCorner().x(), -box1.firstCorner().y(),
                 -box1.firstCorner().z());
  obj2.translate(-box1.firstCorner().x(), -box1.firstCorner().y(),
                 -box1.firstCorner().z());

  obj1.drawStack(stack, 255, 0, 0, NULL);
  obj2.drawStack(stack, 0, 255, 0, NULL);

  C_Stack::write(dataPath + "/test.tif", stack);

  C_Stack::kill(stack);
#endif

#if 0
  ZStackFile stackFile;
  stackFile.import(dataPath + "/biocytin/MC0509C3-2_small_small.tif");
  ZStack *stack = stackFile.readStack();
  Stack *mask = C_Stack::make(GREY, stack->width(), stack->height(), 1);
  Zero_Stack(mask);

  Cuboid_I cuboid;
  Cuboid_I_Set_S(&cuboid, 10, 10, 0, 100, 100, 1);
  Cuboid_I_Label_Stack(&cuboid, 255, mask);

  C_Stack::write(dataPath + "/biocytin/MC0509C3-2_small_small.ROI.tif", mask);

  delete stack;
  C_Stack::kill(mask);
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load(dataPath + "/flyem/skeletonization/session3/len15/adjusted2/L2e_198.swc");
  tree1.rescale(31, 31, 40);
  tree2.load(dataPath + "/flyem/FIB/skeletonization/session9/swc/adjusted/26558.swc");

  ZSwcRangeAnalyzer rangeAnalyzer;
  rangeAnalyzer.setZStep(500);
  rangeAnalyzer.setZMargin(500);
  std::vector<Swc_Tree_Node*> nodeArray =
      rangeAnalyzer.getOutsideNode(tree1, tree2);

  tree2.setType(0);
  SwcTreeNode::setType(nodeArray.begin(), nodeArray.end(), 2);
  tree2.save(dataPath + "/test.swc");
#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 2999;
  blockArray.intersect(boundBox);
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(0, 0, 10);
  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  QString dirPath(
        (dataPath + "/flyem/FIB/skeletonization/session9/bodies/stacked").c_str());
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filter, QDir::NoFilter);

  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(dataPath + "/flyem/FIB/skeletonization/session9/annotations-synapse.json");

  std::vector<int> count = synapseArray.countSynapse();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;

  tic();
  std::cout << "Start checking ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    int bodyId = fileInfo.baseName().toInt();
    if (count[bodyId] > 0) {
      ZObject3dScan obj;
      obj.load(fileInfo.absoluteFilePath().toStdString());
      if (analyzer.isStitchedOrphanBody(obj)) {
        std::cout << fileInfo.baseName().toStdString() << std::endl;
      }
    }
  }
  ptoc();

#endif

#if 0

#endif


#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle6.json");

  const std::vector<ZFlyEmNeuron> &neuronArray =  bundle.getNeuronArray();
  for (size_t i = 0; i < neuronArray.size(); ++i) {
    ZSwcTree *tree = neuronArray[i].getModel();
    double ratio = ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*tree);
    std::cout << neuronArray[i].getName() << " "
               << neuronArray[i].getClass() << " " << ratio << std::endl;
  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(dataPath + "/flyem/FIB/skeletonization/session9/annotations-synapse.json");

  std::vector<int> count = synapseArray.countSynapse();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session13/annotations-synapse.json");

  std::vector<int> count = synapseArray.countPsd();


  FILE *fp = fopen((dataPath +
                   "/flyem/FIB/skeletonization/session13/bodies/bodysize.txt").c_str(),
                   "r");

  std::ofstream stream((dataPath +
                       "/flyem/FIB/skeletonization/session13/psd_count.txt").c_str());

  ZString line;
  while (line.readLine(fp)) {
    vector<int> bodySize = line.toIntegerArray();
    if (bodySize.size() == 2) {
      if (bodySize[0] < (int) count.size()) {
        stream << bodySize[0] << ' ' << bodySize[1] << ' ' << count[bodySize[0]]
            << std::endl;
      }
    }
  }

  fclose(fp);
  stream.close();
#endif

#if 0
  //Show data bundle summary
  std::map<std::string, int> classCountMap;

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath +
                      "/flyem/FIB/skeletonization/session13/len25/adjusted2/data_bundle_with_class.json");

  const std::vector<ZFlyEmNeuron> &neuronArray =  bundle.getNeuronArray();
  for (size_t i = 0; i < neuronArray.size(); ++i) {
    if (classCountMap.count(neuronArray[i].getClass()) == 0) {
      classCountMap[neuronArray[i].getClass()] = 1;
    } else {
      classCountMap[neuronArray[i].getClass()]++;
    }
  }

  std::cout << classCountMap.size() << " classes" << std::endl;
  for (std::map<std::string, int>::const_iterator iter = classCountMap.begin();
       iter != classCountMap.end(); ++iter) {
    std::cout << iter->first << " " << iter->second << std::endl;
  }
#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);

  std::cout << blockArray.getVolume() << std::endl;

  boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
#endif

#if 0
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session13";
  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(sessionDir + "/annotations-synapse.json");

  std::vector<int> count = synapseArray.countPsd();
#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  //blockArray.print();

  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);

  std::cout << blockArray.getVolume() << std::endl;

  boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);
  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_6layer.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session13";
  std::string bodyDir = sessionDir + "/bodies/stacked2";
  QString dirPath(bodyDir.c_str());
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filter, QDir::NoFilter);

  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(sessionDir + "/annotations-synapse.json");

  std::vector<int> count = synapseArray.countPsd();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;

  ofstream out((bodyDir + "/psd_orphan.txt").c_str());

  tic();
  std::cout << "Start checking ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    int bodyId = fileInfo.baseName().toInt();
    if (count[bodyId] > 0) {
      ZObject3dScan obj;
      obj.load(fileInfo.absoluteFilePath().toStdString());
      if (analyzer.isStitchedOrphanBody(obj)) {
        std::cout << fileInfo.baseName().toStdString() << std::endl;
        out << fileInfo.baseName().toStdString() << std::endl;
      }
    }
  }
  ptoc();

  out.close();
#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  //blockArray.print();

  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);

  std::cout << blockArray.getVolume() << std::endl;

  boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);
  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_6layer.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session13";
  std::string bodyDir = sessionDir + "/bodies/stacked2";
  QString dirPath(bodyDir.c_str());
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filter, QDir::NoFilter);

  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(sessionDir + "/annotations-synapse.json");

  std::vector<int> count = synapseArray.countTBar();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;

  ofstream out((bodyDir + "/tbar_orphan.txt").c_str());

  tic();
  std::cout << "Start checking ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    int bodyId = fileInfo.baseName().toInt();
    if (count[bodyId] > 0) {
      ZObject3dScan obj;
      obj.load(fileInfo.absoluteFilePath().toStdString());
      if (analyzer.isStitchedOrphanBody(obj)) {
        std::cout << fileInfo.baseName().toStdString() << std::endl;
        out << fileInfo.baseName().toStdString() << std::endl;
      }
    }
  }
  ptoc();

  out.close();
#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);

  FlyEm::ZIntCuboidArray bottomFace = blockArray;
  boundBox.cb[2] = 4499;
  bottomFace.intersect(boundBox);

  std::cout << blockArray.getVolume() << std::endl;

  boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);
  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_6layer.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session13";
  std::string bodyDir = sessionDir + "/bodies/stacked";
  QString dirPath(bodyDir.c_str());
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filter, QDir::NoFilter);

  /*
  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(sessionDir + "/annotations-synapse.json");

  std::vector<int> count = synapseArray.countPsd();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;
  */

  ofstream out((bodyDir + "/cutoff.txt").c_str());

  tic();
  std::cout << "Start checking ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    //int bodyId = fileInfo.baseName().toInt();
    //if (count[bodyId] > 0) {
      ZObject3dScan obj;
      obj.load(fileInfo.absoluteFilePath().toStdString());
      if (analyzer.isExitingOrphanBody(obj)) {
        std::cout << fileInfo.baseName().toStdString() << std::endl;
        out << fileInfo.baseName().toStdString() << std::endl;
      }
    //}
  }
  ptoc();

  out.close();
#endif

#if 0
  //Calculate L1, L2 volume

  std::string outDir = dataPath + "/flyem/FIB/result1";

  const char *temL1[] = {
    "L1_209", "L1g6_439207", "L1n1_422955", "L1a_285743", "L1h7_984179",
    "L1o2_5425323", "L1b_211940", "L1i8_17802841", "L1p3_636965",
    "L1c_189938", "L1j9_984116", "L1q4_423153", "L1d_181639",
    "L1k10_5053998", "L1r5_423176", "L1e_196", "L1l11_433023",
    "L1f_285714", "L1m12_5134017"
  };

  std::ofstream outStream;

  outStream.open((outDir + "/TEM_L1_size.txt").c_str());

  for (size_t i = 0; i < sizeof(temL1) / sizeof(char*); ++i) {
    Stack *stack = C_Stack::readSc(dataPath + "/flyem/skeletonization/session3/" +
                                   temL1[i] + ".tif");
    Stack_Binarize(stack);
    double volume = Stack_Sum(stack) * 31 * 31 * 40;
    volume /= 0.7;
    outStream << temL1[i] << ", " << volume << std::endl;
    C_Stack::kill(stack);
  }

  outStream.close();

  const char *temL2[] = {
    "L2_212", "L2b_216003", "L2d_133765", "L2f_5858", "L2a_148554",
    "L2c_177365", "L2e_198"
  };

  outStream.open((outDir + "/TEM_L2_size.txt").c_str());

  for (size_t i = 0; i < sizeof(temL2) / sizeof(char*); ++i) {
    Stack *stack = C_Stack::readSc(dataPath + "/flyem/skeletonization/session3/" +
                                   temL2[i] + ".tif");
    Stack_Binarize(stack);
    double volume = Stack_Sum(stack) * 31 * 31 * 40;
    volume /= 0.7;
    outStream << temL2[i] << ", " << volume << std::endl;
    C_Stack::kill(stack);
  }

  outStream.close();


  const char *fibL1[] = {
    "206669", "195030", "203285", "216388", "231769", "222058", "198198"
  };

  outStream.open((outDir + "/FIB_L1_size.txt").c_str());
  for (size_t i = 0; i < sizeof(fibL1) / sizeof(char*); ++i) {
    ZObject3dScan obj;
    obj.load(dataPath +
             "/flyem/FIB/skeletonization/session13/bodies/stacked/" +
             fibL1[i] + ".sobj");
    double volume = obj.getVoxelNumber() * 1000;
    outStream << fibL1[i] << ", " << volume << std::endl;
  }
  outStream.close();

  const char *fibL2[] = {
    "11545", "56", "7762", "26363", "31349", "26062", "4241"
  };

  outStream.open((outDir + "/FIB_L2_size.txt").c_str());
  for (size_t i = 0; i < sizeof(fibL1) / sizeof(char*); ++i) {
    ZObject3dScan obj;
    obj.load(dataPath +
             "/flyem/FIB/skeletonization/session13/bodies/stacked/" +
             fibL2[i] + ".sobj");
    double volume = obj.getVoxelNumber() * 1000;
    outStream << fibL2[i] << ", " << volume << std::endl;
  }
  outStream.close();


  //std::cout << temVolume << ' ' << fibVolume << ' ' << temVolume / fibVolume << std::endl;

#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session14/annotations-synapse.json");

  std::cout << "Psd number: " << synapseArray.getPsdNumber() << std::endl;
  std::cout << "TBar number: " << synapseArray.getTBarNumber() << std::endl;

#endif

#if 0
  ZObject3dScan obj;
  obj.load(dataPath +
           "/flyem/FIB/skeletonization/session13/bodies/stacked/206669.sobj");
  obj.downsampleMax(2, 2, 5);


  Stack *stack = C_Stack::readSc(dataPath + "/flyem/skeletonization/session3/L1_209.tif");
  //Stack_Binarize(stack);

  Stack *stack2 = C_Stack::crop(stack, 0, 0, 0, 800, 800, 495, NULL);
  int offset[3] = {0, 0, 30};
  obj.drawStack(stack2, 100, offset);

  C_Stack::write(dataPath + "/test.tif", stack2);

  C_Stack::kill(stack);
  C_Stack::kill(stack2);

#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/TEM/annotations-synapse.json");

  std::vector<int> psdCount = synapseArray.countPsd();
  std::vector<int> tbarCount = synapseArray.countTBar();

  std::ofstream stream1;
  stream1.open((dataPath + "/flyem/FIB/result1/TEM_L1_Psd.txt").c_str());

  std::ofstream stream2;
  stream2.open((dataPath + "/flyem/FIB/result1/TEM_L1_TBar.txt").c_str());

  std::ofstream stream3;
  stream3.open((dataPath + "/flyem/FIB/result1/TEM_L2_Psd.txt").c_str());

  std::ofstream stream4;
  stream4.open((dataPath + "/flyem/FIB/result1/TEM_L2_TBar.txt").c_str());

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle6.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "L1") {
      stream1 << iter->getId() << " " << psdCount[iter->getId()] //synapseArray.countPsd(iter->getId())
                << std::endl;
      stream2 << iter->getId() << " " << tbarCount[iter->getId()] //synapseArray.countPsd(iter->getId())
                << std::endl;
    }

    if (iter->getClass() == "L2") {
      stream3 << iter->getId() << " " << psdCount[iter->getId()] //synapseArray.countPsd(iter->getId())
                << std::endl;
      stream4 << iter->getId() << " " << tbarCount[iter->getId()] //synapseArray.countPsd(iter->getId())
                << std::endl;
    }
  }

  stream1.close();
  stream2.close();

#endif

#if 0
  std::map<int, int>::value_type rawMap[] =
  {
    std::map<int, int>::value_type(1, 2),
    std::map<int, int>::value_type(3, 4),
    std::map<int, int>::value_type(5, 6)
  };

  std::map<int, int> testMap(rawMap, rawMap + 2);
  std::cout << testMap.size() << std::endl;
#endif

#if 0
  ZProgressReporter reporter;
  reporter.start();
  reporter.advance(0.1);

  reporter.startSubprogress(0.5);
  reporter.start();
  for (int i = 0; i < 10; ++i) {
    reporter.advance(0.1);
  }
  reporter.end();
  reporter.endSubprogress(0.5);

  reporter.advance(0.1);

  reporter.end();


  reporter.start();
  reporter.advance(0.1);

  reporter.start(0.5);
  reporter.start();
  for (int i = 0; i < 10; ++i) {
    reporter.advance(0.1);
  }
  reporter.end();
  reporter.end(0.5);

  reporter.advance(0.1);

  reporter.end();
#endif

#if 0
  NeuTube::getMessageReporter()->report(
        "test", "error 1", ZMessageReporter::ERROR);
  NeuTube::getMessageReporter()->report(
        "test", "warning 1", ZMessageReporter::WARNING);
  NeuTube::getMessageReporter()->report(
        "test", "output 1", ZMessageReporter::INFORMATION);
#endif

#if 0
  ZHistogram hist;
  hist.setInterval(2.0);
  hist.increment(0.5);
  hist.increment(1.5);
  hist.increment(1.0);
  hist.increment(2.0);
  hist.increment(3.0);
  hist.increment(-1.0);
  hist.increment(-0.99);
  hist.increment(-2.0);
  hist.print();

  std::cout << hist.getDensity(0.1) << std::endl;

#endif

#if 0

  ZObject3dScan obj;
  obj.addStripe(0, 0);
  obj.addSegment(1, 2);
  obj.addSegment(4, 5);
  ZHistogram hist = obj.getRadialHistogram(0);

  hist.print();
#endif

#if 0
  ZHistogram hist;
  //hist.increment(0.0);
  hist.increment(1.0);
  hist.increment(2.0);
  hist.increment(2.0);

  hist.print();

  Stack *stack = C_Stack::make(GREY, 5, 5, 1);
  Zero_Stack(stack);

  misc::paintRadialHistogram(hist, 2, 2, 0, stack);

  Print_Stack_Value(stack);
#endif

#if 0
  ZObject3dScan obj;
  obj.load("/groups/flyem/data/zhaot/bundle1/volume/215.sobj");

  obj.downsampleMax(3, 3, 0);

 // hist.print();
  int startZ = obj.getMinZ();
  int endZ = obj.getMaxZ();

  Stack *stack = C_Stack::make(GREY, 1000, 1000, endZ - startZ + 1);
  Zero_Stack(stack);

  for (int z = startZ; z <= endZ; ++z) {
    ZHistogram hist = obj.getRadialHistogram(z);
    misc::paintRadialHistogram(hist, 500, 500, z - startZ, stack);
  }

  stack = C_Stack::boundCrop(stack);

  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0
  //Load tif list

  //For each tif
    //Extract sparse object
  {
    ZObject3dScan obj;
    obj.scanArray(C_Stack::array8(input));

    int startZ = obj.getMinZ();
    int endZ = obj.getMaxZ();

    Stack *stack = C_Stack::make(GREY, 1000, 1000, endZ - startZ + 1);
    Zero_Stack(stack);

    for (int z = startZ; z <= endZ; ++z) {
      ZHistogram hist = obj.getRadialHistogram(z);
      misc::paintRadialHistogram(hist, 500, 500, z - startZ, stack);
    }

    Stack *cropped = C_Stack::boundCrop(stack);
    C_Stack::kill(stack);

    C_Stack::write(dataPath + "/test.tif", cropped);
    C_Stack::kill(cropped);
  }

#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    ZObject3dScan obj;
    obj.load(neuronIter->getVolumePath());
    obj.downsampleMax(3, 3, 0);

    std::vector<ZHistogram> histArray;
    int startZ = obj.getMinZ();
    int endZ = obj.getMaxZ();
    for (int z = startZ; z <= endZ; ++z) {
      ZHistogram hist = obj.getRadialHistogram(z);
      histArray.push_back(hist);
    }

    Stack *stack = C_Stack::make(GREY, 1500, 1500, 1);
    Zero_Stack(stack);
    int cx = 750;
    misc::paintRadialHistogram2D(histArray, cx, startZ, stack);

    ZString baseName = ZString::getBaseName(neuronIter->getModelPath());
    baseName = baseName.changeExt("tif");

    std::ostringstream stream;

    stream << dataPath << "/test/hist_"
           << ZString::getBaseName(neuronIter->getModelPath()) << ".tif";
    C_Stack::write(stream.str(), stack);

    C_Stack::kill(stack);
  }

#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::vector<double> voxelCount;

  std::string volumeDir = dataPath + "/flyem/skeletonization/session3";
  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    std::cout << neuronIter->getName() << std::endl;
    //if (ZString(neuronIter->getClass()).startsWith("L")) {

      if (ZString(neuronIter->getClass()).startsWith("Tm") ||
          ZString(neuronIter->getClass()).startsWith("Mi") ||
          ZString(neuronIter->getClass()).startsWith("C")) {

    //if (neuronIter->getClass() == "Tangential") {
      std::string volumePath = neuronIter->getVolumePath();
      if (!volumePath.empty()) {
        ZObject3dScan obj;
        obj.load(volumePath);
        std::vector<size_t> subVoxelCount = obj.getSlicewiseVoxelNumber();
        for (size_t z = 0; z < subVoxelCount.size(); ++z) {
          if (z >= voxelCount.size()) {
            voxelCount.resize(z + 1, 0.0);
          }
          voxelCount[z] += subVoxelCount[z];
        }
        /*
        int minZ = obj.getMinZ();
        int maxZ = obj.getMaxZ();
        for (int z = minZ; z <= maxZ; ++z) {
          if (z >= (int) voxelCount.size()) {
            voxelCount.resize(z + 1, 0.0);
          }
          voxelCount[z] += obj.getVoxelNumber(z);
        }
        */
      }
    }
  }

  std::ofstream stream;
  std::cout << volumeDir + "/tm_voxel_distr.txt" << std::endl;
  stream.open((volumeDir + "/tm_voxel_distr.txt").c_str());
  for (size_t i = 0; i < voxelCount.size(); ++i) {
    stream << i << " " << voxelCount[i] << std::endl;
  }
  stream.close();
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/FIB/skeletonization/session18/len10/adjusted/data_bundle_with_class.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::vector<double> voxelCount;

  std::string volumeDir = dataPath + "/flyem/FIB/skeletonization/session18/bodies/stacked/";
  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    std::cout << neuronIter->getName() << std::endl;
/*
    if (!neuronIter->getClass().empty() &&
        neuronIter->getClass() != "Tangential" &&
        neuronIter->getClass() != "partial") {
        */
    //if (ZString(neuronIter->getClass()).startsWith("L")) {

    if (ZString(neuronIter->getClass()).startsWith("Tm") ||
        ZString(neuronIter->getClass()).startsWith("Mi") ||
        ZString(neuronIter->getClass()).startsWith("C")) {

      //std::string volumePath = neuronIter->getVolumePath();
      ZString baseName = ZString::removeFileExt(
            ZString::getBaseName(neuronIter->getModelPath()));
      std::string volumePath = ZString::fullPath(volumeDir, baseName, "sobj");

      if (!volumePath.empty()) {
        ZObject3dScan obj;
        obj.load(volumePath);
        //int minZ = obj.getMinZ();
        //int maxZ = obj.getMaxZ();
        std::vector<size_t> subVoxelCount = obj.getSlicewiseVoxelNumber();
        for (size_t z = 0; z < subVoxelCount.size(); ++z) {
          if (z >= voxelCount.size()) {
            voxelCount.resize(z + 1, 0.0);
          }
          voxelCount[z] += subVoxelCount[z];
        }
      }
    }
  }

  std::ofstream stream;
  stream.open((volumeDir + "/voxel_distr2.txt").c_str());
  for (size_t i = 0; i < voxelCount.size(); ++i) {
    stream << i << " " << voxelCount[i] << std::endl;
  }
  stream.close();
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::string volumeDir = dataPath + "/flyem/skeletonization/session3";
  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    ZString baseName = ZString::removeFileExt(
          ZString::getBaseName(neuronIter->getModelPath()));
    std::string volumePath = ZString::fullPath(volumeDir, baseName, "tif");
    if (fexist(volumePath.c_str())) {
      Stack *stack = C_Stack::readSc(volumePath);
      ZObject3dScan obj;
      obj.loadStack(stack);
      C_Stack::kill(stack);

      std::string offsetPath =
          ZString::fullPath(volumeDir, baseName, "tif.offset.txt");
      ZString line;
      FILE *fp = fopen(offsetPath.c_str(), "r");
      line.readLine(fp);
      int zOffset = line.lastInteger();
      fclose(fp);

      obj.addZ(zOffset);

      std::vector<ZHistogram> histArray;
      int startZ = obj.getMinZ();
      int endZ = obj.getMaxZ();
      for (int z = startZ; z <= endZ; ++z) {
        ZHistogram hist = obj.getRadialHistogram(z);
        histArray.push_back(hist);
      }

      stack = C_Stack::make(GREY, 1500, 1500, 1);
      Zero_Stack(stack);
      int cx = 750;
      misc::paintRadialHistogram2D(histArray, cx, startZ, stack);

      ZString baseName = ZString::getBaseName(neuronIter->getModelPath());
      baseName = baseName.changeExt("tif");

      std::ostringstream stream;

      stream << dataPath << "/flyem/TEM/data_release/densitymap/hist_"
             << ZString::getBaseName(neuronIter->getModelPath()) << ".tif";
      C_Stack::write(stream.str(), stack);

      C_Stack::kill(stack);
    }
  }
#endif

#if 0 //Density map matching
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::string densityMapDir = dataPath + "/test";
  std::vector<Stack*> imageArray;
  for (std::vector<ZFlyEmNeuron>::const_iterator
       firstNeuronIter = neuronArray.begin();
       firstNeuronIter != neuronArray.end(); ++firstNeuronIter) {
    ZString baseName = ZString::removeFileExt(
          ZString::getBaseName(firstNeuronIter->getModelPath()));
    std::string imagePath1 = ZString::fullPath(densityMapDir, "hist_" + baseName, "swc.tif");
    Stack *stack1 = C_Stack::readSc(imagePath1);
    imageArray.push_back(stack1);
  }

  ZMatrix mat(neuronArray.size(), neuronArray.size());

  int count = 0;
  int i = 0;
  for (std::vector<ZFlyEmNeuron>::const_iterator
       firstNeuronIter = neuronArray.begin();
       firstNeuronIter != neuronArray.end(); ++firstNeuronIter, ++i) {
    Stack *stack1 = imageArray[i];
    Cuboid_I box1;
    Stack_Bound_Box(stack1, &box1);

    int j = 0;
    for (std::vector<ZFlyEmNeuron>::const_iterator
         secondNeuronIter = neuronArray.begin();
         secondNeuronIter != neuronArray.end(); ++secondNeuronIter, ++j) {
      if (i < j) {
        Stack *stack2 = imageArray[j];
        Cuboid_I box2;
        Stack_Bound_Box(stack2, &box2);

        Cuboid_I box;
        Cuboid_I_Union(&box1, &box2, &box);

        Stack *cropped1 = C_Stack::crop(stack1, box, NULL);
        Stack *cropped2 = C_Stack::crop(stack2, box, NULL);

        double corrcoef = Stack_Corrcoef(cropped1, cropped2);

        //std::cout << i << " " << j << " " << corrcoef << std::endl;

        mat.set(i, j, corrcoef);

        C_Stack::kill(cropped1);
        C_Stack::kill(cropped2);
      } else if (i > j) {
        mat.set(i, j, mat.getValue(j, i));
      }
    }

    int neuronIndex = 0;
    mat.getRowMax(i, &neuronIndex);
    if (firstNeuronIter->getClass() == neuronArray[neuronIndex].getClass()) {
      ++count;
    }

    std::cout << "Prediction: " << count << "/" << i + 1 << " - "
              << firstNeuronIter->getClass() << "(p: "
              << neuronArray[neuronIndex].getClass() << ")" << std::endl;
  }


  for (size_t i = 0; i < imageArray.size(); ++i) {
    C_Stack::kill(imageArray[i]);
  }

  mat.exportCsv(dataPath + "/test.csv");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;
  synapseAnnotation.loadJson(dataPath + "/flyem/TEM/data_release/bundle1/annotations-synapse.json");
  synapseAnnotation.exportTBar(dataPath + "/tbar.csv");
  synapseAnnotation.exportPsd(dataPath + "/psd.csv");
  synapseAnnotation.exportCsvFile(dataPath + "/synapse.csv");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;

  //synapseAnnotation.loadJson(dataPath + "/flyem/FIB/skeletonization/session13/annotations-synapse.json");
  synapseAnnotation.loadJson(dataPath + "/flyem/TEM/data_release/bundle1/annotations-synapse.json");
  ZGraph *graph = synapseAnnotation.getConnectionGraph(false);

  ZGraphCompressor compressor;
  compressor.setGraph(graph);
  compressor.compress();

  const std::vector<int> &uncompressMap = compressor.getUncompressMap();

  ofstream stream((dataPath + "/id.txt").c_str());

  for (size_t i = 0; i < uncompressMap.size(); ++i) {
    stream << uncompressMap[i] << std::endl;
  }

  stream.close();

  ZMatrix mat(graph->getVertexNumber(), graph->getVertexNumber());

  for (int i = 0; i < graph->getEdgeNumber(); ++i) {
    mat.set(graph->getEdgeBegin(i), graph->getEdgeEnd(i), graph->getEdgeWeight(i));
  }

  mat.exportCsv(dataPath + "/conn.csv");
#endif

#if 0
  Stack *stack2 = C_Stack::readSc(dataPath + "/flyem/FIB/movie/frame/00002.tif");
  Stack *stack1 = C_Stack::readSc(dataPath + "/flyem/FIB/movie/12layer1024/00500.tif");

  Stack *blend = C_Stack::make(COLOR, C_Stack::width(stack1), C_Stack::height(stack1),
                               10);


  color_t *array1 = (color_t*) C_Stack::array8(stack1);
  color_t *array2 = (color_t*) C_Stack::array8(stack2);


  size_t voxelNumber = C_Stack::voxelNumber(stack1);

  color_t *blendArray = (color_t*) C_Stack::array8(blend);

  double alpha = 0.0;
  for (int k = 0; k < 10; ++k) {
    for (size_t i =0; i < voxelNumber; ++i) {
      for (int c = 0; c < 3; ++c) {
        double v1 = array1[i][c];
        double v2 = array2[i][c];
        double v = v1 * (1.0 - alpha) + v2 * alpha;
        blendArray[i][c] = Clip_Value(v, 0, 255);
      }
    }
    blendArray += C_Stack::area(stack1);
    alpha += 0.1;
  }
  C_Stack::write(dataPath + "/test.tif", blend);
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  ZSwcTree *tree = neuronArray[0].getModel();

  ZFlyEmNeuronRange range;
  range.setPlaneRange(0, 100);
  range.setPlaneRange(10000, 1000);

  ZFlyEmQualityAnalyzer::labelSwcNodeOutOfRange(neuronArray[0], range, 5);

  tree->setTypeByLabel();

  tree->save(dataPath + "/test.swc");

  /*
  ZFlyEmNeuronAxis axis = neuronArray[0].getAxis();
  ZSwcTree *tree = axis.toSwc(10.0);
  tree->resortId();
  tree->save(dataPath + "/test.swc");
  tree->print();
  delete tree;

  neuronArray[0].getModel()->save(dataPath + "/test2.swc");
  */
#endif

#if 0
  ZSwcTree *tree = ZSwcGenerator::createCircleSwc(0, 0, 0, 10.0);
  tree->resortId();
  tree->save(dataPath + "/test.swc");
#endif

#if 0
  ZFlyEmNeuronRange range;
  range.setPlaneRange(0, 10);
  range.setPlaneRange(30000, 10);

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  ZFlyEmNeuronAxis axis = neuronArray[0].getAxis();

  ZSwcTree *tree = ZSwcGenerator::createSwc(range, axis);

  tree->save(dataPath + "/test.swc");

  delete tree;

  tree = axis.toSwc(10.0);
  tree->save(dataPath + "/test2.swc");
  delete tree;

  /*
  tree = neuronArray[0].getModel();

  ZFlyEmQualityAnalyzer::labelSwcNodeOutOfRange(neuronArray[0], range, 5);

  tree->setTypeByLabel();

  tree->save(dataPath + "/test2.swc");
  */
#endif

#if 0 //Create range for type L1
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  ZFlyEmNeuronRange overallRange;

  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "L1") {
      std::cout << "Neuron:" << iter->getId() << std::endl;
      ZFlyEmNeuronRange range = iter->getRange();
      overallRange.unify(range);
    }
  }

  ZFlyEmNeuronAxis axis = neuronArray[0].getAxis();

  ZSwcTree *tree = ZSwcGenerator::createSwc(overallRange, axis);

  tree->save(dataPath + "/test.swc");

#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  //const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();
  ZFlyEmNeuronAxis axis = bundle.getNeuron(209)->getAxis();
  /*
  ZFlyEmNeuronRange range = bundle.getNeuron(209)->getRange(
        bundle.getImageResolution(NeuTube::X_AXIS),
        bundle.getImageResolution(NeuTube::Z_AXIS));
*/
  std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::set<std::string> classSet;
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    classSet.insert(iter->getClass());
  }

  std::map<std::string, ZFlyEmNeuronRange> overallRange;

  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZString volumePath = dataPath + "/flyem/skeletonization/session3/";
    volumePath.appendNumber(iter->getId());
    volumePath += ".sobj";
    iter->setVolumePath(volumePath);

    std::cout << "Neuron:" << iter->getId() << std::endl;
    /*
    ZFlyEmNeuronRange range = iter->getRange(bundle.getImageResolution(NeuTube::X_AXIS),
                                             bundle.getImageResolution(NeuTube::Z_AXIS));
                                             */
    ZFlyEmNeuronRange range = iter->getRange(bundle.getSwcResolution(NeuTube::X_AXIS),
                                             bundle.getSwcResolution(NeuTube::Z_AXIS));
    if (overallRange.count(iter->getClass()) == 0) {
      overallRange[iter->getClass()] = range;
    }  else {
      overallRange[iter->getClass()].unify(range);
    }
  }

  std::string outDir = dataPath + "/flyem/skeletonization/session3/range";

  for (std::map<std::string, ZFlyEmNeuronRange>::const_iterator
       iter = overallRange.begin(); iter != overallRange.end(); ++iter) {
    iter->second.exportCsvFile(outDir + "/range_" + iter->first + ".csv");

    ZSwcTree *tree = ZSwcGenerator::createSwc(iter->second);
  //ZSwcTree *tree = axis.toSwc(10.0);
    if (tree != NULL) {
      tree->save(outDir + "/range_" + iter->first + ".swc");
    }
    delete tree;
  }

#endif

#if 0
  std::string sessionDir = dataPath + "/flyem/skeletonization/session3";

  QFileInfoList fileList;
  QDir dir(sessionDir.c_str());
  QStringList filters;
  filters << "*.tif";
  fileList = dir.entryInfoList(filters);

  foreach (QFileInfo fileInfo, fileList) {
    ZString objPath = sessionDir + "/";
    objPath.appendNumber(ZString::lastInteger(fileInfo.baseName().toStdString()));
    objPath += ".sobj";

    std::cout << fileInfo.absoluteFilePath().toStdString() << std::endl;

    if (!fexist(objPath.c_str())) {
      Stack *stack = C_Stack::readSc(fileInfo.absoluteFilePath().toStdString());
      ZObject3dScan obj;
      obj.loadStack(stack);
      C_Stack::kill(stack);
      ZDoubleVector offset;
      offset.importTextFile(
            fileInfo.absoluteFilePath().toStdString() + ".offset.txt");
      if (offset.size() == 3) {
        obj.translate(offset[0], offset[1], offset[2]);
        std::cout << "Saving " << objPath << std::endl;
        obj.save(objPath);
      } else {
        std::cout << "Invalid offset. Skip." << std::endl;
      }
    }
  }
#endif


#if 0 //Regnerate all sparse volumes from downsampled stacks
  QDir dir((dataPath + "/flyem/skeletonization/session3").c_str());
  QStringList filters;
  filters << "*.tif";
  QFileInfoList list = dir.entryInfoList(
        filters, QDir::Files | QDir::NoSymLinks);

  foreach (QFileInfo fileInfo, list) {
    ZObject3dScan obj;
    obj.load(fileInfo.absoluteFilePath().toStdString());

  }

#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session18";
  QDir dir((sessionDir + "/bodies/orphan_check").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());
    if (obj.getVoxelNumber() < 100000) {
      if (obj.isEmpty()) {
        std::cout << "Empty object: "
                  << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      }

      if (qc.isStitchedOrphanBody(obj)) {
        std::cout << "Orphan: "
                  << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                  << std::endl;
        ZVoxel voxel = obj.getMarker();
        std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        TZ_ASSERT(voxel.x() >= 0, "invalid point");

        json_array_append(arrayObj, json_integer(voxel.x()));
        json_array_append(arrayObj, json_integer(2598 - voxel.y()));
        json_array_append(arrayObj, json_integer(voxel.z() + 1490));
        json_array_append(dataObj, arrayObj);
      }
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, (dataPath + "/test.json").c_str(), JSON_INDENT(2));
#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");
#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session19";
  QDir dir((sessionDir + "/bodies/orphan_check").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());
    if (obj.getVoxelNumber() < 100000) {
      if (obj.isEmpty()) {
        std::cout << "Empty object: "
                  << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      }

      if (qc.isStitchedOrphanBody(obj)) {
        std::cout << "Orphan: "
                  << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                  << std::endl;
        ZVoxel voxel = obj.getMarker();
        std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        TZ_ASSERT(voxel.x() >= 0, "invalid point");

        json_array_append(arrayObj, json_integer(voxel.x()));
        json_array_append(arrayObj, json_integer(2598 - voxel.y()));
        json_array_append(arrayObj, json_integer(voxel.z() + 1490));
        json_array_append(dataObj, arrayObj);
      }
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, (dataPath + "/orphan.json").c_str(), JSON_INDENT(2));
#endif

#if 0
  std::string sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session18";

#if 0
  ZObject3dScan obj;
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session18";
  obj.load(sessionDir + "/bodies/orphan_check/464300.sobj");
  ZVoxel voxel = obj.getMarker();
  ZCuboid box = obj.getBoundBox();
  voxel.translate(
        -box.firstCorner().x(), -box.firstCorner().y(), -box.firstCorner().z());

  std::ofstream stream((dataPath + "/test.swc").c_str());

  Swc_Tree_Node *tn =
      SwcTreeNode::makePointer(voxel.x(), voxel.y(), voxel.z(), 3.0);
  SwcTreeNode::dump(tn, stream);

  stream.close();
#endif

#endif

#if 0
  std::string sessionDir = dataPath+ "/flyem/FIB/skeletonization/session19";
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;
  synapseAnnotation.loadJson(sessionDir + "/annotations-synapse.json");
  std::vector<int> synapseCount = synapseAnnotation.countSynapse();
  std::cout << "Max bodyid: " << synapseCount.size() << std::endl;

  int count = 0;
  for (size_t i = 0; i < synapseCount.size(); ++i) {
    if (synapseCount[i] > 0) {
      ++count;
    }
  }

  std::cout << count << " bodies have synapses" << std::endl;

  //Record body < 100000 voxels
  FILE *fp = fopen((sessionDir + "/bodies/bodysize.txt").c_str(), "r");
  ZString str;
  while (str.readLine(fp)) {
    std::vector<int> body = str.toIntegerArray();
    if (body.size() == 2) {
      if (body[1] >= 100000) {
        if (body[0] < (int) synapseCount.size()) {
          synapseCount[body[0]] = 0;
        }
      }
    } else if (!body.empty()) {
      std::cout << "Invalid value: " << str << std::endl;
    }
  }
  fclose(fp);


  QFileInfo fileInfo;
  //For each body with synapses
  for (size_t i = 1; i < synapseCount.size(); ++i) {
    if (synapseCount[i] > 0) {
      std::cout << "Checking body " << i << std::endl;

      std::string bodyFilePath = QString("%1/bodies/stacked/%2.sobj").
          arg(sessionDir.c_str()).arg(i).toStdString();

      QString outPath = QString("%1/bodies/orphan_check/%2.sobj").
          arg(sessionDir.c_str()).arg(i);
      if (fexist(bodyFilePath.c_str())) {
        QFile::copy(bodyFilePath.c_str(), outPath);
        std::cout << outPath.toStdString() << " copied" << std::endl;
      } else {
        ZObject3dScan obj;
        //Load the body
        for (int z = 1490; z <= 7509; ++z) {
          fileInfo.setFile(
                QDir(QString("%1/bodies/0%2").
                     arg(sessionDir.c_str()).arg(z)), QString("%1.sobj").arg(i));
          if (fexist(fileInfo.absoluteFilePath().toStdString().c_str())) {
            ZObject3dScan objSlice;
            objSlice.load(fileInfo.absoluteFilePath().toStdString());
            obj.concat(objSlice);
          }
        }

        if (!obj.isEmpty()) {
          obj.save(outPath.toStdString());
        }
      }
    }
    //If it's an orphan body, save the marker
  }
#endif

#if 0
  std::string sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session18";
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;
  synapseAnnotation.loadJson(sessionDir + "/annotations-synapse.json");
  std::vector<int> synapseCount = synapseAnnotation.countSynapse();
  std::cout << "Max bodyid: " << synapseCount.size() << std::endl;

  int count = 0;
  for (size_t i = 0; i < synapseCount.size(); ++i) {
    if (synapseCount[i] > 0) {
      ++count;
    }
    synapseCount[i] = -synapseCount[i];
  }

  std::cout << count << " bodies have synapses" << std::endl;

  //Record body < 100000 voxels
  FILE *fp = fopen((sessionDir + "/bodies/bodysize_ext.txt").c_str(), "r");
  ZString str;
  while (str.readLine(fp)) {
    std::vector<int> body = str.toIntegerArray();
    if (body.size() == 2) {
      if (body[1] >= 100000) {
        if (body[0] < (int) synapseCount.size()) {
          synapseCount[body[0]] = 0;
        }
      } else {
        if (body[0] < (int) synapseCount.size()) {
          synapseCount[body[0]] = -synapseCount[body[0]];
        }
      }
    } else if (!body.empty()) {
      std::cout << "Invalid value: " << str << std::endl;
    }
  }
  fclose(fp);

  QFileInfo fileInfo;
  //For each body with synapses
  for (size_t i = 1; i < synapseCount.size(); ++i) {
    if (synapseCount[i] > 0) {
      std::cout << "Checking body " << i << std::endl;
      ZObject3dScan obj;
      //Load the body
      for (int z = 7094; z <= 7509; ++z) {
        fileInfo.setFile(
              QDir(QString("%1/bodies/0%2").
                   arg(sessionDir.c_str()).arg(z)), QString("%1.sobj").arg(i));
        if (fexist(fileInfo.absoluteFilePath().toStdString().c_str())) {
          ZObject3dScan objSlice;
          objSlice.load(fileInfo.absoluteFilePath().toStdString());
          obj.concat(objSlice);
        }
      }

      obj.save(QString("%1/bodies/orphan_check_ext/%2.sobj").
               arg(sessionDir.c_str()).arg(i).toStdString());
    }
    //If it's an orphan body, save the marker
  }
#endif

#if 0 //merge objects
  std::string sessionDir =
      "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session18";
  std::string stackedDir = sessionDir + "/bodies/stacked";
  std::string stackedExtDir = sessionDir + "/bodies/stacked_ext";

  std::string bodySizeFile = sessionDir + "/bodies/bodysize_merged.txt";

  std::ofstream stream(bodySizeFile.c_str());

  QDir dir(stackedExtDir.c_str());
  QStringList filters;
  filters << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
  foreach (QFileInfo fileInfo, fileList) {
    QFileInfo stackedFile;
    stackedFile.setFile(QDir(stackedDir.c_str()), fileInfo.fileName());
    std::cout << stackedFile.absoluteFilePath().toStdString() << std::endl;
    if (stackedFile.exists()) {
      ZObject3dScan obj;
      obj.load(stackedFile.absoluteFilePath().toStdString());
      ZObject3dScan objExt;
      objExt.load(fileInfo.absoluteFilePath().toStdString());
      obj.concat(objExt);
      obj.canonize();
      obj.save(sessionDir + "/bodies/stacked_merged/" +
               fileInfo.fileName().toStdString());

      ZString objFileStr(fileInfo.baseName().toStdString());
      stream << objFileStr.lastInteger() << ", " << obj.getVoxelNumber() << std::endl;
    }
  }

  stream.close();

#endif

#if 0 //merge objects
  std::string sessionDir =
      "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session18";
  std::string stackedDir = sessionDir + "/bodies/orphan_check";
  std::string stackedExtDir = sessionDir + "/bodies/orphan_check_ext";

  std::string bodySizeFile = sessionDir + "/bodies/orphan_check_bodysize_merged.txt";

  std::ofstream stream(bodySizeFile.c_str());

  QDir dir(stackedExtDir.c_str());
  QStringList filters;
  filters << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
  foreach (QFileInfo fileInfo, fileList) {
    QFileInfo stackedFile;
    stackedFile.setFile(QDir(stackedDir.c_str()), fileInfo.fileName());
    std::cout << stackedFile.absoluteFilePath().toStdString() << std::endl;
    if (stackedFile.exists()) {
      ZObject3dScan obj;
      obj.load(stackedFile.absoluteFilePath().toStdString());
      if (!obj.isEmpty()) {
        ZObject3dScan objExt;
        objExt.load(fileInfo.absoluteFilePath().toStdString());
        if (!objExt.isEmpty()) {
          obj.concat(objExt);
          obj.canonize();
          obj.save(sessionDir + "/bodies/orphan_check_merged/" +
                   fileInfo.fileName().toStdString());

          ZString objFileStr(fileInfo.baseName().toStdString());
          stream << objFileStr.lastInteger() << ", " << obj.getVoxelNumber() << std::endl;
        }
      }
    }
  }

  stream.close();

  json_t *obj = json_object();
  json_t *dataObj = json_array();
  json_t *arrayObj = json_array();

  json_array_append(arrayObj, json_integer(1));
  json_array_append(arrayObj, json_integer(2));
  json_array_append(arrayObj, json_integer(3));

  json_array_append(dataObj, arrayObj);
  json_object_set(obj, "data", dataObj);

  json_dump_file(obj, (dataPath + "/test.json").c_str(), JSON_INDENT(2));
#endif

#if 0 //sort branch length
  std::string sessionDir =
      "/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/skeletonization/session18";

  QFileInfoList fileList;
  QStringList filters;
  filters << "*.swc";

  QDir dir((sessionDir + "/len25_merged").c_str());

  fileList = dir.entryInfoList(filters);

  std::vector<std::pair<double, int> > sizeArray;

  foreach (QFileInfo fileInfo, fileList) {
    std::cout << fileInfo.baseName().toStdString() << std::endl;
    int id = ZString::lastInteger(fileInfo.baseName().toStdString());
    ZSwcTree tree;
    tree.load(fileInfo.absoluteFilePath().toStdString());
    double length = tree.length();
    sizeArray.push_back(std::pair<double, int>(length, id));
  }

  std::sort(sizeArray.begin(), sizeArray.end());

  std::ofstream stream((sessionDir + "/len25_merged/length.txt").c_str());

  for (std::vector<std::pair<double, int> >::const_reverse_iterator
       iter = sizeArray.rbegin();
       iter != sizeArray.rend(); ++iter) {
    stream << iter->second << "," << iter->first << std::endl;
  }

  stream.close();
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  bundle.exportJsonFile(dataPath + "/test.json");
#endif

#if 0
  ZFlyEmNeuronRange reference;
  reference.importCsvFile(dataPath +
        "/flyem/skeletonization/session3/range/range_Mi1.csv");
  //ZSwcTree *tree = ZSwcGenerator::createSwc(reference);

  ZFlyEmNeuronRange range;

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");

  std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getId() == 10051409) {
      ZString volumePath = dataPath + "/flyem/skeletonization/session3/";
      volumePath.appendNumber(iter->getId());
      volumePath += ".sobj";
      iter->setVolumePath(volumePath);

      range = iter->getRange(bundle.getSwcResolution(NeuTube::X_AXIS),
                             bundle.getSwcResolution(NeuTube::Z_AXIS));
    }
  }

  ZSwcTree *tree = ZSwcGenerator::createRangeCompareSwc(range, reference);
  tree->save(dataPath + "/test.swc");
#endif

#if 0
  ZSwcTree tree;
  //tree.load(dataPath + "/benchmark/diadem/golden/e1.swc");
  //tree.load(dataPath + "/benchmark/swc/dense7.swc");
  tree.load(dataPath + "/DH070313-1.Edit.swc");
  ZSwcResampler resampler;
  resampler.optimalDownsample(&tree);
  tree.save(dataPath + "/test.swc");

#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  Print_Cuboid_I(&boundBox);
#endif

#if 0
  ZFlyEmDataBundle bundle1;
  bundle1.loadJsonFile(
        dataPath + "/flyem/FIB/fib_bundle1/data_bundle.json");

  ZFlyEmDataBundle bundle2;
  bundle2.loadJsonFile(
        dataPath + "/flyem/FIB/skeletonization/session18/len10/adjusted/data_bundle_small.json");

  ZFlyEmNeuron *neuron = bundle1.getNeuron(7021);

  ZSwcTree *tree1 = neuron->getModel();

  std::vector<ZFlyEmNeuron>& neuronArray = bundle2.getNeuronArray();

  double minDist = 100.0;

  ZSwcMetric *metric = new ZSwcTerminalSurfaceMetric;

  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    //std::cout << "Checking " << iter->getId() << "..." << std::endl;
    ZSwcTree *tree2 = iter->getModel();
    if (tree2 != NULL) {
      double dist = metric->measureDistance(tree1, tree2);
      if (dist <= minDist) {
        std::cout << "Within range" << "(" << dist << "): " << " " << iter->getId() << std::endl;
      }
    }
  }
#endif

#if 0 //Laplacian map
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");

  std::map<std::string, int> classMap;
  const std::vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (classMap.count(iter->getClass()) == 0) {
      classMap[iter->getClass()] = classMap.size();
    }
  }

  ZMatrix matrix;
  matrix.importTextFile(dataPath + "/test.txt");

  ZNormColorMap colorMap;


  vector<ZVaa3dMarker> markerArray;

  for (int i = 0; i < matrix.getRowNumber(); ++i) {
    ZVaa3dMarker marker;
    int id = iround(matrix.getValue(i, 0));

    ZFlyEmNeuron *neuron = bundle.getNeuron(id);

    marker.setCenter(matrix.getValue(i, 1) * 100, matrix.getValue(i, 2) * 100,
                     matrix.getValue(i, 3) * 100);
    marker.setRadius(1.0);

    QColor color = colorMap.mapColor(classMap[neuron->getClass()]);
    marker.setColor(color.red(), color.green(), color.blue());
    marker.setType(classMap[neuron->getClass()]);
    marker.setName(neuron->getName());
    marker.setComment(neuron->getClass());

    markerArray.push_back(marker);
  }

  FlyEm::ZFileParser::writeVaa3dMakerFile(dataPath + "/test.marker", markerArray);
#endif

#if 0 //Generate layer swc
  int zStart = 1490;
  int zEnd = 7489;
  double res[3] = {10, 10, 10};

  double height = (zEnd - zStart + 1) * res[2];
  double hMarkLength = height / 20;

  double layer[10] = {0.1, 0.2, 0.3, 0.35, 0.43, 0.54, 0.66, 0.73, 0.91, 1};
  double radius = 10.0 * res[2];


  Swc_Tree_Node *tn = SwcTreeNode::makePointer(0, 0, 0, radius);
  Swc_Tree_Node *root = tn;

  Swc_Tree_Node *tn2 =
      SwcTreeNode::makePointer(hMarkLength, 0, SwcTreeNode::z(tn), radius);
  SwcTreeNode::setParent(tn2, tn);

  for (size_t i = 0; i < 10; ++i) {
    double z = height * layer[i];
    tn2 = SwcTreeNode::makePointer(0, 0, z, radius);
    SwcTreeNode::setParent(tn2, tn);
    tn = tn2;
    tn2 = SwcTreeNode::makePointer(hMarkLength, 0, z, radius);
    SwcTreeNode::setParent(tn2, tn);
  }



  ZSwcTree tree;
  tree.setDataFromNode(root);

  tree.resortId();
  tree.save(dataPath + "/test.swc");

#endif

#if 0 //Generate TEM neuron information
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");

  std::map<std::string, int> classMap;
  const std::vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (classMap.count(iter->getClass()) == 0) {
      classMap[iter->getClass()] = classMap.size();
    }
  }

  std::ofstream stream((dataPath + "/flyem/TEM/id_class.txt").c_str());

  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    std::cout << iter->getId() << " " << iter->getName() << " " << iter->getClass()
              << " " << classMap[iter->getClass()] << std::endl;
    stream << iter->getId() << " " << classMap[iter->getClass()] << std::endl;
  }
  stream.close();
#endif

#if 0 //Check overlap
  QDir dir1((dataPath + "/flyem/FIB/skeletonization/session18/bodies/stacked").c_str());
  QStringList filters;
  filters << "*.sobj";
  QFileInfoList fileList1 = dir1.entryInfoList(filters);
  std::cout << fileList1.size() << " objects found in " << dir1.absolutePath().toStdString()
            << std::endl;

  QDir dir2((dataPath + "/flyem/FIB/skeletonization/session19/bodies/500k+").c_str());
  QFileInfoList fileList2 = dir2.entryInfoList(filters);
  std::cout << fileList2.size() << " objects found in " << dir2.absolutePath().toStdString()
            << std::endl;

  QVector<ZObject3dScan> objArray1(fileList1.size());
  QVector<ZObject3dScan> objArray2(fileList2.size());
  //objArray1.resize(100);
  //objArray2.resize(100);

  QVector<int> idArray1(fileList1.size());
  QVector<int> idArray2(fileList2.size());

  std::cout << "Loading objects ..." << std::endl;
  for (int i = 0; i < objArray1.size(); ++i) {
    objArray1[i].load(fileList1[i].absoluteFilePath().toStdString());
    std::cout << i << ": Object size: " << objArray1[i].getVoxelNumber() << std::endl;
    objArray1[i].downsample(3, 3, 3);
    objArray1[i].canonize();
    std::cout << "Object size: " << objArray1[i].getVoxelNumber() << std::endl;
    idArray1[i] = ZString::lastInteger(fileList1[i].baseName().toStdString());
  }

  std::cout << "Loading objects ..." << std::endl;
  for (int i = 0; i < objArray2.size(); ++i) {
    objArray2[i].load(fileList2[i].absoluteFilePath().toStdString());
    objArray2[i].downsample(3, 3, 3);
    objArray2[i].canonize();
    idArray2[i] = ZString::lastInteger(fileList2[i].baseName().toStdString());
  }

  QVector<Cuboid_I> boundBoxArray1(objArray1.size());
  QVector<Cuboid_I> boundBoxArray2(objArray2.size());

  std::cout << "Computing bound box ..." << std::endl;
  for (int i = 0; i < objArray1.size(); ++i) {
    objArray1[i].getBoundBox(&(boundBoxArray1[i]));
  }

  std::cout << "Computing bound box ..." << std::endl;
  for (int i = 0; i < objArray2.size(); ++i) {
    objArray2[i].getBoundBox(&(boundBoxArray2[i]));
  }

  int index1 = 0;

  std::ofstream stream((dataPath + "/test.txt").c_str());

  int offset[3];
  foreach(Cuboid_I boundBox1, boundBoxArray1) {
    Stack *stack = objArray1[index1].toStack(offset);
    for (int i = 0; i < 3; ++i) {
      offset[i] = -offset[i];
    }
    int id1 = idArray1[index1];
    std::cout << "ID: " << id1 << std::endl;
    int index2 = 0;
    foreach (Cuboid_I boundBox2, boundBoxArray2) {
      if (Cuboid_I_Overlap_Volume(&boundBox1, &boundBox2) > 0) {
        int id2 = idArray2[index2];
        size_t overlap =
            objArray2[index2].countForegroundOverlap(stack, offset);
        if (overlap > 0) {
          std::cout << id1 << " " << id2 << " " << overlap << std::endl;
          stream << id1 << " " << id2 << " " << overlap << std::endl;
        }
      }
      ++index2;
    }
    C_Stack::kill(stack);
    ++index1;
  }

  stream.close();

#endif

#if 0
  ZMatrix matrix;
  matrix.importTextFile(dataPath + "/flyem/FIB/overlap.txt");

  int nrow = matrix.getRowNumber();

  QSet<QPair<int, int> > bodyArray;
  for (int i = 0; i < nrow; ++i) {
    bodyArray.insert(QPair<int, int>(0, iround(matrix.at(i, 0))));
    bodyArray.insert(QPair<int, int>(1, iround(matrix.at(i, 1))));
  }

  QList<QPair<int, int> > bodyList = bodyArray.toList();
  QMap<QPair<int, int>, int> bodyMap;
  int index = 0;
  for (QList<QPair<int, int> >::const_iterator iter = bodyList.begin();
       iter != bodyList.end(); ++iter) {
    bodyMap[*iter] = index++;
  }

  ZGraph graph(ZGraph::UNDIRECTED_WITH_WEIGHT);

  for (int i = 0; i < nrow; ++i) {
    graph.addEdge(bodyMap[QPair<int, int>(0, iround(matrix.at(i, 0)))],
        bodyMap[QPair<int, int>(1, iround(matrix.at(i, 1)))],
        matrix.at(i, 2));
  }

  int *degree = graph.getDegree();

  for (int i = 0; i < graph.getVertexNumber(); ++i) {
    if (degree[i] > 1) {
      std::set<int> neighborSet = graph.getNeighborSet(i);
      std::cout << "Overlap:" << std::endl;
      QPair<int, int> body = bodyList[i];
      std::cout << body.first << " " << body.second << std::endl;
      for (std::set<int>::const_iterator iter = neighborSet.begin();
           iter != neighborSet.end(); ++iter) {
        QPair<int, int> body = bodyList[*iter];
        std::cout << "  " << body.first << " " << body.second << std::endl;
      }
    }
  }

#endif

#if 0
  ZSwcTree tree1;
  tree1.load(dataPath + "/flyem/FIB/skeletonization/session18/bodies/changed/stacked/swc/adjusted/427.swc");
  ZSwcTree tree2;
  tree2.load(dataPath + "/flyem/FIB/skeletonization/session18/bodies/changed/stacked/swc/adjusted/167091.swc");

  ZSwcTerminalAngleMetric metric;
  metric.setDistanceWeight(false);
  double dist = metric.measureDistance(&tree1, &tree2);

  std::cout << dist << std::endl;
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITH_WEIGHT);
  graph.importTxtFile(dataPath + "/flyem/FIB/skeletonization/session19/changed.txt");


  const std::vector<ZGraph*> &subGraphList = graph.getConnectedSubgraph();
  for (std::vector<ZGraph*>::const_iterator iter = subGraphList.begin();
       iter != subGraphList.end(); ++iter) {
    ZGraph *subgraph = *iter;
    std::set<int> nodeSet;
    for (int i = 0; i < subgraph->getEdgeNumber(); ++i) {
      nodeSet.insert(subgraph->getEdgeBegin(i));
      nodeSet.insert(subgraph->getEdgeEnd(i));
    }

    for (std::set<int>::const_iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      std::cout << *iter << " ";
    }
    std::cout << "   ||" <<  std::endl;
  }

#endif

#if 0
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  //blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session19";
  QDir dir((sessionDir + "/bodies/500k+").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  int count = 0;
  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());
    if (obj.isEmpty()) {
      std::cout << "Empty object: "
                << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      continue;
    }

    if (qc.isOrphanBody(obj)) {
      std::cout << "Orphan " << ++count << ": "
                << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                << std::endl;
      ZVoxel voxel = obj.getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(voxel.x()));
      json_array_append(arrayObj, json_integer(2598 - voxel.y()));
      json_array_append(arrayObj, json_integer(voxel.z() + 1490));
      json_array_append(dataObj, arrayObj);
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, (dataPath + "/test.json").c_str(), JSON_INDENT(2));
#endif

#if 0 //Change root of the swc trees
  //Load data bundle

  //For each neuron

#endif

#if 0
  //Output a list of orphans that do not touch substack boundaries and are
  //greater than 100,000 voxels in size.
  //Please provide a list of x,y,z points along with the body ID.
  FlyEm::ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  //blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session22";
  QDir dir((sessionDir + "/bodies/stacked").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  std::cout << fileList.size() << " bodies loaded." << std::endl;

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  int count = 0;
  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());

    int id = ZString::lastInteger(objFile.absoluteFilePath().toStdString());
    if (obj.isEmpty()) {
      std::cout << "Empty object: "
                << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      continue;
    }

    if (qc.isOrphanBody(obj)) {
      std::cout << "Orphan " << ++count << ": "
                << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                << std::endl;
      ZVoxel voxel = obj.getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(id));
      json_array_append(arrayObj, json_integer(voxel.x()));
      json_array_append(arrayObj, json_integer(2598 - voxel.y()));
      json_array_append(arrayObj, json_integer(voxel.z() + 1490));
      json_array_append(dataObj, arrayObj);

      ZCuboid box = obj.getBoundBox();
      ZSwcGenerator generator;
      ZSwcTree *tree = generator.createBoxSwc(box);
      tree->save((objFile.absoluteFilePath() + ".swc").toStdString());
      delete tree;
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, (dataPath + "/test.json").c_str(), JSON_INDENT(2));

#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_DATA_DIR + "/flyem/TEM/data_release/bundle1/swc/T4-11_588435.swc");
  ZSwcSubtreeAnalyzer analyzer;
  analyzer.setMinLength(1000.0);
  analyzer.labelSubtree(&tree);
  tree.setTypeByLabel();
  tree.save(GET_DATA_DIR + "/test.swc");
#endif


#if 1
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  ZSwcSubtreeAnalyzer analyzer;
  analyzer.setMinLength(10000.0);

  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    std::cout << neuronIter->getId() << std::endl;
    ZSwcTree *tree = neuronIter->getModel();
    analyzer.labelSubtree(tree);
    tree->setTypeByLabel();
    QString path = QString("%1/%2/%3.swc").arg(GET_DATA_DIR.c_str()).arg("test/subtree").
        arg(neuronIter->getId());
    tree->save(path.toStdString());
  }
#endif

  //testMeanAndCovariance();
  //testKMeans();
  //testGMM();
  //testVBGMM();
  //testPunctaDetection();
  //testBoost();
}



#endif // not mingw
