#pragma once
#include <assert.h>

/*这个有问题  暂时这样吧*/
template <typename T>
class Single
{
public:
	static T& Instance()
	{
		return *s_instance;
	}

	virtual ~Single() {}

protected:
	Single() { }

	static T* s_instance;

};

template<typename T>
T* Single<T>::s_instance = new T;


template <typename T>
class Singleton
{
protected:
	static T* singleton_;

public:
	Singleton(void)
	{
		assert(!singleton_);
#if defined(_MSC_VER) && _MSC_VER < 1200	 
		int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
		singleton_ = (T*)((int)this + offset);
#else
		singleton_ = static_cast<T*>(this);
#endif
	}


	~Singleton(void) { assert(singleton_);  singleton_ = 0; }

	static T& getSingleton(void) { assert(singleton_);  return (*singleton_); }
	static T* getSingletonPtr(void) { return singleton_; }
};

#define KBE_SINGLETON_INIT( TYPE )							\
template <typename TYPE>	 TYPE * Singleton< TYPE >::singleton_ = 0;	\
