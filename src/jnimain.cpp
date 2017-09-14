#include "jnimain.hpp"
#include "filemanager.hpp"
#include "archiver.hpp"


JNIEXPORT void JNICALL Java_Native_test(JNIEnv *env, jclass jc) {
    Archiver *archiver = new Archiver("test/");
    archiver->crush("test.ar");
}