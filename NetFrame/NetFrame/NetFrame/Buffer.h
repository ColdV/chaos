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
			char* readCursor;
			uint32 totalSize;
			uint32 useSize;
		};

		typedef std::list<BufferNode*>	BufferList;
		typedef std::list<BufferNode*>::iterator BufferNodeIt;

	public:
		Buffer();
		virtual ~Buffer();

		//const char* GetBuffer() const { return m_cursor; }

		//从fd中读出数据   写入到m_bufferList
		int ReadFd(Socket* pSocket);

		//读出m_bufferList中的数据  写入到fd
		//如果没有指定size 将所有数据写入fd
		int WriteFd(Socket* pSocket, uint32 size = 0);

		//读出m_bufferList中的数据 写入到参数buffer中
		uint32 ReadBuffer(char* buffer, uint32 size);

		//将buffer数据写入到m_bufferList中
		uint32 WriteBuffer(const char* buffer, uint32 size);

	private:
		int Expand();

		//char* GetCursor();

		//BufferNode* GetCurNode();

		BufferNodeIt GetNextWNodeIt() { if (++m_wNodeIt == m_buffList.end()) m_wNodeIt = m_buffList.begin(); return m_wNodeIt; }

		BufferNodeIt GetNextRNodeIt() { if (++m_rNodeIt == m_buffList.end()) m_rNodeIt = m_buffList.begin(); return m_rNodeIt; }

		uint32 GetLeftSize() { return m_buffList.size() * BUFFER_INIT_SIZE - m_useSize; }

	private:
		/*char* m_buffList;
		uint32 m_totalSize;
		uint32 m_useSize;
		char* m_cursor;*/

		/*BufferNode* m_buffList;
		uint32 m_nodeSize;*/
		BufferList m_buffList;
		BufferNodeIt m_wNodeIt;				//当前写入点
		BufferNodeIt m_rNodeIt;				//当前读出点
		uint32 m_useSize;
	};
}