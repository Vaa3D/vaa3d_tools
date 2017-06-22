// Implementation of Kosaraju's algorithm to print all SCCs
#include <iostream>
#include <list>
#include <stack>
#include "graph.h"
using namespace std;

Graph::Graph(int V){
    this->V = V;
    adj = new list<int> [V];
}

// A recursive function to print DFS starting from v
void Graph::DFSUtil(int v, bool* visited){
    // Mark the current node as visited and print it
    visited[v] = true;
    // Recur for all the vertices adjacent to this vertex
    list<int>::iterator i;
    for (i = adj[v].begin(); i != adj[v].end(); ++i)
        if (!visited[*i])
            DFSUtil(*i, visited);
}

void Graph::addEdge(int v, int w){
    adj[v].push_back(w); // Add w to v’s list.
}

// The main function that finds and returns strongly connected components
std::vector<bool> Graph::scc() {
  stack<int> Stack;
  // Mark all the vertices as not visited (For first DFS)
  bool *visited = new bool[V];
  for (int i = 0; i < V; i++) visited[i] = false;

  // Run a DFS from soma and mark all nodes reachable from soma
  this->DFSUtil(0, visited);

  std::vector<bool> visited_vec(this->V);
  for(int i=0;i<this->V;i++) {
    visited_vec[i] = visited[i];
  }

  if(visited){
    delete visited;
    visited=0;
  }
  return visited_vec;
}
