#pragma once
#include <boost/variant.hpp>
#include <string>
namespace sigen {
typedef boost::variant<bool, int, double, std::string> Variant;
}
