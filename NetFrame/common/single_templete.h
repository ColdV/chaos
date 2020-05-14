#pragma once

template <class T1>
class SingleTmp
{
public:
	static T1& Instance()
	{
		static T1 obj_;
		return obj_;
	}

	virtual ~SingleTmp() {}

protected:
	SingleTmp() {}
};
