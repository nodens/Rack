#pragma once
#include <stdarg.h>
#include <vector>

#include <common.hpp>


namespace rack {


/** Supplemental `std::string` functions
*/
namespace string {


/** Converts a `printf()` format string and optional arguments into a std::string.
Remember that "%s" must reference a `char *`, so use `.c_str()` for `std::string`s, otherwise you might get binary garbage.
*/
__attribute__((format(printf, 1, 2)))
std::string f(const char* format, ...);
std::string fV(const char* format, va_list args);
/** Replaces all characters to lowercase letters */
std::string lowercase(const std::string& s);
/** Replaces all characters to uppercase letters */
std::string uppercase(const std::string& s);
/** Removes whitespace from beginning and end of string. */
std::string trim(const std::string& s);
/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string ellipsize(const std::string& s, size_t len);
std::string ellipsizePrefix(const std::string& s, size_t len);
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);

/** Scores how well a query matches a string.
A score of 0 means no match.
The score is arbitrary and is only meaningful for sorting.
*/
float fuzzyScore(const std::string& s, const std::string& query);

/** Converts a byte array to a Base64-encoded string.
https://en.wikipedia.org/wiki/Base64
*/
std::string toBase64(const uint8_t* data, size_t dataLen);
std::string toBase64(const std::vector<uint8_t>& data);
/** Converts a Base64-encoded string to a byte array.
Throws std::runtime_error if string is invalid.
*/
std::vector<uint8_t> fromBase64(const std::string& str);

struct CaseInsensitiveCompare {
	/** Returns whether `a < b` using case-insensitive lexical comparison. */
	bool operator()(const std::string& a, const std::string& b) const;
};


#if defined ARCH_WIN
/** Performs a Unicode string conversion from UTF-16 to UTF-8.
These are only defined on Windows because the implementation uses Windows' API, and conversion is not needed on other OS's (since everything on Mac and Linux is UTF-8).

std::string and char* variables are considered UTF-8, anywhere in the program.
See https://utf8everywhere.org/ for more information about VCV Rack's philosophy on string encoding, especially section 10 for rules VCV follows for handling text on Windows.
*/
std::string UTF16toUTF8(const std::wstring& w);
std::wstring UTF8toUTF16(const std::string& s);
#endif


} // namespace string
} // namespace rack
