#include "outcome.hpp"
#include <type_traits>
#include <iostream>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>


namespace outcome = OUTCOME_V2_NAMESPACE;

enum class ConversionErrc
{
    Success =     0,
    EmptyString = 1,
    IllegalChar = 2,
    TooLong =     3,
};

// boiler plate to plug into std::error_code...
namespace std
{

template<> struct is_error_code_enum<ConversionErrc> : std::true_type {};

} // ~namespace std

namespace detail
{

class ConversionErrc_category : public std::error_category
{
public:
    virtual const char* name() const noexcept override final
    { return "ConversionError"; }

    virtual std::string message(int c) const override final
    {
        switch (static_cast<ConversionErrc>(c)) {
            case ConversionErrc::Success:
                return "success";
            case ConversionErrc::EmptyString:
                return "empty string";
            case ConversionErrc::IllegalChar:
                return "illegal character";
            case ConversionErrc::TooLong:
                return  "too long";
            default:
                return "unknown";
        }
    }

    // OPTIONAL: allow generic error conditions to be compared
    virtual std::error_condition default_error_condition(int c) const noexcept override final
    {
        switch (static_cast<ConversionErrc>(c)) {
            case ConversionErrc::EmptyString:
                return make_error_condition(std::errc::invalid_argument);
            case ConversionErrc::IllegalChar:
                return make_error_condition(std::errc::invalid_argument);
            case ConversionErrc::TooLong:
                return make_error_condition(std::errc::result_out_of_range);
            default:
                // no mapping for code
                return std::error_condition(c, *this);
        }
    }
};

} // ~namespace detail

extern inline const detail::ConversionErrc_category& ConversionErrc_category()
{
    static detail::ConversionErrc_category c;
    return c;
}

// overload global make_error_code free function; found by ADL
inline std::error_code make_error_code(ConversionErrc e)
{
    return {static_cast<int>(e), ConversionErrc_category()};
}

outcome::result<int> convert(const std::string& str) noexcept {
	if (str.empty()) {
        return ConversionErrc::EmptyString;
    }
    if (!std::all_of(str.begin(), str.end(), ::isdigit)) {
        return ConversionErrc::IllegalChar;
    }
    if (str.length() > 9) {
        return ConversionErrc::TooLong;
    }
    return atoi(str.c_str());
}

int main(int argc, char** argv) {
	std::error_code ec = ConversionErrc::IllegalChar;

	std::cout << "ConversionErrc::IllegalChar is printed by std::error_code as "
		<< ec << " with explanatory message " << ec.message() << std::endl;

	// We can compare ConversionErrc containing error codes to generic conditions
	std::cout << "ec is equivalent to std::errc::invalid_argument = "
		<< (ec == std::errc::invalid_argument) << std::endl;
	std::cout << "ec is equivalent to std::errc::result_out_of_range = "
		<< (ec == std::errc::result_out_of_range) << std::endl;

    std::vector<std::string> test_cases = {
        "1234",
        "1234BadInput789",
        "123456789012345678901234567890",
        "",
    };

    for (auto&& input: test_cases) {
        if (outcome::result<int> r = convert(input)) {
            printf("'%s' converted to %d\n", input.c_str(), r.assume_value());
        } else {
            printf("'%s' failed to convert: %s\n", input.c_str(), r.assume_error().message().c_str());
        }
    }

    return 0;
}
