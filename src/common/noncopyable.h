#pragma once

class NonCopyable
{
protected:
	NonCopyable() {}
	virtual ~NonCopyable() {}

private:
	NonCopyable(const NonCopyable&) = delete;
	void operator=(const NonCopyable&) = delete;
};