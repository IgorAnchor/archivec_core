#ifndef ARCHIVEC_CORE_MESSAGE_HPP
#define ARCHIVEC_CORE_MESSAGE_HPP

#include <iostream>

#include "native.hpp"

namespace Message {
	static void messageBox(std::string message, std::string title, std::string additionalMessage = "") {
#ifdef __WIN32
		MessageBox(nullptr, message.append(additionalMessage).c_str(), title.c_str(), MB_OK);
		std::cout << message.c_str() << std::endl;
#elif __linux__ || __APPLE__
		const std::string command = "zenity --text \"" + msg.c_str() + "\"";
		system(command.c_str());
#else
		std::cout << msg.c_str() << std::endl;
		std::cin.ignore();
#endif
	}

	static bool messageBoxYesNo(std::string message, std::string title, std::string additionalMessage = "") {
#ifdef __WIN32
		int result = MessageBox(nullptr, message.append(additionalMessage).c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNO);
		return result == IDYES;
#endif
	}
}

#endif //ARCHIVEC_CORE_MESSAGE_HPP
