#ifndef FILE_MAPPLE_H
#define FILE_MAPPLE_H

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "log.h"
#include <linux/ashmem.h>
#include <sys/mman.h>

#include <vector>
#include <string>

extern char* g_use_data_path;

const uint64_t MY_MAGIC_NUM = 0x1983546019835460;
struct FileExtraData
{
	uint64_t magic_num;
	FileExtraData* self;
	ShadowZip* shadow_zip;
	FILE* file;
	char* mmap_buffer;
	int fd;
};
bool use_shared_mem = false;
static dev_t g_shm_device_id = -1;
static ino_t g_shm_ino = -1;

// posix shm is not supported on Android.
// we should use memfd, but it is not supported in older android(platform api < 26) 
// https://developer.android.google.cn/ndk/reference/group/memory.html
// file fd is slow theoratically, but it is ok on a flash disk and I don't notice it.
// no other choice, as the mutex is buggy in fclose with timing irq.

static int init_mapping_data()
{
	FILE* file = ::fopen("/dev/ashmem", "r+");
	if (file != NULL){
		int fd = fileno(file);
		struct stat shm_stat;
		memset(&shm_stat, 0, sizeof(struct stat));
		if (fstat(fd, &shm_stat) == 0) {
			::fclose(file);
			g_shm_device_id = shm_stat.st_dev;
			g_shm_ino = shm_stat.st_ino;
			use_shared_mem = true;
			MY_METHOD("shmem mapping is enabled. dev:%lu:%lu", (unsigned long)g_shm_device_id, (unsigned long)g_shm_ino);
			return 0;
		}
		::fclose(file);
	}
	
	char fds_dir[512] = {0};
	snprintf(fds_dir, sizeof(fds_dir), "%s/rt_fd_mappings", g_use_data_path);
	MY_METHOD("init_mapping_data %s with apk_path:%s", TARGET_ARCH_ABI, fds_dir);	
	
	//create dir if not exist
	DIR* dir = opendir(fds_dir);
    if (dir == NULL)
    {
		if (errno == ENOENT)
		{
			mkdir(fds_dir, 0700); 
			dir = opendir(fds_dir);
		}
		else
		{
			MY_ERROR("open %s failed! errno:%d", fds_dir, errno);
			return -1;
		}
    }
	
	//delete all files under the dir
    if (dir == NULL){ 
		MY_ERROR("open %s failed! errno:%d", fds_dir, errno);
		return -1; 
	}

    struct dirent *ent = NULL;
	std::vector<std::string> fd_files;
    while((ent = readdir(dir)) != NULL) {  
        if(ent->d_type & DT_REG) {  
			std::string fd_file = std::string(fds_dir) + "/" + ent->d_name;
            fd_files.push_back(fd_file);
        }  
    }  
    closedir(dir);
	
	for(int i = 0; i < fd_files.size(); i++)
	{
		std::string& fd_file = fd_files[i];
		if (0 != unlink(fd_file.c_str()))
		{
			MY_METHOD("delete fd file:[%s], ret:%d", fd_file.c_str(), errno);
			return -1;
		}
		MY_METHOD("delete fd file:[%s]", fd_file.c_str());
	}
	return 0;
}

static void save_mem_mapping(FileExtraData* file_extra_data){
	file_extra_data->file = ::fopen("/dev/ashmem", "r+");
	if (file_extra_data->file == NULL)
	{		
		MY_ERROR("open /dev/ashmem failed! errno:%d", errno);
		_exit(-1); 
	}
	int fd = fileno(file_extra_data->file);	
	file_extra_data->fd = fd;
	
	char name[256] = {0};
	snprintf(name, sizeof(name), "%08llx_%d", (unsigned long long)file_extra_data->file, file_extra_data->fd);
	ioctl(fd, ASHMEM_SET_NAME, name);
    ioctl(fd, ASHMEM_SET_SIZE, sizeof(FileExtraData));
	
	char* buff = (char*)mmap(NULL, sizeof(FileExtraData), PROT_READ | PROT_WRITE, MAP_SHARED, file_extra_data->fd, 0);
    if(buff == MAP_FAILED)
    {
    	MY_ERROR("mmap /dev/ashmem failed! errno:%d", errno);
    	_exit(-1); 
    }

	file_extra_data->mmap_buffer = buff;
    memcpy(buff, file_extra_data, sizeof(FileExtraData));
	
	msync(buff, sizeof(FileExtraData), MS_SYNC);
	MY_METHOD("FileExtraData saved to mem:0x%08llx. fd:0x%08x, file*: 0x%08llx", (unsigned long long)buff, file_extra_data->fd, (unsigned long long)file_extra_data->file);
}

static void save_file_mapping(FileExtraData* file_extra_data){
	char save_path[512] = {0};
	snprintf(save_path, sizeof(save_path), "%s/rt_fd_mappings/%08llx", g_use_data_path, (unsigned long long)file_extra_data);
	file_extra_data->file = ::fopen(save_path, "wb+");
	if (file_extra_data->file == NULL)
	{		
		MY_ERROR("open save path:%s failed! errno:%d", save_path, errno);
		_exit(-1); 
	}
	file_extra_data->fd = fileno(file_extra_data->file);
	
	fwrite((void*)file_extra_data, 1, sizeof(FileExtraData), file_extra_data->file);
	fflush(file_extra_data->file);
	fseek(file_extra_data->file, 0, SEEK_SET);	
	
	MY_METHOD("FileExtraData saved to %s. fd:0x%08x, file*: 0x%08llx", save_path, file_extra_data->fd, (unsigned long long)file_extra_data->file);
}

static FileExtraData* save_mapping(ShadowZip* shadow_zip)
{
	FileExtraData* file_extra_data = new FileExtraData();	
	file_extra_data->magic_num = MY_MAGIC_NUM;
	file_extra_data->self = file_extra_data;
	file_extra_data->shadow_zip = shadow_zip;
	
	if (use_shared_mem){
		save_mem_mapping(file_extra_data);
	}
	else{
		save_file_mapping(file_extra_data);
	}
	
	return file_extra_data;
}

static FileExtraData* get_mem_mapping(int fd)
{
	struct stat shm_stat;
	memset(&shm_stat, 0, sizeof(struct stat));
	if (fstat(fd, &shm_stat) != 0) {
		return NULL;
	}
	if (g_shm_device_id != shm_stat.st_dev || g_shm_ino != shm_stat.st_ino){
		return NULL;
	}
	
	char* buff = (char*)mmap(NULL, sizeof(FileExtraData), PROT_READ, MAP_SHARED, fd, 0);
	if(buff == MAP_FAILED) { return NULL; }
	if(*((uint64_t*)buff) != MY_MAGIC_NUM){	
		munmap(buff, sizeof(FileExtraData));
		return NULL;
	}
	
	FileExtraData file_extra_data_copy;
	memcpy(&file_extra_data_copy, buff, sizeof(FileExtraData));
	munmap(buff, sizeof(FileExtraData));
	
	//validate
	if (file_extra_data_copy.magic_num != MY_MAGIC_NUM) { return NULL; }
	if (file_extra_data_copy.fd != fd) { return NULL; }
	
	return file_extra_data_copy.self;	
}

static FileExtraData* get_file_mapping(int fd)
{
	off_t pos = lseek(fd, 0, SEEK_CUR);
	if (pos != 0){return NULL;}
	
	lseek(fd, 0, SEEK_SET);
	FileExtraData file_extra_data_copy;
	int read_cnt = read(fd, (void*)&file_extra_data_copy, sizeof(FileExtraData));
	lseek(fd, 0, SEEK_SET);	
	
	//validate
	if (read_cnt != sizeof(FileExtraData)){ return NULL; }	
	if (file_extra_data_copy.magic_num != MY_MAGIC_NUM) { return NULL; }
	if (file_extra_data_copy.fd != fd) { return NULL; }
	
	return file_extra_data_copy.self;	
}

static FileExtraData* get_mapping(int fd)
{
	if (use_shared_mem){
		return get_mem_mapping(fd);
	}
	else{
		return get_file_mapping(fd);
	}
}

static FileExtraData* get_mapping(FILE* file)
{	
	int fd = fileno(file);
	return get_mapping(fd);
}

static void clean_mem_mapping_data(FileExtraData* file_extra_data){
	MY_METHOD("FileExtraData deleted mem: 0x%08llx. fd:0x%08x, file*: 0x%08llx", (unsigned long long)file_extra_data->mmap_buffer, file_extra_data->fd, (unsigned long long)file_extra_data->file);
	munmap(file_extra_data->mmap_buffer, sizeof(FileExtraData));
}

static void clean_file_mapping_data(FileExtraData* file_extra_data){
	char save_path[512] = {0};
	snprintf(save_path, sizeof(save_path), "%s/rt_fd_mappings/%08llx", g_use_data_path, (unsigned long long)file_extra_data->self);
	unlink(save_path);
	MY_METHOD("FileExtraData deleted %s. fd:0x%08x, file*: 0x%08llx", save_path, file_extra_data->fd, (unsigned long long)file_extra_data->file);
}

static void clean_mapping_data(FileExtraData* file_extra_data)
{
	if (use_shared_mem){
		clean_mem_mapping_data(file_extra_data);
	}
	else{
		clean_file_mapping_data(file_extra_data);
	}
	
	fclose(file_extra_data->file);
	delete file_extra_data->shadow_zip;
	
	delete file_extra_data->self;
}


#endif
