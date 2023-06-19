#ifndef JSON_READER_H
#define JSON_READER_H


#include <cassert>
#include <cstddef>
#include <optional>
#include <regex>
#include <stack>
#include <string>
#include <string_view>

namespace json {

template <class Ch>
class basic_reader
{
public:

    struct Location
    {
        size_t line;
        size_t column;
    };

    /**
         * @brief Construct a new Basic Reader object for lexing a string
         *
         * @param input
         */
    explicit basic_reader(std::basic_string_view<Ch> input) noexcept
        : input_(input) {
    }

    basic_reader()                                   = default;
    basic_reader(const basic_reader &other)          = default;
    basic_reader &operator=(const basic_reader &rhs) = default;
    ~basic_reader()                                  = default;

    /**
         * @brief Returns true if the reader is at the end of the stream
         *
         * @return bool
         */
    bool eof() const noexcept {
        return index_ == input_.size();
    }

    /**
         * @brief Returns the next character in the string without advancing the position
         *
         * @return Ch
         */
    Ch peek() const noexcept {
        if (eof()) {
            return '\0';
        }

        return input_[index_];
    }

    /**
         * @brief Returns the next character in the string and advances the position
         *
         * @return Ch
         */
    Ch read() noexcept {
        if (eof()) {
            return '\0';
        }

        return input_[index_++];
    }

    /**
         * @brief Consumes while the next character is in the input set <chars>
         * and returns the number of consumed characters
         *
         * @param chars
         * @return size_t
         */
    size_t consume(std::basic_string_view<Ch> chars) noexcept {
        return consume_while([chars](Ch ch) {
            return chars.find(ch) != std::basic_string_view<Ch>::npos;
        });
    }

    /**
         * @brief Consumes while the next character is whitespace (tab or space)
         * and returns the number of consumed characters
         *
         * @return size_t
         */
    size_t consume_whitespace() noexcept {
        return consume_while([](Ch ch) {
            return (ch == ' ' || ch == '\t');
        });
    }

    /**
         * @brief Consumes while a given predicate function returns true
         * and returns the number of consumed characters
         *
         * @param pred
         * @return size_t
         */
    template <class Pred>
    size_t consume_while(Pred pred) noexcept {
        size_t count = 0;
        while (!eof()) {
            const Ch ch = peek();
            if (!pred(ch)) {
                break;
            }

            ++index_;
            ++count;
        }
        return count;
    }

    /**
         * @brief Returns true and advances the position
         * if the next character matches <ch>
         *
         * @param ch
         * @return bool
         */
    bool match(Ch ch) noexcept {
        if (peek() != ch) {
            return false;
        }

        ++index_;
        return true;
    }

    /**
         * @brief Returns true and advances the position
         * if the next sequences of characters matches <s>
         *
         * @param s
         * @return bool
         */
    bool match(std::basic_string_view<Ch> s) noexcept {
        if (input_.compare(index_, s.size(), s) != 0) {
            return false;
        }

        index_ += s.size();
        return true;
    }

    /**
         * @brief Matches until the end of the input and returns the string matched
         *
         * @return std::optional<std::basic_string<Ch>>
         */
    std::optional<std::basic_string<Ch>> match_any() {
        if (eof()) {
            return {};
        }

        std::basic_string<Ch> m = input_.substr(index_);
        index_ += m.size();
        return m;
    }

    /**
         * @brief Returns the matching string and advances the position
         * if the next sequences of characters matches <regex>
         *
         * @param s
         * @return bool
         */
    std::optional<std::basic_string<Ch>> match(const std::basic_regex<Ch> &regex) {
        std::match_results<const Ch *> matches;

        const Ch *first = &input_[index_];
        const Ch *last  = &input_[input_.size()];

        if (std::regex_search(first, last, matches, regex, std::regex_constants::match_continuous)) {
            std::basic_string<Ch> m(matches[0].first, matches[0].second);
            index_ += m.size();
            return m;
        }

        return {};
    }

    /**
         * @brief Returns the matching string and advances the position
         * for each character satisfying the given predicate
         *
         * @param pred
         * @return std::optional<std::basic_string<Ch>>
         */
    template <class Pred>
    std::optional<std::basic_string<Ch>> match_while(Pred pred) {

        size_t start = index_;
        while (!eof()) {
            const Ch ch = peek();
            if (!pred(ch)) {
                break;
            }

            ++index_;
        }

        std::basic_string<Ch> m(&input_[start], &input_[index_]);
        if (!m.empty()) {
            return m;
        }

        return {};
    }

    /**
         * @brief Returns the current position in the string
         *
         * @return size_t
         */
    size_t index() const noexcept {
        return index_;
    }

    /**
         * @brief Returns the current position in the string as a line/column pair
         *
         * @return Location
         */
    Location location() const noexcept {
        return location(index_);
    }

    /**
         * @brief Returns the position of <index> in the string as line/column pair
         *
         * @param index
         * @return Location
         */
    Location location(size_t index) const noexcept {
        size_t line = 1;
        size_t col  = 1;

        if (index < input_.size()) {

            for (size_t i = 0; i < index; ++i) {
                if (input_[i] == '\n') {
                    ++line;
                    col = 1;
                } else {
                    ++col;
                }
            }
        }

        return Location{line, col};
    }

    /**
         * @brief Stores the current state of the reader onto the stack
         *
         */
    void push_state() {
        state_.push(index_);
    }

    /**
         * @brief Removes the most recently pushed state from the stack
         *
         */
    void pop_state() {
        assert(!state_.empty());
        state_.pop();
    }

    /**
         * @brief Sets the current state to the most recently pushed state from the stack, and then
         * removes the most recently pushed state from the stack
         *
         */
    void restore_state() {
        assert(!state_.empty());
        index_ = state_.top();
        state_.pop();
    }

private:
    std::basic_string_view<Ch> input_;
    size_t index_ = 0;
    std::stack<size_t> state_;
};

using reader = basic_reader<char>;

}


#endif // JSON_READER_H
