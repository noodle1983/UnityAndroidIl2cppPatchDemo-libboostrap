//
// Created by lizhen on 2020/11/18.
//

#ifndef PTHREAD_SPIN_LOCK_SHIM
#define PTHREAD_SPIN_LOCK_SHIM

#include <errno.h>

typedef int pthread_spinlock_t;

static inline int pthread_spin_init(pthread_spinlock_t *lock, int pshared) {
	__asm__ __volatile__ ("" ::: "memory");
	*lock = 0;
	return 0;
}

static inline int pthread_spin_destroy(pthread_spinlock_t *lock) {
	return 0;
}

static inline int pthread_spin_lock(pthread_spinlock_t *lock) {
	while (1) {
        if (__sync_bool_compare_and_swap(lock, 0, 1))
            return 0;
	}
}

static inline int pthread_spin_trylock(pthread_spinlock_t *lock) {
	if (__sync_bool_compare_and_swap(lock, 0, 1)) {
		return 0;
	}
	return EBUSY;
}

static inline int pthread_spin_unlock(pthread_spinlock_t *lock) {
	__asm__ __volatile__ ("" ::: "memory");
	*lock = 0;
	return 0;
}

#endif
