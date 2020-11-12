#ifndef SHADOW_ZIP_H
#define SHADOW_ZIP_H

#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include "singleton.hpp"

struct FilePartitionInfo
{
    FilePartitionInfo(uint64_t _shadow_start, uint64_t _shadow_stop, int _file_index, uint64_t _start_in_file, uint64_t _stop_in_file)
        : shadow_start_(_shadow_start), shadow_stop_(_shadow_stop), file_index_(_file_index),
        start_in_file_(_start_in_file), stop_in_file_(_stop_in_file)
    {}
    //[shadow_start_, shadow_stop_)
    uint64_t shadow_start_;
    uint64_t shadow_stop_;

    uint64_t file_index_;
    uint64_t start_in_file_;
    uint64_t stop_in_file_;
};

struct ShadowZipGlobalData
{
	PthreadRwMutex mutex;
    std::vector<FilePartitionInfo> patch_partitions_;
    std::vector<std::string> all_files_;
    uint64_t end_of_file_;
};
#define g_shadowzip_global_data (LeakSingleton<ShadowZipGlobalData, 0>::instance())

class ShadowZip
{
public:
    ShadowZip();
    virtual ~ShadowZip(){
        fclose(NULL);
    }

    FILE* fopen();
    off64_t fseek(FILE *stream, off64_t offset, int whence);
    long ftell(FILE *stream);
    void rewind(FILE *stream);
    size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
	char* fgets(char *s, int size, FILE *stream);
    int fclose(FILE* _fp);

    static int init(const char* _patch_dir, const char* _sys_apk_file, ShadowZipGlobalData* global_data);
    static void log(ShadowZipGlobalData* global_data);
	static uint64_t get_eof_pos();
	static void output_apk(const char* _patch_dir);

	static bool contains_path(const char* _apk_file, const char* _check_path);
private:
    FILE* prepare_file(int _file_index);

private:
    int64_t pos_;
    std::vector<FILE *> fp_array_;
	
	//copy from global data
    std::vector<FilePartitionInfo> patch_partitions_;
    std::vector<std::string> all_files_;
    uint64_t end_of_file_;
};

#endif /* SHADOW_ZIP_H */
