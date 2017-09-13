#ifndef ARCHIVEC_CORE_NATIVE_HPP
#define ARCHIVEC_CORE_NATIVE_HPP

#ifdef __WIN32

#include <windows.h>

#elif __linux__ || __APPLE__
#include <sys/types.h>
#include <dirent.h>
#endif // __linux__

#endif //ARCHIVEC_CORE_NATIVE_HPP
