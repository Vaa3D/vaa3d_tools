#pragma once
namespace sigen {
// http://faithandbrave.hateblo.jp/entry/20080620/1213951924
struct noncopyable {
  noncopyable() = default;
  noncopyable(const noncopyable &) = delete;
  noncopyable &operator=(const noncopyable &) = delete;
};
};
