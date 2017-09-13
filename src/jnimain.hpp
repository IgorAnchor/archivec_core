#include <jni.h>
/* Header for class Native */

#ifndef _Included_Native
#define _Included_Native

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     Native
 * Method:    test
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_Native_test
  (JNIEnv *, jclass);


#ifdef __cplusplus
}
#endif

#endif
