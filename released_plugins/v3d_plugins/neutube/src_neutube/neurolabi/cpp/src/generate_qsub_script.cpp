#include <iostream>
#include <sstream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tz_utilities.h"
#include "zstring.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.1") == 1) {
    return 0;
  }

  static char const *Spec[] = {"--option <string> <input:string> [-o <string>]",
    "[--args <string>] --cmd_path <string> [--work_dir <string>]",
    "[--data_dir <string>] [--plain_test]",  NULL};

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  if (strcmp(Get_String_Arg(const_cast<char*>("--option")), 
        "skeletonize") == 0) {
    char *neuronListFile = Get_String_Arg(const_cast<char*>("input"));
    ZString line;
    FILE *fp = fopen(neuronListFile, "r");
    if (fp == NULL) {
      printf("Unable to open %s\n", neuronListFile);
      return 1;
    }

    FILE *out_fp = stdout;
    if (Is_Arg_Matched(const_cast<char*>("-o"))) {
      out_fp = fopen(Get_String_Arg(const_cast<char*>("-o")), "w");
    }

    while (line.readLine(fp)) {
      int bodyId = line.firstInteger();
      if (bodyId > 0) {
        line.replace(bodyId, "");
        line.replace(" ", "");
        line.replace("\t", "");
        line.replace("(", "_");
        line.replace(")", "_");
        line.replace("?", "_x_");
        line.replace("\\", "_");
        line.replace("/", "_");
        line.replace("-", "_");
        if (Is_Arg_Matched(const_cast<char*>("--plain_test"))) {
          string args;
          if (Is_Arg_Matched(const_cast<char*>("--args"))) {
            args = Get_String_Arg(const_cast<char*>("--args"));
          }
          fprintf(out_fp, "%s %d %s %s %s > %s/skeletonize_%d.txt\n",
              Get_String_Arg(const_cast<char*>("--cmd_path")),
              bodyId,
              Get_String_Arg(const_cast<char*>("--work_dir")), 
              line.c_str(), args.c_str(),
              Get_String_Arg(const_cast<char*>("--work_dir")), 
              bodyId);
        } else {
          fprintf(out_fp, "qsub -P flyemproj -pe batch 6 -N skeletonize -j y -o /dev/null -b y -cwd -V '%s %d %s %s > %s/skeletonize_%d.txt'\n",
              Get_String_Arg(const_cast<char*>("--cmd_path")),
              bodyId,
              Get_String_Arg(const_cast<char*>("--work_dir")), 
              line.c_str(),
              Get_String_Arg(const_cast<char*>("--work_dir")), 
              bodyId);
        }
      }
    }

    fclose(out_fp);
    fclose(fp);
  }
}
