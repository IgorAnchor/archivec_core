#include <sstream>
#include "archiveccore.hpp"

void init(JNIEnv *env) {
    java_util_ArrayList = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
    java_util_ArrayList_ = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
    java_util_ArrayList_size = env->GetMethodID(java_util_ArrayList, "size", "()I");
    java_util_ArrayList_get = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");
    java_util_ArrayList_add = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");
}

void arrayListToVectorUint32(JNIEnv *env, std::vector<uint32_t> &ids, jobject arrayList) {
    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID getVal = env->GetMethodID(integerClass, "intValue", "()I");

    jint arrayListSize = env->CallIntMethod(arrayList, java_util_ArrayList_size);

    for (jint i = 0; i < arrayListSize; ++i) {
        jobject jInteger = env->CallObjectMethod(arrayList, java_util_ArrayList_get, i);

        auto id = static_cast<uint32_t>(env->CallIntMethod(jInteger, getVal));

        ids.push_back(id);
        env->DeleteLocalRef(jInteger);
    }
}

void arrayListToVectorStringView(JNIEnv *env, std::vector<std::string_view> &ids, jobject arrayList) {
    jint arrayListSize = env->CallIntMethod(arrayList, java_util_ArrayList_size);
    for (jint i = 0; i < arrayListSize; ++i) {
        auto jPath = static_cast<jstring>(env->CallObjectMethod(arrayList, java_util_ArrayList_get, i));

        const char *path = env->GetStringUTFChars(jPath, nullptr);

        ids.push_back((std::basic_string_view<char> &&) path);

        env->ReleaseStringUTFChars(jPath, path);
        env->DeleteLocalRef(jPath);
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
        (JNIEnv *env, jclass jcs, jstring jArchive) {

    const char *archive = env->GetStringUTFChars(jArchive, JNI_FALSE);
    auto lastId = static_cast<jint>(archiver->getLastId(archive));

    env->ReleaseStringUTFChars(jArchive, archive);

    return lastId;
}

JNIEXPORT jobject JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesInfoNative
        (JNIEnv *env, jclass jcs, jstring jArchive) {

    const char *archive = env->GetStringUTFChars(jArchive, JNI_FALSE);
    const auto files = archiver->extractInfo(archive);

    jobject arrayList = env->NewObject(java_util_ArrayList, java_util_ArrayList_, files.size());

    for (auto &&file : files) {
        std::stringstream temp;
        temp << file.id << "|";
        temp << reinterpret_cast<const char *>(file.name) << "|";
        temp << file.size << "|";
        temp << file.compressedSize;

        jstring fileInfo = env->NewStringUTF(temp.str().c_str());

        env->CallBooleanMethod(arrayList, java_util_ArrayList_add, fileInfo);
        env->DeleteLocalRef(fileInfo);
    }
    env->ReleaseStringUTFChars(jArchive, archive);

    return arrayList;
}

JNIEXPORT jboolean JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesNative
        (JNIEnv *env, jclass jcs, jstring jArchive, jstring jDest, jobject jIds, jboolean jAskReplace) {

    const char *archive = env->GetStringUTFChars(jArchive, JNI_FALSE);
    const char *dest = env->GetStringUTFChars(jDest, JNI_FALSE);

    std::vector<uint32_t> ids;
    arrayListToVectorUint32(env, ids, jIds);

    bool extracted = archiver->extractFiles(archive, dest, ids, static_cast<bool>(jAskReplace));

    env->ReleaseStringUTFChars(jArchive, archive);
    env->ReleaseStringUTFChars(jDest, dest);

    return static_cast<jboolean>(extracted);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractNative
        (JNIEnv *env, jclass jcs, jstring jArchive, jstring jDest, jboolean jAskReplace) {

    const char *archive = env->GetStringUTFChars(jArchive, JNI_FALSE);
    const char *dest = env->GetStringUTFChars(jDest, JNI_FALSE);

    archiver->extract(archive, dest, static_cast<bool>(jAskReplace));

    env->ReleaseStringUTFChars(jArchive, archive);
    env->ReleaseStringUTFChars(jDest, dest);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_addToArchiveNative
        (JNIEnv *env, jclass jcs, jobject jFiles) {

    std::vector<std::string_view> files;
    arrayListToVectorStringView(env, files, jFiles);

    archiver->add(files);
}


JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_addToExistingAtchiveNative
        (JNIEnv *env, jclass jcs, jobject jFiles, jstring jArchive) {

    const char *archive = env->GetStringUTFChars(jArchive, JNI_FALSE);
    std::vector<std::string_view> files;
    arrayListToVectorStringView(env, files, jFiles);

    archiver->addToExisting(files, archive);

    env->ReleaseStringUTFChars(jArchive, archive);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_removeFromArchiveNative
        (JNIEnv *env, jclass jcs, jobject jFiles, jstring jArchive) {

    const char *archive = env->GetStringUTFChars(jArchive, JNI_FALSE);
    std::vector<uint32_t> ids;
    arrayListToVectorUint32(env, ids, jFiles);

    archiver->remove(ids, archive);

    env->ReleaseStringUTFChars(jArchive, archive);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_crushNative
        (JNIEnv *env, jclass jcs, jstring jArcive, jboolean jAskReplace) {

    const char *archive = env->GetStringUTFChars(jArcive, JNI_FALSE);

    archiver->crush(archive, (bool) jAskReplace);

    env->ReleaseStringUTFChars(jArcive, archive);
}