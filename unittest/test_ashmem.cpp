#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "log.h"
#include <linux/ashmem.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>


const size_t THREAD_NUM = 6; 
void *thread_proc(void *arg)
{
	char name[16] = {0};
	snprintf(name, sizeof(name), "%d", *((int*)arg)); 
    FILE* file = ::fopen("/dev/ashmem", "r+");
	if (file == NULL)
	{		
		printf("open /dev/ashmem failed! errno:%d\n", errno);
		return 0; 
	}
	int fd = fileno(file);
	int size = strlen(name) + 1;
	ioctl(fd, ASHMEM_SET_NAME, name);
    ioctl(fd, ASHMEM_SET_SIZE, size);
	
	char* buff1 = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(buff1 == MAP_FAILED)
    {
    	printf("mmap /dev/ashmem failed! errno:%d", errno);
    	return 0; 
    }	
	strcpy(buff1, name);
	printf("thread %s fd:%d, buff1 init %s\n", name, fd, buff1);
	
	{
		char* buff2 = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		if(buff2 == MAP_FAILED)
		{
			printf("mmap /dev/ashmem failed! errno:%d", errno);
			return 0; 
		}
		printf("thread %s fd:%d buff2 read %s\n", name, fd, buff2);
		munmap(buff2, size);
	}
	
	printf("thread %s fd:%d buff1 re-read %s\n", name, fd, buff1);
	munmap(buff1, size);
	::fclose(file);
	return 0;
}

int main(int argc, char *argv[])
{
	pthread_t threadids[THREAD_NUM];
	int args[THREAD_NUM];
	for(int i = 0; i < THREAD_NUM; i++){
		args[i] = i;
	}
    for(int i = 0; i < THREAD_NUM; i++){
        int err = pthread_create(&(threadids[i]), NULL, &thread_proc, args+ i);
    }

    for(int i = 0; i < THREAD_NUM; i++) {
        void *status = NULL;
        int rc = pthread_join(threadids[i], &status);
    }

    return 0;
}