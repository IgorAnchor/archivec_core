#include <sstream>
#include <iostream>
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

void arrayListToVectorStringView(JNIEnv *env, std::vector<std::wstring_view> &ids, jobject arrayList) {
	jint arrayListSize = env->CallIntMethod(arrayList, java_util_ArrayList_size);
	for (jint i = 0; i < arrayListSize; ++i) {
		auto jPath = static_cast<jstring>(env->CallObjectMethod(arrayList, java_util_ArrayList_get, i));

//		ids.push_back((std::basic_string_view<wchar_t> &&) path);
		ids.push_back((std::basic_string_view<wchar_t> &&) jtow(env, jPath));

		env->DeleteLocalRef(jPath);
	}
}

std::wstring jtow(JNIEnv *env, jstring string) {
	std::wstring value;

	const jchar *raw = env->GetStringChars(string, 0);
	jsize len = env->GetStringLength(string);

	value.assign(raw, raw + len);

	env->ReleaseStringChars(string, raw);

	return value;
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

	jint lastId;
	try {
		lastId = static_cast<jint>(archiver->getLastId(jtow(env, jArchive)));
	} catch (...) {
		lastId = -1;
	}

	return lastId;
}

JNIEXPORT jobject JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesInfoNative
		(JNIEnv *env, jclass jcs, jstring jArchive) {

	const auto files = archiver->extractInfo(jtow(env, jArchive));

	jobject arrayList = env->NewObject(java_util_ArrayList, java_util_ArrayList_, files.size());

	for (auto &&file : files) {
		std::wstringstream temp;
		temp << file.id << "|";
		temp << file.name << "|";
		temp << file.size << "|";
		temp << file.compressedSize;

		std::wcout<< file.name << std::endl;


		jstring fileInfo = env->NewString(reinterpret_cast<const jchar *>(temp.str().c_str()), static_cast<jsize>(temp.str().length()));

		env->CallBooleanMethod(arrayList, java_util_ArrayList_add, fileInfo);
		env->DeleteLocalRef(fileInfo);
	}

	return arrayList;
}

JNIEXPORT jboolean JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractFilesNative
		(JNIEnv *env, jclass jcs, jstring jArchive, jstring jDest, jobject jIds, jboolean jAskReplace, jboolean jIsFullPath) {
	std::vector<uint32_t> ids;
	arrayListToVectorUint32(env, ids, jIds);

	bool extracted = archiver->extractFiles(jtow(env, jArchive), jtow(env, jDest), ids, static_cast<bool>(jAskReplace), static_cast<bool>(jIsFullPath));

	return static_cast<jboolean>(extracted);
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_extractNative
		(JNIEnv *env, jclass jcs, jstring jArchive, jstring jDest, jboolean jAskReplace) {
	archiver->extract(jtow(env, jArchive), jtow(env, jDest), static_cast<bool>(jAskReplace));
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_addToArchiveNative
		(JNIEnv *env, jclass jcs, jobject jFiles) {

	std::vector<std::wstring_view> files;
	arrayListToVectorStringView(env, files, jFiles);

	archiver->add(files);
}


JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_addToExistingAtchiveNative
		(JNIEnv *env, jclass jcs, jobject jFiles, jstring jArchive) {

	std::vector<std::wstring_view> files;
	arrayListToVectorStringView(env, files, jFiles);

	archiver->addToExisting(files, jtow(env, jArchive));
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_removeFromArchiveNative
		(JNIEnv *env, jclass jcs, jobject jFiles, jstring jArchive) {

	std::vector<uint32_t> ids;
	arrayListToVectorUint32(env, ids, jFiles);

	archiver->remove(ids, jtow(env, jArchive));
}

JNIEXPORT void JNICALL Java_ua_chillcrew_archivec_core_ArchivecCore_crushNative
		(JNIEnv *env, jclass jcs, jstring jArchive, jboolean jAskReplace) {

	archiver->crush(jtow(env, jArchive).c_str(), (bool) jAskReplace);
}