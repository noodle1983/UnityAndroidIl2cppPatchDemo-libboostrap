#ifndef LOG_H
#define LOG_H

#if 0
#include <android/log.h>
#define MY_VERBOSE(...)
#define MY_LOG(fmt,...)  __android_log_print(ANDROID_LOG_DEBUG  , "il2cpp", fmt "\n", ##__VA_ARGS__)
#define MY_METHOD(fmt,...)  __android_log_print(ANDROID_LOG_INFO  , "il2cpp", fmt "\n", ##__VA_ARGS__)
#define MY_INFO(fmt, ...) __android_log_print(ANDROID_LOG_INFO  , "il2cpp", fmt "\n", ##__VA_ARGS__)
#define MY_ERROR(fmt,...) __android_log_print(ANDROID_LOG_ERROR  , "il2cpp", fmt "\n", ##__VA_ARGS__)

#elif 0
#define MY_VERBOSE(fmt, ...)
#define MY_LOG(fmt, ...) printf( fmt "\n", ##__VA_ARGS__)
#define MY_METHOD(fmt, ...) printf( fmt "\n", ##__VA_ARGS__)
#define MY_ERROR(fmt, ...) printf( fmt "\n", ##__VA_ARGS__)

#elif __ANDROID__
#include <android/log.h>
#define MY_VERBOSE(fmt, ...)
#define MY_LOG(fmt, ...) 
#define MY_METHOD(fmt, ...)
#define MY_INFO(fmt, ...) __android_log_print(ANDROID_LOG_INFO  , "il2cpp", fmt "\n", ##__VA_ARGS__)
#define MY_ERROR(fmt, ...) __android_log_print(ANDROID_LOG_ERROR  , "il2cpp", fmt "\n", ##__VA_ARGS__)
#endif

#define ALOGD MY_LOG
#define ALOGW MY_ERROR
#define ALOGV MY_VERBOSE
#define ALOGE MY_ERROR
#define LOG_TAG "il2cpp"


static inline bool verify_bundle_id(const char* _bundle_id)
{
	const char* const check_bundle_id = BUNDLE_ID;
	for(int i = 0; i < 100; i++){
		if (check_bundle_id[i] == '*')
		{
			return true;
		}
		else if(check_bundle_id[i] == '\0')
		{
			return _bundle_id[i] == '\0';
		}
		else if (_bundle_id[i] != check_bundle_id[i])
		{
			return false;
		}
		//else continue;
	}
	return false;
}

#endif

