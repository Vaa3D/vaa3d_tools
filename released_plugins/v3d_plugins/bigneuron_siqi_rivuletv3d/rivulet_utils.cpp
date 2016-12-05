#include "rivulet.h"
#include "utils/graph.h"
#include <map>
#include <iterator>
using namespace rivulet;

/* Try to match a node with its closest neighbour in this SWC
    return a postive index if matched
    return -2 if unmatched
*/
long SWC::match(SWCNode n) {
  float mindist = 1.0e4;
  long minidx = -2;
  // Find the closest node
  for (long i = 0; i < this->size(); i++) {
    float dist = n.p.dist(this->nodes[i].p);
    if (mindist > dist) {
      mindist = dist;
      minidx = i;
    }
  }

  // Return its index if it is in range
  if (n.radius > mindist || this->nodes[minidx].radius > mindist) {
    return minidx;
  } else {
    return -2;
  }
}

void SWC::add_node(SWCNode n) { this->nodes.push_back(n); }

SWCNode SWC::get_node(int i) { return this->nodes[i]; }

void SWC::add_branch(Branch &branch, long connect_id) {
  if (branch.get_length() < 5) return;
  /* generate secret number between 1 and 10: */
  int rand_node_type = rand() % 256 + 0;
  vector<float> rlist = branch.get_radius();

  vector<SWCNode> swc_branch(branch.get_length());
  long idstart;
  if (this->size() == 1) // First branch to add
  {
    idstart = 1;
  } else {
    idstart = this->nodes.back().id + 1;
  }

  // Make the new swc branch
  int pid = -2;
  for (int i = 0; i < branch.get_length(); i++) {
    if (i == branch.get_length() - 1) {
      pid = connect_id >= 0 ? connect_id : -2;
    } else {
      pid = idstart + i + 1;
    }

    swc_branch[i].id = idstart + i;
    swc_branch[i].type = rand_node_type;
    Point<float> p = branch.get_point(i);
    swc_branch[i].p = p;
    swc_branch[i].radius = max2(branch.get_radius_at(i) * 0.6, 1); // The radius estimated for Rivulet2 tracing was a bit too large for visualisation
    swc_branch[i].pid = pid;
  }

  // Check if any tail should be connected to its head
  SWCNode tail = swc_branch[0];
  long minidx = this->match(tail);
  if (minidx > 0 && this->nodes[minidx].pid == -2) {
    this->nodes[minidx].pid = tail.id;
    printf("Connect tail to %d. Dist: %.2f\n", minidx,
           this->nodes[minidx].p.dist(tail.p));
  }

  // Stack swc_branch to nodes
  this->nodes.reserve(this->nodes.size() +
                      distance(swc_branch.begin(), swc_branch.end()));
  this->nodes.insert(this->nodes.end(), swc_branch.begin(), swc_branch.end());
}

void SWC::pad(CropRegion rg){
  for(std::vector<SWCNode>::iterator it=this->nodes.begin(); it != this->nodes.end(); ++it){
    it->p.x += rg.xmin;
    it->p.y += rg.ymin;
    it->p.z += rg.zmin;
  }
}

void SWC::prune_unreached(){
  Graph g(this->size());
  // Create a graph with the SWC Node IDs

  for (vector<SWCNode>::iterator it = this->nodes.begin();
       it != this->nodes.end(); ++it) {
    if (it->pid >= 0) {
      g.addEdge(it->id, it->pid);
      g.addEdge(it->pid, it->id);
    }
  }


  vector<bool> visited = g.scc();
  std::vector<SWCNode> pruned_nodes; 
  std::vector<SWCNode>::iterator it;
  std::vector<bool>::iterator vt;

  for(it=this->nodes.begin(), vt=visited.begin(); it != this->nodes.end(); ++it, ++vt){
    if(*vt){
      pruned_nodes.push_back(*it);
    }
  }

  this->nodes = pruned_nodes;
}

void SWC::prune_leaves(){
  // Count number of children for each node
  std::vector<int> pid_vec(this->size());
  std::vector<SWCNode>::iterator it;
  std::vector<int>::iterator pt;
  for(it=this->nodes.begin(), pt=pid_vec.begin(); it != this->nodes.end(); ++it, ++pt){
    *pt = it->pid;
  }

  std::map<int, int> child_ctr;
  for(it=this->nodes.begin(); it != this->nodes.end(); ++it){
    int c = std::count(pid_vec.begin(), pid_vec.end(), it->id);
    child_ctr[it->id] = c;
  }

  // Find all leaf node
  vector<SWCNode> leaf_nodes;
  for(it=this->nodes.begin(); it != this->nodes.end(); ++it){
    if (child_ctr[it->id] == 0){
      leaf_nodes.push_back(*it);
    }
  }

  int node_id=-1;
  vector<SWCNode> nodes_to_dump;
  // Iterate each leaf node
  for(it=leaf_nodes.begin(); it != leaf_nodes.end(); ++it){
    SWCNode node = *it;
    vector<SWCNode> branch;
    while(true){ // Get the leaf branch out
      if(node.id == -2 || child_ctr[node.id] > 1){
        break;
      }
      branch.push_back(node);
      node = this->get_parent(node);
    }

    // Calculate the curve length of this branch
    float blen = 0;
    for(vector<SWCNode>::iterator bt=branch.begin(); bt!=branch.end()-1; ++bt){
      blen += bt->p.dist(std::next(bt, 1)->p);
    }

    // Prune if this branch is too short
    if(blen < 5){
      nodes_to_dump.reserve(nodes_to_dump.size() + distance(branch.begin(), branch.end()));
      nodes_to_dump.insert(nodes_to_dump.end(), branch.begin(), branch.end());
    }
  }

  // Only keep the swc nodes not in nodes_to_dump
  for(it=this->nodes.begin(); it != this->nodes.end();){
    if (std::find(nodes_to_dump.begin(), nodes_to_dump.end(), *it) != nodes_to_dump.end()){
      it = this->nodes.erase(it);
    }
    else{
      ++it;
    }
  }  
}

SWCNode SWC::get_parent(SWCNode n) {
  SWCNode pnode;
  pnode.id = n.pid;
  std::vector<SWCNode>::iterator pnode_itr =
      std::find(this->nodes.begin(), this->nodes.end(), pnode);
  if (pnode_itr == this->nodes.end()) {  // Not found
    pnode.id = -2;
    return pnode;
  } else {
    return *pnode_itr;
  }
}

void SWC::prune(){
  // Find the largest connected sub-graph 
  this->prune_unreached();

  // Remove leaf branches with lengths < 5
  this->prune_leaves();
}

Image3<unsigned char> *Soma::get_mask() { return this->mask; }

long SWC::size() { return nodes.size(); }

Soma::Soma(Point<float> centroid, float radius)
    : centroid(centroid), radius(radius) {
  this->mask = NULL;
}

Soma::~Soma() {

  if (this->mask) {
    delete this->mask;
    this->mask = NULL;
  }
}

void Soma::make_mask(Image3<unsigned char> *bimg) {
  unsigned char *mask1d = new unsigned char[bimg->size()]();

  for (int i = 0; i < bimg->size(); i++) {
    Point<float> p(i, bimg->get_dims());
    mask1d[i] =
        (bimg->get_1d(i) > 0 && p.dist(this->centroid) < 1.5 * this->radius)
            ? 1
            : 0;
  }

  this->mask = new Image3<unsigned char>(mask1d, bimg->get_dims());
}

