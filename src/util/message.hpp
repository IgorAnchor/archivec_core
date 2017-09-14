//
// Created by IgorTheMLGPro on 9/13/2017.
//

#ifndef ARCHIVEC_CORE_MESSAGE_HPP
#define ARCHIVEC_CORE_MESSAGE_HPP

#include <iostream>

#include "Native.hpp"

namespace Message {
    static void message_box(const char *message, const char *title, const int argsc = 0, const char **argsv = nullptr) {
        std::string msg = message;
        if (argsc > 0 && argsv != nullptr) {
            for (int i = 0; i < argsc; ++i) {
                msg.append(argsv[i]);
            }
        }
#ifdef __WIN32
        MessageBox(nullptr, msg.c_str(), title, MB_OK);
        std::cout << msg.c_str() << std::endl;
#elif __linux__ || __APPLE__
        const std::string command = "zenity --text \"" + msg.c_str() + "\"";
        system(command.c_str());
#else
        std::cout << msg.c_str() << std::endl;
        std::cin.ignore();
#endif
    }
}

#endif //ARCHIVEC_CORE_MESSAGE_HPP
