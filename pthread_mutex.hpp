#ifndef PTHREADMUTEX_H
#define PTHREADMUTEX_H

#include <mutex>
//#include <shared_mutex>

typedef std::mutex PthreadMutex;
typedef std::mutex PthreadRwMutex;
//typedef std::shared_mutex PthreadRwMutex;
typedef std::lock_guard<std::mutex> PthreadGuard;
typedef std::lock_guard<std::mutex> PthreadReadGuard;
typedef std::lock_guard<std::mutex> PthreadWriteGuard;
//typedef std::shared_lock<std::shared_mutex> PthreadReadGuard;
//typedef std::unique_lock<std::shared_mutex> PthreadWriteGuard;

#endif /* PTHREADMUTEX_H */

