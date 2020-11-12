#ifndef SERIAL_UTILS_H
#define SERIAL_UTILS_H

#include "shadow_zip.h"
#include <string>
#include <vector>

void serial_uint32(FILE* fp, uint32_t i)
{
    int write_cnt = fwrite(&i, 1, sizeof(i), fp);
    if (write_cnt != sizeof(i))
    {
        MY_ERROR("write uint32 error %d", errno);
        exit(-1);
    }
}

void serial_uint64(FILE* fp, uint64_t i)
{
    int write_cnt = fwrite(&i, 1, sizeof(i), fp);
    if (write_cnt != sizeof(i))
    {
        MY_ERROR("write uint64 error %d", errno);
        exit(-1);
    }
}


void serial_string(FILE* fp, const std::string& str)
{
    uint32_t len = (uint32_t)str.length();
    serial_uint32(fp, len);
    
    int write_cnt = fwrite(str.c_str(), 1, len, fp);
    if (write_cnt != len)
    {
        MY_ERROR("write string error %d", errno);
        exit(-1);
    }
}

void serial_string_vector(FILE* fp, const std::vector<std::string>& v)
{
    uint32_t len = (uint32_t)v.size();
    serial_uint32(fp, len);

    for(int i = 0; i < len; i++){
        serial_string(fp, v[i]);
    }
}

void serial_partition_vector(FILE* fp, const std::vector<FilePartitionInfo>& v)
{
    uint32_t len = (uint32_t)v.size();
    serial_uint32(fp, len);

    int write_cnt = fwrite(v.data(), 1, sizeof(FilePartitionInfo) * len, fp);
    if (write_cnt != len*sizeof(FilePartitionInfo))
    {
        MY_ERROR("write partition_vector error %d", errno);
        exit(-1);
    }
}


uint32_t unserial_uint32(FILE* fp)
{
    uint32_t len = 0;
    int read_cnt = ::fread(&len, 1, sizeof(len), fp);
    if (read_cnt != sizeof(len) )
    {
        MY_ERROR("read uint32 error %d", errno);
        return (uint32_t)-1;
    }
    return len;
}

uint64_t unserial_uint64(FILE* fp)
{
    uint64_t len = 0;
    int read_cnt = ::fread(&len, 1, sizeof(len), fp);
    if (read_cnt != sizeof(len) )
    {
        MY_ERROR("read uint64 error %d", errno);
        exit(-1);
    }
    return len;
}


std::string unserial_string(FILE* fp)
{
    uint32_t len = unserial_uint32(fp);
    
    char buffer[4096] = {0};
    int read_cnt = ::fread(buffer, 1, len, fp);
    if (read_cnt != len)
    {
        MY_ERROR("read string error %d", errno);
        exit(-1);
    }
    return std::string(buffer, len);
}

void unserial_string_vector(FILE* fp, std::vector<std::string>& v)
{
    v.clear();
    uint32_t len = unserial_uint32(fp);

    for(int i = 0; i < len; i++){
        v.push_back(unserial_string(fp));
    }
}

void unserial_partition_vector(FILE* fp, std::vector<FilePartitionInfo>& v)
{
    v.clear();
    uint32_t len = unserial_uint32(fp);
    v.reserve(len);

    FilePartitionInfo* f = (FilePartitionInfo*)new char[len * sizeof(FilePartitionInfo)];
    int read_cnt = fread(f, 1, sizeof(FilePartitionInfo) * len, fp);
    if (read_cnt != len*sizeof(FilePartitionInfo))
    {
        MY_ERROR("read partition_vector error  %d", errno);
        exit(-1);
    }
    for(int i = 0; i < len; i++){
        v.push_back(f[i]);
    }
    delete[] f;
}

#endif /* SERIAL_UTILS_H */

