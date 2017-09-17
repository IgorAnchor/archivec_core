#include <iostream>
#include "archiveccore.hpp"

void init(JNIEnv *env) {
    java_util_ArrayList = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
    java_util_ArrayList_ = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
    java_util_ArrayList_size = env->GetMethodID(java_util_ArrayList, "size", "()I");
    java_util_ArrayList_get = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");
    java_util_ArrayList_add = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesNative
        (JNIEnv *env, jclass jcs, jstring j_path_to_archive, jstring j_dest_path, jobject j_ids) {

    init(env);


    const char *path_to_archive = env->GetStringUTFChars(j_path_to_archive, JNI_FALSE);
    const char *dest_path = env->GetStringUTFChars(j_dest_path, JNI_FALSE);

    std::vector<uint32_t> ids;
    array_list_to_vector(env, ids, j_ids);

    archiver->extract_files(path_to_archive, dest_path, ids);

    env->ReleaseStringUTFChars(j_path_to_archive, path_to_archive);
    env->ReleaseStringUTFChars(j_dest_path, dest_path);
}

void array_list_to_vector(JNIEnv *env, std::vector<uint32_t> &ids, jobject array_list) {

    jclass integer_class = env->FindClass("java/lang/Integer");
    jmethodID getVal = env->GetMethodID(integer_class, "intValue", "()I");

    jint array_list_size = env->CallIntMethod(array_list, java_util_ArrayList_size);

    for (jint i = 0; i < array_list_size; ++i) {
        jobject j_integer = env->CallObjectMethod(array_list, java_util_ArrayList_get, i);

        uint32_t id = static_cast<uint32_t>(env->CallIntMethod(j_integer, getVal));

        ids.push_back(id);
        env->DeleteLocalRef(j_integer);
    }
}