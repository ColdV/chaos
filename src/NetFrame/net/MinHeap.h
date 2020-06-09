#pragma once

#include <stdio.h>
#include <cstring>


//有空可以实现一个iterater

template <typename T = void>
struct Less
{
	constexpr bool operator() (const T& left, const T& right) const
	{
		return left < right;
	}
};


template<typename T, typename Cmp = Less<T>>
class MinHeap
{
public:
	MinHeap();
	virtual ~MinHeap();

	int Push(const T& node);

	void Pop();

	const T& Front() const { return m_heap[0]; }

	//void Erase(int pos);

	T* Erase(const T* node);

	//int GetPos(const T* node);

	int Size() const { return m_curSize; }

	int TotalSize() const { return m_totalSize; }

	//因为是连续内存, Begin和End这两个函数可以充当下iterator的遍历作用
	const T* Begin() const { return &m_heap[0]; }

	const T* End() const { return &m_heap[m_curSize]; }

protected:
	//pos从1开始
	void Erase(int pos);

	const T* GetHeap() const { return m_heap; }

	T* Parent(int pos);

	int GetParentPos(int pos) { return pos / 2; }

	T* Left(int pos);

	int GetLeftPos(int pos) { return pos * 2; }

	T* Right(int pos);

	int GetRightPos(int pos) { return pos * 2 + 1; }

	int ShiftUp(int pos);

	void ShiftDown(int pos);

	T* Expand();

	T* Compare(T* left, T* right) const;

private:
	T* m_heap;
	int m_totalSize;
	int m_curSize;

	Cmp m_cmp;
};




template<typename T, typename Cmp>
MinHeap<T, Cmp>::MinHeap() :m_heap(0)
, m_totalSize(0)
, m_curSize(0)
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

	return &m_heap[pos / 2 - 1];
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
	return &m_heap[pos * 2 - 1];
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
	return &m_heap[pos * 2];
}


template<typename T, typename Cmp>
int MinHeap<T, Cmp>::Push(const T& node)
{
	if (m_curSize >= m_totalSize && !Expand())
		return -1;

	m_curSize += 1;
	//m_heap[m_curSize - 1] = node;
	memcpy(&m_heap[m_curSize - 1], &node, sizeof(T));
	//ShiftUp(m_curSize);

	return ShiftUp(m_curSize);
}

template<typename T, typename Cmp>
void MinHeap<T, Cmp>::Pop()
{
	Erase(1);
	//return Erase(1);
}

template<typename T, typename Cmp>
void MinHeap<T, Cmp>::Erase(int pos)
{
	if (0 >= pos || pos > m_curSize)
		return;

	//T* node = &m_heap[pos - 1];
	//m_heap[pos - 1] = m_heap[m_curSize - 1];
	memcpy(&m_heap[pos - 1], &m_heap[m_curSize - 1], sizeof(T));
	m_curSize -= 1;

	ShiftDown(pos);

	//return NULL;
}


template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Erase(const T* node)
{
	Erase(node - m_heap + 1);

	return &m_heap[node - m_heap + 1];
}
//
//
//template<typename T, typename Cmp>
//int MinHeap<T, Cmp>::GetPos(const T* node)
//{
//	for (int i = 0; i < m_curSize; ++i)
//	{
//		if (!m_cmp(node, m_heap[i]) && !m_cmp(m_heap[i], node))
//			return i + 1;
//	}
//
//	return 0;
//}


template<typename T, typename Cmp>
int MinHeap<T, Cmp>::ShiftUp(int pos)
{
	if (0 >= pos || pos > m_curSize)
		return -1;

	/*T* node = &m_heap[pos - 1];
	if (!node)
		return -1;*/
	T node;
	memcpy(&node, &m_heap[pos - 1], sizeof(T));


	while (pos >= 1)
	{
		T* parent = Parent(pos);
		if (!parent)
			break;

		if (m_cmp(node, *parent))
		{
			//m_heap[pos - 1] = parent;
			memcpy(&m_heap[pos - 1], parent, sizeof(T));
			pos /= 2;
			continue;
		}

		break;
	}

	//m_heap[pos - 1] = node;
	memcpy(&m_heap[pos - 1], &node, sizeof(T));

	return pos;
}


template<typename T, typename Cmp>
void MinHeap<T, Cmp>::ShiftDown(int pos)
{
	if (0 >= pos || pos > m_curSize)
		return;

	//T* node = &m_heap[pos - 1];
	T node;
	memcpy(&node, &m_heap[pos - 1], sizeof(T));

	while (pos <= m_curSize)
	{
		T* left = Left(pos);
		//if (!left)
		//	break;

		T* right = Right(pos);
		/*if (!right)
			break;*/

		T* child = Compare(left, right);
		if (!child)
			break;

		if (m_cmp(*child, node))
		{
			//m_heap[pos - 1] = child;
			memcpy(&m_heap[pos - 1], child, sizeof(T));
			if (child == left)
				pos *= 2;
			else
				pos = pos * 2 + 1;

			continue;
		}

		break;
	}

	//m_heap[pos - 1] = node;
	memcpy(&m_heap[pos - 1], &node, sizeof(T));
}


template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Expand()
{
	if (0 >= m_totalSize)
		m_totalSize = 32;		//init 1/2 size;

	m_totalSize *= 2;
	if (0 >= m_totalSize || 0x7FFFFFFF < m_totalSize)
		m_totalSize = 0x7FFFFFFF;

	T* pHeap = new T[m_totalSize];	//expand double;
	if (!pHeap)
		return NULL;

	memset(pHeap, 0, sizeof(T) * m_totalSize);

	if (m_heap)
	{
		memcpy(pHeap, m_heap, sizeof(T) * m_curSize);
		delete[] m_heap;
	}

	m_heap = pHeap;

	return m_heap;
}


template<typename T, typename Cmp>
T* MinHeap<T, Cmp>::Compare(T* left, T* right) const
{
	if (!left && !right)
		return NULL;

	else if (left && !right)
		return left;

	else if (!left && right)
		return right;

	if (m_cmp(*left, *right))
		return left;

	return right;
}