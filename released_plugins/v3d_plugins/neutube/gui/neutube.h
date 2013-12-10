#ifndef NEUTUBE_H
#define NEUTUBE_H

#include <string>

class ZMessageReporter;
class ZLogMessageReporter;

namespace NeuTube {

enum EDocumentableType {
  Documentable_SWC, Documentable_PUNCTUM, Documentable_OBJ3D,
  Documentable_STROKE, Documentable_LOCSEG_CHAIN, Documentable_CONN
};

enum EImageBackground {
  IMAGE_BACKGROUND_BRIGHT, IMAGE_BACKGROUND_DARK
};

enum ESizeHintOption {
  SIZE_HINT_DEFAULT, SIZE_HINT_CURRENT_BEST, SIZE_HINT_TAKING_SPACE
};

enum EAxis {
  X_AXIS, Y_AXIS, Z_AXIS
};

enum EColor {
  RED, GREEN, BLUE
};

ZMessageReporter *getMessageReporter();
ZLogMessageReporter* getLogMessageReporter();

std::string getErrorFile();
std::string getWarnFile();
std::string getInfoFile();
}



#endif // NEUTUBE_H
