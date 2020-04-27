#pragma once
#include "../common/stdafx.h"


#if 0

class MsgChain
{
public:
	struct Msg
	{
		Msg(int size)
		{
			msg = new char[size];
			cursor = msg;
			msgSize = size;
			totalSize = size;
			next = NULL;
		}

		Msg() {}

		~Msg()
		{
			if (msg)
				delete[] msg;
		}

		char*	msg;
		char*	cursor;			//当前消息起点
		uint32	msgSize;
		uint32	totalSize;
		Msg*	next;
	};


	MsgChain();
	~MsgChain();


	int ReadMsg(char* buf, int bufSize);
	char* NextChain(int& size);
	uint32 MsgSize() { return m_totalMsgSize; }

private:
	Msg*	m_chain;
	Msg*	m_curChain;
	uint32	m_chainSize;
	uint32	m_totalMsgSize;
};

#endif