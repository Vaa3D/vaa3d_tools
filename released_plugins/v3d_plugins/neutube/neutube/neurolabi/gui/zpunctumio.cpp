#include "zpunctumio.h"

#include "zpunctum.h"
#include "zfiletype.h"
#include "zvaa3dmarker.h"
#include "flyem/zfileparser.h"
#include "zstring.h"
#include <QFile>
#include "QsLog.h"

//using namespace Eigen;

ZPunctumIO::ZPunctumIO()
{
}

QList<ZPunctum*> ZPunctumIO::load(const QString &file)
{
  QList<ZPunctum*> punctaList;
  switch (ZFileType::fileType(file.toStdString())) {
  case ZFileType::V3D_APO_FILE:
    readV3DApoFile(file, punctaList);
    break;
  case ZFileType::V3D_MARKER_FILE:
    readV3DMarkerFile(file, punctaList);
    break;
  case ZFileType::RAVELER_BOOKMARK:
    readRavelerBookmarkFile(file, punctaList);
    break;
  default:
    LWARN() << "Not supported puncta file type:" << file;
    break;
  }
  LINFO() << "read" << punctaList.size() << "puncta from" << file;

  return punctaList;
}

bool ZPunctumIO::save(const QString &filename, const QList<ZPunctum *> &puncta)
{
  return save(filename, puncta.begin(), puncta.end());
}

void ZPunctumIO::readV3DApoFile(const QString &file, QList<ZPunctum *> &punctaList)
{
  QFile qFile(file);
  if (!qFile.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QTextStream stream(&qFile);
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    QStringList fieldList = line.split(",");
    if (fieldList.size() >= 12) {
      ZPunctum* punctum = new ZPunctum();
      punctum->setSource(file);
      bool ok;
      fieldList[0].toInt(&ok);
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setName(fieldList[2]);
      punctum->setComment(fieldList[3]);
      punctum->setZ(fieldList[4].toDouble(&ok));
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setX(fieldList[5].toDouble(&ok));
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setY(fieldList[6].toDouble(&ok));
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setMaxIntensity(fieldList[7].toDouble(&ok));
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setMeanIntensity(fieldList[8].toDouble(&ok));
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setSDevOfIntensity(fieldList[9].toDouble(&ok));
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setVolSize(fieldList[10].toDouble(&ok));
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setMass(fieldList[11].toDouble(&ok));
      if (!ok) {
        delete punctum;
        continue;
      }
      punctum->setProperty1(fieldList[12]);
      punctum->setProperty2(fieldList[13]);
      punctum->setProperty3(fieldList[14]);
      if (fieldList.size() >= 18) {
        bool ok1, ok2;

        punctum->setColor(QColor(fieldList[15].toInt(&ok), fieldList[16].toInt(&ok1), fieldList[17].toInt(&ok2)));
        if (!ok || !ok1 || !ok2) {
          if (fieldList[15].isEmpty() && fieldList[16].isEmpty() && fieldList[17].isEmpty())
            punctum->setColor(QColor(0,0,0));
          else {
            delete punctum;
            continue;
          }
        }
      }
      punctum->setRadius(Cube_Root(0.75 / M_PI * punctum->volSize()));
      punctaList.push_back(punctum);
    }
  }
  qFile.close();
}

void ZPunctumIO::readV3DMarkerFile(const QString &file, QList<ZPunctum *> &punctaList)
{
  std::vector<ZVaa3dMarker> markerArray =
      FlyEm::ZFileParser::readVaa3dMarkerFile(file.toStdString());
  for (size_t i = 0; i < markerArray.size(); ++i) {
    ZPunctum* punctum = new ZPunctum();
    punctum->setName(markerArray[i].name().c_str());
    punctum->setComment(markerArray[i].comment().c_str());
    ZPoint center = markerArray[i].center();
    punctum->setZ(center[2]);
    punctum->setX(center[0]);
    punctum->setY(center[1]);
    punctum->setMaxIntensity(255);
    punctum->setMeanIntensity(255);
    punctum->setSDevOfIntensity(0);
    double r = markerArray[i].radius();
    punctum->setVolSize(TZ_PI * r * r * r);
    punctum->setMass(r);
    punctum->setColor(
          QColor(markerArray[i].colorR(), markerArray[i].colorG(),
                 markerArray[i].colorB()));
    punctum->setRadius(r);
    ZString source(markerArray[i].name() + "_");
    source.appendNumber(markerArray[i].type(), 1);
    punctum->setSource(source.c_str());
    punctaList.push_back(punctum);
  }
}

void ZPunctumIO::readRavelerBookmarkFile(const QString &file, QList<ZPunctum *> &punctaList)
{
  FILE *fp = fopen(file.toStdString().c_str(), "r");
  if (fp != NULL) {
    ZString str;
    bool bookmarkStart = false;
    while (str.readLine(fp)) {
      if (!bookmarkStart) {
        if (str.startsWith("bookmarks")) {
          bookmarkStart = true;
        }
      } else {
        std::vector<int> coordinates = str.toIntegerArray();
        if (coordinates.size() >= 3) {
          ZPunctum* punctum = new ZPunctum();
          punctum->setX(coordinates[0]);
          punctum->setY(coordinates[1]);
          punctum->setZ(coordinates[2]);
          punctum->setSource(file);
          punctaList.push_back(punctum);
        }
      }

    }
  }
  fclose(fp);
}

void ZPunctumIO::punctaFprint(FILE *fp, const ZPunctum &p, int id)
{
  fprintf(fp, "%d,%s,%s,%s,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%s,%s,%s,%d,%d,%d\n",
          id, "", qPrintable(p.name()), qPrintable(p.comment()), p.z(), p.x(), p.y(), p.maxIntensity(), p.meanIntensity(),
          p.sDevOfIntensity(), p.volSize(), p.mass(), qPrintable(p.property1()), qPrintable(p.property2()), qPrintable(p.property3()),
          p.color().red(), p.color().green(), p.color().blue());
}
