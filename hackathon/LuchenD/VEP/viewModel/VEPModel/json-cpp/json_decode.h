
#ifndef JSON_DECODE_H_
#define JSON_DECODE_H_

/* TODO(eteran): support unicode
	00 00 00 xx  UTF-32BE
	00 xx 00 xx  UTF-16BE
	xx 00 00 00  UTF-32LE
	xx 00 xx 00  UTF-16LE
	xx xx xx xx  UTF-8
*/

#include "json_detail.h"
#include "json_error.h"
#include "json_reader.h"
#include "json_value.h"

namespace json {

/**
 * @brief The parser class
 */
class parser {
public:
	enum token_type {
		ArrayBegin,
		ArrayEnd,
		Colon,
		Comma,
		ObjectBegin,
		ObjectEnd,
		Quote,
		Number,
		String,
		LiteralTrue,
		LiteralFalse,
		LiteralNull,
		End,
	};

	struct token {
		token_type type;
		std::string value;
		size_t index;
	};

private:
	token next_token() {
		// consume whitespace
		while (detail::is_space(reader_.peek())) {
			reader_.read();
		}

		const size_t token_start = reader_.index();

		// if we reach end of the stream, we're done!
		if (reader_.eof()) {
			return token{token_type::End, "", token_start};
		}

		// extract the next token...
		if (reader_.match('[')) {
			return token{token_type::ArrayBegin, "[", token_start};
		} else if (reader_.match(']')) {
			return token{token_type::ArrayEnd, "]", token_start};
		} else if (reader_.match(':')) {
			return token{token_type::Colon, ":", token_start};
		} else if (reader_.match(',')) {
			return token{token_type::Comma, ",", token_start};
		} else if (reader_.match('{')) {
			return token{token_type::ObjectBegin, "{", token_start};
		} else if (reader_.match('}')) {
			return token{token_type::ObjectEnd, "}", token_start};
		} else if (reader_.match("true")) {
			return token{token_type::LiteralTrue, "true", token_start};
		} else if (reader_.match("false")) {
			return token{token_type::LiteralFalse, "false", token_start};
		} else if (reader_.match("null")) {
			return token{token_type::LiteralNull, "null", token_start};
		} else if (reader_.match('"')) {

			std::string s;
			std::back_insert_iterator<std::string> out = back_inserter(s);

			while (reader_.peek() != '"' && reader_.peek() != '\n') {

				char ch = reader_.read();
				if (ch == '\\') {
					switch (reader_.read()) {
					case '"':
						*out++ = '"';
						break;
					case '\\':
						*out++ = '\\';
						break;
					case '/':
						*out++ = '/';
						break;
					case 'b':
						*out++ = '\b';
						break;
					case 'f':
						*out++ = '\f';
						break;
					case 'n':
						*out++ = '\n';
						break;
					case 'r':
						*out++ = '\r';
						break;
					case 't':
						*out++ = '\t';
						break;
					case 'u': {
						// convert \uXXXX escape sequences to UTF-8
						char hex[4];

						if (!std::isxdigit(hex[0] = reader_.read())) JSON_THROW(invalid_unicode_character(reader_.index()));
						if (!std::isxdigit(hex[1] = reader_.read())) JSON_THROW(invalid_unicode_character(reader_.index()));
						if (!std::isxdigit(hex[2] = reader_.read())) JSON_THROW(invalid_unicode_character(reader_.index()));
						if (!std::isxdigit(hex[3] = reader_.read())) JSON_THROW(invalid_unicode_character(reader_.index()));

						uint16_t w1 = 0;
						uint16_t w2 = 0;

						w1 |= (detail::to_hex(hex[0]) << 12);
						w1 |= (detail::to_hex(hex[1]) << 8);
						w1 |= (detail::to_hex(hex[2]) << 4);
						w1 |= (detail::to_hex(hex[3]));

						if ((w1 & 0xfc00) == 0xdc00) {
							JSON_THROW(invalid_unicode_character(reader_.index()));
						}

						if ((w1 & 0xfc00) == 0xd800) {
							// part of a surrogate pair
							if (!reader_.match("\\u")) {
								JSON_THROW(utf16_surrogate_expected(reader_.index()));
							}

							// convert \uXXXX escape sequences for surrogate pairs to UTF-8
							if (!std::isxdigit(hex[0] = reader_.read())) JSON_THROW(invalid_unicode_character(reader_.index()));
							if (!std::isxdigit(hex[1] = reader_.read())) JSON_THROW(invalid_unicode_character(reader_.index()));
							if (!std::isxdigit(hex[2] = reader_.read())) JSON_THROW(invalid_unicode_character(reader_.index()));
							if (!std::isxdigit(hex[3] = reader_.read())) JSON_THROW(invalid_unicode_character(reader_.index()));

							w2 |= (detail::to_hex(hex[0]) << 12);
							w2 |= (detail::to_hex(hex[1]) << 8);
							w2 |= (detail::to_hex(hex[2]) << 4);
							w2 |= (detail::to_hex(hex[3]));
						}

						detail::surrogate_pair_to_utf8(w1, w2, out);
						break;
					}
					default:
						*out++ = '\\';
						break;
					}
				} else {
					*out++ = ch;
				}
			}

			if (!reader_.match('"')) {
				JSON_THROW(quote_expected(reader_.index()));
			}

			return token{token_type::String, std::move(s), token_start};

		} else {

			std::string s;
			s.reserve(10);
			std::back_insert_iterator<std::string> out = back_inserter(s);

			// JSON numbers fit the regex: -?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?

			// -?
			if (reader_.match('-')) {
				*out++ = '-';
			}

			// (0|[1-9][0-9]*)
			if (reader_.match('0')) {
				*out++ = '0';
			} else {
				if (!std::isdigit(reader_.peek())) {
					JSON_THROW(invalid_number(reader_.index()));
				}

				while (std::isdigit(reader_.peek())) {
					*out++ = reader_.read();
				}
			}

			// (\.[0-9]+)?
			if (reader_.match('.')) {
				*out++ = '.';
				if (!std::isdigit(reader_.peek())) {
					JSON_THROW(invalid_number(reader_.index()));
				}

				while (std::isdigit(reader_.peek())) {
					*out++ = reader_.read();
				}
			}

			// ([eE][+-]?[0-9]+)?
			if (reader_.peek() == 'e' || reader_.peek() == 'E') {
				*out++ = reader_.read();
				if (reader_.peek() == '+' || reader_.peek() == '-') {
					*out++ = reader_.read();
				}

				if (!std::isdigit(reader_.peek())) {
					JSON_THROW(invalid_number(reader_.index()));
				}

				while (std::isdigit(reader_.peek())) {
					*out++ = reader_.read();
				}
			}

			return token{token_type::Number, std::move(s), token_start};
		}
	}

public:
	parser(std::string_view s)
		: reader_(s) {
	}

public:
	value parse() {
		return get_value();
	}

private:
	array_pointer get_array() {

		auto arr = std::make_unique<array>();

		token next = next_token();

		if (next.type != token_type::ArrayEnd) {

			while (true) {
				arr->push_back(get_value(next));
				next = next_token();

				if (next.type != token_type::Comma) {
					break;
				}

				next = next_token();
			}

			if (next.type != token_type::ArrayEnd) {
				JSON_THROW(bracket_expected());
			}
		}

		return arr;
	}

	object_pointer get_object() {

		auto obj = std::make_unique<object>();

		token next = next_token();

		if (next.type != token_type::ObjectEnd) {

			while (true) {
				obj->insert(get_entry(next));
				next = next_token();

				if (next.type != token_type::Comma) {
					break;
				}

				next = next_token();
			}

			if (next.type != token_type::ObjectEnd) {
				JSON_THROW(brace_expected());
			}
		}

		return obj;
	}

	object_entry get_entry(const token &key) {
		if (key.type != token_type::String) {
			JSON_THROW(string_expected());
		}

		token colon = next_token();
		if (colon.type != token_type::Colon) {
			JSON_THROW(colon_expected());
		}

		return object_entry(key.value, get_value());
	}

	value get_value(const token &tok) {
		switch (tok.type) {
		case token_type::ObjectBegin:
			return value(get_object());
		case token_type::ArrayBegin:
			return value(get_array());
		case token_type::LiteralTrue:
			return value(true);
		case token_type::LiteralFalse:
			return value(false);
		case token_type::LiteralNull:
			return value(nullptr);
		case token_type::String:
			return value(std::move(tok.value));
		case token_type::Number:
			return value(std::move(tok.value), value::numeric_type());
		default:
			JSON_THROW(value_expected());
		}
	}

	value get_value() {
		return get_value(next_token());
	}

private:
	reader reader_;
};

inline value parse(std::string_view s) {
	parser p(s);
	return p.parse();
}

}

#endif
