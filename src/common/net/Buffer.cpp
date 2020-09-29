#include "Buffer.h"

namespace chaos
{
	Buffer::Buffer():
		m_useSize(0)
	{
		m_buffList.clear();
		m_wNodeIt = m_buffList.end();
		m_rNodeIt = m_buffList.end();
	}


	Buffer::~Buffer()
	{
		if (!m_buffList.empty())
		{
			for (auto it = m_buffList.begin(); it != m_buffList.end(); ++it)
			{
				delete[](*it)->buffer;
				delete (*it);
			}
		}

		m_buffList.clear();
	}


	bool Buffer::Reserver(uint32 size)
	{
		while (GetLeftSize() < size)
		{
			if (Expand() != 0)
			{
				return false;
			}
		}

		return true;
	}


	uint32 Buffer::ReadBuffer(char* buffer, uint32 size)
	{
		if (!buffer)
			return 0;

		uint32 realSize = size > m_useSize ? m_useSize : size;
		uint32 leftSize = realSize;

		if (m_rNodeIt == m_buffList.end() || !(*m_rNodeIt))
			return 0;

		BufferNode* pCurNode = *m_rNodeIt;
		uint32 cpSize = 0;

		while (0 < leftSize)
		{
			cpSize = leftSize > pCurNode->useSize ? pCurNode->useSize : leftSize;
			memcpy(buffer + (realSize - leftSize), pCurNode->readCursor, cpSize);

			pCurNode->useSize -= cpSize;
			pCurNode->readCursor += cpSize;

			//当前节点数据读完
			if (0 >= pCurNode->useSize)
				pCurNode = *GetNextRNodeIt();

			leftSize -= cpSize;
		}

		m_useSize -= realSize;
		return realSize;
	}


	char* Buffer::ReadBuffer(uint32* size)
	{
		if (!size)
			return NULL;

		*size = 0;

		if (m_rNodeIt == m_buffList.end() || !(*m_rNodeIt))
			return NULL;

		BufferNode* pCurNode = *m_rNodeIt;
		if (!pCurNode)
			return NULL;

		pCurNode = *GetNextRNodeIt();

		*size = pCurNode->useSize;

		return pCurNode->readCursor;
	}


	uint32 Buffer::GetReadSize()
	{
		return m_useSize;
	}


	uint32 Buffer::WriteBuffer(const char* buffer, uint32 size)
	{
		//适配空间
		while (GetLeftSize() < size)
		{
			if (Expand() != 0)
			{
				return 0;
			}
		}

		if (m_wNodeIt == m_buffList.end() || !(*m_wNodeIt))
			return 0;

		BufferNode* pCurNode = *m_wNodeIt;
		unsigned int leftLen = size;
		uint32 cpSize = 0;

		while (0 < leftLen)
		{
			//当前buffer节点数据已满 使用下一个节点
			if (pCurNode->totalSize == pCurNode->useSize)
			{
				m_wNodeIt = GetNextWNodeIt();
				pCurNode = *m_wNodeIt;
			}

			cpSize = leftLen > pCurNode->totalSize - pCurNode->useSize ? 
				pCurNode->totalSize - pCurNode->useSize : leftLen;

			memcpy(pCurNode->buffer + pCurNode->useSize, buffer, cpSize);

			pCurNode->useSize += cpSize;
			leftLen -= cpSize;
		}

		m_useSize += size;

		return size;
	}


	char* Buffer::GetWriteBuffer(uint32* size)
	{
		if (!size)
			return NULL;

		*size = 0;

		if (GetLeftSize() <= 0 && 0 != Expand())
			return NULL;

		if (m_wNodeIt == m_buffList.end())
			return NULL;

		BufferNode* pCurNode = *m_wNodeIt;
		if (!pCurNode)
			return NULL;

		*size = pCurNode->totalSize - pCurNode->useSize;
		if (0 >= *size)
		{
			pCurNode = *GetNextWNodeIt();
			*size = pCurNode->totalSize - pCurNode->useSize;
		}

		return pCurNode->buffer + pCurNode->useSize;

	}


	void Buffer::MoveWriteBufferPos(uint32 size)
	{
		if (m_wNodeIt == m_buffList.end())
			return;

		BufferNode* pCurNode = *m_wNodeIt;
		if (!pCurNode)
			return;

		pCurNode->useSize += size;
		m_useSize += size;
	}



	int Buffer::Expand()
	{
		//buffer上限值
		if (m_buffList.size() * BUFFER_INIT_SIZE >= 0xFFFFFFFF)
			return -1;

		BufferNode* pNewNode = new BufferNode;
		if (!pNewNode)
			return -1;

		pNewNode->buffer = new char[BUFFER_INIT_SIZE] {0};
		pNewNode->readCursor = pNewNode->buffer;
		pNewNode->totalSize = BUFFER_INIT_SIZE;
		pNewNode->useSize = 0;

		if (m_buffList.empty())
		{
			m_buffList.push_back(pNewNode);
			m_wNodeIt = m_buffList.begin();
			m_rNodeIt = m_buffList.begin();
		}
		else
		{
			m_buffList.insert(++m_wNodeIt, pNewNode);
			--(--m_wNodeIt);
		}

		return 0;
	}

}