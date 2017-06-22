// Implementation of Kosaraju's algorithm to print all SCCs
#include <iostream>
#include <list>
#include <stack>
#include <vector>
using namespace std;

class Graph{
    int V; // No. of vertices
    list<int> *adj; // An array of adjacency lists
    // A recursive function to print DFS starting from v
    void DFSUtil(int v, bool* visited);
public:
    Graph(int V);
    void addEdge(int v, int w);
    std::vector<bool> scc();
};

