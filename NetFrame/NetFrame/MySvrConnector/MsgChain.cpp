#include "MsgChain.h"


MsgChain::MsgChain():m_chain(NULL)
,m_curChain(NULL)
,m_chainSize(0)
,m_totalMsgSize(0)
{

}

MsgChain::~MsgChain()
{
	if (m_chain)
		delete m_chain;

	if (m_curChain)
		delete m_curChain;
}


int MsgChain::ReadMsg(char* buf, int bufSize)
{
	char* tmp = buf;
	while (0 < bufSize)
	{
		if (bufSize < m_curChain->msgSize)
		{
			memcpy(tmp, m_curChain->cursor, bufSize);
			m_curChain->cursor += bufSize;
			m_curChain->msgSize -= bufSize;
			tmp += bufSize;
			bufSize -= bufSize;
		}
		else
		{
			memcpy(tmp, m_curChain->cursor, m_curChain->msgSize);
			m_chain = m_curChain->next;
			tmp += m_curChain->msgSize;
			bufSize -= m_curChain->msgSize;

			delete m_curChain;
			m_curChain = m_chain;
		}
	}

	m_totalMsgSize -= bufSize;

	return bufSize;
}


char* MsgChain::NextChain(int& size)
{
	if (!m_chain)
	{
		m_chain = new Msg(size);
		if (!m_chain)
			return NULL;

		m_curChain = m_chain;
		m_chainSize += 1;
		m_totalMsgSize += size;
	}

	size = m_curChain->totalSize;
	return m_curChain->cursor;
}
