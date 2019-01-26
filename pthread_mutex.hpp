#ifndef PTHREADMUTEX_H
#define PTHREADMUTEX_H

#include <pthread.h>
#include <errno.h>

class PthreadCond;
class PthreadReadCond;
class PthreadWriteCond;

class PthreadMutex
{
public:
	PthreadMutex()
	{
		pthread_mutex_init(&mutexM, NULL);
	}
	~PthreadMutex()
	{
		pthread_mutex_destroy(&mutexM);
	}

	void lock()
	{
		pthread_mutex_lock(&mutexM);
	}

	void unlock()
	{
		pthread_mutex_unlock(&mutexM);
	}
	friend class PthreadCond;
private:
	pthread_mutex_t mutexM;

};


//http://code.metager.de/source/xref/android/2.3.7/bionic/libc/bionic/pthread-rwlocks.c
//#define PTHREAD_RWLOCK_INITIALIZER  { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0, 0, 0, { NULL, NULL, NULL, NULL }}
struct __pthread_cleanup_t;
class PthreadRwUtils
{
public:
	typedef int pthread_rwlockattr_t;

	typedef struct {
		pthread_mutex_t  lock;
		pthread_cond_t   cond;
		int              numLocks;
		int              writerThreadId;
		int              pendingReaders;
		int              pendingWriters;
		void*            reserved[4];  /* for future extensibility */
	} pthread_rwlock_t;

	typedef struct pthread_internal_t
	{
	    struct pthread_internal_t*  next;
		struct pthread_internal_t** pref;
		pthread_attr_t              attr;
		pid_t                       kernel_id;
		pthread_cond_t              join_cond;
		int                         join_count;
		void*                       return_value;
		int                         intern;
		__pthread_cleanup_t*        cleanup_stack;
		void**                      tls;         /* thread-local storage area */
	} pthread_internal_t;


#define  __likely(cond)    __builtin_expect(!!(cond), 1)
#define  __unlikely(cond)  __builtin_expect(!!(cond), 0)

#define  RWLOCKATTR_DEFAULT     0
#define  RWLOCKATTR_SHARED_MASK 0x0010


	/* Return a global kernel ID for the current thread */
	static int __get_thread_id(void)
	{
		return ((pthread_internal_t*)pthread_self())->kernel_id;
	}

	static  int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
	{
		if (!attr)
			return EINVAL;

		*attr = PTHREAD_PROCESS_PRIVATE;
		return 0;
	}

	static  int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
	{
		if (!attr)
			return EINVAL;

		*attr = -1;
		return 0;
	}

	static  int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int  pshared)
	{
		if (!attr)
			return EINVAL;

		switch (pshared) {
		case PTHREAD_PROCESS_PRIVATE:
		case PTHREAD_PROCESS_SHARED:
			*attr = pshared;
			return 0;
		default:
			return EINVAL;
		}
	}

	static  int pthread_rwlockattr_getpshared(pthread_rwlockattr_t *attr, int *pshared)
	{
		if (!attr || !pshared)
			return EINVAL;

		*pshared = *attr;
		return 0;
	}

	static  int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
	{
		pthread_mutexattr_t*  lock_attr = NULL;
		pthread_condattr_t*   cond_attr = NULL;
		pthread_mutexattr_t   lock_attr0;
		pthread_condattr_t    cond_attr0;
		int                   ret;

		if (rwlock == NULL)
			return EINVAL;

		if (attr && *attr == PTHREAD_PROCESS_SHARED) {
			lock_attr = &lock_attr0;
			pthread_mutexattr_init(lock_attr);
			pthread_mutexattr_setpshared(lock_attr, PTHREAD_PROCESS_SHARED);

			cond_attr = &cond_attr0;
			pthread_condattr_init(cond_attr);
			pthread_condattr_setpshared(cond_attr, PTHREAD_PROCESS_SHARED);
		}

		ret = pthread_mutex_init(&rwlock->lock, lock_attr);
		if (ret != 0)
			return ret;

		ret = pthread_cond_init(&rwlock->cond, cond_attr);
		if (ret != 0) {
			pthread_mutex_destroy(&rwlock->lock);
			return ret;
		}

		rwlock->numLocks = 0;
		rwlock->pendingReaders = 0;
		rwlock->pendingWriters = 0;
		rwlock->writerThreadId = 0;

		return 0;
	}

	static  int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
	{
		int  ret;

		if (rwlock == NULL)
			return EINVAL;

		if (rwlock->numLocks > 0)
			return EBUSY;

		pthread_cond_destroy(&rwlock->cond);
		pthread_mutex_destroy(&rwlock->lock);
		return 0;
	}

	/* Returns TRUE iff we can acquire a read lock. */
	static __inline__ int read_precondition(pthread_rwlock_t *rwlock, int  thread_id)
	{
		/* We can't have the lock if any writer is waiting for it (writer bias).
		 * This tries to avoid starvation when there are multiple readers racing.
		 */
		if (rwlock->pendingWriters > 0)
			return 0;

		/* We can have the lock if there is no writer, or if we write-own it */
		/* The second test avoids a self-dead lock in case of buggy code. */
		if (rwlock->writerThreadId == 0 || rwlock->writerThreadId == thread_id)
			return 1;

		/* Otherwise, we can't have it */
		return 0;
	}

	/* returns TRUE iff we can acquire a write lock. */
	static __inline__ int write_precondition(pthread_rwlock_t *rwlock, int  thread_id)
	{
		/* We can get the lock if nobody has it */
		if (rwlock->numLocks == 0)
			return 1;

		/* Or if we already own it */
		if (rwlock->writerThreadId == thread_id)
			return 1;

		/* Otherwise, not */
		return 0;
	}

	/* This function is used to waken any waiting thread contending
	 * for the lock. One of them should be able to grab it after
	 * that.
	 */
	static void _pthread_rwlock_pulse(pthread_rwlock_t *rwlock)
	{
		if (rwlock->pendingReaders > 0 || rwlock->pendingWriters > 0)
			pthread_cond_broadcast(&rwlock->cond);
	}


	static  int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
	{
		return pthread_rwlock_timedrdlock(rwlock, NULL);
	}

	static  int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
	{
		int ret = 0;

		if (rwlock == NULL)
			return EINVAL;

		pthread_mutex_lock(&rwlock->lock);
		if (__unlikely(!read_precondition(rwlock, __get_thread_id())))
			ret = EBUSY;
		else
			rwlock->numLocks ++;
		pthread_mutex_unlock(&rwlock->lock);

		return ret;
	}

	static  int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, const struct timespec *abs_timeout)
	{
		int thread_id, ret = 0;

		if (rwlock == NULL)
			return EINVAL;

		pthread_mutex_lock(&rwlock->lock);
		thread_id = __get_thread_id();
		if (__unlikely(!read_precondition(rwlock, thread_id))) {
			rwlock->pendingReaders += 1;
			do {
				ret = pthread_cond_timedwait(&rwlock->cond, &rwlock->lock, abs_timeout);
			} while (ret == 0 && !read_precondition(rwlock, thread_id));
			rwlock->pendingReaders -= 1;
			if (ret != 0)
				goto EXIT;
		}
		rwlock->numLocks ++;
	EXIT:
		pthread_mutex_unlock(&rwlock->lock);
		return ret;
	}


	static  int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
	{
		return pthread_rwlock_timedwrlock(rwlock, NULL);
	}

	static  int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
	{
		int thread_id, ret = 0;

		if (rwlock == NULL)
			return EINVAL;

		pthread_mutex_lock(&rwlock->lock);
		thread_id = __get_thread_id();
		if (__unlikely(!write_precondition(rwlock, thread_id))) {
			ret = EBUSY;
		} else {
			rwlock->numLocks ++;
			rwlock->writerThreadId = thread_id;
		}
		pthread_mutex_unlock(&rwlock->lock);
		return ret;
	}

	static  int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock, const struct timespec *abs_timeout)
	{
		int thread_id, ret = 0;

		if (rwlock == NULL)
			return EINVAL;

		pthread_mutex_lock(&rwlock->lock);
		thread_id = __get_thread_id();
		if (__unlikely(!write_precondition(rwlock, thread_id))) {
			/* If we can't read yet, wait until the rwlock is unlocked
			 * and try again. Increment pendingReaders to get the
			 * cond broadcast when that happens.
			 */
			rwlock->pendingWriters += 1;
			do {
				ret = pthread_cond_timedwait(&rwlock->cond, &rwlock->lock, abs_timeout);
			} while (ret == 0 && !write_precondition(rwlock, thread_id));
			rwlock->pendingWriters -= 1;
			if (ret != 0)
				goto EXIT;
		}
		rwlock->numLocks ++;
		rwlock->writerThreadId = thread_id;
	EXIT:
		pthread_mutex_unlock(&rwlock->lock);
		return ret;
	}


	static  int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
	{
		int  ret = 0;

		if (rwlock == NULL)
			return EINVAL;

		pthread_mutex_lock(&rwlock->lock);

		/* The lock must be held */
		if (rwlock->numLocks == 0) {
			ret = EPERM;
			goto EXIT;
		}

		/* If it has only readers, writerThreadId is 0 */
		if (rwlock->writerThreadId == 0) {
			if (--rwlock->numLocks == 0)
				_pthread_rwlock_pulse(rwlock);
		}
		/* Otherwise, it has only a single writer, which
		 * must be ourselves.
		 */
		else {
			if (rwlock->writerThreadId != __get_thread_id()) {
				ret = EPERM;
				goto EXIT;
			}
			if (--rwlock->numLocks == 0) {
				rwlock->writerThreadId = 0;
				_pthread_rwlock_pulse(rwlock);
			}
		}
	EXIT:
		pthread_mutex_unlock(&rwlock->lock);
		return ret;
	}
};



class PthreadRwMutex
{
public:
	PthreadRwMutex()
	{
		PthreadRwUtils::pthread_rwlock_init(&mutexM, NULL);
	}
	~PthreadRwMutex()
	{
		PthreadRwUtils::pthread_rwlock_destroy(&mutexM);
	}

	void rdlock()
	{
		PthreadRwUtils::pthread_rwlock_rdlock(&mutexM);
	}
	void wrlock()
	{
		PthreadRwUtils::pthread_rwlock_wrlock(&mutexM);
	}

	void unlock()
	{
		PthreadRwUtils::pthread_rwlock_unlock(&mutexM);
	}
	friend class PthreadReadCond;
	friend class PthreadWriteCond;
private:
	PthreadRwUtils::pthread_rwlock_t  mutexM;

};

class PthreadGuard
{
public:
	PthreadGuard(PthreadMutex* theMutex)
		:mutexM(theMutex)
	{
		mutexM->lock();
	}
	PthreadGuard(PthreadMutex& theMutex)
		:mutexM(&theMutex)
	{
		mutexM->lock();
	}
	~PthreadGuard()
	{
		mutexM->unlock();
	}
	friend class PthreadCond;
private:
	PthreadMutex* mutexM;

};

class PthreadUpgradeGuard;
class PthreadReadGuard
{
public:
	PthreadReadGuard(PthreadRwMutex* theMutex)
		:mutexM(theMutex)
	{
		mutexM->rdlock();
	}
	PthreadReadGuard(PthreadRwMutex& theMutex)
		:mutexM(&theMutex)
	{
		mutexM->rdlock();
	}
	~PthreadReadGuard()
	{
		release();
	}
protected:
	void release()
	{
		if (mutexM != NULL)
		{
			mutexM->unlock();
			mutexM = NULL;
		}
	}
	friend class PthreadUpgradeGuard;
private:
	PthreadRwMutex* mutexM;

};

class PthreadWriteGuard
{
public:
	PthreadWriteGuard(PthreadRwMutex* theMutex)
		:mutexM(theMutex)
	{
		mutexM->wrlock();
	}
	PthreadWriteGuard(PthreadRwMutex& theMutex)
		:mutexM(&theMutex)
	{
		mutexM->wrlock();
	}
	~PthreadWriteGuard()
	{
		mutexM->unlock();
	}
private:
	PthreadRwMutex* mutexM;

};

class PthreadUpgradeGuard
{
public:
	PthreadUpgradeGuard(PthreadReadGuard& theGuard)
		:mutexM(theGuard.mutexM)
	{
		theGuard.release();
		mutexM->wrlock();
	}
	~PthreadUpgradeGuard()
	{
		mutexM->unlock();
	}
private:
	PthreadRwMutex* mutexM;

};


class PthreadCond
{
public:
	PthreadCond()
	{
		pthread_cond_init(&condM, NULL);
	}
	~PthreadCond()
	{
		pthread_cond_destroy(&condM);
	}

	int wait(PthreadMutex &theMutex)
	{
		return pthread_cond_wait(&condM, &theMutex.mutexM);
	}

	int wait(PthreadGuard &theGuard)
	{
		return wait(*theGuard.mutexM);
	}

	int timed_wait(PthreadMutex &theMutex, unsigned long msec)
	{
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec += msec/1000;
		abstime.tv_nsec += msec%1000*1000000;
		return pthread_cond_timedwait(&condM, &theMutex.mutexM, &abstime);
	}

	int timed_wait(PthreadGuard &theGuard, unsigned long msec)
	{
		return timed_wait(*theGuard.mutexM, msec);
	}

	int signal()
	{
		return pthread_cond_signal(&condM);
	}

	int broadcast()
	{
		return pthread_cond_broadcast(&condM);
	}

private:
	pthread_cond_t condM;
};

template<typename DataType>
class PthreadSpecificPtr
{
public:
	static void destructor(void* theData){delete (DataType*)theData;}

	PthreadSpecificPtr()
	{
		pthread_key_create(&keyM, destructor);
	}
	~PthreadSpecificPtr(){};

	DataType* get(){return (DataType*)pthread_getspecific(keyM);}
	void reset(DataType* theData)
	{
		DataType* oldData = get();
		if (oldData != NULL && oldData != theData)
		{
			delete oldData;
		}
		pthread_setspecific(keyM, theData);
	}
private:
	pthread_key_t keyM;
};

#endif /* PTHREADMUTEX_H */

