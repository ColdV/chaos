#include "Buffer.h"

namespace NetFrame
{
	Buffer::Buffer():
		//m_totalSize(BUFFER_INIT_SIZE),
		m_useSize(0)
	{
		/*m_buff = new char[BUFFER_INIT_SIZE];
		m_cursor = m_buff;*/

		m_buff.clear();
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


	uint32 Buffer::ReadFd(socket_t fd)
	{
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

		while (m_buff.size() * BUFFER_INIT_SIZE - m_useSize < n)
		{
			if (Expand() == 0)
			{
				return 0;
			}
		}


		while (0 < n)
		{

		}
	}


	uint32 Buffer::Expand()
	{
		BufferNode* pNewNode = new BufferNode;
		if (!pNewNode)
			return 0;

		pNewNode->buff = new char[BUFFER_INIT_SIZE];
		pNewNode->cursor = pNewNode->buff;
		pNewNode->totalSize = BUFFER_INIT_SIZE;
		pNewNode->useSize = 0;

		m_buff.push_back(pNewNode);
	}
}