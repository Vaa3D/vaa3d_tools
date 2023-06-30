
#ifndef JSON_VALUE__H_
#define JSON_VALUE__H_

#include "json_error.h"
#include "json_ptr.h"
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace json {

class value;
class array;
class object;
class ptr;

using object_entry   = std::pair<std::string, value>;
using object_pointer = std::shared_ptr<object>;
using array_pointer  = std::shared_ptr<array>;

// type testing
inline bool is_string(const value &v) noexcept;
inline bool is_bool(const value &v) noexcept;
inline bool is_number(const value &v) noexcept;
inline bool is_object(const value &v) noexcept;
inline bool is_array(const value &v) noexcept;
inline bool is_null(const value &v) noexcept;

// conversion (you get a copy)
inline std::string to_string(const value &v);
inline bool to_bool(const value &v);
inline object to_object(const value &v);
inline array to_array(const value &v);

template <class T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
T to_number(const value &v);

// interpretation (you get a reference)
inline object &as_object(value &v);
inline array &as_array(value &v);
inline std::string &as_string(value &v);
inline const object &as_object(const value &v);
inline const array &as_array(const value &v);
inline const std::string &as_string(const value &v);

// does the given object have a given key?
inline bool has_key(const value &v, std::string_view key) noexcept;
inline bool has_key(const object &o, std::string_view key) noexcept;

template <class T, class>
T to_number(const value &v) {
	if (!is_number(v)) {
		JSON_THROW(invalid_type_cast());
	}

	if constexpr (std::is_same_v<T, int64_t>) {
		return stoll(as_string(v), nullptr);
	} else if constexpr (std::is_same_v<T, int32_t>) {
		return static_cast<int32_t>(stol(as_string(v), nullptr));
	} else if constexpr (std::is_same_v<T, int16_t>) {
		return static_cast<int16_t>(stol(as_string(v), nullptr));
	} else if constexpr (std::is_same_v<T, int8_t>) {
		return static_cast<int8_t>(stol(as_string(v), nullptr));
	} else if constexpr (std::is_same_v<T, uint64_t>) {
		return stoull(as_string(v), nullptr);
	} else if constexpr (std::is_same_v<T, uint32_t>) {
		return static_cast<uint32_t>(stoul(as_string(v), nullptr));
	} else if constexpr (std::is_same_v<T, uint16_t>) {
		return static_cast<uint16_t>(stoul(as_string(v), nullptr));
	} else if constexpr (std::is_same_v<T, uint8_t>) {
		return static_cast<uint8_t>(stoul(as_string(v), nullptr));
	} else if constexpr (std::is_same_v<T, double>) {
		return stod(as_string(v), nullptr);
	} else if constexpr (std::is_same_v<T, float>) {
		return stof(as_string(v), nullptr);
	} else {
		JSON_THROW(invalid_type_cast());
	}
}

/**
 * @brief The object class
 */
class object {
	friend bool operator==(const object &lhs, const object &rhs) noexcept;
	friend bool operator!=(const object &lhs, const object &rhs) noexcept;

	friend class parser;

private:
	using C = std::vector<object_entry>;

public:
	using allocator_type  = typename C::allocator_type;
	using reference       = typename C::reference;
	using const_reference = typename C::const_reference;
	using pointer         = typename C::pointer;
	using const_pointer   = typename C::const_pointer;
	using iterator        = typename C::iterator;
	using const_iterator  = typename C::const_iterator;
	using difference_type = typename C::difference_type;
	using size_type       = typename C::size_type;

public:
	object()                             = default;
	object(const object &other)          = default;
	object(object &&other)               = default;
	object &operator=(const object &rhs) = default;
	object &operator=(object &&rhs)      = default;
	object(std::initializer_list<object_entry> list);

public:
	iterator begin() noexcept { return values_.begin(); }
	iterator end() noexcept { return values_.end(); }
	const_iterator begin() const noexcept { return values_.begin(); }
	const_iterator end() const noexcept { return values_.end(); }
	const_iterator cbegin() const noexcept { return values_.begin(); }
	const_iterator cend() const noexcept { return values_.end(); }

public:
	iterator find(std::string_view s) noexcept;
	const_iterator find(std::string_view s) const noexcept;

public:
	size_type size() const noexcept {
		return values_.size();
	}

	size_type max_size() const noexcept {
		return values_.max_size();
	}

	bool empty() const noexcept {
		return values_.empty();
	}

public:
	value operator[](std::string_view key) const;
	value &operator[](std::string_view key);

	value at(std::string_view key) const;
	value &at(std::string_view key);

public:
	template <class T>
	std::pair<iterator, bool> insert(std::string key, const T &v);

	template <class T>
	std::pair<iterator, bool> insert(std::string key, T &&v);

	template <class T>
	std::pair<iterator, bool> insert(std::pair<std::string, T> &&p);

public:
	void swap(object &other) noexcept;

private:
	C values_;

	// NOTE(eteran): The values are stored in insertion order above,
	// but we use this map to have a fast lookup of key -> index
	std::map<std::string, size_t, std::less<>> index_map_;
};

inline object::iterator begin(object &obj) noexcept {
	return obj.begin();
}

inline object::iterator end(object &obj) noexcept {
	return obj.end();
}

inline object::const_iterator begin(const object &obj) noexcept {
	return obj.begin();
}

inline object::const_iterator end(const object &obj) noexcept {
	return obj.end();
}

inline object::const_iterator cbegin(const object &obj) noexcept {
	return obj.begin();
}

inline object::const_iterator cend(const object &obj) noexcept {
	return obj.end();
}

/**
 * @brief The array class
 */
class array {
	friend bool operator==(const array &lhs, const array &rhs) noexcept;
	friend bool operator!=(const array &lhs, const array &rhs) noexcept;

private:
	using C = std::vector<value>;

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
	array()                            = default;
	array(array &&other)               = default;
	array(const array &other)          = default;
	array &operator=(array &&rhs)      = default;
	array &operator=(const array &rhs) = default;
	array(std::initializer_list<value> list);

	template <class In>
	array(In first, In last) {
		values_.insert(values_.end(), first, last);
	}

public:
	iterator begin() noexcept { return values_.begin(); }
	iterator end() noexcept { return values_.end(); }
	const_iterator begin() const noexcept { return values_.begin(); }
	const_iterator end() const noexcept { return values_.end(); }
	const_iterator cbegin() const noexcept { return values_.begin(); }
	const_iterator cend() const noexcept { return values_.end(); }
	reverse_iterator rbegin() noexcept { return values_.rbegin(); }
	reverse_iterator rend() noexcept { return values_.rend(); }
	const_reverse_iterator rbegin() const noexcept { return values_.rbegin(); }
	const_reverse_iterator rend() const noexcept { return values_.rend(); }
	const_reverse_iterator crbegin() const noexcept { return values_.rbegin(); }
	const_reverse_iterator crend() const noexcept { return values_.rend(); }

public:
	size_type size() const noexcept { return values_.size(); }
	size_type max_size() const noexcept { return values_.max_size(); }
	bool empty() const noexcept { return values_.empty(); }

public:
	value operator[](std::size_t n) const;
	value &operator[](std::size_t n);
	value at(std::size_t n) const;
	value &at(std::size_t n);

public:
	template <class T>
	void push_back(T &&v) {
		values_.emplace_back(std::forward<T>(v));
	}

	template <class T>
	void push_back(const T &v) {
		values_.emplace_back(v);
	}

	void pop_back() noexcept {
		values_.pop_back();
	}

public:
	void swap(array &other) noexcept {
		using std::swap;
		swap(values_, other.values_);
	}

private:
	C values_;
};

inline array::iterator begin(array &arr) noexcept {
	return arr.begin();
}

inline array::iterator end(array &arr) noexcept {
	return arr.end();
}

inline array::const_iterator begin(const array &arr) noexcept {
	return arr.begin();
}

inline array::const_iterator end(const array &arr) noexcept {
	return arr.end();
}

inline array::const_iterator cbegin(const array &arr) noexcept {
	return arr.begin();
}

inline array::const_iterator cend(const array &arr) noexcept {
	return arr.end();
}

inline array::reverse_iterator rbegin(array &arr) noexcept {
	return arr.rbegin();
}

inline array::reverse_iterator rend(array &arr) noexcept {
	return arr.rend();
}

inline array::const_reverse_iterator rbegin(const array &arr) noexcept {
	return arr.rbegin();
}

inline array::const_reverse_iterator rend(const array &arr) noexcept {
	return arr.rend();
}

inline array::const_reverse_iterator crbegin(const array &arr) noexcept {
	return arr.rbegin();
}

inline array::const_reverse_iterator crend(const array &arr) noexcept {
	return arr.rend();
}

/**
 * @brief The value class
 */
class value {
	friend bool to_bool(const value &v);

	friend bool operator==(const value &lhs, const value &rhs);
	friend bool operator!=(const value &lhs, const value &rhs);

	friend class parser;

private:
	struct numeric_type {};
	// create a value from a numeric string, internal use only!
	value(std::string s, const numeric_type &)
		: storage_(std::move(s)), type_(type_number) {
	}

public:
	// initialize from basic types
	explicit value(const array &a);
	explicit value(const object &o);

	value(array &&a);
	value(object &&o);

	value(bool b)
		: storage_(b ? Boolean::True : Boolean::False), type_(type_boolean) {
	}

	// NOTE(eteran): we don't use string_view here because of the bool overload
	// which necessitates that we have a const char * overload to prevent value("hello")
	// from creating a "True" value. Since we need this overload anyway, no real benefit
	// to using a string_view
	value(const char *s)
		: storage_(std::string(s)), type_(type_string) {
	}

	value(std::string s)
		: storage_(std::move(s)), type_(type_string) {
	}

	template <class T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
	value(T n)
		: storage_(std::to_string(n)), type_(type_number) {
	}

	value(const std::nullptr_t &)
		: storage_(Null()), type_(type_null) {
	}

	value()
		: storage_(Null()), type_(type_null) {
	}

public:
	~value() = default;

private:
	explicit value(object_pointer o);
	explicit value(array_pointer a);

public:
	value(const value &other)
		: storage_(other.storage_), type_(other.type_) {
	}

	value(value &&other)
		: storage_(std::move(other.storage_)), type_(other.type_) {
	}

public:
	value &operator=(const value &rhs);
	value &operator=(value &&rhs);

public:
	void swap(value &other) noexcept {
		using std::swap;
		swap(storage_, other.storage_);
		swap(type_, other.type_);
	}

public:
	enum Type {
		type_null,
		type_boolean,
		type_object,
		type_array,
		type_string,
		type_number,
	};

	Type type() const noexcept { return type_; }

public:
	value operator[](std::string_view key) const;
	value operator[](std::size_t n) const;
	value &operator[](std::string_view key);
	value &operator[](std::size_t n);

public:
	inline value at(std::size_t n) const;
	inline value &at(std::size_t n);
	inline value at(std::string_view key) const;
	inline value &at(std::string_view key);

public:
	value operator[](const ptr &ptr) const;
	value &operator[](const ptr &ptr);

	value &create(const ptr &ptr);

public:
	// array like interface
	template <class T>
	void push_back(T &&v);

	template <class T>
	void push_back(const T &v);

public:
	// object like interface
	template <class T>
	std::pair<object::iterator, bool> insert(std::string key, const T &v);

	template <class T>
	std::pair<object::iterator, bool> insert(std::string key, T &&v);

	template <class T>
	std::pair<object::iterator, bool> insert(std::pair<std::string, T> &&p);

public:
	// object/array like
	size_t size() const {
		if (is_object()) {
			return as_object().size();
		} else if (is_array()) {
			return as_array().size();
		}

		JSON_THROW(invalid_type_cast());
	}

public:
	bool is_string() const noexcept {
		return (type_ == value::type_string);
	}

	bool is_bool() const noexcept {
		return (type_ == value::type_boolean);
	}

	bool is_number() const noexcept {
		return (type_ == value::type_number);
	}

	bool is_object() const noexcept {
		return (type_ == value::type_object);
	}

	bool is_array() const noexcept {
		return (type_ == value::type_array);
	}

	bool is_null() const noexcept {
		return (type_ == value::type_null);
	}

public:
	const std::string &as_string() const {

		switch (type_) {
		case value::type_string:
		case value::type_number:
			return std::get<std::string>(storage_);
		default:
			JSON_THROW(invalid_type_cast());
		}
	}

	std::string &as_string() {
		switch (type_) {
		case value::type_string:
		case value::type_number:
			return std::get<std::string>(storage_);
		default:
			JSON_THROW(invalid_type_cast());
		}
	}

	const object &as_object() const {
		if (type_ != type_object) {
			JSON_THROW(invalid_type_cast());
		}

		return *std::get<object_pointer>(storage_);
	}

	object &as_object() {
		if (type_ != type_object) {
			JSON_THROW(invalid_type_cast());
		}

		return *std::get<object_pointer>(storage_);
	}

	const array &as_array() const {
		if (type_ != type_array) {
			JSON_THROW(invalid_type_cast());
		}

		return *std::get<array_pointer>(storage_);
	}

	array &as_array() {
		if (type_ != type_array) {
			JSON_THROW(invalid_type_cast());
		}

		return *std::get<array_pointer>(storage_);
	}

private:
	struct Null {};

	enum class Boolean {
		False,
		True,
	};

	std::variant<Null, Boolean, object_pointer, array_pointer, std::string> storage_;
	Type type_;
};

inline value array::operator[](std::size_t n) const {
	return at(n);
}

inline value &array::operator[](std::size_t n) {
	return at(n);
}

inline value array::at(std::size_t n) const {
	if (n < values_.size()) {
		return values_[n];
	}

	JSON_THROW(invalid_index());
}

inline value &array::at(std::size_t n) {
	if (n < values_.size()) {
		return values_[n];
	}

	JSON_THROW(invalid_index());
}

inline std::string to_string(const value &v) {
	return as_string(v);
}

inline bool to_bool(const value &v) {
	if (!is_bool(v)) {
		JSON_THROW(invalid_type_cast());
	}

	return std::get<value::Boolean>(v.storage_) == value::Boolean::True;
}

inline object to_object(const value &v) {
	return as_object(v);
}

inline array to_array(const value &v) {
	return as_array(v);
}

inline object &as_object(array &v) {
	(void)v;
	JSON_THROW(invalid_type_cast());
}

inline array &as_array(object &v) {
	(void)v;
	JSON_THROW(invalid_type_cast());
}

inline const object &as_object(const array &v) {
	(void)v;
	JSON_THROW(invalid_type_cast());
}

inline const array &as_array(const object &v) {
	(void)v;
	JSON_THROW(invalid_type_cast());
}

inline object &as_object(value &v) {
	if (!is_object(v)) {
		JSON_THROW(invalid_type_cast());
	}

	return v.as_object();
}

inline const object &as_object(const value &v) {
	if (!is_object(v)) {
		JSON_THROW(invalid_type_cast());
	}

	return v.as_object();
}

inline array &as_array(value &v) {
	if (!is_array(v)) {
		JSON_THROW(invalid_type_cast());
	}

	return v.as_array();
}

inline const array &as_array(const value &v) {
	if (!is_array(v)) {
		JSON_THROW(invalid_type_cast());
	}

	return v.as_array();
}

const std::string &as_string(const value &v) {
	if (!is_string(v) && !is_number(v)) {
		JSON_THROW(invalid_type_cast());
	}

	return v.as_string();
}

std::string &as_string(value &v) {
	if (!is_string(v) && !is_number(v)) {
		JSON_THROW(invalid_type_cast());
	}

	return v.as_string();
}

inline bool has_key(const value &v, std::string_view key) noexcept {
	if (is_object(v)) {
		return has_key(as_object(v), key);
	}
	return false;
}

inline bool has_key(const object &o, std::string_view key) noexcept {
	return o.find(key) != o.end();
}

inline bool is_string(const value &v) noexcept {
	return v.is_string();
}

inline bool is_bool(const value &v) noexcept {
	return v.is_bool();
}

inline bool is_number(const value &v) noexcept {
	return v.is_number();
}

inline bool is_object(const value &v) noexcept {
	return v.is_object();
}

inline bool is_array(const value &v) noexcept {
	return v.is_array();
}

inline bool is_null(const value &v) noexcept {
	return v.is_null();
}

/**
 * @brief object::swap
 * @param other
 */
inline void object::swap(object &other) noexcept {
	using std::swap;
	swap(values_, other.values_);
	swap(index_map_, other.index_map_);
}

/**
 * @brief object::object
 * @param list
 */
inline object::object(std::initializer_list<object_entry> list) {

	for (auto &entry : list) {
		insert(entry.first, entry.second);
	}
}

inline value object::operator[](std::string_view key) const {
	return at(key);
}

inline value &object::operator[](std::string_view key) {
	return at(key);
}

inline object::iterator object::find(std::string_view s) noexcept {

	auto it = index_map_.find(s);
	if (it != index_map_.end()) {
		return values_.begin() + it->second;
	}

	return values_.end();
}

inline object::const_iterator object::find(std::string_view s) const noexcept {
	auto it = index_map_.find(s);
	if (it != index_map_.end()) {
		return values_.begin() + it->second;
	}

	return values_.end();
}

/**
 * @brief object::at
 * @param key
 * @return
 */
inline value object::at(std::string_view key) const {

	auto it = index_map_.find(key);
	if (it != index_map_.end()) {
		return values_[it->second].second;
	}

	JSON_THROW(invalid_index());
}

/**
 * @brief object::at
 * @param key
 * @return
 */
inline value &object::at(std::string_view key) {

	auto it = index_map_.find(key);
	if (it != index_map_.end()) {
		return values_[it->second].second;
	}

	JSON_THROW(invalid_index());
}

/**
 * @brief object::insert
 * @param p
 * @return
 */
template <class T>
auto object::insert(std::pair<std::string, T> &&p) -> std::pair<iterator, bool> {
	return insert(std::move(p.first), std::move(p.second));
}

/**
 * @brief object::insert
 * @param key
 * @param v
 * @return
 */
template <class T>
auto object::insert(std::string key, const T &v) -> std::pair<iterator, bool> {

	auto it = find(key);
	if (it != values_.end()) {
		return std::make_pair(it, false);
	}

	auto n = values_.emplace(it, std::move(key), value(v));
	index_map_.emplace(n->first, values_.size() - 1);
	return std::make_pair(n, true);
}

/**
 * @brief object::insert
 * @param key
 * @param v
 * @return
 */
template <class T>
auto object::insert(std::string key, T &&v) -> std::pair<iterator, bool> {

	auto it = find(key);
	if (it != values_.end()) {
		return std::make_pair(it, false);
	}

	auto n = values_.emplace(it, std::move(key), value(std::forward<T>(v)));
	index_map_.emplace(n->first, values_.size() - 1);
	return std::make_pair(n, true);
}

/**
 * @brief array::array
 * @param list
 */
inline array::array(std::initializer_list<value> list) {
	for (const auto &x : list) {
		values_.emplace_back(x);
	}
}

/**
 * @brief value::value
 * @param o
 */
inline value::value(object_pointer o)
	: storage_(std::move(o)), type_(type_object) {
}

/**
 * @brief value::value
 * @param a
 */
inline value::value(array_pointer a)
	: storage_(std::move(a)), type_(type_array) {
}

/**
 * @brief value::operator =
 * @param rhs
 * @return
 */
inline value &value::operator=(value &&rhs) {
	if (this != &rhs) {
		storage_ = std::move(rhs.storage_);
		type_    = std::move(rhs.type_);
	}

	return *this;
}

/**
 * @brief value::operator =
 * @param rhs
 * @return
 */
inline value &value::operator=(const value &rhs) {

	if (this != &rhs) {
		storage_ = rhs.storage_;
		type_    = rhs.type_;
	}

	return *this;
}

/**
 * @brief value::at
 * @param n
 * @return
 */
inline value value::at(std::size_t n) const {
	return as_array().at(n);
}

/**
 * @brief value::at
 * @param n
 * @return
 */
inline value &value::at(std::size_t n) {
	return as_array().at(n);
}

/**
 * @brief value::at
 * @param key
 * @return
 */
inline value value::at(std::string_view key) const {
	return as_object().at(key);
}

/**
 * @brief value::at
 * @param key
 * @return
 */
inline value &value::at(std::string_view key) {
	return as_object().at(key);
}

/**
 * @brief value::operator []
 * @param key
 * @return
 */
inline value value::operator[](std::string_view key) const {
	return as_object()[key];
}

/**
 * @brief value::operator []
 * @param n
 * @return
 */
inline value value::operator[](std::size_t n) const {
	return as_array()[n];
}

/**
 * @brief value::operator []
 * @param key
 * @return
 */
inline value &value::operator[](std::string_view key) {
	return as_object()[key];
}

/**
 * @brief value::operator []
 * @param n
 * @return
 */
inline value &value::operator[](std::size_t n) {
	return as_array()[n];
}

inline value value::operator[](const ptr &ptr) const {

	// this cast makes sure we don't get references to temps along the way
	// but the final return will create a copy
	value *result = const_cast<value *>(this);
	for (const std::string &ref : ptr) {

		if (result->is_object()) {
			result = &result->at(ref);
		} else if (result->is_array()) {

			if (ref == "-") {
				result->push_back(value());
				result = &result->at(result->size() - 1);
			} else {
				std::size_t n = std::stoul(ref);
				result        = &result->at(n);
			}
		} else {
			JSON_THROW(invalid_path());
		}
	}

	return *result;
}

inline value &value::operator[](const ptr &ptr) {

	value *result = this;
	for (const std::string &ref : ptr) {

		if (result->is_object()) {
			result = &result->at(ref);
		} else if (result->is_array()) {
			if (ref == "-") {
				result->push_back(value());
				result = &result->at(result->size() - 1);
			} else {
				std::size_t n = std::stoul(ref);
				result        = &result->at(n);
			}
		} else {
			JSON_THROW(invalid_path());
		}
	}

	return *result;
}

inline value &value::create(const ptr &ptr) {
	value *result = this;
	for (const std::string &ref : ptr) {

		if (result->is_object()) {
			if (!has_key(result, ref)) {
				result->insert(ref, object());
			}
			result = &result->at(ref);
		} else if (result->is_array()) {
			if (ref == "-") {
				result->push_back(value());
				result = &result->at(result->size() - 1);
			} else {
				std::size_t n = std::stoul(ref);
				result        = &result->at(n);
			}
		} else {
			JSON_THROW(invalid_path());
		}
	}

	return *result;
}

/**
 * @brief value::value
 * @param a
 */
inline value::value(const array &a)
	: type_(type_array) {
	storage_ = std::make_shared<array>(a);
}

/**
 * @brief value::value
 * @param o
 */
inline value::value(const object &o)
	: type_(type_object) {
	storage_ = std::make_shared<object>(o);
}

/**
 * @brief value::value
 * @param a
 */
inline value::value(array &&a)
	: type_(type_array) {
	storage_ = std::make_shared<array>(std::move(a));
}

/**
 * @brief value::value
 * @param o
 */
inline value::value(object &&o)
	: type_(type_object) {
	storage_ = std::make_shared<object>(std::move(o));
}

/**
 * @brief operator ==
 * @param lhs
 * @param rhs
 * @return
 */
inline bool operator==(const value &lhs, const value &rhs) {
	if (lhs.type_ == rhs.type_) {
		switch (lhs.type_) {
		case value::type_string:
			return as_string(lhs) == as_string(rhs);
		case value::type_number:
			// NOTE(eteran): this is kinda a "best effort", we can't compare the string
			// contents because things like 1e5 equals 100000
			return to_number<double>(lhs) == to_number<double>(rhs);
		case value::type_null:
			return true;
		case value::type_boolean:
			return to_bool(lhs) == to_bool(rhs);
		case value::type_array:
			return as_array(lhs) == as_array(rhs);
		case value::type_object:
			return as_object(lhs) == as_object(rhs);
		}
	}
	return false;
}

/**
 * @brief operator !=
 * @param lhs
 * @param rhs
 * @return
 */
inline bool operator!=(const value &lhs, const value &rhs) {
	return !(lhs == rhs);
}

/**
 * @brief operator ==
 * @param lhs
 * @param rhs
 * @return
 */
inline bool operator==(const object &lhs, const object &rhs) noexcept {
	if (lhs.values_.size() == rhs.values_.size()) {
		return lhs.values_ == rhs.values_;
	}
	return false;
}

/**
 * @brief operator !=
 * @param lhs
 * @param rhs
 * @return
 */
inline bool operator!=(const object &lhs, const object &rhs) noexcept {
	return !(lhs == rhs);
}

/**
 * @brief operator ==
 * @param lhs
 * @param rhs
 * @return
 */
inline bool operator==(const array &lhs, const array &rhs) noexcept {
	if (lhs.values_.size() == rhs.values_.size()) {
		return lhs.values_ == rhs.values_;
	}
	return false;
}

/**
 * @brief operator !=
 * @param lhs
 * @param rhs
 * @return
 */
inline bool operator!=(const array &lhs, const array &rhs) noexcept {
	return !(lhs == rhs);
}

template <class T>
void value::push_back(T &&v) {
	as_array().push_back(std::forward<T>(v));
}

template <class T>
void value::push_back(const T &v) {
	as_array().push_back(v);
}

template <class T>
std::pair<object::iterator, bool> value::insert(std::string key, const T &v) {
	return as_object().insert(std::move(key), v);
}

template <class T>
std::pair<object::iterator, bool> value::insert(std::string key, T &&v) {
	return as_object().insert(std::move(key), std::forward<T>(v));
}

template <class T>
std::pair<object::iterator, bool> value::insert(std::pair<std::string, T> &&p) {
	return as_object().insert(std::forward<T>(p));
}
}

#endif
