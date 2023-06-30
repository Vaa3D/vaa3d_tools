
#ifndef JSON_ERROR_H_
#define JSON_ERROR_H_

#include <cstddef>

#if defined(__EXCEPTIONS)
#define JSON_THROW(x) throw x
#else
#include <cassert>
#define JSON_THROW(...) assert(0)
#endif

namespace json {

// general error
struct exception {};

// parsing errors
struct brace_expected : exception {};
struct bracket_expected : exception {};
struct colon_expected : exception {};
struct string_expected : exception {};
struct value_expected : exception {};

// lexing errors
struct lexing_error : exception {
	lexing_error(size_t index)
		: index_(index) {}

	size_t index() const {
		return index_;
	}

private:
	size_t index_ = 0;
};
struct invalid_unicode_character : lexing_error {
	invalid_unicode_character(size_t index)
		: lexing_error(index) {}
};

struct utf16_surrogate_expected : lexing_error {
	utf16_surrogate_expected(size_t index)
		: lexing_error(index) {}
};
struct quote_expected : lexing_error {
	quote_expected(size_t index)
		: lexing_error(index) {}
};
struct invalid_number : lexing_error {
	invalid_number(size_t index)
		: lexing_error(index) {}
};

// serialization errors
struct invalid_utf8_string : exception {};

// usage errors
struct invalid_type_cast : exception {};
struct invalid_index : exception {};

// pointer errors
struct invalid_path : exception {};
struct invalid_pointer_syntax : exception {};
}

#endif
