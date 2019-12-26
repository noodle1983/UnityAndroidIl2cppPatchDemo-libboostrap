#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "pthread_mutex.hpp"


template<typename DataType, int instanceId = 0>
class LeakSingleton 
{
public: 
	static DataType* instance()
	{
		return dataM;
	}
	
	static void init()
	{
		dataM = new DataType;		
	}
private:
	LeakSingleton(){};
	~LeakSingleton(){};

	static DataType* dataM;
	static PthreadMutex* dbLockMutexM;
};

template<typename DataType, int instanceId>
DataType* LeakSingleton<DataType, instanceId>::dataM = NULL;

#endif /* SINGLETON_HPP */

