
#include "android.h"

#include "SDL_android.h"

#include <android/log.h>
#include <android/asset_manager_jni.h>

const char* className_wmeliteFunctions = "org/deadcode/wmelite";

static jobject callbackObject;

AAssetManager *assetManager;

void Java_org_deadcode_wmelite_WMELiteFunctions_nativeInit(JNIEnv* env, jobject o, jobject assetMgr)
{

	callbackObject = (*env)->NewGlobalRef(env, o);

	assetManager = AAssetManager_fromJava(env, assetMgr);

	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "Global ref to WMELITE=%s", (callbackObject == NULL) ? "NULL" : "OK");
	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "Asset manager ref=%s", (assetManager == NULL) ? "NULL" : "OK");

	// localEnv = env;
}

void android_getLogFileDirectory(char *buffer, int length)
{
	const char *tmp;

	// get the proper jni env from SDL
	// JNIEnv *env = Android_JNI_GetEnv();
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);
	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getLogFileDirectory", "()Ljava/lang/String;");
	jstring str = (*env)->CallObjectMethod(env, callbackObject, callbackID);

	tmp = (*env)->GetStringUTFChars(env, str, NULL);

	if (strnlen(tmp, length) < length) {
		strncpy(buffer, tmp, length);
	} else {
		buffer[0] = 0;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getLogFileDirectory() returns %s", buffer);

	(*env)->ReleaseStringUTFChars(env, str, tmp);
	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, str);
}

void android_getPrivateFilesPath(char *buffer, int length)
{
	const char *tmp;

	// get the proper jni env from SDL
	// JNIEnv *env = Android_JNI_GetEnv();
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);
	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getPrivateFilesPath", "()Ljava/lang/String;");
	jstring str = (*env)->CallObjectMethod(env, callbackObject, callbackID);

	tmp = (*env)->GetStringUTFChars(env, str, NULL);

	if (strnlen(tmp, length) < length) {
		strncpy(buffer, tmp, length);
	} else {
		buffer[0] = 0;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getPrivateFilesPath() returns %s", buffer);

	(*env)->ReleaseStringUTFChars(env, str, tmp);
	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, str);
}

void android_getDeviceTypeHint(char *buffer, int length)
{
	const char *tmp;

	// get the proper jni env from SDL
	// JNIEnv *env = Android_JNI_GetEnv();
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);
	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getDeviceTypeHint", "()Ljava/lang/String;");
	jstring str = (*env)->CallObjectMethod(env, callbackObject, callbackID);

	tmp = (*env)->GetStringUTFChars(env, str, NULL);

	if (strnlen(tmp, length) < length) {
		strncpy(buffer, tmp, length);
	} else {
		buffer[0] = 0;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getDeviceTypeHint() returns %s", buffer);

	(*env)->ReleaseStringUTFChars(env, str, tmp);
	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, str);
}

void android_getGamePackagePath(char *buffer, int length)
{
	const char *tmp;

	// get the proper jni env from SDL
	// JNIEnv *env = Android_JNI_GetEnv();
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);

	// __android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getGamePackagePath() env=%s", (env == NULL) ? "NULL" : "OK");

	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getGamePackagePath", "()Ljava/lang/String;");

	// __android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getGamePackagePath() callbackid=%d", callbackID);

	jstring str = (*env)->CallObjectMethod(env, callbackObject, callbackID);

	// __android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getGamePackagePath() jstring=%s", (str == NULL) ? "NULL" : "OK");

	tmp = (*env)->GetStringUTFChars(env, str, NULL);

	if (strnlen(tmp, length) < length) {
		strncpy(buffer, tmp, length);
	} else {
		buffer[0] = 0;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getGamePackagePath() returns %s", buffer);

	(*env)->ReleaseStringUTFChars(env, str, tmp);
	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, str);
}

void android_getGameFilePath(char *buffer, int length)
{
	const char *tmp;

	// get the proper jni env from SDL
	// JNIEnv *env = Android_JNI_GetEnv();
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);
	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getGameFilePath", "()Ljava/lang/String;");
	jstring str = (*env)->CallObjectMethod(env, callbackObject, callbackID);

	tmp = (*env)->GetStringUTFChars(env, str, NULL);

	if (strnlen(tmp, length) < length) {
		strncpy(buffer, tmp, length);
	} else {
		buffer[0] = 0;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getGameFilePath() returns %s", buffer);

	(*env)->ReleaseStringUTFChars(env, str, tmp);
	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, str);
}

void android_getFontPath(char *buffer, int length)
{
	const char *tmp;

	// get the proper jni env from SDL
	// JNIEnv *env = Android_JNI_GetEnv();
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);
	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getFontPath", "()Ljava/lang/String;");
	jstring str = (*env)->CallObjectMethod(env, callbackObject, callbackID);

	tmp = (*env)->GetStringUTFChars(env, str, NULL);

	if (strnlen(tmp, length) < length) {
		strncpy(buffer, tmp, length);
	} else {
		buffer[0] = 0;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getFontPath() returns %s", buffer);

	(*env)->ReleaseStringUTFChars(env, str, tmp);
	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, str);
}

void android_getLocalSettingsPath(char *buffer, int length)
{
	const char *tmp;

	// get the proper jni env from SDL
	// JNIEnv *env = Android_JNI_GetEnv();
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);
	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getLocalSettingsPath", "()Ljava/lang/String;");
	jstring str = (*env)->CallObjectMethod(env, callbackObject, callbackID);

	tmp = (*env)->GetStringUTFChars(env, str, NULL);

	if (strnlen(tmp, length) < length) {
		strncpy(buffer, tmp, length);
	} else {
		buffer[0] = 0;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, "org.libsdl.app", "android_getLocalSettingsPath() returns %s", buffer);

	(*env)->ReleaseStringUTFChars(env, str, tmp);
	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, str);
}

void android_getEncodedString(char *inputString, char *encoding, char *buffer, int *length)
{
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);
	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getEncodedString", "(Ljava/lang/String;Ljava/lang/String;)[B");
	jstring input = (*env)->NewStringUTF(env, inputString);
	jstring enc = NULL;
	if (encoding != NULL) {
		enc = (*env)->NewStringUTF(env, encoding);
	}

	jbyteArray result = (jbyteArray) (*env)->CallObjectMethod(env, callbackObject, callbackID, input, enc);

	// for debugging
	// (*env)->ExceptionDescribe(env);
	// (*env)->ExceptionClear(env);

	if (result != NULL) {
		jsize retLen = (*env)->GetArrayLength(env, result);
		jbyte *retBuffer = (*env)->GetByteArrayElements(env, result, NULL);

		if (retBuffer != NULL) {
			if (retLen < (*length)) {
				memcpy(buffer, retBuffer, retLen);
				buffer[retLen] = 0;
				*length = retLen;
			} else {
				__android_log_print(ANDROID_LOG_ERROR, "org.libsdl.app", "android_getEncodedString() length %d out of range!", retLen);
			}
			(*env)->ReleaseByteArrayElements(env, result, retBuffer, 0);
		} else {
			__android_log_print(ANDROID_LOG_ERROR, "org.libsdl.app", "android_getEncodedString() buffer not accessible!");
		}
	} else {
		__android_log_print(ANDROID_LOG_ERROR, "org.libsdl.app", "android_getEncodedString() returns NULL!");
	}

	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, input);
	if (encoding != NULL) {
		(*env)->DeleteLocalRef(env, enc);
	}
	if (result != NULL) {
		(*env)->DeleteLocalRef(env, result);
	}
}

void android_getUTFString(char *inputString, char *encoding, char *buffer, int *length)
{
	JNIEnv *env = Android_JNI_GetEnv();
	jclass cls = (*env)->GetObjectClass(env, callbackObject);
	jmethodID callbackID = (*env)->GetMethodID(env, cls, "getUTFString", "([BLjava/lang/String;)Ljava/lang/String;");

	size_t inLen = strnlen(inputString, 32768);
	jbyteArray input = (*env)->NewByteArray(env, inLen);
	jbyte *inputBuffer = (*env)->GetByteArrayElements(env, input, NULL);
	memcpy(inputBuffer, inputString, inLen);
	(*env)->ReleaseByteArrayElements(env, input, inputBuffer, 0);

	jstring enc = NULL;
	if (encoding != NULL) {
		enc = (*env)->NewStringUTF(env, encoding);
	}

	jstring result = (jstring) (*env)->CallObjectMethod(env, callbackObject, callbackID, input, enc);

	// for debugging
	// (*env)->ExceptionDescribe(env);
	// (*env)->ExceptionClear(env);

	if (result != NULL) {
		jsize retLen = (*env)->GetStringUTFLength(env, result);
		jbyte *retBuffer = (*env)->GetStringUTFChars(env, result, NULL);

		if (retBuffer != NULL) {
			if (retLen < (*length)) {
				memcpy(buffer, retBuffer, retLen);
				buffer[retLen] = 0;
				*length = retLen;
			} else {
				__android_log_print(ANDROID_LOG_ERROR, "org.libsdl.app", "android_getUTFString() length %d out of range!", retLen);
			}
			(*env)->ReleaseStringUTFChars(env, result, retBuffer);
		} else {
			__android_log_print(ANDROID_LOG_ERROR, "org.libsdl.app", "android_getUTFString() buffer not accessible!");
		}
	} else {
		__android_log_print(ANDROID_LOG_ERROR, "org.libsdl.app", "android_getUTFString() returns NULL!");
	}

	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, input);
	if (encoding != NULL) {
		(*env)->DeleteLocalRef(env, enc);
	}
	if (result != NULL) {
		(*env)->DeleteLocalRef(env, result);
	}
}

void android_showURLInBrowser(char *URL)
{
  JNIEnv *env = Android_JNI_GetEnv();
  jclass cls = (*env)->GetObjectClass(env, callbackObject);
  jmethodID callbackID = (*env)->GetMethodID(env, cls, "showURLInBrowser", "(Ljava/lang/String;)V");

  jstring urlString = (*env)->NewStringUTF(env,URL);

  (*env)->CallVoidMethod(env, callbackObject, callbackID, urlString);

  (*env)->DeleteLocalRef(env, urlString);
}

int android_advertisementPrepare(char *key, int number)
{
	  JNIEnv *env = Android_JNI_GetEnv();
	  jclass cls = (*env)->GetObjectClass(env, callbackObject);
	  jmethodID callbackID = (*env)->GetMethodID(env, cls, "advertisementPrepare", "(Ljava/lang/String;I)I");

	  jstring keyString = (*env)->NewStringUTF(env, key);
	  jint numberVal = (jint) number;
	  jint ret;

	  ret = (*env)->CallIntMethod(env, callbackObject, callbackID, keyString, numberVal);

	  (*env)->DeleteLocalRef(env, keyString);

	  return (int) ret;
}

int android_advertisementShow(char *key, int number)
{
	  JNIEnv *env = Android_JNI_GetEnv();
	  jclass cls = (*env)->GetObjectClass(env, callbackObject);
	  jmethodID callbackID = (*env)->GetMethodID(env, cls, "advertisementShow", "(Ljava/lang/String;I)I");

	  jstring keyString = (*env)->NewStringUTF(env, key);
	  jint numberVal = (jint) number;
	  jint ret;

	  ret = (*env)->CallIntMethod(env, callbackObject, callbackID, keyString, numberVal);

	  (*env)->DeleteLocalRef(env, keyString);

	  return (int) ret;
}

