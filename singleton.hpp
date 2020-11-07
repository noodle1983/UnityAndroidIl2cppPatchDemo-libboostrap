#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "pthread_mutex.hpp"


class PthreadMutex;
typedef PthreadMutex PthreadRwMutex;
class PthreadGuard;
typedef PthreadGuard PthreadReadGuard;
typedef PthreadGuard PthreadWriteGuard;

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
};

template<typename DataType, int instanceId>
DataType* LeakSingleton<DataType, instanceId>::dataM = NULL;

//--------------------------------------------------------------

#include <memory>
template<typename DataType, int instanceId = 0>
class Singleton
{
public:
	static DataType* instance()
	{
		if (NULL == dataHolderM.get())
		{
			PthreadWriteGuard lock(dbLockMutexM);
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

	static std::shared_ptr<DataType> dataHolderM;
	static PthreadRwMutex dbLockMutexM;
};

template<typename DataType, int instanceId>
std::shared_ptr<DataType> Singleton<DataType, instanceId>::dataHolderM;

template<typename DataType, int instanceId>
PthreadRwMutex Singleton<DataType, instanceId>::dbLockMutexM;


#endif /* SINGLETON_HPP */

