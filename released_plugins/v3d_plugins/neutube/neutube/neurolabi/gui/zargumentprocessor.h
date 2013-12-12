#ifndef ZARGUMENTPROCESSOR_H
#define ZARGUMENTPROCESSOR_H

class ZArgumentProcessor
{
public:
    ZArgumentProcessor();

public:
    static void processArguments(int argc, char **argv, const char **Spec);
    static const char* getStringArg(const char *arg, int index = 1);
    static int getIntArg(const char *arg, int index = 1);
    static double getDoubleArg(const char *arg, int index = 1);
    static bool isArgMatched(const char *arg);
    static int getRepeatCount(const char *arg);
};

#endif // ZARGUMENTPROCESSOR_H
