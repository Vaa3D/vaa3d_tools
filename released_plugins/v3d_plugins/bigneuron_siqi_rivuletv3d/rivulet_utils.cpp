#include "rivulet.h"
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

void SWC::plus1(){
  for (long i=0; i < this->size(); i++){
    this->nodes[i].p.x = this->nodes[i].p.x + 1;
    this->nodes[i].p.y = this->nodes[i].p.y + 1;
    this->nodes[i].p.z = this->nodes[i].p.z + 1;
  }
}

void SWC::add_node(SWCNode n) { this->nodes.push_back(n); }

SWCNode SWC::get_node(int i) { return this->nodes[i]; }

void SWC::add_branch(Branch &branch, long connect_id) {
  if (branch.get_length() < 5) return;
  printf("Branch to add:%d\n", branch.get_length());
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
    swc_branch[i].radius = branch.get_radius_at(i);
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

  cout << "Making soma mask. radius:" << this->radius << endl;
  for (int i = 0; i < bimg->size(); i++) {
    Point<float> p(i, bimg->get_dims());
    mask1d[i] =
        (bimg->get_1d(i) > 0 && p.dist(this->centroid) < 1.5 * this->radius)
            ? 1
            : 0;
  }

  this->mask = new Image3<unsigned char>(mask1d, bimg->get_dims());
}
