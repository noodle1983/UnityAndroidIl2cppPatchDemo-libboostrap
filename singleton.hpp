#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "pthread_mutex.hpp"
#include <memory>

template<typename DataType, int instanceId = 0>
class Singleton 
{
public: 
	static DataType* instance()
	{
		if (NULL == dataHolderM.get())
		{
			PthreadGuard lock(dbLockMutexM);
			if (NULL == dataHolderM.get())
			{
				dataHolderM.reset(new DataType);
			}
		}
		return dataHolderM.get();
	}
private:
	Singleton(){};
	~Singleton(){};

	static std::unique_ptr<DataType> dataHolderM;
	static PthreadMutex dbLockMutexM;
};

template<typename DataType, int instanceId>
std::unique_ptr<DataType> Singleton<DataType, instanceId>::dataHolderM;

template<typename DataType, int instanceId>
PthreadMutex Singleton<DataType, instanceId>::dbLockMutexM;

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

