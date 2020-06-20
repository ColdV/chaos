#pragma once


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
T* Single<typename T>::s_instance = new T;