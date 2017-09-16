#include "archiveccore.hpp"
#include "message.hpp"

void init(JNIEnv *env) {
    java_util_ArrayList = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
    java_util_ArrayList_ = env->GetMethodID(java_util_ArrayList, "<init_dir>", "(I)V");
    java_util_ArrayList_size = env->GetMethodID(java_util_ArrayList, "size", "()I");
    java_util_ArrayList_get = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");
    java_util_ArrayList_add = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_initNative
        (JNIEnv *env, jclass cl, jstring j_path) {

    init(env);

    const char *path = env->GetStringUTFChars(j_path, JNI_FALSE);
    archiver->init_dir(path, path);
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
    /*const char *out_path = env->GetStringUTFChars(j_path, JNI_FALSE);
    auto vec = archiver->extract_file_names(out_path);
    env->ReleaseStringUTFChars(j_path, out_path);

    return std_vec_to_array_list(env, vec);*/
}

JNIEXPORT jboolean JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFileNative
        (JNIEnv *env, jclass cl, jstring j_title, jstring j_dest_path, jint j_id) {
    const char *title = env->GetStringUTFChars(j_title, JNI_FALSE);
    const char *dest_path = env->GetStringUTFChars(j_dest_path, JNI_FALSE);
    const auto id = (uint32_t) j_id;

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


jobject std_vec_to_array_list(JNIEnv *env, std::vector<std::string> &vector) {
    jobject array_list = env->NewObject(java_util_ArrayList, java_util_ArrayList_, vector.size());
    for (std::string s: vector) {
        jstring element = env->NewStringUTF(s.c_str());
        env->CallBooleanMethod(array_list, java_util_ArrayList_add, element);
        env->DeleteLocalRef(element);
    }
    return array_list;
}