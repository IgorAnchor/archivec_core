#ifndef ARCHIVEC_CORE_MESSAGE_HPP
#define ARCHIVEC_CORE_MESSAGE_HPP

#include <iostream>

#include "Native.hpp"

namespace Message {
    static void message_box(std::string message, std::string title, std::string additional_message = "") {
#ifdef __WIN32
        MessageBox(nullptr, message.append(additional_message).c_str(), title.c_str(), MB_OK);
        std::cout << message.c_str() << std::endl;
#elif __linux__ || __APPLE__
        const std::string command = "zenity --text \"" + msg.c_str() + "\"";
        system(command.c_str());
#else
        std::cout << msg.c_str() << std::endl;
        std::cin.ignore();
#endif
    }

    static bool message_box_yes_no(std::string message, std::string title, std::string additional_message = "") {
#ifdef __WIN32
        int32_t result = MessageBox(nullptr, message.append(additional_message).c_str(), title.c_str(),
                                    MB_ICONQUESTION | MB_YESNO);
        return result == IDYES;
#elif __linux__ || __APPLE__
#endif
    }
}

#endif //ARCHIVEC_CORE_MESSAGE_HPP
