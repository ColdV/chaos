#pragma once 

#include "../common/stdafx.h"
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
			char* buff;
			char* cursor;
			uint32 totalSize;
			uint32 useSize;
		};

		typedef std::list<BufferNode*>	BufferNodeList;
		typedef std::list<BufferNode*>::iterator BufferNodeIt;

	public:
		Buffer();
		virtual ~Buffer();

		//const char* GetBuffer() const { return m_cursor; }

		int ReadFd(Socket* pSocket);

		int WriteFd(Socket* pSocket);

		uint32 ReadBuffer(char* buffer, uint32 size);

		uint32 WriteBuffer(const char* buffer, uint32 size);

	private:
		int Expand();

		char* GetCursor();

		BufferNode* GetCurNode();

	private:
		/*char* m_buff;
		uint32 m_totalSize;
		uint32 m_useSize;
		char* m_cursor;*/

		/*BufferNode* m_buff;
		uint32 m_nodeSize;*/
		BufferNodeList m_buff;
		BufferNodeIt m_curNodeIt;
		BufferNode* m_pCurNode;
		uint32 m_useSize;
	};
}