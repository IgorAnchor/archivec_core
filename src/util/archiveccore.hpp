#include <jni.h>
#include "../archiver.hpp"
/* Header for class ua_chillcrew_archivec_core_ArchivecCore */

#ifndef _Included_ua_chillcrew_archivec_core_ArchivecCore
#define _Included_ua_chillcrew_archivec_core_ArchivecCore
#ifdef __cplusplus
extern "C" {
#endif

std::shared_ptr<Archiver> archiver(new Archiver);

static jclass java_util_ArrayList;
static jmethodID java_util_ArrayList_;
jmethodID java_util_ArrayList_size;
jmethodID java_util_ArrayList_get;
jmethodID java_util_ArrayList_add;

void init(JNIEnv *);

void arrayListToVectorUint32(JNIEnv *, std::vector<unsigned> &, jobject);

void arrayListToVectorStringView(JNIEnv *, std::vector<std::wstring_view> &, jobject);

std::wstring jtow(JNIEnv *, jstring);


JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_initNative
        (JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_crushNative
        (JNIEnv *, jclass, jstring, jboolean);

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_addToArchiveNative
        (JNIEnv *, jclass, jobject);

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_addToExistingAtchiveNative
        (JNIEnv *, jclass, jobject, jstring);

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractNative
        (JNIEnv *, jclass, jstring, jstring, jboolean);

JNIEXPORT jboolean JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesNative
        (JNIEnv *, jclass, jstring, jstring, jobject, jboolean, jboolean);

JNIEXPORT jobject JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesInfoNative
        (JNIEnv *, jclass, jstring);

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_removeFromArchiveNative
        (JNIEnv *, jclass, jobject, jstring);

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_setBufferSizeNative
        (JNIEnv *, jclass, jint);

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_resetNative
        (JNIEnv *, jclass);

JNIEXPORT jint JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_getLastIdNative
        (JNIEnv *, jclass, jstring);

#ifdef __cplusplus
}
#endif
#endif
