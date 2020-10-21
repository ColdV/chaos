#pragma once 

#include "stdafx.h"
#include <list>



namespace chaos
{
	const uint32 BUFFER_INIT_SIZE = 1024;

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

		//获取当前可读数据大小
		uint32 GetReadSize();

		//将buffer数据写入到m_bufferList中
		//@param size:待写入的字节数
		//return:已写入的字节数
		uint32 WriteBuffer(const char* buffer, uint32 size);

		//可写缓冲区
		//@param size:传出可写缓冲区大小
		char* GetWriteBuffer(uint32* size);

		//移动可写缓冲区
		//将可写缓冲区浮标从当前位置移动size个位置
		void MoveWriteBufferPos(uint32 size);

	private:
		//扩张Buffer,每次新增一个BUFFER_INIT_SIZE大小的节点
		//从最后一次写入的节点之后插入
		int Expand();

		//使list在使用时成为环状
		BufferNodeIt GetNextWNodeIt() { if (++m_wNodeIt == m_buffList.end()) m_wNodeIt = m_buffList.begin(); return m_wNodeIt; }

		BufferNodeIt GetNextRNodeIt() { if (++m_rNodeIt == m_buffList.end()) m_rNodeIt = m_buffList.begin(); return m_rNodeIt; }

		uint32 GetLeftSize() { return m_buffList.size() * BUFFER_INIT_SIZE - m_useSize; }

	private:
		BufferList m_buffList;

		BufferNodeIt m_wNodeIt;				//当前写入点

		BufferNodeIt m_rNodeIt;				//当前读出点

		uint32 m_useSize;					//已使用的总大小
	};
}