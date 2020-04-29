#include "Buffer.h"

namespace NetFrame
{
	Buffer::Buffer():
		//m_totalSize(BUFFER_INIT_SIZE),
		m_pCurNode(0),
		m_useSize(0)
	{
		/*m_buff = new char[BUFFER_INIT_SIZE];
		m_cursor = m_buff;*/
		m_buff.clear();
		m_curNodeIt = m_buff.end();
	}

	Buffer::~Buffer()
	{
		/*if (m_buff)
			delete[] m_buff;*/
		if (!m_buff.empty())
		{
			for (auto it = m_buff.begin(); it != m_buff.end(); ++it)
			{
				delete[] (*it);
			}
		}

		m_buff.clear();
	}


	int Buffer::ReadFd(Socket* pSocket)
	{
		socket_t fd = pSocket->GetFd();

#ifdef _WIN32
		unsigned long n = 0;
		if (ioctlsocket(fd, FIONREAD, &n) < 0)
		{
			printf("socket[llu%] ready recv msg len:%lu\n", fd, n);
			return 0;
		}

#else
		int n = 0;
		if (ioctl(fd, FIONREAD, &n) >= 0)
		{
			printf("socket[%u] ready recv msg len:%llu\n", fd, n);
			return 0;
		}

#endif // _WIN32

		//适配空间
		while (m_buff.size() * BUFFER_INIT_SIZE - m_useSize < n)
		{
			if (Expand() != 0)
			{
				return 0;
			}
		}

		if (m_curNodeIt == m_buff.end() || !(*m_curNodeIt))
			return 0;

		BufferNode* pCurNode = *m_curNodeIt;

		int recvLen = 0;
		//int totalLen = 0;

		while (0 < n)
		{
			recvLen = pSocket->Recv(pCurNode->buff + pCurNode->useSize, pCurNode->totalSize - pCurNode->useSize);
			if (0 >= recvLen)
				return recvLen;
			else
			{
				pCurNode->useSize += recvLen;
				//totalLen += recvLen;

				n -= recvLen;

				//当前buffer节点数据已满 使用下一个节点
				if (pCurNode->totalSize == pCurNode->useSize)
				{
					/*auto it = std::find(m_buff.begin(), m_buff.end(), m_pCurNode);
					if (it == m_buff.end())
						return -1;*/

					if(++m_curNodeIt == m_buff.end())
						m_curNodeIt = m_buff.begin();

					pCurNode = *m_curNodeIt;
				}
			}
		}

		m_useSize += n;

		return n;
	}


	int Buffer::Expand()
	{
		BufferNode* pNewNode = new BufferNode;
		if (!pNewNode)
			return -1;

		pNewNode->buff = new char[BUFFER_INIT_SIZE];
		pNewNode->cursor = pNewNode->buff;
		pNewNode->totalSize = BUFFER_INIT_SIZE;
		pNewNode->useSize = 0;

		if (m_buff.empty())
		{
			m_pCurNode = pNewNode;
		}

		m_buff.push_back(pNewNode);

		if (1 == m_buff.size())
		{
			m_curNodeIt = m_buff.begin();
		}

		return 0;
	}

}