#ifndef ZCOMMANDLINE_H
#define ZCOMMANDLINE_H

#include <string>
#include <vector>

class ZCommandLine
{
public:
  ZCommandLine();

  enum ECommand {
    OBJECT_MARKER, BOUNDARY_ORPHAN, OBJECT_OVERLAP,
    SYNAPSE_OBJECT, UNKNOWN_COMMAND
  };

  int run(int argc, char *argv[]);


private:
  static ECommand getCommand(const char *cmd);
  int runObjectMarker();
  int runBoundaryOrphan();
  int runObjectOverlap();
  int runSynapseObjectList();

private:
  std::vector<std::string> m_input;
  std::string m_output;
  std::string m_blockFile;
  int m_ravelerHeight;
  int m_zStart;
  int m_intv[3];
  bool m_fullOverlapScreen;
};

#endif // ZCOMMANDLINE_H
