#include <sstream>
#include "archiveccore.hpp"

void init(JNIEnv *env) {
    java_util_ArrayList = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
    java_util_ArrayList_ = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
    java_util_ArrayList_size = env->GetMethodID(java_util_ArrayList, "size", "()I");
    java_util_ArrayList_get = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");
    java_util_ArrayList_add = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");
}


void array_list_to_vector_uint32(JNIEnv *env, std::vector<uint32_t> &ids, jobject array_list) {

    jclass integer_class = env->FindClass("java/lang/Integer");
    jmethodID getVal = env->GetMethodID(integer_class, "intValue", "()I");

    jint array_list_size = env->CallIntMethod(array_list, java_util_ArrayList_size);

    for (jint i = 0; i < array_list_size; ++i) {
        jobject j_integer = env->CallObjectMethod(array_list, java_util_ArrayList_get, i);

        auto id = static_cast<uint32_t>(env->CallIntMethod(j_integer, getVal));

        ids.push_back(id);
        env->DeleteLocalRef(j_integer);
    }
}

void array_list_to_vector_string_view(JNIEnv *env, std::vector<std::string_view> &ids, jobject array_list) {

    jint array_list_size = env->CallIntMethod(array_list, java_util_ArrayList_size);
    for (jint i = 0; i < array_list_size; ++i) {
        auto j_path = static_cast<jstring>(env->CallObjectMethod(array_list, java_util_ArrayList_get, i));

        const char *path = env->GetStringUTFChars(j_path, nullptr);

        ids.push_back((std::basic_string_view<char> &&) path);

        env->ReleaseStringUTFChars(j_path, path);

        env->DeleteLocalRef(j_path);
    }
}


JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_initNative
        (JNIEnv *env, jclass jcs) {
    init(env);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_resetNative
        (JNIEnv *env, jclass jcs) {
    archiver->reset();
}

JNIEXPORT jint JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_getLastIdNative
        (JNIEnv *env, jclass jcs, jstring j_path_to_archive) {
    const char *path_to_archive = env->GetStringUTFChars(j_path_to_archive, JNI_FALSE);

    auto last_id = static_cast<jint>(archiver->get_last_id(path_to_archive));

    env->ReleaseStringUTFChars(j_path_to_archive, path_to_archive);

    return last_id;
}

JNIEXPORT jobject JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesInfoNative
        (JNIEnv *env, jclass jcs, jstring j_path_to_archive) {

    const char *path_to_archive = env->GetStringUTFChars(j_path_to_archive, JNI_FALSE);

    std::vector<ArchivedFile> files = archiver->extract_files_info(path_to_archive);

    jobject array_list = env->NewObject(java_util_ArrayList, java_util_ArrayList_, files.size());

    for (auto &&file : files) {
        std::stringstream temp;
        temp << file.id << "|";
        temp << reinterpret_cast<const char *>(file.name) << "|";
        temp << file.size;
        temp << file.compressed_size;

        jstring file_info = env->NewStringUTF(temp.str().c_str());

        env->CallBooleanMethod(array_list, java_util_ArrayList_add, file_info);
        env->DeleteLocalRef(file_info);
    }

    env->ReleaseStringUTFChars(j_path_to_archive, path_to_archive);

    return array_list;
}

JNIEXPORT jboolean JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesNative
        (JNIEnv *env, jclass jcs, jstring j_path_to_archive, jstring j_dest_path, jobject j_ids, jboolean j_ask_replace) {

    const char *path_to_archive = env->GetStringUTFChars(j_path_to_archive, JNI_FALSE);
    const char *dest_path = env->GetStringUTFChars(j_dest_path, JNI_FALSE);

    std::vector<uint32_t> ids;
    array_list_to_vector_uint32(env, ids, j_ids);

    bool extracted = archiver->extract_files(path_to_archive, dest_path, ids, (bool)j_ask_replace);

    env->ReleaseStringUTFChars(j_path_to_archive, path_to_archive);
    env->ReleaseStringUTFChars(j_dest_path, dest_path);

    return static_cast<jboolean>(extracted);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractNative
        (JNIEnv *env, jclass jcs, jstring j_path_to_archive, jstring j_dest_path, jboolean j_ask_replace) {
    const char *path_to_archive = env->GetStringUTFChars(j_path_to_archive, JNI_FALSE);
    const char *dest_path = env->GetStringUTFChars(j_dest_path, JNI_FALSE);

    archiver->extract(path_to_archive, dest_path, (bool)j_ask_replace);

    env->ReleaseStringUTFChars(j_path_to_archive, path_to_archive);
    env->ReleaseStringUTFChars(j_dest_path, dest_path);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_addToArchiveNative
        (JNIEnv *env, jclass jcs, jobject j_files) {
    std::vector<std::string_view> files;
    array_list_to_vector_string_view(env, files, j_files);

    archiver->add_to_archive(files);
}


JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_addToExistingAtchiveNative
        (JNIEnv *env, jclass jcs, jobject j_files, jstring j_path_to_archive) {
    const char *path_to_archive = env->GetStringUTFChars(j_path_to_archive, JNI_FALSE);

    std::vector<std::string_view> files;
    array_list_to_vector_string_view(env, files, j_files);

    archiver->add_to_existing_archive(files, path_to_archive);

    env->ReleaseStringUTFChars(j_path_to_archive, path_to_archive);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_removeFromArchiveNative
        (JNIEnv *env, jclass jcs, jobject j_files, jstring j_path_to_archive) {

    const char *path_to_archive = env->GetStringUTFChars(j_path_to_archive, JNI_FALSE);

    std::vector<uint32_t> ids;
    array_list_to_vector_uint32(env, ids, j_files);

    archiver->remove_from_archive(ids, path_to_archive);

    env->ReleaseStringUTFChars(j_path_to_archive, path_to_archive);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_crushNative
        (JNIEnv *env, jclass jcs, jstring j_path_to_archive, jboolean j_ask_replace) {
    const char *path_to_archive = env->GetStringUTFChars(j_path_to_archive, JNI_FALSE);

    archiver->crush(path_to_archive, (bool) j_ask_replace);

    env->ReleaseStringUTFChars(j_path_to_archive, path_to_archive);
}