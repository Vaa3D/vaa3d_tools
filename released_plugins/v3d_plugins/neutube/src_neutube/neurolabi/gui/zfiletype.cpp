#include "zfiletype.h"

//#include <QString>
#include "zstring.h"

ZFileType::ZFileType()
{
}

ZFileType::EFileType ZFileType::fileType(const std::string &filePath)
{
  ZString str(filePath.c_str());

  if (str.endsWith(".swc", ZString::CASE_INSENSITIVE)) {
    return SWC_FILE;
  } else if (str.endsWith(".tif", ZString::CASE_INSENSITIVE) ||
             str.endsWith(".tiff", ZString::CASE_INSENSITIVE)) {
    return TIFF_FILE;
  } else if (str.endsWith(".lsm", ZString::CASE_INSENSITIVE)) {
    return LSM_FILE;
  } else if (str.endsWith(".png", ZString::CASE_INSENSITIVE)) {
    return PNG_FILE;
  } else if (str.endsWith(".tb", ZString::CASE_INSENSITIVE)) {
    return LOCSEG_CHAIN_FILE;
  } else if (str.endsWith(".nnt", ZString::CASE_INSENSITIVE)) {
    return SWC_NETWORK_FILE;
  } else if (str.endsWith(".json", ZString::CASE_INSENSITIVE)) {
    return JSON_FILE;
  } else if (str.endsWith(".fnt", ZString::CASE_INSENSITIVE) ) {
    return FLYEM_NETWORK_FILE;
  } else if (str.endsWith(".xml", ZString::CASE_INSENSITIVE)) {
    return XML_FILE;
  } else if (str.endsWith(".apo", ZString::CASE_INSENSITIVE)) {
    return V3D_APO_FILE;
  } else if (str.endsWith(".marker", ZString::CASE_INSENSITIVE)) {
    return V3D_MARKER_FILE;
  } else if (str.endsWith(".raw", ZString::CASE_INSENSITIVE)) {
    return V3D_RAW_FILE;
  } else if (str.endsWith(".v3dpbd", ZString::CASE_INSENSITIVE) ||
             str.endsWith(".pbd", ZString::CASE_INSENSITIVE)) {
    return V3D_PBD_FILE;
  } else if (str.endsWith(".txt", ZString::CASE_INSENSITIVE)) {
    FILE *fp = fopen(str.c_str(), "r");
    if (fp != NULL) {
      ZString str;
      int patternCount = 0;
      while (str.readLine(fp)) {
        if (str.startsWith("session:")) {
          patternCount++;
        }
        if (patternCount == 1) {
          if (str.startsWith("bookmarks")) {
            patternCount++;
          }
        }
      }
      fclose(fp);

      if (patternCount >= 2) {
        return RAVELER_BOOKMARK;
      }
    }
  } else if (str.endsWith(".nsp", ZString::CASE_INSENSITIVE)) {
    return MYERS_NSP_FILE;
  } else if (str.endsWith(".sobj", ZString::CASE_INSENSITIVE)) {
    return OBJECT_SCAN_FILE;
  } else if (str.endsWith(".jpg", ZString::CASE_INSENSITIVE)) {
    return JPG_FILE;
  } else if (str.endsWith(".dvid", ZString::CASE_INSENSITIVE)) {
    return DVID_OBJECT_FILE;
  }

  return UNIDENTIFIED_FILE;
}

std::string ZFileType::typeName(EFileType type)
{
  switch (type) {
  case SWC_FILE:
    return "SWC";
  case SWC_NETWORK_FILE:
    return "SWC Network";
  case LOCSEG_CHAIN_FILE:
    return "Locseg Chain";
  case TIFF_FILE:
    return "TIFF";
  case LSM_FILE:
    return "LSM";
  case PNG_FILE:
    return "PNG";
  case V3D_RAW_FILE:
    return "Vaa3D raw";
  case SYNAPSE_ANNOTATON_FILE:
    return "Synapse Annotation";
  case FLYEM_NETWORK_FILE:
    return "FlyEM Network";
  case XML_FILE:
    return "XML";
  case JSON_FILE:
    return "Json";
  case V3D_APO_FILE:
    return "Vaa3d APO";
  case V3D_PBD_FILE:
    return "v3dpbd";
  case V3D_MARKER_FILE:
    return "Vaa3d marker";
  case RAVELER_BOOKMARK:
    return "Reveler bookmark";
  case MYERS_NSP_FILE:
    return "Neuron segmentation";
  default:
    return "Unknown";
  }
}

bool ZFileType::isImageFile(EFileType type)
{
  return (type == TIFF_FILE) || (type == LSM_FILE) || (type == PNG_FILE) ||
      (type == V3D_RAW_FILE) || (type == V3D_PBD_FILE) ||
      (type == MYERS_NSP_FILE) || (type == OBJECT_SCAN_FILE) ||
      (type == JPG_FILE) || (type == DVID_OBJECT_FILE);
}

bool ZFileType::isImageFile(const std::string &filePath)
{
  return isImageFile(fileType(filePath));
}

bool ZFileType::isObjectFile(EFileType type)
{
  return (type == SWC_FILE) || (type == SWC_NETWORK_FILE) ||
      (type == LOCSEG_CHAIN_FILE) || (type == SYNAPSE_ANNOTATON_FILE) ||
      (type == FLYEM_NETWORK_FILE) || (type == JSON_FILE) ||
      (type == V3D_APO_FILE) || (type == V3D_MARKER_FILE) ||
      (type == RAVELER_BOOKMARK);
}

bool ZFileType::isObjectFile(const std::string &filePath)
{
  return isObjectFile(fileType(filePath));
}

bool ZFileType::isNeutubeOpenable(const std::string &filePath)
{
  return isNeutubeOpenable(fileType(filePath));
}

bool ZFileType::isNeutubeOpenable(EFileType type)
{
  return isImageFile(type) || isObjectFile(type);
}
