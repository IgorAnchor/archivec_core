#include "jnimain.hpp"
#include "filemanager.hpp"


JNIEXPORT void JNICALL Java_Native_test(JNIEnv *env, jclass jc) {
    FileManager::open_file("1.txt");
}