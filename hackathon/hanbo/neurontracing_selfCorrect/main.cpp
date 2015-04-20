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
    func.loadData(argv[1],argv[2]);
    func.calculateScore();
    func.getTrainingSample();
    func.performTraining();
    func.predictExisting();
    func.correctExisting();
    func.saveData(argv[3]);
    return 0;
}
