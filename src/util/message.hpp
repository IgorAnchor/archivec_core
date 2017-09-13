//
// Created by IgorTheMLGPro on 9/13/2017.
//

#ifndef ARCHIVEC_CORE_MESSAGE_HPP
#define ARCHIVEC_CORE_MESSAGE_HPP

#include <iostream>

#include "Native.hpp"

namespace Message {

    static void error_message(const char *message) {
#ifdef __WIN32
        MessageBox(nullptr, message, "Error", MB_OK);
        std::cerr << message << std::endl;
#elif __linux__ || __APPLE__
        const std::string command = "zenity --error --text \"" + message + "\"";
        system(command.c_str());
#else
        std::cerr << message << std::endl;
        std::cin.ignore();
#endif
    }

    static void message(const char *message) {
#ifdef __WIN32
        MessageBox(nullptr, message, "Message", MB_OK);
        std::cout << message << std::endl;
#elif __linux__ || __APPLE__
        const std::string command = "zenity --text \"" + message + "\"";
        system(command.c_str());
#else
        std::cout << message << std::endl;
        std::cin.ignore();
#endif
    }
}

#endif //ARCHIVEC_CORE_MESSAGE_HPP
