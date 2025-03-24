#pragma once
#include <new.h>

namespace myLibrary
{
	template<class Object>
	class ObjectFreeList
	{
	public:
		struct Node
		{
			Object* m_obj;
			Node* m_next;
		};

		// ������ / �Ҹ���
		ObjectFreeList(int initNodeNum = 0) : m_capacityCnt(initNodeNum), m_usedCnt(0)
		{
			m_top = new Node;
		}

		virtual ~ObjectFreeList()
		{

		}

		// �Ҵ�
		Object* Alloc()
		{
			if (m_capacityCnt > m_usedCnt)
			{
				
			}
			else
			{
				Node* newNode = new Node;
				m_capacityCnt++;
				m_usedCnt++;
				return newNode->m_obj;
			}
		}

		// ����
		bool Free(Object* obj)
		{
			
		}

		// FreeList ��ü ��� ����
		int GetCapacityCount() { return m_capacityCnt; }

		// ��� ���� ��� ����
		int GetUsedCount() { return m_usedCnt; }

	private:
		Node* m_top;
		int m_capacityCnt;
		int m_usedCnt;
	};
}