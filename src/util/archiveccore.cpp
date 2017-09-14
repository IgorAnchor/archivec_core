#include "archiveccore.hpp"
#include "message.hpp"

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_initNative
        (JNIEnv *env, jclass cl, jstring j_path) {
    const char *path = env->GetStringUTFChars(j_path, JNI_FALSE);
    Message::message_box(path, "init");
    archiver->init(path);
    env->ReleaseStringUTFChars(j_path, path);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_crushNative
        (JNIEnv *env, jclass cl, jstring j_path_out) {
    const char *out_path = env->GetStringUTFChars(j_path_out, JNI_FALSE);
    archiver->crush(out_path);
    env->ReleaseStringUTFChars(j_path_out, out_path);
}

JNIEXPORT jint JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_getFilesCountNative
        (JNIEnv *env, jclass cl, jstring j_path) {
    const char *out_path = env->GetStringUTFChars(j_path, JNI_FALSE);
    uint32_t count = archiver->extract_files_count(out_path);
    env->ReleaseStringUTFChars(j_path, out_path);

    return (jint) count;
}

JNIEXPORT jobject JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_getFileNamesNative
        (JNIEnv *env, jclass cl, jstring j_path) {
    const char *out_path = env->GetStringUTFChars(j_path, JNI_FALSE);
    archiver->extract_files_names(out_path);
    env->ReleaseStringUTFChars(j_path, out_path);
}

JNIEXPORT jboolean JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFileNative
        (JNIEnv *env, jclass cl, jstring j_title, jstring j_dest_path, jint j_id) {
    const char *title = env->GetStringUTFChars(j_title, JNI_FALSE);
    const char *dest_path = env->GetStringUTFChars(j_dest_path, JNI_FALSE);
    const uint32_t id = (uint32_t) j_id;

    bool extracted = archiver->extract_file(title, dest_path, id);

    env->ReleaseStringUTFChars(j_title, title);
    env->ReleaseStringUTFChars(j_dest_path, dest_path);

    return (jboolean) extracted;
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractNative
        (JNIEnv *env, jclass cl, jstring j_title, jstring j_dest_path) {

    const char *title = env->GetStringUTFChars(j_title, JNI_FALSE);
    const char *dest_path = env->GetStringUTFChars(j_dest_path, JNI_FALSE);

    archiver->extract(title, dest_path);

    env->ReleaseStringUTFChars(j_title, title);
    env->ReleaseStringUTFChars(j_dest_path, dest_path);
}