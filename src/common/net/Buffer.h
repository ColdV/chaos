#pragma once 

#include "stdafx.h"
#include <list>

namespace chaos
{
	const uint32 BUFFER_INIT_SIZE = 1024 * 4;

	//该Buffer是一个环状结构,写入数据时,当前空间不够会自动扩张环
	//扩张点从当前的写入游标指向的节点开始,在此节点后插入新节点
	//但是从Buffer中读出数据时并不会缩小环,而是移动读出游标,使游标
	//指向下一次读时的起始位置
	class Buffer
	{
	public:
		struct BufferNode
		{
			char* buffer;			//buffer起点
			char* readCursor;		//当前读出点
			uint32 totalSize;		
			uint32 useSize;			//已写入大小
		};

		typedef std::list<BufferNode*>	BufferList;
		typedef std::list<BufferNode*>::iterator BufferNodeIt;

	public:
		Buffer();
		virtual ~Buffer();

		//预分配size个字节空间
		bool Reserver(uint32 size);

		//读出m_bufferList中的数据 写入到参数buffer中
		//@param size:待读取的字节数
		//@return:已读取的字节数
		uint32 ReadBuffer(char* buffer, uint32 size);

		//读出当前buffer节点的数据
		//@param in/out size:待读取/已读取 字节数
		//@return 当前节点数据地址
		char* ReadBuffer(uint32* size);

		//将size个待读字节分成最多iovcnt个填充到iov中
		//该调用并未实际消耗buffer中的数据,之后可根据实际使用情况调用MoveReadBuffePos
		//@param in/out iov:可读数据的buffer数组
		//@param iovcnt:传入的iov数组长度
		//@param size:期望读取的字节数
		//@return 返回实际已填充的iov个数
		int ReadBuffer(IOVEC_TYPE* iov, int iovcnt, uint32 size);

		//移动读浮标
		//将读浮标从当前位置移动size个位置
		void MoveReadPos(uint32 size);

		//获取当前可读数据大小
		uint32 GetReadSize() const { return m_useSize; };

		//将buffer数据写入到m_bufferList中
		//@param size:待写入的字节数
		//return:已写入的字节数
		uint32 WriteBuffer(const char* buffer, uint32 size);

		//获取size个字节可写入的buffer, 并将这些buffer填充到iov中
		//该调用并未实际消耗buffer,之后可根据实际写入了多少字节调用MoveReadBuffePos
		//@param in/out iov:将写入数据的buffer数组
		//@param iovcnt:iov最大长度
		//@param size:待写入的数据的长度
		//@return  返回实际填充的iov个数
		int GetWriteBuffer(IOVEC_TYPE* iov, int iovcnt, uint32 size);

		//移动写浮标
		//将写浮标从当前位置移动size个位置
		void MoveWritePos(uint32 size);

	private:
		//扩张Buffer,每次新增一个BUFFER_INIT_SIZE大小的节点
		//从最后一次写入的节点之后插入
		int Expand();

		//使list在使用时成为环状
		BufferNodeIt GetNextWNodeIt() { if (++m_wNodeIt == m_buffList.end()) m_wNodeIt = m_buffList.begin(); return m_wNodeIt; }

		BufferNodeIt GetNextRNodeIt()
		{
			if (++m_rNodeIt == m_buffList.end())
			{
				if (m_useSize > 0)
					m_rNodeIt = m_buffList.begin();
				else
					m_rNodeIt = m_wNodeIt;
			} 
			else
			{
				if (m_useSize <= 0)
					m_rNodeIt = m_wNodeIt;
			}
			return m_rNodeIt;
		}

		uint32 GetLeftSize() { return (uint32)m_buffList.size() * BUFFER_INIT_SIZE - m_useSize; }

	private:
		BufferList m_buffList;

		BufferNodeIt m_wNodeIt;				//当前写入点

		BufferNodeIt m_rNodeIt;				//当前读出点

		uint32 m_useSize;					//已使用的总大小
	};
}