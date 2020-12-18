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
			m_useSize -= cpSize;

			//当前节点数据读完
			if (0 >= pCurNode->useSize)
			{
				//当前阶段数据读完后readCursor复位
				pCurNode->readCursor = pCurNode->buffer;
				pCurNode = *GetNextRNodeIt();
			}
			else
				pCurNode->readCursor += cpSize;

			leftSize -= cpSize;
		}

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

		char* readPos = pCurNode->readCursor;

		*size = pCurNode->useSize;

		return readPos;
	}


	int Buffer::ReadBuffer(IOVEC_TYPE* iov, int iovcnt, uint32 size)
	{
		if (size > m_useSize)
			size = m_useSize;

		int i = 0;
		BufferNodeIt it = m_rNodeIt;

		while (i < iovcnt && size > 0)
		{
			BufferNode* pCurNode = *it;
			if (!pCurNode)
				break;

			IOVEC_TYPE& iove = iov[i++];

			iove.IOVEC_BUF = pCurNode->readCursor;
			iove.IOVEC_LEN = pCurNode->useSize > size ? size : pCurNode->useSize;
			size -= iove.IOVEC_LEN;

			if (++it == m_buffList.end())
				it = m_buffList.begin();
		}

		return i;
	}


	void Buffer::MoveReadPos(uint32 size)
	{
		if (m_rNodeIt == m_buffList.end())
			return;

		BufferNode* pCurNode = *m_rNodeIt;
		if (!pCurNode)
			return;

		if (size > m_useSize)
			size = m_useSize;

		while (0 < size)
		{
			BufferNode* pCurNode = *m_rNodeIt;
			if (!pCurNode)
				break;

			uint32 validSize = pCurNode->useSize > size ? size : pCurNode->useSize;

			m_useSize -= validSize;
			pCurNode->useSize -= validSize;
			pCurNode->readCursor += validSize;
			size -= validSize;

			if (0 == pCurNode->useSize)
			{
				pCurNode->readCursor = pCurNode->buffer;
				GetNextRNodeIt();
			}
		}
	}


	uint32 Buffer::WriteBuffer(const char* buffer, uint32 size)
	{
		//适配空间
		while (GetLeftSize() < size)
		{
			if (Expand() != 0)
			{
				printf("[error]:buffer expand failed!\n");
				return 0;
			}
		}

		if (m_wNodeIt == m_buffList.end() || !(*m_wNodeIt))
			return 0;

		BufferNode* pCurNode = *m_wNodeIt;
		uint32 leftLen = size;
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


	int Buffer::GetWriteBuffer(IOVEC_TYPE* iov, int iovcnt, uint32 size)
	{
		size = iovcnt * BUFFER_INIT_SIZE > size ? size : iovcnt * BUFFER_INIT_SIZE;

		while (GetLeftSize() < size)
		{
			if (Expand() != 0)
			{
				printf("[error]:get write buffer expand failed!\n");
				return 0;
			}
		}

		auto it = m_wNodeIt;
		int i = 0;

		while (i < iovcnt && size > 0)
		{
			IOVEC_TYPE& iovec = iov[i++];

			iovec.IOVEC_BUF = (*it)->buffer + (*it)->useSize;
			iovec.IOVEC_LEN = (*it)->totalSize - (*it)->useSize > size ? size : (*it)->totalSize - (*it)->useSize;
			size -= iovec.IOVEC_LEN;

			if (++it == m_buffList.end())
				it = m_buffList.begin();
		}

		return i;
	}


	void Buffer::MoveWritePos(uint32 size)
	{
		size = GetLeftSize() > size ? size : GetLeftSize();

		while (size > 0)
		{
			int addSize = ((*m_wNodeIt)->useSize + size) > (*m_wNodeIt)->totalSize ? 
				(*m_wNodeIt)->totalSize - (*m_wNodeIt)->useSize : size;

			m_useSize += addSize;
			(*m_wNodeIt)->useSize += addSize;
			size -= addSize;

			m_wNodeIt = GetNextWNodeIt();
		}
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
			//处理读写节点相同并且该节点有数据未读完时,新插入的节点应当在该节点之前,
			//否则会覆盖后面的未读节点导致数据错乱
			if (m_wNodeIt == m_rNodeIt && (*m_wNodeIt)->useSize > 0)
			{
				auto writeNodeIt = m_wNodeIt;
				m_wNodeIt = m_buffList.insert(writeNodeIt, pNewNode);
 
				//处理当前需要扩展的节点既是读节点也是写节点,并且在当前节点中,前一半是已写入,后一般是未读
				//此时新插入的节点在该节点之前, 应当将该节点readCursor之前已写入的数据拷贝到新节点之中,并重新计算该节点的useSize 
				int realUseSize = (*writeNodeIt)->totalSize - ((*writeNodeIt)->readCursor - (*writeNodeIt)->buffer);
				if (realUseSize < (*writeNodeIt)->useSize)
				{
					int moveSize = (*writeNodeIt)->useSize - realUseSize;
					memcpy((*m_wNodeIt)->buffer, (*writeNodeIt)->buffer, moveSize);
					(*m_wNodeIt)->useSize = moveSize;
					(*writeNodeIt)->useSize = realUseSize;
				}
			}
			else
			{
				m_wNodeIt = m_buffList.insert(++m_wNodeIt, pNewNode);
				--m_wNodeIt;
			}
		}

		return 0;
	}

}