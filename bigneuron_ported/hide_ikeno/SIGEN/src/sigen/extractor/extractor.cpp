#include "sigen/extractor/extractor.h"
#include "sigen/common/point.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <cassert>
#include <iterator>
#include <map>
#include <queue>
#include <utility>
#include <vector>
namespace sigen {

static void clearFrame(BinaryCube &c) {
  for (int i = 0; i < c.x_; ++i) {
    for (int j = 0; j < c.y_; ++j) {
      c[i][j][0] = false;
      c[i][j][c.z_ - 1] = false;
    }
  }
  for (int j = 0; j < c.y_; ++j) {
    for (int k = 0; k < c.z_; ++k) {
      c[0][j][k] = false;
      c[c.x_ - 1][j][k] = false;
    }
  }
  for (int k = 0; k < c.z_; ++k) {
    for (int i = 0; i < c.x_; ++i) {
      c[i][0][k] = false;
      c[i][c.y_ - 1][k] = false;
    }
  }
}

static void removeIsolatedPoints(BinaryCube &c) {
  BinaryCube cc = c;
  for (int x = 1; x < c.x_ - 1; ++x) {
    for (int y = 1; y < c.y_ - 1; ++y) {
      for (int z = 1; z < c.z_ - 1; ++z) {
        bool any = false;
        for (int dx = -1; dx <= 1; ++dx) {
          for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
              if (dx == 0 && dy == 0 && dz == 0)
                continue;
              if (cc[x + dx][y + dy][z + dz])
                any = true;
            }
          }
        }
        if (!any) {
          c[x][y][z] = false;
        }
      }
    }
  }
}

static void beforeFilter(BinaryCube &c) {
  clearFrame(c);
  removeIsolatedPoints(c);
}

static void setLabel(Voxel *p, const int label) {
  p->flag_ = true;
  p->label_ = label;
  BOOST_FOREACH (Voxel *next, p->adjacent_) {
    if (next->flag_ == false) {
      setLabel(next, label);
    }
  }
}

template <class T>
class compareSize {
public:
  bool operator()(const T &lhs, const T &rhs) const {
    return lhs.size() < rhs.size();
  }
};

// This functions is HOT SPOT.
// This is worth to tune.
void Extractor::Labeling() {
  beforeFilter(cube_);
  std::map<IPoint, VoxelPtr> voxels;
  for (int x = 1; x < cube_.x_ - 1; ++x) {
    for (int y = 1; y < cube_.y_ - 1; ++y) {
      for (int z = 1; z < cube_.z_ - 1; ++z) {
        if (cube_[x][y][z]) {
          voxels[IPoint(x, y, z)] = boost::make_shared<Voxel>(x, y, z);
        }
      }
    }
  }
  typedef std::pair<IPoint, VoxelPtr> iter_type;
  BOOST_FOREACH (iter_type p, voxels) {
    // enumerate 26 neighbors
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dz = -1; dz <= 1; ++dz) {
          if (dx == 0 && dy == 0 && dz == 0)
            continue;
          int x = p.first.x_ + dx;
          int y = p.first.y_ + dy;
          int z = p.first.z_ + dz;
          IPoint t(x, y, z);
          if (voxels.count(t)) {
            p.second->AddConection(voxels[t]);
          }
        }
      }
    }
  }
  // reset flags
  BOOST_FOREACH (iter_type p, voxels)
    p.second->flag_ = false;
  // set flags
  int label = 0;
  BOOST_FOREACH (iter_type p, voxels) {
    if (p.second->flag_ == false) {
      setLabel(p.second.get(), label++);
    }
  }
  components_.assign(label, std::vector<VoxelPtr>());
  BOOST_FOREACH (iter_type p, voxels) {
    components_[p.second->label_].push_back(p.second);
  }
  std::sort(components_.begin(), components_.end(), compareSize<std::vector<VoxelPtr> >());
  std::reverse(components_.begin(), components_.end());
}

static void resetFlag(std::vector<VoxelPtr> &voxels) {
  BOOST_FOREACH (VoxelPtr p, voxels)
    p->flag_ = false;
}

static Voxel *findSingleSeed(std::vector<VoxelPtr> &group) {
  assert(!group.empty());
  Voxel *last = group[0].get();
  for (int i = 0; i < 2; ++i) {
    resetFlag(group);
    last->flag_ = true;
    std::queue<Voxel *> que;
    que.push(last);
    while (!que.empty()) {
      last = que.front();
      que.pop();
      BOOST_FOREACH (Voxel *next, last->adjacent_) {
        if (!next->flag_) {
          next->flag_ = true;
          que.push(next);
        }
      }
    }
  }
  return last;
}

static void setDistance(std::vector<VoxelPtr> &group, Voxel *seed) {
  resetFlag(group);
  std::queue<Voxel *> que;
  seed->flag_ = true;
  seed->label_ = 0;
  que.push(seed);
  while (!que.empty()) {
    Voxel *p = que.front();
    que.pop();
    BOOST_FOREACH (Voxel *next, p->adjacent_) {
      if (!next->flag_) {
        next->flag_ = true;
        next->label_ = p->label_ + 1;
        que.push(next);
      }
    }
  }
}

static std::vector<Voxel *> extractSameDistance(Voxel *seed) {
  std::vector<Voxel *> ret;
  std::queue<Voxel *> que;
  seed->flag_ = true;
  ret.push_back(seed);
  que.push(seed);
  while (!que.empty()) {
    Voxel *p = que.front();
    que.pop();
    BOOST_FOREACH (Voxel *next, p->adjacent_) {
      if (!next->flag_ && next->label_ == seed->label_) {
        next->flag_ = true;
        ret.push_back(next);
        que.push(next);
      }
    }
  }
  return ret;
}

static std::vector<IPoint> voxelsToPoints(const std::vector<Voxel *> &vs) {
  std::vector<IPoint> ps;
  BOOST_FOREACH (Voxel *v, vs) {
    ps.push_back(IPoint(v->x_, v->y_, v->z_));
  }
  return ps;
}

std::vector<ClusterPtr> Extractor::Extract() {
  Labeling();
  std::vector<ClusterPtr> ret;
  // NOT const
  BOOST_FOREACH (std::vector<VoxelPtr> &group, components_) {
    Voxel *seed = findSingleSeed(group);
    setDistance(group, seed);
    resetFlag(group);
    BOOST_FOREACH (VoxelPtr p, group) {
      if (p->flag_ == false) {
        std::vector<Voxel *> vs = extractSameDistance(p.get());
        std::vector<IPoint> ps = voxelsToPoints(vs);
        assert(!ps.empty());
        ret.push_back(boost::make_shared<Cluster>(ps));
      }
    }
  }
  return ret;
}
} // namespace sigen
