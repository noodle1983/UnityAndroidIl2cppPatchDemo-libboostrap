#include "pthread_mutex.hpp"
#include "singleton.hpp"
#include <map>
#include <iostream>
#include <string>


struct GlobalData
{	
    std::map<int, int> g_fd_to_file;
    PthreadRwMutex g_file_to_shadowzip_mutex;
};
#define g_global_data (LeakSingleton<GlobalData, 0>::instance())

int a = 0;
int b = 0;
int c = 0;
int d = 0;
const size_t THREAD_NUM = 6; 

void increment_count()
{
    PthreadWriteGuard guard(g_global_data->g_file_to_shadowzip_mutex);
    a++;
    b++;
    c++;
    d++;
}

void verify_count()
{
    PthreadReadGuard guard(g_global_data->g_file_to_shadowzip_mutex);
    if (a != b || a != c || a != d) {
        std::cout << "pthread unit test failed: " << a << " " << b << " " << c << " " << d << std::endl;
    }
}

//-----------------------------------------------------------------------
void *thread_proc(void *arg)
{
    int thread_id = (int) (intptr_t) arg;
    std::cout << "thread " << thread_id << " started" << std::endl;
    for(int i = 0; i < 1000000; i++){
        increment_count();
        verify_count();
    }
    std::cout << "thread " << thread_id << " end" << std::endl;
    pthread_exit(0);
    return 0;
}

int main(int argc, char *argv[])
{
    LeakSingleton<GlobalData, 0>::init();
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