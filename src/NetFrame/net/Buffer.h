#pragma once 

#include "../../common/stdafx.h"
#include "Socket.h"
#include <list>



namespace NetFrame
{
	const uint32 BUFFER_INIT_SIZE = 1024;

	class Buffer
	{
	public:
		struct BufferNode
		{
			char* buffer;
			char* readCursor;		//当前读出点
			uint32 totalSize;		
			uint32 useSize;			//已写入大小
		};

		typedef std::list<BufferNode*>	BufferList;
		typedef std::list<BufferNode*>::iterator BufferNodeIt;

	public:
		Buffer();
		virtual ~Buffer();

		//从socket中读出数据   写入到m_bufferList
		int ReadSocket(Socket* pSocket);

		//读出m_bufferList中的数据  写入到socket
		//如果没有指定size 将所有数据写入socket
		int WriteSocket(Socket* pSocket, uint32 size = 0);

		//读出m_bufferList中的数据 写入到参数buffer中
		uint32 ReadBuffer(char* buffer, uint32 size);

		//将buffer数据写入到m_bufferList中
		uint32 WriteBuffer(const char* buffer, uint32 size);

		//可写缓冲区
		//@size:传出可写缓冲区大小
		char* GetWriteBuffer(uint32* size);

		//移动可写缓冲区
		//将可写缓冲区浮标从当前位置移动size个位置
		void MoveWriteBuffer(uint32 size);

	private:
		int Expand();

		//使list在使用时成为环状
		BufferNodeIt GetNextWNodeIt() { if (++m_wNodeIt == m_buffList.end()) m_wNodeIt = m_buffList.begin(); return m_wNodeIt; }

		BufferNodeIt GetNextRNodeIt() { if (++m_rNodeIt == m_buffList.end()) m_rNodeIt = m_buffList.begin(); return m_rNodeIt; }

		uint32 GetLeftSize() { return m_buffList.size() * BUFFER_INIT_SIZE - m_useSize; }

	private:
		BufferList m_buffList;
		BufferNodeIt m_wNodeIt;				//当前写入点
		BufferNodeIt m_rNodeIt;				//当前读出点
		uint32 m_useSize;
	};
}