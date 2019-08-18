#include "pthread_mutex.hpp"
#include "singleton.hpp"
#include <map>
#include <iostream>


struct GlobalData
{	
	std::map<int, int> g_fd_to_file;	
	PthreadRwMutex g_file_to_shadowzip_mutex;
};
#define g_global_data (Singleton<GlobalData, 0>::instance())

long long count;
const size_t THREAD_NUM = 6; 

void increment_count()
{
	{
		PthreadWriteGuard guard(g_global_data->g_file_to_shadowzip_mutex);
		count = count + 1;
		std::cout << "write count:" << count << std::endl;
	}
}

long long get_count()
{
    long long c;   
	{
		PthreadReadGuard guard(g_global_data->g_file_to_shadowzip_mutex);
		c = count;		
		std::cout << "read count:" << count << std::endl;
	}
    return (c);
}

//-----------------------------------------------------------------------
void *thread_proc(void *arg)
{
    for(int i = 0; i < 1000000; i++){
		increment_count();
		get_count();
	}

    pthread_exit(0);
    return 0;
}

int main(int argc, char *argv[])
{
	
    pthread_t threadids[THREAD_NUM];
    for(size_t i = 0; i < THREAD_NUM; i++)
    {
        int err = pthread_create(&(threadids[i]), NULL, &thread_proc, (void *)(intptr_t)i);
    }

    for(size_t i = 0; i < THREAD_NUM; i++)
    {

        void *status = NULL;
        int rc = pthread_join(threadids[i], &status);
    }

    return 0;
}