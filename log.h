#ifndef LOG_H
#define LOG_H

#include <pthread.h>

#if 0
#include <android/log.h>
#define MY_VERBOSE(...)
#define MY_LOG(fmt,...)  __android_log_print(ANDROID_LOG_DEBUG  , "il2cpp", "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_METHOD(fmt,...)  __android_log_print(ANDROID_LOG_INFO  , "il2cpp", "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_INFO(fmt, ...) __android_log_print(ANDROID_LOG_INFO  , "il2cpp", "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_ERROR(fmt,...) __android_log_print(ANDROID_LOG_ERROR  , "il2cpp", "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)

#elif 0
#define MY_VERBOSE(fmt, ...)
#define MY_LOG(fmt, ...) printf( "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_METHOD(fmt, ...) printf( "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_ERROR(fmt, ...) printf( "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)

#elif __ANDROID__
#include <android/log.h>
#define MY_VERBOSE(fmt, ...)
#define MY_LOG(fmt, ...) 
#define MY_METHOD(fmt, ...)
#define MY_INFO(fmt, ...) __android_log_print(ANDROID_LOG_INFO  , "il2cpp", "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_ERROR(fmt, ...) __android_log_print(ANDROID_LOG_ERROR  , "il2cpp", "[%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#endif

#define ALOGD MY_LOG
#define ALOGW MY_ERROR
#define ALOGV MY_VERBOSE
#define ALOGE MY_ERROR
#define LOG_TAG "il2cpp"

#endif

