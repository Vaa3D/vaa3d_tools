#include "zargumentprocessor.h"

#include "tz_utilities.h"

ZArgumentProcessor::ZArgumentProcessor()
{
}

void ZArgumentProcessor::processArguments(
    int argc, char **argv, const char **Spec)
{
  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);
}

const char* ZArgumentProcessor::getStringArg(const char *arg, int index)
{
  return Get_String_Arg(const_cast<char*>(arg), index);
}

int ZArgumentProcessor::getIntArg(const char *arg, int index)
{
  return Get_Int_Arg(const_cast<char*>(arg), index);
}

bool ZArgumentProcessor::isArgMatched(const char *arg)
{
  return Is_Arg_Matched(const_cast<char*>(arg));
}

double ZArgumentProcessor::getDoubleArg(const char *arg, int index)
{
  return Get_Double_Arg(const_cast<char*>(arg), index);
}

int ZArgumentProcessor::getRepeatCount(const char *arg)
{
  return Get_Repeat_Count(const_cast<char*>(arg));
}
