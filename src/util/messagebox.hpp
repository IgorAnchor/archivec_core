#ifndef ARCHIVEC_CORE_MESSAGE_HPP
#define ARCHIVEC_CORE_MESSAGE_HPP

#include <iostream>

#include "native.hpp"

namespace Message {
	static void messageBox(const std::wstring &message, const std::wstring &title,const  std::wstring &additionalMessage = L"") {
#ifdef __WIN32
		std::wstring temp = message;
		temp.append(additionalMessage);
		MessageBoxW(nullptr, temp.c_str(), title.c_str(), MB_OK);
		std::wcout << temp.c_str() << std::endl;
#elif __linux__ || __APPLE__
		const std::string command = "zenity --text \"" + msg.c_str() + "\"";
		system(command.c_str());
#else
		std::cout << msg.c_str() << std::endl;
		std::cin.ignore();
#endif
	}

	static bool messageBoxYesNo(const std::wstring &message, const std::wstring &title,const std::wstring &additionalMessage = L"") {
#ifdef __WIN32
		std::wstring temp = message;
		temp.append(additionalMessage);
		int result = MessageBoxW(nullptr, temp.c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNO);
		return result == IDYES;
#endif
	}
}

#endif //ARCHIVEC_CORE_MESSAGE_HPP
