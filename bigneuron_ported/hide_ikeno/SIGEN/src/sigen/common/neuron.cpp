#include "sigen/common/neuron.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include <cassert>
#include <map>
#include <set>
namespace sigen {

void NeuronNode::RemoveConnections(const std::set<int> &nodes) {
  // `std::set.erase` invalidates iterator. Be careful.
  // http://qiita.com/satoruhiga/items/fa6eae09c9d89bd48b5d
  std::set<NeuronNode *>::iterator it = adjacent_.begin();
  while (it != adjacent_.end()) {
    if (nodes.count((*it)->id_)) {
      adjacent_.erase(it++);
    } else {
      ++it;
    }
  }
}

Neuron Neuron::Clone() const {
  assert(this->root_ != NULL);
  Neuron ret;
  std::map<NeuronNode *, int> ptr_to_index;
  for (int i = 0; i < (int)this->storage_.size(); ++i) {
    NeuronNodePtr ptr = this->storage_[i];
    ptr_to_index[ptr.get()] = i;
    ret.storage_.push_back(ptr->Clone());
  }

  for (int i = 0; i < (int)this->storage_.size(); ++i) {
    BOOST_FOREACH (NeuronNode *adj, this->storage_[i].get()->adjacent_) {
      NeuronNode *p = ret.storage_[ptr_to_index[adj]].get();
      ret.storage_[i]->AddConnection(p);
    }
  }

  ret.UpdateRoot(ptr_to_index[this->root_]);
  return ret;
}
} // namespace sigen
