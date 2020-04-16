#ifndef SPLIT_SUBTREE_H
#define SPLIT_SUBTREE_H
#include<vector>
#include<QString>
#include<string>
#include<Windows.h>
#include<direct.h>
#include<v3d_message.h>
#include<basic_surf_objs.h>
using namespace std;
struct Subtree: public NeuronTree{
    int order;
    Subtree(){
        order=0;
    }
};
bool find_all_files(QString datadir,vector<QString> &swcfiles);
bool SplitSubtree(NeuronTree nt,vector<bool> &selected,QString para,vector<Subtree> &tree);

#endif // SPLIT_SUBTREE_H
