#pragma once
#include <string>
namespace sigen {
class FileUtils {
public:
  // let extention = ".swc"
  // add ".swc" if fname does not end with ".swc"
  // hello -> hello.swc
  // hello.out -> hello.out.swc
  // hello.swc -> hello.swc
  static std::string AddExtension(const std::string &fname,
                                  const std::string &extention);
}; // class FileUtils
} // namespace sigen
