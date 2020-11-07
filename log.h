#ifndef LOG_H
#define LOG_H

#include <pthread.h>

#if 0

#include <android/log.h>
#include "singleton.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
class FileLogger
{
public:
    FileLogger()
    {
		//extern const char* g_data_file_path;	
		//char log_path[256] = {0};
		//snprintf(log_path, sizeof(log_path), "%s/il2cpp.log",  g_data_file_path);
		const char* log_path = "/data/local/tmp/il2cpp.log";
		__android_log_print(ANDROID_LOG_DEBUG  , "il2cpp", "log to file:%s", log_path);
        file_handle_ = ::fopen(log_path, "w");
		if (file_handle_ == NULL){		
			__android_log_print(ANDROID_LOG_ERROR, "il2cpp", "log to file:%s, errno:%d", log_path, errno);
			exit(-1);
		}
    };
    virtual ~FileLogger()
    {
        if (file_handle_)
        {
            ::fclose(file_handle_);
        }
    }

    void log(const char* _fmt, ...)
    {
        if (file_handle_ == NULL) return;

        char log_buf[2048] = {0};
        va_list arg_list;
        va_start(arg_list, _fmt);
        int i = vsnprintf(log_buf, sizeof(log_buf), _fmt, arg_list);
        va_end(arg_list);

        if (i > 0)
        {
            fwrite(log_buf, sizeof(char), i, file_handle_ );
            fflush( file_handle_ );
        }

    }

private:
    FILE* file_handle_;
};
#define g_file_logger (Singleton<FileLogger, 0>::instance())
#define MY_VERBOSE(...)
#define MY_LOG(fmt,...)    g_file_logger->log("LOG    [%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_METHOD(fmt,...) g_file_logger->log("METHOD [%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_INFO(fmt, ...)  g_file_logger->log("INFO   [%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)
#define MY_ERROR(fmt,...)  g_file_logger->log("ERROR  [%zx]" fmt "\n", (size_t)pthread_self(), ##__VA_ARGS__)

#elif 0
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

