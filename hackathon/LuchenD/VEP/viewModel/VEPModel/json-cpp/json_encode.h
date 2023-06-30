
#ifndef JSON_ENCODE_H_
#define JSON_ENCODE_H_

#include "json_value.h"
#include <ostream>
#include <sstream>
#include <string>

namespace json {

constexpr int IndentWidth = 4;

// convert a value to a JSON string
enum Options {
	None          = 0x00,
	EscapeUnicode = 0x01,
	PrettyPrint   = 0x02,
};

constexpr inline Options operator&(Options lhs, Options rhs) noexcept {
	using T = std::underlying_type<Options>::type;
	return static_cast<Options>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

constexpr inline Options operator|(Options lhs, Options rhs) noexcept {
	using T = std::underlying_type<Options>::type;
	return static_cast<Options>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template <class T, class = typename std::enable_if<std::is_same<T, value>::value || std::is_same<T, object>::value || std::is_same<T, array>::value>::type>
std::string stringify(const T &v, Options options = Options::None);

template <class T, class = typename std::enable_if<std::is_same<T, value>::value || std::is_same<T, object>::value || std::is_same<T, array>::value>::type>
void stringify(std::ostream &os, const T &v, Options options = Options::None);

namespace detail {

inline std::string escape_string(std::string_view s, Options options) {

	std::string r;
	r.reserve(s.size());

	if (options & Options::EscapeUnicode) {
		struct state_t {
			unsigned int
				expected : 4,
				seen : 4,
				reserved : 24;
		};

		state_t shift_state = {0, 0, 0};
		char32_t result     = 0;

		for (auto it = s.begin(); it != s.end(); ++it) {

			const auto ch = static_cast<uint8_t>(*it);

			if (shift_state.seen == 0) {

				if ((ch & 0x80) == 0) {
					switch (ch) {
					case '\"':
						r += "\\\"";
						break;
					case '\\':
						r += "\\\\";
						break;
#if 0
					case '/':  r += "\\/"; break;
#endif
					case '\b':
						r += "\\b";
						break;
					case '\f':
						r += "\\f";
						break;
					case '\n':
						r += "\\n";
						break;
					case '\r':
						r += "\\r";
						break;
					case '\t':
						r += "\\t";
						break;
					default:
						if (!isprint(ch)) {
							r += "\\u";
							char buf[5];
							snprintf(buf, sizeof(buf), "%04X", ch);
							r += buf;
						} else {
							r += static_cast<char>(ch);
						}
						break;
					}
				} else if ((ch & 0xe0) == 0xc0) {
					// 2 byte
					result               = ch & 0x1f;
					shift_state.expected = 2;
					shift_state.seen     = 1;
				} else if ((ch & 0xf0) == 0xe0) {
					// 3 byte
					result               = ch & 0x0f;
					shift_state.expected = 3;
					shift_state.seen     = 1;
				} else if ((ch & 0xf8) == 0xf0) {
					// 4 byte
					result               = ch & 0x07;
					shift_state.expected = 4;
					shift_state.seen     = 1;
				} else if ((ch & 0xfc) == 0xf8) {
					// 5 byte
					JSON_THROW(invalid_utf8_string()); // Restricted by RFC 3629
				} else if ((ch & 0xfe) == 0xfc) {
					// 6 byte
					JSON_THROW(invalid_utf8_string()); // Restricted by RFC 3629
				} else {
					JSON_THROW(invalid_utf8_string()); // should never happen
				}
			} else if (shift_state.seen < shift_state.expected) {
				if ((ch & 0xc0) == 0x80) {
					result <<= 6;
					result |= ch & 0x3f;
					// increment the shift state
					++shift_state.seen;

					if (shift_state.seen == shift_state.expected) {
						// done with this character

						char buf[5];

						if (result < 0xd800 || (result >= 0xe000 && result < 0x10000)) {
							r += "\\u";
							snprintf(buf, sizeof(buf), "%04X", result);
							r += buf;
						} else {
							result = (result - 0x10000);

							r += "\\u";
							snprintf(buf, sizeof(buf), "%04X", 0xd800 + ((result >> 10) & 0x3ff));
							r += buf;

							r += "\\u";
							snprintf(buf, sizeof(buf), "%04X", 0xdc00 + (result & 0x3ff));
							r += buf;
						}

						shift_state.seen     = 0;
						shift_state.expected = 0;
						result               = 0;
					}

				} else {
					JSON_THROW(invalid_utf8_string()); // should never happen
				}
			} else {
				JSON_THROW(invalid_utf8_string()); // should never happen
			}
		}
	} else {

		for (char ch : s) {

			switch (ch) {
			case '\"':
				r += "\\\"";
				break;
			case '\\':
				r += "\\\\";
				break;
#if 0
			case '/':  r += "\\/"; break;
#endif
			case '\b':
				r += "\\b";
				break;
			case '\f':
				r += "\\f";
				break;
			case '\n':
				r += "\\n";
				break;
			case '\r':
				r += "\\r";
				break;
			case '\t':
				r += "\\t";
				break;
			default:
				r += ch;
				break;
			}
		}
	}
	return r;
}

inline std::string escape_string(std::string_view s) {
	return escape_string(s, Options::None);
}

// pretty print as a string
inline void value_to_string(std::ostream &os, const value &v, Options options, int indent, bool ignore_initial_ident);

inline void value_to_string(std::ostream &os, const object &o, Options options, int indent, bool ignore_initial_ident) {

	if (!ignore_initial_ident) {
		os << std::string(indent * IndentWidth, ' ');
	}

	if (o.empty()) {
		os << "{}";
	} else {
		os << "{\n";

		auto it = o.begin();
		auto e  = o.end();

		++indent;
		os << std::string(indent * IndentWidth, ' ') << '"' << escape_string(it->first, options) << "\" : ";
		value_to_string(os, it->second, options, indent, true);

		++it;
		for (; it != e; ++it) {
			os << ',';
			os << '\n';
			os << std::string(indent * IndentWidth, ' ') << '"' << escape_string(it->first, options) << "\" : ";
			value_to_string(os, it->second, options, indent, true);
		}
		--indent;

		os << "\n";
		os << std::string(indent * IndentWidth, ' ') << "}";
	}
}

inline void value_to_string(std::ostream &os, const array &a, Options options, int indent, bool ignore_initial_ident) {

	if (!ignore_initial_ident) {
		os << std::string(indent * IndentWidth, ' ');
	}

	if (a.empty()) {
		os << "[]";
	} else {
		os << "[\n";

		auto it = a.begin();
		auto e  = a.end();

		++indent;
		value_to_string(os, *it++, options, indent, false);

		for (; it != e; ++it) {
			os << ',';
			os << '\n';
			value_to_string(os, *it, options, indent, false);
		}
		--indent;

		os << "\n";
		os << std::string(indent * IndentWidth, ' ') << "]";
	}
}

inline void value_to_string(std::ostream &os, const value &v, Options options, int indent, bool ignore_initial_ident) {

	if (!ignore_initial_ident) {
		os << std::string(indent * IndentWidth, ' ');
	}

	switch (v.type()) {
	case value::type_string:
		os << '"' << escape_string(as_string(v), options) << '"';
		break;
	case value::type_number:
		os << as_string(v);
		break;
	case value::type_null:
		os << "null";
		break;
	case value::type_boolean:
		os << (to_bool(v) ? "true" : "false");
		break;
	case value::type_object:
		value_to_string(os, as_object(v), options, indent, true);
		break;
	case value::type_array:
		value_to_string(os, as_array(v), options, indent, true);
		break;
	}
}

inline std::string value_to_string(const value &v, Options options, int indent, bool ignore_initial_ident) {

	std::stringstream ss;
	value_to_string(ss, v, options, indent, ignore_initial_ident);
	return ss.str();
}

inline std::string value_to_string(const value &v, Options options) {
	return value_to_string(v, options, 0, false);
}

inline void value_to_string(std::ostream &os, const value &v, Options options) {
	value_to_string(os, v, options, 0, false);
}

// serialize, not pretty printed
inline void serialize(std::ostream &os, const value &v, Options options);

inline void serialize(std::ostream &os, const array &a, Options options) {
	os << "[";
	if (!a.empty()) {
		auto it = a.begin();
		auto e  = a.end();

		serialize(os, *it++, options);

		for (; it != e; ++it) {
			os << ',';
			serialize(os, *it, options);
		}
	}
	os << "]";
}

inline void serialize(std::ostream &os, const object &o, Options options) {
	os << "{";
	if (!o.empty()) {
		auto it = o.begin();
		auto e  = o.end();

		os << '"' << escape_string(it->first, options) << "\":";
		serialize(os, it->second, options);
		++it;
		for (; it != e; ++it) {
			os << ',';
			os << '"' << escape_string(it->first, options) << "\":";
			serialize(os, it->second, options);
		}
	}
	os << "}";
}

inline void serialize(std::ostream &os, const value &v, Options options) {

	switch (v.type()) {
	case value::type_string:
		os << '"' << escape_string(as_string(v), options) << '"';
		break;
	case value::type_number:
		os << as_string(v);
		break;
	case value::type_null:
		os << "null";
		break;
	case value::type_boolean:
		os << (to_bool(v) ? "true" : "false");
		break;
	case value::type_object:
		serialize(os, as_object(v), options);
		break;
	case value::type_array:
		serialize(os, as_array(v), options);
		break;
	}
}

template <class T, class = typename std::enable_if<std::is_same<T, value>::value || std::is_same<T, object>::value || std::is_same<T, array>::value>::type>
std::string serialize(const T &v, Options options) {
	std::stringstream ss;

	std::locale c_locale("C");
	ss.imbue(c_locale);

	serialize(ss, v, options);
	return ss.str();
}

template <class T, class = typename std::enable_if<std::is_same<T, value>::value || std::is_same<T, object>::value || std::is_same<T, array>::value>::type>
std::string pretty_print(const T &v, Options options) {
	return value_to_string(value(v), options);
}

template <class T, class = typename std::enable_if<std::is_same<T, value>::value || std::is_same<T, object>::value || std::is_same<T, array>::value>::type>
void pretty_print(std::ostream &os, const T &v, Options options) {
	value_to_string(os, value(v), options);
}

}

template <class T, class>
std::string stringify(const T &v, Options options) {
	if (options & Options::PrettyPrint) {
		return detail::pretty_print(v, options);
	} else {
		return detail::serialize(v, options);
	}
}

template <class T, class>
void stringify(std::ostream &os, const T &v, Options options) {

	std::locale c_locale("C");
	os.imbue(c_locale);

	if (options & Options::PrettyPrint) {
		detail::pretty_print(os, v, options);
	} else {
		detail::serialize(os, v, options);
	}
}

}

#endif
