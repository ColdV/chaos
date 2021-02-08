#pragma once
#include <assert.h>

template <typename T>
class Single
{
public:
	static T& Instance()
	{
		assert(s_instance);
		return *s_instance;
	}

	virtual ~Single() {}

protected:
	Single() { }

private:
	static T* s_instance;

};

template<typename T>
T* Single<T>::s_instance = new T;
