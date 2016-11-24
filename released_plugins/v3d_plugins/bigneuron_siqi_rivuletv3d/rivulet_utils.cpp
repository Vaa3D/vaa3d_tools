#include "rivulet.h"
using namespace rivulet;

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

void SWC::add_node(SWCNode n){
  this->nodes.push_back(n);
}

void SWC::add_branch(vector< Point<float> > branch, vector<int> rlist, long pid) {
  /* generate secret number between 1 and 10: */
  int rand_node_type = rand() % 256 + 0;

  vector<SWCNode> swc_branch(branch.size());
  long idstart;
  if (this->size() == 1) // First branch to add
  {
    idstart = 1;
  } else {
    idstart = this->nodes.back().id + 1;
  }

  // Make the new swc branch
  for (int i = 0; i < branch.size(); i++) {
    if (i == this->size() - 1) {
      pid = pid > -2 ? pid : -2;
    } else {
      pid = idstart + i + 1;
    }

    swc_branch[i].id = idstart + i;
    swc_branch[i].type = rand_node_type;
    swc_branch[i].p.x = branch[i].x;
    swc_branch[i].p.y = branch[i].y;
    swc_branch[i].p.z = branch[i].z;
    swc_branch[i].radius = rlist[i];
    swc_branch[i].pid = pid;
  }

  // Check if any tail should be connected to its head
  SWCNode head = swc_branch[0];
  long minidx = this->match(head);
  if (minidx >= 0 && this->nodes[minidx].pid == -2) {
    this->nodes[minidx].pid = head.id;
  }

  // Stack swc_branch to nodes
  this->nodes.reserve(this->nodes.size() +
                      distance(swc_branch.begin(), swc_branch.end()));
  this->nodes.insert(this->nodes.end(), swc_branch.begin(), swc_branch.end());
}


Image3<unsigned char>* Soma::get_mask(){return this->mask;}


long SWC::size() { return nodes.size(); }

Soma::Soma(Point<float> centroid, float radius)
    : centroid(centroid), radius(radius) {
  this->mask = NULL;
}

Soma::~Soma() {
  if (this->mask){
    cout<<"Trying to delete soma mask"<<endl;
    delete this->mask;
    this->mask = NULL;
    cout<<"Finished delete soma mask"<<endl;
  }
}

void Soma::make_mask(Image3<unsigned char> *bimg) {
  unsigned char *mask1d = new unsigned char[bimg->size()];

  for (int i = 0; i < bimg->size(); i++) {
    Point<float> p(i, bimg->get_dims());
    mask1d[i] =
        (bimg->get_1d(i) > 0 && p.dist(this->centroid) > 1.5 * this->radius)
            ? 1
            : 0;
  }

  this->mask = new Image3<unsigned char>(mask1d, bimg->get_dims());
}
