#ifndef ARCHIVEC_CORE_UTIL_HPP
#define ARCHIVEC_CORE_UTIL_HPP


#include <sstream>
#include <vector>

namespace util {
	template<typename T>
	static void split(const std::wstring &s, wchar_t delim, T result) {
		std::wstringstream ss(s);
		std::wstring item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}

	std::vector<std::wstring> split(const std::wstring &s, wchar_t delim) {
		std::vector<std::wstring> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}
}

#endif //ARCHIVEC_CORE_UTIL_HPP
