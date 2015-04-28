#include "Lm/Neuron.h"

int main(int argc, char ** argv)
{
    if(argc >= 3){
        Neuron neuronfunc(argv[1]);
        neuronfunc.printSWC(argv[2]);
    }
    return 0;
}
