
#ifndef JSON_PTR_H_
#define JSON_PTR_H_

#include "json_reader.h"
#include "json_detail.h"
#include "json_error.h"
#include <string>
#include <string_view>
#include <vector>

namespace json {

class value;

/**
 * @brief The ptr class
 */
class ptr {
private:
	using C = std::vector<std::string>;

public:
	using allocator_type         = typename C::allocator_type;
	using reference              = typename C::reference;
	using const_reference        = typename C::const_reference;
	using pointer                = typename C::pointer;
	using const_pointer          = typename C::const_pointer;
	using iterator               = typename C::iterator;
	using const_iterator         = typename C::const_iterator;
	using reverse_iterator       = typename C::reverse_iterator;
	using const_reverse_iterator = typename C::const_reverse_iterator;
	using difference_type        = typename C::difference_type;
	using size_type              = typename C::size_type;

public:
	explicit ptr(std::string_view path) {

		if (path.empty() || path == "#") {
			return;
		}

		reader input(path);

		// normal or URI fragment notation?
		const bool uri_format = input.match('#');

		if (!input.match('/')) {
			JSON_THROW(invalid_pointer_syntax());
		}

		std::string reference_token;
		while (!input.eof()) {

			if (!uri_format) {
				if (input.match("~0")) {
					reference_token.push_back('~');
				} else if (input.match("~1")) {
					reference_token.push_back('/');
				} else if (input.match("/")) {
					path_.push_back(reference_token);
					reference_token.clear();
				} else if (input.match("~")) {
					JSON_THROW(invalid_pointer_syntax());
				} else {
					reference_token.push_back(input.read());
				}
			} else {
				static const std::regex hex_regex(R"(%[0-9A-Fa-f]{2})");
				if (input.match("~0")) {
					reference_token.push_back('~');
				} else if (input.match("~1")) {
					reference_token.push_back('/');
				} else if (input.match("/")) {
					path_.push_back(reference_token);
					reference_token.clear();
				} else if (input.match("~")) {
					JSON_THROW(invalid_pointer_syntax());
				} else if (auto hex_value = input.match(hex_regex)) {
					// %XX -> char(0xXX)
					reference_token.push_back(static_cast<char>((detail::to_hex(hex_value->data()[1]) << 4) | (detail::to_hex(hex_value->data()[2]))));
				} else if (input.match("%")) {
					JSON_THROW(invalid_pointer_syntax());
				} else {
					reference_token.push_back(input.read());
				}
			}
		}

		path_.push_back(reference_token);
	}

public:
	ptr()                          = default;
	ptr(ptr &&other)               = default;
	ptr(const ptr &other)          = default;
	ptr &operator=(ptr &&rhs)      = default;
	ptr &operator=(const ptr &rhs) = default;

public:
	iterator begin() noexcept { return path_.begin(); }
	iterator end() noexcept { return path_.end(); }
	const_iterator begin() const noexcept { return path_.begin(); }
	const_iterator end() const noexcept { return path_.end(); }
	const_iterator cbegin() const noexcept { return path_.begin(); }
	const_iterator cend() const noexcept { return path_.end(); }
	reverse_iterator rbegin() noexcept { return path_.rbegin(); }
	reverse_iterator rend() noexcept { return path_.rend(); }
	const_reverse_iterator rbegin() const noexcept { return path_.rbegin(); }
	const_reverse_iterator rend() const noexcept { return path_.rend(); }
	const_reverse_iterator crbegin() const noexcept { return path_.rbegin(); }
	const_reverse_iterator crend() const noexcept { return path_.rend(); }

public:
	size_type size() const noexcept { return path_.size(); }
	size_type max_size() const noexcept { return path_.max_size(); }
	bool empty() const noexcept { return path_.empty(); }

public:
	value operator[](std::size_t n) const;
	value &operator[](std::size_t n);
	value at(std::size_t n) const;
	value &at(std::size_t n);

private:
	C path_;
};

}

#endif
