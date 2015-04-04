#ifndef __ANDROID_H__
#define __ANDROID_H__

/* This is for C++ and does no harm in C */
#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

void Java_org_deadcode_wmelite_WMELiteFunctions_nativeInit(JNIEnv* env, jobject o, jobject assetMgr);

void android_getLogFileDirectory(char *buffer, int length);

void android_getPrivateFilesPath(char *buffer, int length);

void android_getDeviceTypeHint(char *buffer, int length);

void android_getGamePackagePath(char *buffer, int length);

void android_getGameFilePath(char *buffer, int length);

void android_getFontPath(char *buffer, int length);

void android_getLocalSettingsPath(char *buffer, int length);

void android_getEncodedString(char *inputString, char *encoding, char *buffer, int *length);

void android_getUTFString(char *inputString, char *encoding, char *buffer, int *length);

void android_showURLInBrowser(char *URL);

int android_advertisementPrepare(char *key, int number);

int android_advertisementShow(char *key, int number);

#ifdef __cplusplus
}
#endif

#endif // __ANDROID_H__
