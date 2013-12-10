#include "zmoviemaker.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include "z3dwindow.h"
#include "zfiletype.h"
#include "zswcmovieactor.h"
#include "zswctree.h"
#include "zstackdoc.h"
#include "z3dinteractionhandler.h"
#include "tz_error.h"
#include "z3dswcfilter.h"
#include "zstack.hxx"
#include "zstackmovieactor.h"
#include "z3dvolumeraycaster.h"
#include "z3dvolumeraycasterrenderer.h"
#include "zpunctamovieactor.h"
#include "zmoviestage.h"
#include "z3dcompositor.h"
#include "zpunctum.h"
#include "z3dvolumesource.h"
#include "zpunctumio.h"

using namespace std;

ZMovieMaker::ZMovieMaker() : m_stage(NULL),
  m_width(1024), m_height(1024), m_frameInterval(50)
{
}

ZMovieMaker::~ZMovieMaker()
{
  delete m_stage;
  dismissCast();
}

void ZMovieMaker::dismissCast()
{
  for (std::vector<ZMovieActor*>::iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    delete *iter;
  }
}

void ZMovieMaker::prepareStage()
{
  Z3DWindow *window = new Z3DWindow(m_academy, Z3DWindow::NORMAL_INIT,
                                    false, NULL);
  m_stage = new ZMovieStage(window);

  m_photographer.setStage(m_stage);
  window->getVolumeSource()->setMaxVoxelNumber(1024 * 1024 * 512);
  window->getVolumeSource()->reloadVolume();

  window->show();
  window->getSwcFilter()->setColorMode("Intrinsic");
  window->getVolumeRaycaster()->getRenderer()->setOpaque(true);
  window->getVolumeRaycaster()->hideBoundBox();
  window->getVolumeRaycasterRenderer()->setCompositeMode("Direct Volume Rendering");
  window->getVolumeRaycasterRenderer()->setTextureFilterMode("Nearest");
  window->getCompositor()->setBackgroundFirstColor(
        glm::vec3(m_backgroundColor.redF(), m_backgroundColor.greenF(),
                  m_backgroundColor.blueF()));
  window->getCompositor()->setBackgroundSecondColor(
        glm::vec3(m_backgroundColor.redF(), m_backgroundColor.greenF(),
                  m_backgroundColor.blueF()));

   //stage->getVolumeSource()->setZScale(zScale);
  //m_clipperState.init(window);

  for (std::vector<ZMovieActor*>::iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    (*iter)->setStage(m_stage);
  }
}

void ZMovieMaker::recruitCast()
{
  std::map<string, string> cast = m_script.getCast();

  ZStackDoc *academy = getAcademy();

  academy->blockSignals(true);
  for (std::map<string, string>::const_iterator iter = cast.begin();
       iter != cast.end(); ++iter) {
    switch (ZFileType::fileType(iter->second)) {
    case ZFileType::SWC_FILE:
    {
      ZSwcTree *tree = new ZSwcTree;
      tree->load(iter->second.c_str());
      tree->setVisible(false);
      academy->addSwcTree(tree);
      ZSwcMovieActor *actor = new ZSwcMovieActor;
      actor->setActor(tree);
      actor->setId(iter->first);
      actor->setVisible(false);
      m_cast.push_back(actor);
    }
      break;
    case ZFileType::TIFF_FILE:
    {
      ZStack *stack = new ZStack();
      stack->load(iter->second);

      if (academy->stack() != NULL) {
        cout << "Warning: " << "multiple volume detected. Only one allowed."
             << endl;
      } else {
        academy->loadStack(stack);
      }
      ZStackMovieActor *actor = new ZStackMovieActor;
      actor->setVisible(false);
      actor->setId(iter->first);
      actor->setActor(stack);
      m_cast.push_back(actor);
    }
      break;
    case ZFileType::V3D_MARKER_FILE:
    {
      QList<ZPunctum*> punctaList =
          ZPunctumIO::load(iter->second.c_str());
      for (int i=0; i<punctaList.size(); i++) {
        punctaList[i]->setVisible(false);
        academy->addPunctum(punctaList[i]);
      }

      ZPunctaMovieActor *actor = new ZPunctaMovieActor;
      actor->setVisible(false);
      actor->setId(iter->first);
      vector<ZPunctum*> puncta;
      for (QList<ZPunctum*>::const_iterator iter = punctaList.begin();
           iter != punctaList.end(); ++iter) {
        puncta.push_back(*iter);
      }
      actor->setActor(puncta);
      m_cast.push_back(actor);
    }
    default:
      break;
    }
  }
  academy->blockSignals(false);
}

string ZMovieMaker::getFramePath(const string &dirPath, int index)
{
  ostringstream stream;
  stream << dirPath << '/' << std::setw(5) << std::setfill('0') << index << ".tif";

  return stream.str();
}

int ZMovieMaker::make(const std::string &filePath)
{  
  //prepareStage();
  recruitCast();
  prepareStage();

#ifdef _DEBUG_
  printSummary();
#endif

  int index = 0;

  //m_stage->getVolumeRaycaster()->setZCutLower(30);

  bool takingPicture = true;

  bool hasMoreScene = true;

  while (hasMoreScene) {
    cout << "Frame " << index << endl;

    int state = updateScene();

    hasMoreScene = state > 0;
    takingPicture = state == 2;

    if (takingPicture) {
      m_photographer.takePicture(getFramePath(filePath, index),
                                 getMovieWidth(), getMovieHeight());
    }

    if (index == 0) { //Initialize the clipper from the first scene
                      //***Need improvement***
      m_clipperState.init(m_stage->getWindow());
    }

    if (takingPicture) {
      ++index;
    }
    //cout << "Z cut value after:" << m_stage->getVolumeEntryExitPoints()->zCutLowerValue() << endl;
    TZ_ASSERT(index < 10000, "Too many frames");

    //cout << "Z cut value: :" << m_stage->getVolumeRaycaster()->zCutLowerValue() << endl;

    //m_stage->updateDisplay();
    /*
    const Z3DRotation& rotation = m_script.getCurrentScene()->getCameraRotation();
    if (rotation.getAngle() > 0.0) {
      glm::vec3 axis(rotation.getAxis().x(), rotation.getAxis().y(), rotation.getAxis().z());
      m_stage->getInteractionHandler()->getTrackball()->rotate(axis, rotation.getAngle());
    }
    */

    //cout << "Z cut value after:" << m_stage->getVolumeRaycaster()->zCutLowerValue() << endl;
  }

  return index;
}

void ZMovieMaker::makeSlideShow(const string &filePath)
{
  //under development
  UNUSED_PARAMETER(filePath.c_str());
}

int ZMovieMaker::updateAction()
{
  int actionNumber = 1;
  int timeStep = 0;

  if (m_script.isStarted()) {
    actionNumber = getFrameInterval() / getActionTimeStep();
    timeStep = getActionTimeStep();
  }

  ZMovieScene *scene = NULL;
  for (int i = 0; i < actionNumber; ++i) {
    scene = m_script.nextScene(timeStep);

    if (scene == NULL) {
      return 0;
    }

#ifdef _DEBUG_2
    scene->print();
#endif

    scene->updateCamera(m_stage->getWindow(), timeStep);

    scene->updateClip(m_stage->getWindow(), &m_clipperState, timeStep);

    setupAction(*scene);
    act(timeStep);
  }

  if (scene->isBackground()) {
    return 1;
  }

  return 2;
}

int ZMovieMaker::updateScene()
{
  int state = updateAction();

  m_stage->updateWindow();

  return state;
}

ZMovieActor* ZMovieMaker::getActor(string id)
{
  ZMovieActor *actor = NULL;
  for (vector<ZMovieActor*>::iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    if ((*iter)->getId() == id) {
      actor = *iter;
      break;
    }
  }

  return actor;
}

void ZMovieMaker::setupAction(const ZMovieScene &scene)
{
  for (vector<ZMovieActor*>::iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    (*iter)->setActive(false);
  }

  for (size_t i = 0; i < scene.getActionNumber(); ++i) {
    MovieAction action = scene.getAction(i);
    ZMovieActor *actor = getActor(action.actorId);

    if (actor != NULL) {
      if (scene.isNewScene()) {
        if (action.settingAlpha) {
          actor->takeAlpha(action.alpha);
        }
        if (action.settingColor) {
          actor->takeColor(action.red, action.green, action.blue);
        }
        actor->reset();
      }
#ifdef _DEBUG_
      if (actor->getId() == "slice_colored") {
        cout << "debug here" << endl;
      }
#endif
      actor->setActive(true);
      actor->setVisible(action.isVisible);
      actor->setMovingOffset(action.movingOffset);

      actor->setFadingFactor(action.fadingFactor);
      actor->setTransitFactor(action.transitFactor[0], action.transitFactor[1],
          action.transitFactor[2]);
    } else {
      cout << "Cannot find actor " << action.actorId << endl;
    }
  }
}

void ZMovieMaker::act(int timeStep)
{
  for (vector<ZMovieActor*>::const_iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    (*iter)->perform(timeStep);
  }
}

ZStackDoc* ZMovieMaker::getAcademy()
{
  if (m_academy == NULL) {
    m_academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);
  }

  return m_academy.get();
}

void ZMovieMaker::printSummary()
{
  cout << "Academy: " << getAcademy()->toString().toStdString();
  cout << "Stage: " << m_stage << endl;

  cout << m_cast.size() << " actors";

  for (vector<ZMovieActor*>::const_iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    cout << "  " << (*iter)->getId() << endl;
  }
  /*
  ZMovieScript m_script;
  ZMoviePhotographer m_photographer;
  int m_width;
  int m_height;
  double m_frameInterval;
  */
}
