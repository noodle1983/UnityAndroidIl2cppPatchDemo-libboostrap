#ifndef PTHREADMUTEX_H
#define PTHREADMUTEX_H

//#include <mutex>
////#include <shared_mutex>
//
//typedef std::mutex PthreadMutex;
//typedef std::mutex PthreadRwMutex;
////typedef std::shared_mutex PthreadRwMutex;
//typedef std::lock_guard<std::mutex> PthreadGuard;
//typedef std::lock_guard<std::mutex> PthreadReadGuard;
//typedef std::lock_guard<std::mutex> PthreadWriteGuard;
////typedef std::shared_lock<std::shared_mutex> PthreadReadGuard;
////typedef std::unique_lock<std::shared_mutex> PthreadWriteGuard;


#include <pthread.h>
#include "pthread_spin_lock.h"
//#include "log.h"

class PthreadMutex  
{
public:
    PthreadMutex(){pthread_spin_init(&mutexlock, 0);}
    virtual ~PthreadMutex(){pthread_spin_destroy(&mutexlock);}

    void Lock(){
		pthread_spin_lock(&mutexlock);
		//MY_METHOD("lock at mutex: 0x%08llx", (unsigned long long)&mutexlock);
	}
    void Unlock(){	
		//MY_METHOD("unlock at mutex: 0x%08llx", (unsigned long long)&mutexlock);
		pthread_spin_unlock(&mutexlock);
	}
private:
    pthread_spinlock_t mutexlock;
};



typedef PthreadMutex PthreadRwMutex;

class PthreadGuard
{
public:
	PthreadGuard(PthreadMutex& m){mutex = &m; mutex->Lock();}
    virtual ~PthreadGuard(){mutex->Unlock();}
	
private:
	PthreadMutex* mutex;
};
typedef PthreadGuard PthreadReadGuard;
typedef PthreadGuard PthreadWriteGuard;


#endif /* PTHREADMUTEX_H */

