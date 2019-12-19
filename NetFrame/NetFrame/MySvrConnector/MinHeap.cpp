#include "MinHeap.h"
#if 0
template<typename T, typename Cmp>
MinHeap<T, Cmp>::MinHeap():m_heap(0)
,m_totalSize(0)
,m_curSize(0)
{
}


template<typename T, typename Cmp>
MinHeap<T, Cmp>::~MinHeap()
{
	if (m_heap)
		delete[] m_heap;
}


template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Parent(int pos)
{
	if (!m_heap)
		return NULL;
	
	//tree parent node pos = pos / 2
	//in array idx need pos - 1
	if (pos / 2 - 1 < 0)	//root node
		return NULL;

	return m_heap[pos / 2 - 1];
}

template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Left(int pos)
{
	if (!m_heap)
		return NULL;
	
	if (pos * 2 > m_curSize)
		return NULL;

	//tree left node pos = pos * 2;
	//in array idx need pos - 1
	return m_heap[pos * 2 - 1];
}


template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Right(int pos)
{
	if (!m_heap)
		return NULL;

	if (pos * 2 + 1 > m_curSize)
		return NULL;

	//tree right node pos = pos * 2 + 1;
	//in array idx need pos - 1
	return m_heap[pos * 2];
}


template<typename T, typename Cmp>
bool MinHeap<T, Cmp>::Push(const T* node)
{
	if (m_curSize >= m_totalSize && !Expand())
		return false;

	m_curSize += 1;
	m_heap[m_curSize] = node;

	ShiftUp(m_curSize);

	return true;
}

template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Pop()
{
	T* node = m_heap[0];
	m_heap[0] = m_heap[m_curSize - 1];
	m_curSize -= 1;

	ShiftDown(1);

	return node;
}

template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Erase(int pos)
{
	if (0 >= pos || pos > m_curSize)
		return NULL;

	T* node = m_heap[pos - 1];
	m_heap[pos - 1] = m_heap[m_curSize - 1];
	m_curSize -= 1;

	ShiftDown(1);

	return node;
}


template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Erase(const T* node)
{
	return Erase(GetPos(node));
}


template<typename T, typename Cmp>
int MinHeap<T, Cmp>::GetPos(const T* node)
{
	for (int i = 0; i < m_curSize; ++i)
	{
		if (!Cmp(node, m_heap[i]) && !Cmp(m_heap[i], node))
			return i + 1;
	}

	return 0;
}


template<typename T, typename Cmp>
void MinHeap<T, Cmp>::ShiftUp(int pos)
{
	if (0 >= pos || pos > m_curSize)
		return;

	T* node = m_heap[pos];
	int lastPost = pos;

	while (pos >= 1)
	{
		T* parent = Parent(pos);
		if (Cmp(node, parent))
		{
			m_heap[pos - 1] = parent;
			lastPos = pos;
			pos /= 2;
			continue;
		}
	}

	if (lastPos > 0)
		m_heap[lastPos - 1] = node;
}


template<typename T, typename Cmp>
void MinHeap<T, Cmp>::ShiftDown(int pos)
{
	if (0 >= pos || pos > m_curSize)
		return;

	T* node = m_heap[pos - 1];
	int lastPos = pos;

	while (pos <= m_curSize)
	{
		T* left = Left(pos);
		if (Cmp(*left, *node))
		{
			m_heap[pos - 1] = left;
			lastPos = pos;
			pos *= 2;
			continue;
		}

		T* right = Right(pos);
		if (Cmp(*right, *node))
		{
			m_heap[pos - 1] = right;
			lastPos = pos
			pos *= 2 + 1;
			continue;
		}
	}

	if (lastPos > 0)
		m_heap[lastPos - 1] = node;
}


template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Expand()
{
	if (0 >= m_totalSize)
		m_totalSize = 32;		//init 1/2 size;

	T** pHeap = new T * [m_totalSize * 2];	//expand double;
	memset(pHeap, 0, sizeof(T*) * m_totalSize * 2);

	memcpy(pHeap, m_heap, sizeof(T*) * m_totalSize);

	m_totalSize *= 2;	

	return m_heap;
}


template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Compare(const T* left, const T* right) const
{
	if (Cmp(*left, *right))
		return left;

	return right;
}

#endif