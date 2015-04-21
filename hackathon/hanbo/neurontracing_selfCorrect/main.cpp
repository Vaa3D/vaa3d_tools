#include "src/nt_selfcorrect_func.h"

#include <iostream>

using namespace std;

int main(int argc, char ** argv)
{
    if(argc < 3)
    {
        return 0;
    }
    nt_selfcorrect_func func;
    if(argc == 4)
        func.correct_tracing(argv[1],argv[2],argv[3]);
    else if(argc==3)
        func.smart_tracing(argv[1],argv[2]);
    return 0;
}
